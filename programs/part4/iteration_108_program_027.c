/* caller-save-test.c
 * Designed to trigger specific uncovered lines in GCC's caller-save pass
 * Compile with: gcc -O3 -m32 -fno-inline -fno-ipa-ra -fno-omit-frame-pointer caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile variable to prevent optimizations */
volatile int global_seed = 42;

/* Function that clobbers many registers - declared noinline to prevent optimization */
#ifdef __x86_64__
#define CLOBBER_LIST "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
#elif __i386__
#define CLOBBER_LIST "eax", "ecx", "edx", "esi", "edi", "memory"
#elif __riscv
#define CLOBBER_LIST "t0", "t1", "t2", "t3", "t4", "t5", "t6", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "memory"
#else
#define CLOBBER_LIST "memory"
#endif

__attribute__((noinline, noclone))
void clobber_callee(int *p1, int *p2, int *p3) {
    /* Opaque assembly to clobber registers */
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3) : CLOBBER_LIST);
    
    /* Force memory side effects */
    if (p1) *p1 += 1;
    if (p2) *p2 += 2;
    if (p3) *p3 += 3;
}

/* Another clobbering function with different signature */
__attribute__((noinline, noclone))
void clobber_callee2(float *f1, float *f2) {
    asm volatile("" : : "r"(f1), "r"(f2) : CLOBBER_LIST);
    if (f1) *f1 += 1.0f;
    if (f2) *f2 += 2.0f;
}

/* Function with high register pressure around calls */
__attribute__((noinline))
int high_pressure_function(int condition, int iter) {
    /* Declare many local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    float f1, f2, f3, f4, f5;
    
    /* Initialize with complex, non-removable computations */
    volatile int seed = global_seed + iter;
    
    /* Force variables to be live by using them in computations */
    v1 = seed * 1;
    v2 = seed * 2 + v1;
    v3 = seed * 3 + v2;
    v4 = seed * 4 + v3;
    v5 = seed * 5 + v4;
    v6 = seed * 6 + v5;
    v7 = seed * 7 + v6;
    v8 = seed * 8 + v7;
    v9 = seed * 9 + v8;
    v10 = seed * 10 + v9;
    
    v11 = v1 + v2;
    v12 = v3 + v4;
    v13 = v5 + v6;
    v14 = v7 + v8;
    v15 = v9 + v10;
    
    v16 = v11 * v12;
    v17 = v13 * v14;
    v18 = v15 * v16;
    v19 = v17 * v18;
    v20 = v19 + seed;
    
    /* Float computations to use floating point registers */
    f1 = (float)v1 * 1.1f;
    f2 = (float)v2 * 1.2f;
    f3 = (float)v3 * 1.3f;
    f4 = (float)v4 * 1.4f;
    f5 = (float)v5 * 1.5f;
    
    /* Complex conditional to create different basic blocks */
    if (condition > 0) {
        /* High register pressure path with function call at block end */
        
        /* More computations to keep variables live */
        v1 = v20 + v19;
        v2 = v18 + v17;
        v3 = v16 + v15;
        
        /* Call that clobbers registers - many variables are live across this call */
        clobber_callee(&v1, &v2, &v3);
        
        /* Use results immediately to keep them live */
        v4 = v1 + v2 + v3;
        v5 = v4 * 2;
        
        /* Another call with different register types */
        clobber_callee2(&f1, &f2);
        
        f3 = f1 + f2;
        
        /* Third call with more live variables */
        clobber_callee(&v5, &v6, &v7);
        
        /* This should be the end of a basic block */
        v8 = v5 + v6 + v7 + (int)f3;
        
    } else {
        /* Lower pressure path - simpler computations */
        v8 = v1 + v2 + v3;
        f3 = f1 + f2;
    }
    
    /* More computations after the conditional to use all variables */
    v9 = v8 + v4 + v5 + v6;
    v10 = v7 + v8 + v9;
    
    f4 = f3 * 2.0f + f4;
    f5 = f4 + f5;
    
    /* Final computation that uses all variables to prevent DCE */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                 (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5;
    
    return result;
}

/* Function with multiple call sites in a loop */
__attribute__((noinline))
int loop_with_calls(int iterations) {
    int total = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Varying condition to create different paths */
        int cond = (i % 3) - 1;  /* Results: -1, 0, 1 */
        
        /* Each iteration creates new register pressure scenario */
        int result = high_pressure_function(cond, i);
        
        /* Use result in next iteration to create data dependency */
        total += result;
        
        /* Additional call in loop with varying pressure */
        if (i % 2 == 0) {
            int temp1 = result * 2;
            int temp2 = result * 3;
            int temp3 = result * 4;
            
            clobber_callee(&temp1, &temp2, &temp3);
            
            total += temp1 + temp2 + temp3;
        }
    }
    
    return total;
}

/* Main function with varying conditions */
int main(int argc, char *argv[]) {
    /* Use argc for some determinism but variation */
    int seed = argc;
    
    /* Initialize global volatile */
    global_seed = seed;
    
    /* First test case - single high pressure call */
    int result1 = high_pressure_function(1, 0);
    printf("Result 1: %d\n", result1);
    
    /* Second test case - multiple calls in loop */
    int iterations = (argc > 1) ? atoi(argv[1]) : 5;
    if (iterations < 3) iterations = 3;
    
    int result2 = loop_with_calls(iterations);
    printf("Result 2: %d\n", result2);
    
    /* Third test case - nested conditionals with calls */
    int result3 = 0;
    for (int i = 0; i < 3; i++) {
        /* Complex nested condition to create interesting CFG */
        if (i == 0) {
            int a = i * 10, b = i * 20, c = i * 30;
            clobber_callee(&a, &b, &c);
            result3 += a + b + c;
        } else if (i == 1) {
            float x = i * 1.5f, y = i * 2.5f;
            clobber_callee2(&x, &y);
            result3 += (int)(x + y);
        } else {
            int p = i * 100, q = i * 200, r = i * 300, s = i * 400;
            /* Multiple calls in same block */
            clobber_callee(&p, &q, &r);
            clobber_callee(&r, &s, &p);
            result3 += p + q + r + s;
        }
    }
    printf("Result 3: %d\n", result3);
    
    return (result1 + result2 + result3) % 100;
}
