/* reload_test.c - Stress GCC's reload pass to cover reload record initialization */
#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Opaque function to prevent optimization */
extern int barrier(int x) __asm__("barrier");
int barrier(int x) {
    volatile int result = x;
    return result;
}

/* Complex structure with mixed types */
struct nested {
    int a;
    long b;
    float c;
    double d;
    int arr[4];
};

union type_pun {
    int i;
    float f;
    unsigned u;
};

/* Global arrays to force complex addressing */
volatile int global_array[256];
volatile struct nested global_structs[16];
volatile long multi_dim[8][8][8];

/* Noinline test function with many parameters */
__attribute__((noinline, optimize("no-crossjumping", "no-schedule-insns")))
long test_reloads(int p1, int p2, int p3, int p4, int p5,
                  int p6, int p7, int p8, int p9, int p10,
                  long p11, long p12, long p13, long p14, long p15,
                  float p16, float p17, double p18, double p19) {
    
    /* Declare many local variables to exhaust registers */
    register int r0 asm ("r12") = p1;  /* Explicit register variable */
    register int r1 asm ("r13") = p2;
    int v1 = p3, v2 = p4, v3 = p5, v4 = p6, v5 = p7;
    int v6 = p8, v7 = p9, v8 = p10;
    long l1 = p11, l2 = p12, l3 = p13, l4 = p14, l5 = p15;
    float f1 = p16, f2 = p17;
    double d1 = p18, d2 = p19;
    
    /* Additional locals for more pressure */
    int v9 = barrier(v1), v10 = barrier(v2), v11 = barrier(v3);
    int v12 = barrier(v4), v13 = barrier(v5), v14 = barrier(v6);
    long l6 = barrier(l1), l7 = barrier(l2), l8 = barrier(l3);
    float f3 = f1 * 2.0f, f4 = f2 / 3.0f;
    double d3 = d1 + 1.0, d4 = d2 - 2.0;
    
    /* Complex addressing with volatile indices */
    volatile int idx1 = barrier(v1) % 256;
    volatile int idx2 = barrier(v2) % 16;
    volatile int idx3 = barrier(v3) % 8;
    volatile int idx4 = barrier(v4) % 8;
    volatile int idx5 = barrier(v5) % 8;
    
    /* Force memory reloads with complex addressing modes */
    /* SIB addressing on x86: array[index*scale + base] */
    int val1 = global_array[idx1 * 4 + v1];
    long val2 = global_structs[idx2].arr[idx3 * 2 + 1];
    double val3 = multi_dim[idx3][idx4][idx5];
    
    /* Mixed register class operations */
    union type_pun pun1, pun2;
    pun1.f = f1;
    pun2.i = v1;
    
    /* Force moves between integer and float registers */
    float f_from_int = (float)pun2.i;
    int int_from_float = (int)pun1.f;
    
    /* Inline assembly that clobbers many registers */
    /* This forces reloads around the asm block */
    asm volatile (
        "# Complex inline assembly\n\t"
        "mov %[in1], %[out1]\n\t"
        "add %[in2], %[out2]\n\t"
        : [out1] "=r" (v1), [out2] "=r" (v2)
        : [in1] "m" (global_array[idx1]), 
          [in2] "r" (v3),
          "0" (v1), "1" (v2)
        : "r0", "r1", "r2", "r3", "r4", "r5", 
          "r6", "r7", "r8", "r9", "r10", "r11",
          "memory", "cc"
    );
    
    /* Use explicit register variables in complex expressions */
    int complex_expr = (r0 * v1) + (r1 * v2) - (v3 / v4) | (v5 & v6);
    
    /* Atomic operations with memory ordering */
    _Atomic int atomic_var = ATOMIC_VAR_INIT(0);
    __atomic_store(&atomic_var, &complex_expr, __ATOMIC_RELAXED);
    
    int atomic_load;
    __atomic_load(&atomic_var, &atomic_load, __ATOMIC_RELAXED);
    
    /* More arithmetic to create dependency chain */
    v9 = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8;
    v10 = v9 * 2 - v1;
    v11 = v10 / 3 + v2;
    v12 = v11 | v3;
    v13 = v12 & v4;
    v14 = v13 ^ v5;
    
    l6 = l1 + l2 + l3 + l4 + l5;
    l7 = l6 * 3 - l1;
    l8 = l7 / 2 + l2;
    
    f3 = f1 + f2 + f_from_int;
    f4 = f3 * 2.0f - f1;
    
    d3 = d1 + d2 + val3;
    d4 = d3 / 2.0 - d1;
    
    /* Another inline asm with memory constraint */
    long final_result;
    asm volatile (
        "# Secondary reload test\n\t"
        "lea (%[base], %[index], 4), %[result]\n\t"
        : [result] "=r" (final_result)
        : [base] "r" (&global_array[0]),
          [index] "r" (idx1)
        : "memory"
    );
    
    /* Use all variables in final computation */
    final_result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 +
                   v9 + v10 + v11 + v12 + v13 + v14 +
                   l1 + l2 + l3 + l4 + l5 + l6 + l7 + l8 +
                   (int)f1 + (int)f2 + (int)f3 + (int)f4 +
                   (long)d1 + (long)d2 + (long)d3 + (long)d4 +
                   val1 + val2 + (int)val3 +
                   atomic_load + complex_expr + int_from_float;
    
    return final_result;
}

int main(int argc, char *argv[]) {
    /* Initialize global arrays */
    for (int i = 0; i < 256; i++) {
        global_array[i] = barrier(i);
    }
    
    for (int i = 0; i < 16; i++) {
        global_structs[i].a = i;
        global_structs[i].b = i * 2L;
        global_structs[i].c = i * 3.0f;
        global_structs[i].d = i * 4.0;
        for (int j = 0; j < 4; j++) {
            global_structs[i].arr[j] = i + j;
        }
    }
    
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            for (int k = 0; k < 8; k++) {
                multi_dim[i][j][k] = i * 64 + j * 8 + k;
            }
        }
    }
    
    /* Create many live variables */
    int a1 = barrier(argc), a2 = barrier(argc + 1), a3 = barrier(argc + 2);
    int a4 = barrier(argc + 3), a5 = barrier(argc + 4), a6 = barrier(argc + 5);
    int a7 = barrier(argc + 6), a8 = barrier(argc + 7), a9 = barrier(argc + 8);
    int a10 = barrier(argc + 9);
    
    long b1 = barrier(argc) * 10L, b2 = barrier(argc + 1) * 20L;
    long b3 = barrier(argc + 2) * 30L, b4 = barrier(argc + 3) * 40L;
    long b5 = barrier(argc + 4) * 50L;
    
    float c1 = barrier(argc) * 1.5f, c2 = barrier(argc + 1) * 2.5f;
    double d1 = barrier(argc) * 1.25, d2 = barrier(argc + 1) * 2.25;
    
    /* Call test function multiple times with different args */
    long result1 = test_reloads(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                               b1, b2, b3, b4, b5, c1, c2, d1, d2);
    
    /* Modify variables and call again */
    a1 = barrier(result1); a2 = barrier(result1 + 1);
    b1 = barrier(result1) * 100L;
    c1 = barrier(result1) * 3.5f;
    d1 = barrier(result1) * 4.5;
    
    long result2 = test_reloads(a1, a2, a3, a4, a5, a6, a7, a8, a9, a10,
                               b1, b2, b3, b4, b5, c1, c2, d1, d2);
    
    /* Final checksum */
    long final_result = result1 + result2 + 
                       a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 +
                       b1 + b2 + b3 + b4 + b5;
    
    printf("Result: %ld\n", final_result);
    return (final_result > 0) ? 0 : 1;
}
