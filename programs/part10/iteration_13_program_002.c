/* reload_stress.c - Stress GCC's reload pass to cover reload record initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");
int barrier(int x) {
    /* Inline asm to prevent optimization */
    __asm__ volatile ("" : "+r" (x));
    return x;
}

/* Complex structure with mixed types */
struct Nested {
    int a[3];
    long b;
    float c;
    struct {
        short s1, s2;
    } inner;
};

/* Global arrays to force complex addressing */
volatile int global_array[256];
volatile struct Nested nested_array[16];
volatile long *volatile ptr_array[32];

/* Register variables to force specific register allocation */
register int reg_var1 asm ("r12");
register int reg_var2 asm ("r13");
register long reg_var3 asm ("r14");

/* Test function with many parameters and complex operations */
__attribute__((noinline, optimize("O1")))
int test_reloads(int a1, int a2, int a3, int a4, int a5,
                 int a6, int a7, int a8, int a9, int a10,
                 long l1, long l2, long l3, long l4, long l5,
                 float f1, float f2, double d1, double d2) {
    
    /* Declare many local variables to increase register pressure */
    int v1 = barrier(a1);
    int v2 = barrier(a2);
    int v3 = barrier(a3);
    int v4 = barrier(a4);
    int v5 = barrier(a5);
    int v6 = barrier(a6);
    int v7 = barrier(a7);
    int v8 = barrier(a8);
    int v9 = barrier(a9);
    int v10 = barrier(a10);
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    
    long lv1 = barrier(l1);
    long lv2 = barrier(l2);
    long lv3 = barrier(l3);
    long lv4 = barrier(l4);
    long lv5 = barrier(l5);
    long lv6, lv7, lv8, lv9, lv10;
    
    float fv1 = f1;
    float fv2 = f2;
    double dv1 = d1;
    double dv2 = d2;
    
    /* Complex arithmetic creating dependency chain */
    v11 = v1 + v2 * v3 - v4 / (v5 | 1);
    v12 = v6 ^ v7 & v8 | v9 % (v10 | 1);
    v13 = v11 * v12 + v1 - v2;
    v14 = v3 + v4 * v5 - v6;
    v15 = v7 ^ v8 & v9 | v10;
    v16 = v13 + v14 * v15 - v11;
    v17 = v12 ^ v13 & v14 | v15;
    v18 = v16 * v17 + v18 - v19;
    v19 = v14 + v15 * v16 - v17;
    v20 = v18 ^ v19 & v20 | v21;
    
    lv6 = lv1 + lv2 * lv3 - lv4;
    lv7 = lv5 ^ lv6 & lv1 | lv2;
    lv8 = lv3 + lv4 * lv5 - lv6;
    lv9 = lv7 ^ lv8 & lv9 | lv10;
    lv10 = lv6 * lv7 + lv8 - lv9;
    
    /* Mixed integer/float operations */
    fv1 = (float)v1 + (float)v2 * fv1;
    fv2 = (float)v3 - (float)v4 / fv2;
    dv1 = (double)lv1 + (double)lv2 * dv1;
    dv2 = (double)lv3 - (double)lv4 / dv2;
    
    /* Inline assembly that clobbers many registers */
    __asm__ volatile (
        "# Complex inline assembly\n"
        "mov %[val1], %[tmp1]\n\t"
        "add %[val2], %[tmp1]\n\t"
        "mov %[tmp1], %[val3]\n\t"
        :
        : [val1] "r" (v1), [val2] "r" (v2), [val3] "r" (v3),
          [tmp1] "r" (v4)
        : "memory", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
          "r8", "r9", "r10", "r11", "cc"
    );
    
    /* Complex addressing modes - SIB addressing on x86 */
    volatile int idx1 = barrier(v1) & 0xF;
    volatile int idx2 = barrier(v2) & 0xF;
    volatile int idx3 = barrier(v3) & 0xF;
    
    /* Force complex memory addressing */
    v1 = global_array[idx1 * 8 + idx2];
    v2 = global_array[idx2 * 4 + idx3];
    v3 = global_array[idx3 * 2 + idx1];
    
    /* Nested structure access with complex addressing */
    struct Nested *nptr = (struct Nested *)&nested_array[0];
    v4 = nptr[idx1].a[idx2];
    v5 = nptr[idx2].inner.s1;
    v6 = nptr[idx3].inner.s2;
    
    /* Use register variables in complex expressions */
    reg_var1 = v1 + v2;
    reg_var2 = v3 * v4;
    reg_var3 = lv1 + lv2;
    
    /* Inline assembly with memory constraint and complex address */
    int temp;
    __asm__ volatile (
        "movl %[addr], %[temp]\n\t"
        "addl $1, %[temp]\n\t"
        "movl %[temp], %[addr]\n\t"
        : [temp] "=r" (temp), [addr] "=m" (global_array[idx1 * 8 + idx2])
        : 
        : "memory", "cc"
    );
    
    /* Atomic operations with memory ordering */
    __atomic_store_n(&global_array[idx1], v1, __ATOMIC_RELAXED);
    v2 = __atomic_load_n(&global_array[idx2], __ATOMIC_RELAXED);
    __atomic_add_fetch(&global_array[idx3], v3, __ATOMIC_RELAXED);
    
    /* More complex addressing through pointer arrays */
    volatile int pidx = barrier(v4) & 0x1F;
    long *ptr = (long *)ptr_array[pidx];
    if (ptr) {
        lv1 = ptr[idx1];
        ptr[idx2] = lv2;
    }
    
    /* Union for type punning between int and float */
    union {
        int i;
        float f;
    } pun;
    pun.i = v1;
    fv1 = pun.f;
    pun.f = fv2;
    v2 = pun.i;
    
    /* Vector-like operations using GCC vector extensions */
    typedef int v4si __attribute__((vector_size(16)));
    v4si vec1 = {v1, v2, v3, v4};
    v4si vec2 = {v5, v6, v7, v8};
    v4si vec3 = vec1 + vec2;
    
    /* Extract elements from vector to force register moves */
    v1 = vec3[0];
    v2 = vec3[1];
    v3 = vec3[2];
    v4 = vec3[3];
    
    /* Final computation mixing all types */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    result += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    result += (int)lv1 + (int)lv2 + (int)lv3 + (int)lv4 + (int)lv5;
    result += (int)lv6 + (int)lv7 + (int)lv8 + (int)lv9 + (int)lv10;
    result += (int)fv1 + (int)fv2 + (int)dv1 + (int)dv2;
    result += reg_var1 + reg_var2 + (int)reg_var3;
    
    return barrier(result);
}

int main(int argc, char **argv) {
    /* Initialize global arrays */
    for (int i = 0; i < 256; i++) {
        global_array[i] = i;
    }
    
    for (int i = 0; i < 32; i++) {
        ptr_array[i] = (long *)malloc(64 * sizeof(long));
        for (int j = 0; j < 64; j++) {
            ptr_array[i][j] = i * 100 + j;
        }
    }
    
    /* Create many live variables */
    int a1 = barrier(argc);
    int a2 = barrier(argc + 1);
    int a3 = barrier(argc + 2);
    int a4 = barrier(argc + 3);
    int a5 = barrier(argc + 4);
    int a6 = barrier(argc + 5);
    int a7 = barrier(argc + 6);
    int a8 = barrier(argc + 7);
    int a9 = barrier(argc + 8);
    int a10 = barrier(argc + 9);
    
    long l1 = barrier(argc) * 1000L;
    long l2 = barrier(argc + 1) * 1000L;
    long l3 = barrier(argc + 2) * 1000L;
    long l4 = barrier(argc + 3) * 1000L;
    long l5 = barrier(argc + 4) * 1000L;
    
    float f1 = (float)argc * 1.5f;
    float f2 = (float)(argc + 1) * 2.5f;
    double d1 = (double)argc * 3.14159;
    double d2 = (double)(argc + 1) * 2.71828;
    
    /* Call test function multiple times with different arguments */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += test_reloads(a1 + i, a2 + i, a3 + i, a4 + i, a5 + i,
                           a6 + i, a7 + i, a8 + i, a9 + i, a10 + i,
                           l1 + i, l2 + i, l3 + i, l4 + i, l5 + i,
                           f1 + i, f2 + i, d1 + i, d2 + i);
    }
    
    printf("Result: %d\n", sum);
    
    /* Cleanup */
    for (int i = 0; i < 32; i++) {
        free(ptr_array[i]);
    }
    
    return 0;
}
