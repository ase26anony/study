/* reload_stress_test.c
 * A program designed to stress GCC's reload pass to cover the reload record
 * initialization block in reload.cc lines 1381-1399.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");
int barrier(int x) {
    /* Inline assembly to make it opaque */
    __asm__ volatile ("" : "+r" (x));
    return x;
}

/* Function that clobbers many registers */
__attribute__((noinline))
static void clobber_registers(void) {
    /* Clobber a wide range of registers */
    __asm__ volatile (
        "# Clobber many registers\n"
        "movl $0, %%eax\n"
        "movl $0, %%ebx\n"
        "movl $0, %%ecx\n"
        "movl $0, %%edx\n"
        "movl $0, %%esi\n"
        "movl $0, %%edi\n"
        :
        :
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
}

/* Complex structure to force complex addressing */
struct nested {
    int a[3][4];
    long b[2];
    struct {
        int x;
        int y[2];
    } inner;
};

/* Test function with many parameters and complex operations */
__attribute__((noinline, optimize("O1")))
static long test_function(
    int a1, int a2, int a3, int a4, int a5,
    int a6, int a7, int a8, int a9, int a10,
    long l1, long l2, long l3, long l4, long l5,
    volatile int* volatile_ptr,
    struct nested* nested_ptr,
    int idx1, int idx2
) {
    /* Declare many local variables to increase register pressure */
    register int r1 asm("ebx") = a1;
    register int r2 asm("edi") = a2;
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int w1, w2, w3, w4, w5;
    long x1, x2, x3, x4, x5;
    float f1, f2, f3;
    double d1, d2;
    
    /* Initialize with arithmetic to create dependencies */
    v1 = barrier(a1 + a2);
    v2 = barrier(a3 * a4);
    v3 = barrier(a5 ^ a6);
    v4 = barrier(a7 | a8);
    v5 = barrier(a9 & a10);
    
    /* Force register pressure with many live variables */
    w1 = v1 + v2;
    w2 = v3 - v4;
    w3 = v5 * v1;
    w4 = v2 / (v3 + 1);
    w5 = (v4 << 2) | (v5 >> 1);
    
    /* Use register variables in complex expressions */
    r1 = barrier(r1 + w1);
    r2 = barrier(r2 ^ w2);
    
    /* Complex addressing mode: array with scaled index */
    volatile int* volatile_array = volatile_ptr;
    v6 = volatile_array[idx1 * 2 + idx2];  /* SIB addressing on x86 */
    v7 = volatile_array[idx2 * 4 + 3];
    
    /* More complex addressing with structure */
    v8 = nested_ptr->a[idx1][idx2];
    v9 = nested_ptr->inner.y[idx1 & 1];
    
    /* Force spill by using all variables in a big expression */
    x1 = (long)v1 * (long)v2 + (long)v3;
    x2 = (long)v4 * (long)v5 - (long)v6;
    x3 = (long)v7 * (long)v8 + (long)v9;
    x4 = (long)w1 * (long)w2 - (long)w3;
    x5 = (long)w4 * (long)w5 + (long)r1;
    
    /* Mix integer and floating point to use different register classes */
    f1 = (float)x1 / 3.14159f;
    f2 = (float)x2 * 2.71828f;
    d1 = (double)x3 + 1.41421356;
    d2 = (double)x4 * 0.70710678;
    
    /* Type punning through union to force moves between register classes */
    union {
        float f;
        int i;
    } pun;
    pun.f = f1;
    v10 = pun.i ^ v1;  /* Forces move between float and int registers */
    
    /* Inline assembly with complex constraints to force reloads */
    int temp1, temp2;
    __asm__ volatile (
        "# Complex inline assembly with constraints\n"
        "movl %[input1], %[temp1]\n"
        "addl %[input2], %[temp1]\n"
        "movl %[temp1], %[output]\n"
        : [output] "=rm" (temp2),  /* Memory or register output */
          [temp1] "=&r" (temp1)    /* Early clobber register */
        : [input1] "rm" (v10),     /* Memory or register input */
          [input2] "rm" (r2)       /* Memory or register input */
        : "cc"                     /* Clobbers condition codes */
    );
    
    /* More register pressure */
    v1 = barrier(v1 + temp2);
    v2 = barrier(v2 - temp1);
    
    /* Access complex structure with volatile to prevent optimization */
    volatile struct nested* volatile_nested = nested_ptr;
    int struct_val = volatile_nested->a[1][2] + volatile_nested->inner.x;
    
    /* Atomic operation to force specific reload patterns */
    int atomic_val = 0;
    __atomic_store_n(&atomic_val, struct_val, __ATOMIC_RELAXED);
    int loaded_val = __atomic_load_n(&atomic_val, __ATOMIC_RELAXED);
    
    /* Clobber registers in the middle of computation */
    clobber_registers();
    
    /* Force reloads after clobber */
    w1 = barrier(w1 + loaded_val);
    w2 = barrier(w2 ^ v1);
    
    /* Complex expression using all variables */
    long result = (long)v1 + (long)v2 + (long)v3 + (long)v4 + (long)v5 +
                  (long)v6 + (long)v7 + (long)v8 + (long)v9 + (long)v10 +
                  (long)w1 + (long)w2 + (long)w3 + (long)w4 + (long)w5 +
                  x1 + x2 + x3 + x4 + x5 +
                  (long)(f1 * 1000) + (long)(f2 * 1000) +
                  (long)(d1 * 1000) + (long)(d2 * 1000) +
                  (long)temp1 + (long)temp2 +
                  (long)struct_val + (long)loaded_val +
                  l1 + l2 + l3 + l4 + l5;
    
    return barrier(result);
}

/* Another test function focusing on secondary reloads */
__attribute__((noinline, optimize("O1")))
static int test_secondary_reloads(volatile int* mem_base, int index) {
    /* Use explicit register variables to allocate specific registers */
    register int reg1 asm("eax");
    register int reg2 asm("ebx");
    register int reg3 asm("ecx");
    
    /* Force memory load with complex addressing */
    int offset = index * 8 + 16;
    
    /* This should force a secondary reload on some architectures */
    reg1 = mem_base[offset];
    
    /* Complex inline assembly that forces input/output reloads */
    int result;
    __asm__ volatile (
        "# Force secondary reload\n"
        "movl %[addr], %[result]\n"
        : [result] "=r" (result)
        : [addr] "m" (mem_base[offset * 2])  /* Complex memory address */
        : "memory"
    );
    
    /* Use the register variables */
    reg2 = barrier(reg1);
    reg3 = barrier(result);
    
    /* Force them to be live across a function call */
    clobber_registers();
    
    return barrier(reg1 + reg2 + reg3);
}

int main(int argc, char* argv[]) {
    /* Initialize many variables to prevent constant propagation */
    int a1 = barrier(argc + 1);
    int a2 = barrier(argc + 2);
    int a3 = barrier(argc + 3);
    int a4 = barrier(argc + 4);
    int a5 = barrier(argc + 5);
    int a6 = barrier(argc + 6);
    int a7 = barrier(argc + 7);
    int a8 = barrier(argc + 8);
    int a9 = barrier(argc + 9);
    int a10 = barrier(argc + 10);
    
    long l1 = barrier(argc + 100);
    long l2 = barrier(argc + 200);
    long l3 = barrier(argc + 300);
    long l4 = barrier(argc + 400);
    long l5 = barrier(argc + 500);
    
    /* Create volatile memory area */
    volatile int volatile_array[100];
    for (int i = 0; i < 100; i++) {
        volatile_array[i] = barrier(i * 3 + argc);
    }
    
    /* Create and initialize nested structure */
    struct nested nested_obj;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            nested_obj.a[i][j] = barrier(i * 10 + j + argc);
        }
    }
    nested_obj.b[0] = barrier(1000 + argc);
    nested_obj.b[1] = barrier(2000 + argc);
    nested_obj.inner.x = barrier(3000 + argc);
    nested_obj.inner.y[0] = barrier(4000 + argc);
    nested_obj.inner.y[1] = barrier(5000 + argc);
    
    /* Use volatile indices to prevent optimization */
    volatile int idx1 = barrier(argc * 2);
    volatile int idx2 = barrier(argc * 3);
    
    /* Call test function with many arguments */
    long result1 = test_function(
        a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
        l1, l2, l3, l4, l5,
        volatile_array,
        &nested_obj,
        idx1 & 3,  /* Ensure bounds */
        idx2 & 3   /* Ensure bounds */
    );
    
    /* Test secondary reloads specifically */
    int result2 = test_secondary_reloads(volatile_array, idx1 & 7);
    
    /* Final result to prevent dead code elimination */
    long final_result = barrier(result1 + result2);
    
    printf("Result: %ld\n", final_result);
    
    return (final_result > 0) ? 0 : 1;
}
