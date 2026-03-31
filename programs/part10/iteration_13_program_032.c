/* reload_test.c - Stress GCC's reload pass to cover reload record initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");
int barrier(int x) {
    volatile int v = x;
    return v + 1;
}

/* Complex structure with nested arrays */
struct nested {
    int data[4][4];
    long offsets[8];
    volatile int volatile_field;
};

/* Global arrays to force complex addressing */
static int global_array[256];
static long global_long_array[128];
static volatile int volatile_global = 42;

/* Inline assembly helper that clobbers many registers */
#define CLOBBER_MANY_ASM() \
    __asm__ volatile ("" : : : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", \
                      "r8", "r9", "r10", "r11", "r12", "memory")

/* Test function with high register pressure */
__attribute__((noinline, optimize("O1")))
static long test_function(int a, int b, int c, int d, int e,
                         int f, int g, int h, int i, int j,
                         long k, long l, long m, long n, long o) {
    /* Declare many local variables to exhaust registers */
    register int r0 asm ("r12") = a + 1;
    register int r1 asm ("r11") = b + 2;
    int v1 = c * 3;
    int v2 = d / 4;
    int v3 = e ^ f;
    int v4 = g | h;
    int v5 = i & j;
    long l1 = k + 1000;
    long l2 = l - 2000;
    long l3 = m * 3;
    long l4 = n / 4;
    long l5 = o ^ 0xFFFF;
    
    /* Force spills with arithmetic on all variables */
    v1 = barrier(v1 + r0);
    v2 = barrier(v2 + r1);
    v3 = barrier(v3 + v1);
    v4 = barrier(v4 + v2);
    v5 = barrier(v5 + v3);
    
    l1 = barrier(l1 + v4);
    l2 = barrier(l2 + v5);
    l3 = barrier(l3 + l1);
    l4 = barrier(l4 + l2);
    l5 = barrier(l5 + l3);
    
    /* Complex addressing mode: array[index*scale + base] */
    volatile int idx1 = barrier(v1) % 64;
    volatile int idx2 = barrier(v2) % 32;
    
    /* Force SIB addressing on x86 or similar complex addressing */
    int *ptr1 = &global_array[idx1 * 4 + idx2];
    long *ptr2 = &global_long_array[idx2 * 2 + idx1 % 16];
    
    /* Access with volatile to prevent optimization */
    int temp1 = *ptr1;
    long temp2 = *ptr2;
    
    /* Inline assembly with memory constraint and complex address */
    int result1, result2;
    __asm__ volatile (
        "add %[in1], %[out1]\n\t"
        "add %[in2], %[out2]"
        : [out1] "=r" (result1), [out2] "=r" (result2)
        : [in1] "m" (*ptr1), [in2] "m" (*ptr2),
          "0" (temp1), "1" (temp2)
        : "cc"
    );
    
    /* Mixed register classes: integer to floating point */
    float f1 = (float)result1;
    float f2 = (float)result2;
    
    /* Force move between register files */
    union {
        float f;
        int i;
    } punner;
    punner.f = f1 + f2;
    int int_from_float = punner.i;
    
    /* More complex addressing with structure */
    struct nested nested_array[8];
    volatile int struct_idx = barrier(v3) % 8;
    volatile int row = barrier(v4) % 4;
    volatile int col = barrier(v5) % 4;
    
    /* Complex nested array access - may need secondary reload */
    int struct_val = nested_array[struct_idx].data[row][col];
    nested_array[struct_idx].offsets[col] = l4 + struct_val;
    
    /* Atomic operations that force specific reload patterns */
    _Atomic int atomic_var = ATOMIC_VAR_INIT(0);
    __atomic_store_n(&atomic_var, result1 + result2, __ATOMIC_RELAXED);
    int atomic_val = __atomic_load_n(&atomic_var, __ATOMIC_RELAXED);
    
    /* Another inline asm that clobbers many registers */
    CLOBBER_MANY_ASM();
    
    /* Use all variables in final computation */
    long final_result = (long)r0 + r1 + v1 + v2 + v3 + v4 + v5 +
                       l1 + l2 + l3 + l4 + l5 +
                       result1 + result2 + int_from_float +
                       struct_val + atomic_val;
    
    /* Complex addressing in return expression */
    return final_result + nested_array[struct_idx].offsets[col % 4];
}

/* Secondary test for specific secondary reload patterns */
__attribute__((noinline, optimize("O1")))
static int test_secondary_reloads(void) {
    /* Use explicit register variables to force specific allocations */
    register int reg_a asm ("eax");
    register int reg_b asm ("ebx");
    register int reg_c asm ("ecx");
    
    volatile int mem_var = 12345;
    volatile int mem_array[100];
    
    /* Force input reload from memory with complex constraint */
    int result;
    __asm__ volatile (
        "movl %[input], %[output]\n\t"
        "addl $100, %[output]"
        : [output] "=r" (result)
        : [input] "m" (mem_array[mem_var % 50 * 2]), "0" (mem_var)
        : "cc"
    );
    
    /* Force output reload to memory with complex address */
    int idx = barrier(mem_var) % 25;
    __asm__ volatile (
        "movl %[in], %%eax\n\t"
        "movl %%eax, %[out]"
        : [out] "=m" (mem_array[idx * 3 + 1])
        : [in] "r" (result)
        : "eax", "memory"
    );
    
    /* Mixed-size operations that may need mode changes */
    short short_var = result;
    long long_var = (long)short_var * 1000;
    
    /* Access with scaled index (forces SIB on x86) */
    int *complex_ptr = &mem_array[idx * 4 + (result % 10)];
    *complex_ptr = long_var;
    
    return *complex_ptr + result;
}

int main(int argc, char *argv[]) {
    /* Initialize global arrays */
    for (int i = 0; i < 256; i++) {
        global_array[i] = barrier(i);
    }
    for (int i = 0; i < 128; i++) {
        global_long_array[i] = barrier(i) * 1000L;
    }
    
    /* Create many live variables */
    int v1 = barrier(argc);
    int v2 = barrier(v1 + 1);
    int v3 = barrier(v2 * 2);
    int v4 = barrier(v3 / 3);
    int v5 = barrier(v4 ^ 0xAA);
    int v6 = barrier(v5 | 0x55);
    int v7 = barrier(v6 & 0xFF);
    int v8 = barrier(v7 + 100);
    int v9 = barrier(v8 - 50);
    int v10 = barrier(v9 * 3);
    
    long l1 = barrier(v10) * 1000L;
    long l2 = barrier(v1) * 2000L;
    long l3 = barrier(v2) * 3000L;
    long l4 = barrier(v3) * 4000L;
    long l5 = barrier(v4) * 5000L;
    
    /* Call test function with many arguments */
    long result1 = test_function(v1, v2, v3, v4, v5,
                                v6, v7, v8, v9, v10,
                                l1, l2, l3, l4, l5);
    
    /* Test secondary reload patterns */
    int result2 = test_secondary_reloads();
    
    /* Use results to prevent dead code elimination */
    volatile_global = result1 + result2;
    
    printf("Result1: %ld, Result2: %d, Global: %d\n", 
           result1, result2, volatile_global);
    
    return (result1 + result2) > 0 ? 0 : 1;
}

/* Force implementation of barrier function */
int barrier(int x) {
    static volatile int counter = 0;
    counter += x;
    return counter ^ (x * 0x5A827999);
}
