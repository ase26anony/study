/* reload_stress.c - Stress GCC's reload pass to cover reload.cc lines 1381-1399 */

#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");
int barrier(int x) {
    /* Inline assembly to make it opaque */
    __asm__ volatile ("" : "+r" (x));
    return x;
}

/* Complex structure to force complex addressing */
struct nested {
    int a[3];
    long b[2];
    struct {
        short c;
        int d;
    } inner;
    volatile int v;
};

/* Multi-dimensional array with volatile indices */
volatile int idx1 = 1, idx2 = 2, idx3 = 3;

/* Test function with many registers and complex operations */
__attribute__((noinline, optimize("O1")))
long test_reloads(int a1, int a2, int a3, int a4, int a5,
                  int a6, int a7, int a8, int a9, int a10,
                  long b1, long b2, long b3, long b4, long b5,
                  float f1, float f2, double d1, double d2) {
    
    /* Declare many local variables to exhaust registers */
    register int r0 asm ("r12") = a1;
    register int r1 asm ("r13") = a2;
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int w1, w2, w3, w4, w5, w6, w7, w8, w9, w10;
    long l1, l2, l3, l4, l5, l6, l7, l8, l9, l10;
    float f3, f4, f5;
    double d3, d4, d5;
    
    /* Initialize with arithmetic to create dependencies */
    v1 = barrier(a1 + a2);
    v2 = barrier(a3 * a4);
    v3 = barrier(a5 ^ a6);
    v4 = barrier(a7 | a8);
    v5 = barrier(a9 & a10);
    v6 = barrier(v1 + v2);
    v7 = barrier(v3 - v4);
    v8 = barrier(v5 * v6);
    v9 = barrier(v7 ^ v8);
    v10 = barrier(v9 + r0);
    
    w1 = barrier(r1 + v1);
    w2 = barrier(v2 * v3);
    w3 = barrier(v4 | v5);
    w4 = barrier(v6 & v7);
    w5 = barrier(v8 ^ v9);
    w6 = barrier(w1 + w2);
    w7 = barrier(w3 - w4);
    w8 = barrier(w5 * w6);
    w9 = barrier(w7 ^ w8);
    w10 = barrier(w9 + v10);
    
    /* Long integer chain */
    l1 = barrier(b1 + b2);
    l2 = barrier(b3 * b4);
    l3 = barrier(b5 ^ l1);
    l4 = barrier(l2 | l3);
    l5 = barrier(l1 & l2);
    l6 = barrier(l3 + l4);
    l7 = barrier(l5 - l6);
    l8 = barrier(l7 * l4);
    l9 = barrier(l8 ^ l5);
    l10 = barrier(l9 + l6);
    
    /* Floating point operations to use different register classes */
    f3 = f1 * f2 + 1.0f;
    f4 = f2 / f1 - 2.0f;
    f5 = f3 * f4;
    
    d3 = d1 * d2 + 1.0;
    d4 = d2 / d1 - 2.0;
    d5 = d3 * d4;
    
    /* Complex array access with volatile indices - forces complex addressing */
    int array[10][10][10];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 10; k++) {
                /* SIB-like addressing with all components */
                array[i][j][k] = i * 100 + j * 10 + k;
            }
        }
    }
    
    /* Force complex addressing mode that may need secondary reload */
    int *ptr = &array[idx1][idx2][idx3];
    volatile int *volatile_ptr = ptr;
    
    /* Inline assembly that clobbers many registers */
    __asm__ volatile (
        "# Complex inline assembly\n"
        "mov %[val1], %[tmp1]\n\t"
        "add %[val2], %[tmp1]\n\t"
        "mov %[tmp1], %[out1]\n\t"
        : [out1] "=r" (v1), [tmp1] "=&r" (v2)
        : [val1] "m" (*volatile_ptr), [val2] "r" (r0)
        : "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "memory", "cc"
    );
    
    /* More complex inline assembly with memory constraints */
    long temp;
    __asm__ volatile (
        "# More complex addressing\n"
        "lea (%[base], %[index], 4), %[temp]\n\t"
        "mov (%[temp]), %[temp]\n\t"
        : [temp] "=&r" (temp)
        : [base] "r" (ptr), [index] "r" (idx1)
        : "memory"
    );
    
    /* Type punning between int and float to force register class changes */
    union {
        int i;
        float f;
    } pun;
    pun.i = v1;
    f3 = pun.f * 2.0f;
    pun.f = f3;
    v2 = pun.i;
    
    /* Atomic operations that prevent optimizations */
    _Atomic int atomic_var = 0;
    __atomic_store_n(&atomic_var, v1, __ATOMIC_RELAXED);
    int atomic_val = __atomic_load_n(&atomic_var, __ATOMIC_RELAXED);
    
    /* Complex structure access */
    struct nested nested_array[5];
    for (int i = 0; i < 5; i++) {
        nested_array[i].a[0] = i;
        nested_array[i].a[1] = i * 2;
        nested_array[i].a[2] = i * 3;
        nested_array[i].b[0] = i * 100L;
        nested_array[i].b[1] = i * 200L;
        nested_array[i].inner.c = i * 10;
        nested_array[i].inner.d = i * 20;
        nested_array[i].v = atomic_val + i;
    }
    
    /* Access with complex addressing - may need secondary reload */
    int complex_addr = nested_array[idx2].a[idx1] + 
                      nested_array[idx1].inner.d * idx3;
    
    /* Another inline asm with explicit register variable */
    register int forced_reg asm ("ebx") = complex_addr;
    int result;
    __asm__ volatile (
        "imul %[reg], %[val]\n\t"
        "add $1, %[val]\n\t"
        : [val] "=r" (result)
        : [reg] "r" (forced_reg), "[val]" (v10)
        : "cc"
    );
    
    /* Final computation using all variables */
    long checksum = 
        (long)v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
        w1 + w2 + w3 + w4 + w5 + w6 + w7 + w8 + w9 + w10 +
        l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9 + l10 +
        (int)f3 + (int)f4 + (int)f5 +
        (long)d3 + (long)d4 + (long)d5 +
        atomic_val + complex_addr + result + temp;
    
    return checksum;
}

int main(int argc, char *argv[]) {
    /* Initialize many variables with non-constant values */
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
    
    long b1 = barrier(argc + 100);
    long b2 = barrier(argc + 200);
    long b3 = barrier(argc + 300);
    long b4 = barrier(argc + 400);
    long b5 = barrier(argc + 500);
    
    float f1 = barrier(argc + 1000) / 100.0f;
    float f2 = barrier(argc + 2000) / 100.0f;
    
    double d1 = barrier(argc + 3000) / 100.0;
    double d2 = barrier(argc + 4000) / 100.0;
    
    /* Call test function multiple times with different arguments */
    long total = 0;
    for (int i = 0; i < 3; i++) {
        total += test_reloads(a1 + i, a2 + i, a3 + i, a4 + i, a5 + i,
                             a6 + i, a7 + i, a8 + i, a9 + i, a10 + i,
                             b1 + i, b2 + i, b3 + i, b4 + i, b5 + i,
                             f1 + i, f2 + i, d1 + i, d2 + i);
    }
    
    printf("Checksum: %ld\n", total);
    return 0;
}
