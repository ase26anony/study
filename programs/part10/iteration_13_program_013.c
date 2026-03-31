/* reload_stress.c - Stress GCC's reload pass to cover reload record initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");
int barrier(int x) {
    /* Inline assembly to prevent optimization */
    __asm__ volatile ("" : "+r" (x));
    return x;
}

/* Complex structure with mixed types */
struct ComplexStruct {
    int a[10];
    long b[5];
    float c[8];
    double d[4];
    struct {
        int x;
        long y;
    } nested;
};

/* Volatile array to prevent optimization */
volatile int volatile_array[100] = {0};

/* Test function with many parameters to force register pressure */
__attribute__((noinline, noipa))
long test_function(
    int p1, int p2, int p3, int p4, int p5,
    int p6, int p7, int p8, int p9, int p10,
    long p11, long p12, long p13, long p14, long p15,
    float p16, float p17, double p18, double p19
) {
    /* Many local variables to exhaust registers */
    register int r1 asm ("r12") = p1;
    register int r2 asm ("r13") = p2;
    int v1 = p3, v2 = p4, v3 = p5, v4 = p6, v5 = p7;
    int v6 = p8, v7 = p9, v8 = p10;
    long l1 = p11, l2 = p12, l3 = p13, l4 = p14, l5 = p15;
    float f1 = p16, f2 = p17;
    double d1 = p18, d2 = p19;
    
    /* Additional variables for more pressure */
    int v9 = 0, v10 = 0, v11 = 0, v12 = 0, v13 = 0;
    int v14 = 0, v15 = 0, v16 = 0, v17 = 0, v18 = 0;
    long l6 = 0, l7 = 0, l8 = 0, l9 = 0, l10 = 0;
    
    /* Complex array access with volatile index */
    volatile int idx1 = barrier(p1) % 50;
    volatile int idx2 = barrier(p2) % 50;
    
    /* Multi-dimensional array with complex addressing */
    int md_array[10][10][10];
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < 10; j++) {
            for (int k = 0; k < 10; k++) {
                /* Complex addressing with all components */
                md_array[i][j][k] = i * 100 + j * 10 + k + p1;
            }
        }
    }
    
    /* Force register spilling with many operations */
    v1 = barrier(v1 + r1);
    v2 = barrier(v2 + r2);
    v3 = barrier(v3 + v1);
    v4 = barrier(v4 + v2);
    v5 = barrier(v5 + v3);
    v6 = barrier(v6 + v4);
    v7 = barrier(v7 + v5);
    v8 = barrier(v8 + v6);
    
    /* Long dependency chain */
    l1 = l1 + v1 + v2;
    l2 = l2 + v3 + v4;
    l3 = l3 + v5 + v6;
    l4 = l4 + v7 + v8;
    l5 = l5 + l1 + l2;
    l6 = l6 + l3 + l4;
    l7 = l7 + l5 + l6;
    l8 = l8 + l7 * 2;
    l9 = l9 + l8 / 3;
    l10 = l10 + l9 - l1;
    
    /* Mixed integer/float operations */
    union { int i; float f; } pun;
    pun.f = f1;
    v9 = barrier(v9 + pun.i);
    pun.i = v1;
    f2 = f2 + pun.f;
    
    /* Inline assembly that clobbers many registers */
    /* This forces reloads around the asm block */
    __asm__ volatile (
        "/* Begin clobbering block */\n\t"
        "movl $0x12345678, %%eax\n\t"
        "movl $0x87654321, %%ebx\n\t"
        "movl $0x11111111, %%ecx\n\t"
        "movl $0x22222222, %%edx\n\t"
        "movl $0x33333333, %%esi\n\t"
        "movl $0x44444444, %%edi\n\t"
        "/* End clobbering block */"
        : /* no outputs */
        : /* no inputs */
        : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory"
    );
    
    /* Complex addressing mode - SIB addressing on x86 */
    /* Force secondary reload for index register */
    int* base_ptr = &md_array[0][0][0];
    int scale = 4;
    int index = barrier(v1) % 100;
    int base = barrier(v2) % 1000;
    
    /* This may require secondary reload on some archs */
    int complex_load = *(int*)((char*)base_ptr + index * scale + base);
    v10 = barrier(v10 + complex_load);
    
    /* More arithmetic to use all variables */
    v11 = v1 * v2 + v3 * v4 - v5 * v6 + v7 * v8;
    v12 = v11 / (v1 + 1) + v2 * v3;
    v13 = (v12 << 3) | (v11 >> 2);
    v14 = v13 ^ v12 ^ v11;
    v15 = (v14 * 1103515245 + 12345) & 0x7fffffff;
    v16 = v15 % 100 + v1;
    v17 = v16 * v2 - v3 * v4 + v5;
    v18 = v17 & 0xFF;
    
    /* Atomic operations with memory ordering */
    _Atomic int atomic_var = 0;
    __atomic_store_n(&atomic_var, v18, __ATOMIC_RELAXED);
    int atomic_load = __atomic_load_n(&atomic_var, __ATOMIC_RELAXED);
    
    /* Access volatile array with complex index */
    v9 += volatile_array[idx1 * 2 + idx2];
    volatile_array[idx2 * 3 + idx1] = v10;
    
    /* Another inline asm with complex constraints */
    /* Forces input/output reloads with memory operands */
    int input_val = v11 + v12;
    int output_val;
    __asm__ volatile (
        "movl %1, %%eax\n\t"
        "addl $0x100, %%eax\n\t"
        "movl %%eax, %0\n\t"
        : "=m" (output_val)  /* memory output */
        : "r" (input_val)    /* register input */
        : "eax", "memory"
    );
    
    /* Use all variables in final computation */
    long result = (long)v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                  v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 +
                  l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 + l9 + l10 +
                  (long)atomic_load + output_val +
                  (long)(f1 * 100) + (long)(f2 * 100) +
                  (long)(d1 * 100) + (long)(d2 * 100);
    
    return barrier(result);
}

int main(int argc, char** argv) {
    /* Initialize many variables with non-constant values */
    int base = (argc > 1) ? atoi(argv[1]) : 12345;
    
    int p1 = barrier(base + 1);
    int p2 = barrier(base + 2);
    int p3 = barrier(base + 3);
    int p4 = barrier(base + 4);
    int p5 = barrier(base + 5);
    int p6 = barrier(base + 6);
    int p7 = barrier(base + 7);
    int p8 = barrier(base + 8);
    int p9 = barrier(base + 9);
    int p10 = barrier(base + 10);
    long p11 = barrier(base + 11);
    long p12 = barrier(base + 12);
    long p13 = barrier(base + 13);
    long p14 = barrier(base + 14);
    long p15 = barrier(base + 15);
    float p16 = (float)barrier(base + 16) / 100.0f;
    float p17 = (float)barrier(base + 17) / 100.0f;
    double p18 = (double)barrier(base + 18) / 100.0;
    double p19 = (double)barrier(base + 19) / 100.0;
    
    /* Initialize volatile array */
    for (int i = 0; i < 100; i++) {
        volatile_array[i] = barrier(i * 3 + base);
    }
    
    /* Call test function multiple times with different args */
    long total = 0;
    for (int i = 0; i < 3; i++) {
        long result = test_function(
            p1 + i, p2 + i, p3 + i, p4 + i, p5 + i,
            p6 + i, p7 + i, p8 + i, p9 + i, p10 + i,
            p11 + i, p12 + i, p13 + i, p14 + i, p15 + i,
            p16 + i, p17 + i, p18 + i, p19 + i
        );
        total += result;
    }
    
    printf("Result: %ld\n", total);
    return (total > 0) ? 0 : 1;
}
