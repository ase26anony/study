/* caller-save-test.c
 * Designed to trigger uncovered lines in GCC's caller-save.cc
 * Compile with: gcc -O3 -m32 -fno-inline -fno-ipa-ra -fno-omit-frame-pointer caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent optimizations */
volatile int global_seed;

/* Function that clobbers many registers - marked noinline to prevent optimization */
#ifdef __x86_64__
#define CLOBBER_LIST "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
#elif __i386__
#define CLOBBER_LIST "eax", "ecx", "edx", "esi", "edi", "memory"
#elif __riscv
#define CLOBBER_LIST "t0", "t1", "t2", "t3", "t4", "t5", "t6", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "memory"
#else
#define CLOBBER_LIST "memory"
#endif

void __attribute__((noinline, noclone)) clobber_callee(int *p1, int *p2, int *p3, int *p4) {
    /* Opaque assembly to clobber registers */
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4) : CLOBBER_LIST);
    
    /* Prevent dead code elimination */
    if (p1) *p1 += 1;
    if (p2) *p2 += 2;
    if (p3) *p3 += 3;
    if (p4) *p4 += 4;
}

/* Another clobbering function with different signature */
void __attribute__((noinline, noclone)) clobber_callee2(float *f1, float *f2, int *i1) {
    asm volatile("" : : "r"(f1), "r"(f2), "r"(i1) : CLOBBER_LIST);
    if (f1) *f1 += 1.0f;
    if (f2) *f2 += 2.0f;
    if (i1) *i1 += 3;
}

/* Function with complex control flow and high register pressure */
int __attribute__((noinline)) high_pressure_function(int condition, int iter) {
    /* Declare many local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    float f1, f2, f3, f4, f5;
    
    /* Initialize with arithmetic to create data dependencies */
    volatile int seed = global_seed + iter;
    
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
    
    v11 = v1 * v2;
    v12 = v3 * v4;
    v13 = v5 * v6;
    v14 = v7 * v8;
    v15 = v9 * v10;
    
    v16 = v11 + v12;
    v17 = v13 + v14;
    v18 = v15 + v16;
    v19 = v17 + v18;
    v20 = v19 * seed;
    
    f1 = (float)v1 * 1.1f;
    f2 = (float)v2 * 1.2f;
    f3 = (float)v3 * 1.3f;
    f4 = (float)v4 * 1.4f;
    f5 = (float)v5 * 1.5f;
    
    /* Complex control flow - creates different basic blocks */
    if (condition & 0x1) {
        /* Path 1: High register pressure before call */
        int t1 = v1 + v2 + v3;
        int t2 = v4 + v5 + v6;
        int t3 = v7 + v8 + v9;
        int t4 = v10 + v11 + v12;
        
        /* Use volatile read to prevent moving computations */
        volatile int barrier = seed;
        
        /* Call that clobbers registers - many variables are live across this call */
        clobber_callee(&t1, &t2, &t3, &t4);
        
        /* Use results after call */
        v1 = t1 + barrier;
        v2 = t2 + barrier;
        v3 = t3 + barrier;
        v4 = t4 + barrier;
        
        /* Another conditional inside to create more block boundaries */
        if (condition & 0x2) {
            float ftmp = f1 + f2;
            clobber_callee2(&ftmp, &f3, &v5);
            f1 = ftmp * 2.0f;
        }
    } else if (condition & 0x4) {
        /* Path 2: Different call pattern */
        clobber_callee(&v13, &v14, &v15, &v16);
        
        /* Loop to create more caller-save opportunities */
        for (int i = 0; i < 3; i++) {
            int loop_var = v17 + i;
            clobber_callee(&loop_var, &v18, NULL, NULL);
            v17 += loop_var;
        }
    } else {
        /* Path 3: Simpler path for contrast */
        v20 = v19 * 2;
    }
    
    /* Switch statement to create more control flow complexity */
    switch (condition & 0x3) {
        case 0:
            /* Another call at block end */
            clobber_callee(&v1, &v2, NULL, NULL);
            v19 = v1 + v2;
            break;
        case 1:
            /* Different call with float args */
            f4 = f1 + f2;
            clobber_callee2(&f4, &f5, &v3);
            v19 = (int)f4 + v3;
            break;
        default:
            /* Nested condition with call */
            if (v4 > 0) {
                clobber_callee(&v4, &v5, &v6, NULL);
            }
            v19 = v4 + v5 + v6;
            break;
    }
    
    /* Use all variables in final computation to keep them live */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                 (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5;
    
    return result;
}

/* Main function with multiple iterations */
int main(int argc, char **argv) {
    /* Initialize volatile seed */
    global_seed = argc;
    
    int total = 0;
    
    /* Multiple iterations with different conditions */
    for (int i = 0; i < 10; i++) {
        int condition = (global_seed + i * 37) & 0x7;
        
        /* Call high-pressure function */
        int result = high_pressure_function(condition, i);
        
        total += result;
        
        /* Print intermediate results to prevent optimization */
        if (i % 3 == 0) {
            printf("Iteration %d: result = %d\n", i, result);
        }
    }
    
    printf("Total: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
