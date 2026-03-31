/* reload_stress.c - Stress GCC's reload pass to cover reload record initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");

/* Force register pressure with many live variables */
__attribute__((noinline, noipa))
static int test_reloads(int seed) {
    /* Declare many scalar variables to exhaust registers */
    register int r0 asm ("r12") = seed + 1;
    register int r1 asm ("r13") = seed + 2;
    volatile int v0 = seed + 3;
    volatile int v1 = seed + 4;
    int a0 = seed + 5, a1 = seed + 6, a2 = seed + 7, a3 = seed + 8;
    int b0 = seed + 9, b1 = seed + 10, b2 = seed + 11, b3 = seed + 12;
    int c0 = seed + 13, c1 = seed + 14, c2 = seed + 15, c3 = seed + 16;
    int d0 = seed + 17, d1 = seed + 18, d2 = seed + 19, d3 = seed + 20;
    int e0 = seed + 21, e1 = seed + 22, e2 = seed + 23, e3 = seed + 24;
    
    /* Mixed types to engage different register classes */
    float f0 = seed * 1.1f, f1 = seed * 1.2f;
    double dbl0 = seed * 1.3, dbl1 = seed * 1.4;
    
    /* Complex array with volatile indices */
    volatile int idx0 = seed % 16;
    volatile int idx1 = (seed + 7) % 16;
    int array[16][16];
    
    /* Initialize array with pattern */
    for (int i = 0; i < 16; i++) {
        for (int j = 0; j < 16; j++) {
            array[i][j] = i * 17 + j * 13 + seed;
        }
    }
    
    /* Long dependency chain with all variables */
    a0 = barrier(a0 + r0);
    a1 = barrier(a1 + r1);
    a2 = barrier(a2 + v0);
    a3 = barrier(a3 + v1);
    
    b0 = barrier(b0 + a0);
    b1 = barrier(b1 + a1);
    b2 = barrier(b2 + a2);
    b3 = barrier(b3 + a3);
    
    c0 = barrier(c0 + b0);
    c1 = barrier(c1 + b1);
    c2 = barrier(c2 + b2);
    c3 = barrier(c3 + b3);
    
    d0 = barrier(d0 + c0);
    d1 = barrier(d1 + c1);
    d2 = barrier(d2 + c2);
    d3 = barrier(d3 + c3);
    
    e0 = barrier(e0 + d0);
    e1 = barrier(e1 + d1);
    e2 = barrier(e2 + d2);
    e3 = barrier(e3 + d3);
    
    /* Complex addressing modes - SIB addressing on x86 */
    /* array[base + index*scale] where scale=4 (int size) */
    int base = idx0;
    int index = idx1;
    int scale = 4;
    
    /* Force secondary reloads with complex memory operands */
    int complex_load;
    asm volatile (
        "movl %[array], %%eax\n\t"
        "movl %[index], %%ecx\n\t"
        "movl (%[base],%%ecx,%[scale]), %[result]\n\t"
        : [result] "=r" (complex_load)
        : [array] "m" (array),
          [base] "r" (base),
          [index] "r" (index),
          [scale] "i" (4)
        : "eax", "ecx", "memory"
    );
    
    /* Use the loaded value */
    r0 = barrier(r0 + complex_load);
    
    /* Inline asm that clobbers many registers */
    asm volatile (
        "# Clobber many registers\n\t"
        "movl $0, %%eax\n\t"
        "movl $0, %%ebx\n\t"
        "movl $0, %%ecx\n\t"
        "movl $0, %%edx\n\t"
        "movl $0, %%esi\n\t"
        "movl $0, %%edi\n\t"
        : 
        : 
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    /* Atomic operations with memory ordering */
    _Atomic int atomic_var = 0;
    __atomic_store_n(&atomic_var, r0, __ATOMIC_RELAXED);
    int atomic_load = __atomic_load_n(&atomic_var, __ATOMIC_RELAXED);
    
    /* Type punning between int and float */
    union {
        int i;
        float f;
    } punner;
    
    punner.i = atomic_load;
    f0 = punner.f * 1.5f;
    punner.f = f0;
    r1 = punner.i;
    
    /* More complex array accesses with volatile */
    volatile int* volatile_ptr = &array[0][0];
    for (int i = 0; i < 8; i++) {
        /* Complex addressing: ptr + offset * stride */
        int offset = i * 2;
        int stride = 3;
        int val = volatile_ptr[offset * stride];
        
        /* Force reloads around arithmetic */
        asm volatile (
            "addl %[val], %[sum]\n\t"
            : [sum] "+r" (r0)
            : [val] "rm" (val)
            : "cc"
        );
    }
    
    /* Double-register addressing simulation for RISC */
    struct nested {
        int data[8];
        struct {
            int inner[4];
        } nested_struct;
    } nested_array[4];
    
    /* Initialize nested structure */
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 8; j++) {
            nested_array[i].data[j] = i * 100 + j * 10 + seed;
        }
        for (int j = 0; j < 4; j++) {
            nested_array[i].nested_struct.inner[j] = i * 50 + j * 5 + seed;
        }
    }
    
    /* Access with complex index calculation */
    volatile int struct_idx = seed % 4;
    volatile int inner_idx = (seed * 3) % 4;
    
    int nested_val = nested_array[struct_idx].nested_struct.inner[inner_idx];
    
    /* Final computation using all variables */
    int result = r0 + r1 + v0 + v1 + a0 + a1 + a2 + a3 +
                 b0 + b1 + b2 + b3 + c0 + c1 + c2 + c3 +
                 d0 + d1 + d2 + d3 + e0 + e1 + e2 + e3 +
                 complex_load + atomic_load + nested_val;
    
    /* Mix in floating point results */
    result += (int)f0 + (int)f1 + (int)dbl0 + (int)dbl1;
    
    return barrier(result);
}

/* Force multiple calls with different parameters */
__attribute__((noinline))
static int test_multiple_calls(int base) {
    int sum = 0;
    
    /* Multiple calls create different reload contexts */
    sum += test_reloads(base);
    sum += test_reloads(base + 100);
    sum += test_reloads(base + 200);
    sum += test_reloads(base + 300);
    
    /* Additional arithmetic to keep values live */
    sum = barrier(sum * 3);
    sum = barrier(sum / 2);
    sum = barrier(sum + base);
    
    return sum;
}

int main(int argc, char **argv) {
    int seed = (argc > 1) ? atoi(argv[1]) : 12345;
    
    /* Create register pressure in main too */
    int x0 = seed, x1 = seed + 1, x2 = seed + 2, x3 = seed + 3;
    int x4 = seed + 4, x5 = seed + 5, x6 = seed + 6, x7 = seed + 7;
    int x8 = seed + 8, x9 = seed + 9, x10 = seed + 10, x11 = seed + 11;
    int x12 = seed + 12, x13 = seed + 13, x14 = seed + 14, x15 = seed + 15;
    
    /* Use all variables before call */
    int pre_sum = x0 + x1 + x2 + x3 + x4 + x5 + x6 + x7 +
                  x8 + x9 + x10 + x11 + x12 + x13 + x14 + x15;
    
    int result = test_multiple_calls(pre_sum);
    
    /* Use variables after call too */
    int post_sum = x0 - x1 + x2 - x3 + x4 - x5 + x6 - x7 +
                   x8 - x9 + x10 - x11 + x12 - x13 + x14 - x15;
    
    result = barrier(result + post_sum);
    
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}

/* Implementation of barrier function */
int barrier(int x) {
    /* Use volatile asm to prevent optimization */
    volatile int result = x;
    asm volatile ("" : "+r" (result) : : "memory");
    return result;
}
