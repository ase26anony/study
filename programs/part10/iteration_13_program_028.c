/* reload_stress.c - Stress GCC's reload pass to cover rld initialization block */
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");
int barrier(int x) {
    volatile int v = x;
    return v + 1;
}

/* Complex structure with mixed types */
struct nested {
    int a;
    long b;
    float c;
    double d;
    int arr[3];
};

struct container {
    struct nested n1;
    struct nested n2;
    volatile int v_index;
    long long big_array[8][8];
};

/* Global volatile to force memory operations */
volatile int global_seed = 42;
volatile struct container *volatile global_container;

/* Test function with high register pressure */
__attribute__((noinline, optimize("O1")))
long test_reloads(int a1, int a2, int a3, int a4, int a5,
                  int a6, int a7, int a8, int a9, int a10,
                  long l1, long l2, long l3, long l4, long l5,
                  float f1, float f2, double d1, double d2) {
    
    /* Declare many local variables to exhaust registers */
    register int r0 asm ("r12") = a1;
    register int r1 asm ("r13") = a2;
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    long lv1, lv2, lv3, lv4, lv5, lv6, lv7, lv8;
    float fv1, fv2, fv3, fv4;
    double dv1, dv2, dv3;
    
    /* Initialize with arithmetic to create dependencies */
    v1 = barrier(a1 + a2);
    v2 = barrier(a3 * a4);
    v3 = barrier(a5 | a6);
    v4 = barrier(a7 ^ a8);
    v5 = barrier(a9 + a10);
    v6 = barrier(r0 + r1);
    v7 = barrier(v1 * v2);
    v8 = barrier(v3 ^ v4);
    v9 = barrier(v5 + v6);
    v10 = barrier(v7 | v8);
    
    v11 = barrier(v9 * v10);
    v12 = barrier(v1 + v11);
    v13 = barrier(v2 * v12);
    v14 = barrier(v3 + v13);
    v15 = barrier(v4 * v14);
    v16 = barrier(v5 + v15);
    v17 = barrier(v6 * v16);
    v18 = barrier(v7 + v17);
    v19 = barrier(v8 * v18);
    v20 = barrier(v9 + v19);
    
    /* Long variable chain */
    lv1 = l1 + v1;
    lv2 = l2 * lv1 + v2;
    lv3 = l3 + lv2 * v3;
    lv4 = l4 | lv3 ^ v4;
    lv5 = l5 + lv4 * v5;
    lv6 = lv1 * lv2 + lv3;
    lv7 = lv4 | lv5 ^ lv6;
    lv8 = lv7 * v6 + v7;
    
    /* Floating point operations to use FP registers */
    fv1 = f1 + (float)v1;
    fv2 = f2 * (float)v2 + fv1;
    fv3 = (float)v3 / fv2;
    fv4 = fv1 * fv2 - fv3;
    
    dv1 = d1 + (double)lv1;
    dv2 = d2 * (double)lv2 + dv1;
    dv3 = (double)v4 / dv2;
    
    /* Complex addressing with SIB-like calculation */
    volatile int* volatile mem_ptr = (volatile int* volatile)&global_seed;
    int idx1 = v1 % 8;
    int idx2 = v2 % 8;
    int scale = 4;
    
    /* Force complex addressing mode that may need secondary reload */
    int complex_addr_result;
    {
        /* Use inline assembly with memory constraint and clobbered registers */
        asm volatile (
            "movl %[idx1], %%eax\n\t"
            "movl %[scale], %%ecx\n\t"
            "imull %%ecx, %%eax\n\t"
            "addl %[idx2], %%eax\n\t"
            "movl (%%eax), %[result]\n\t"
            : [result] "=r" (complex_addr_result)
            : [idx1] "r" (idx1), [idx2] "r" (idx2), [scale] "r" (scale),
              "m" (*(volatile int(*)[64])global_container->big_array)
            : "eax", "ecx", "memory", "cc"
        );
    }
    
    /* More register pressure with atomic operations */
    _Atomic int atomic_var = 42;
    int atomic_val = __atomic_load_n(&atomic_var, __ATOMIC_RELAXED);
    __atomic_store_n(&atomic_var, atomic_val + v1, __ATOMIC_RELAXED);
    
    /* Type punning between int and float to force register class moves */
    union {
        int i;
        float f;
    } punner;
    
    punner.i = v10;
    fv1 = punner.f * 1.5f;
    punner.f = fv1;
    v11 = punner.i;
    
    /* Another inline asm that clobbers many registers */
    asm volatile (
        "# Clobber many registers\n\t"
        "pushl %%eax\n\t"
        "pushl %%ebx\n\t"
        "pushl %%ecx\n\t"
        "pushl %%edx\n\t"
        "pushl %%esi\n\t"
        "pushl %%edi\n\t"
        "# Do some dummy work\n\t"
        "movl %0, %%eax\n\t"
        "addl %1, %%eax\n\t"
        "movl %%eax, %0\n\t"
        "# Restore\n\t"
        "popl %%edi\n\t"
        "popl %%esi\n\t"
        "popl %%edx\n\t"
        "popl %%ecx\n\t"
        "popl %%ebx\n\t"
        "popl %%eax\n\t"
        : "+m" (v20)
        : "r" (v19)
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory", "cc"
    );
    
    /* Access nested structure with volatile index */
    volatile int vidx = global_container->v_index;
    int nested_result = global_container->n1.arr[vidx % 3] +
                       global_container->n2.arr[(vidx + 1) % 3];
    
    /* Final computation using all variables */
    long final_result = 
        (long)v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
        v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
        lv1 + lv2 + lv3 + lv4 + lv5 + lv6 + lv7 + lv8 +
        (long)fv1 + (long)fv2 + (long)fv3 + (long)fv4 +
        (long)dv1 + (long)dv2 + (long)dv3 +
        complex_addr_result + atomic_val + nested_result;
    
    return final_result;
}

int main() {
    /* Initialize global container */
    struct container cont = {0};
    global_container = &cont;
    
    /* Fill with some data */
    for (int i = 0; i < 3; i++) {
        cont.n1.arr[i] = i * 10;
        cont.n2.arr[i] = i * 20;
    }
    cont.n1.a = 100;
    cont.n1.b = 200;
    cont.n1.c = 3.14f;
    cont.n1.d = 2.71828;
    cont.v_index = 1;
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            cont.big_array[i][j] = i * 100 + j;
        }
    }
    
    /* Create many live variables */
    int a1 = barrier(1);
    int a2 = barrier(2);
    int a3 = barrier(3);
    int a4 = barrier(4);
    int a5 = barrier(5);
    int a6 = barrier(6);
    int a7 = barrier(7);
    int a8 = barrier(8);
    int a9 = barrier(9);
    int a10 = barrier(10);
    
    long l1 = barrier(100);
    long l2 = barrier(200);
    long l3 = barrier(300);
    long l4 = barrier(400);
    long l5 = barrier(500);
    
    float f1 = 1.1f * barrier(1);
    float f2 = 2.2f * barrier(2);
    double d1 = 3.3 * barrier(3);
    double d2 = 4.4 * barrier(4);
    
    /* Call test function multiple times with different args */
    long result1 = test_reloads(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                               l1, l2, l3, l4, l5, f1, f2, d1, d2);
    
    /* Modify some values and call again */
    a1 = barrier(a1 + 1);
    a2 = barrier(a2 + 2);
    long result2 = test_reloads(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                               l1 + 1, l2 + 2, l3 + 3, l4 + 4, l5 + 5,
                               f1 * 2, f2 * 3, d1 * 4, d2 * 5);
    
    printf("Result 1: %ld\n", result1);
    printf("Result 2: %ld\n", result2);
    printf("Checksum: %ld\n", result1 + result2);
    
    return 0;
}
