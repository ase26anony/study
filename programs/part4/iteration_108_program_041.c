/* caller-save-test.c
 * Designed to trigger specific uncovered lines in GCC's caller-save pass
 * Compile with: gcc -O3 -m32 -fno-inline -fno-ipa-ra -fno-omit-frame-pointer caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent optimizations */
volatile int global_seed = 42;

/* Function that clobbers many registers - prevent inlining */
__attribute__((noinline, noclone))
void clobber_callee(int *p1, int *p2, int *p3, int *p4) {
    /* Inline asm to clobber specific x86 registers */
    asm volatile("" 
                 : 
                 : "r"(p1), "r"(p2), "r"(p3), "r"(p4)
                 : "memory", "eax", "ecx", "edx", "esi", "edi", "ebx");
    
    /* Force memory clobber */
    *p1 = *p1 + 1;
    *p2 = *p2 + 2;
    *p3 = *p3 + 3;
    *p4 = *p4 + 4;
}

/* Another clobbering function with different signature */
__attribute__((noinline, noclone))
void clobber_callee2(float *f1, float *f2, int *i1, int *i2) {
    asm volatile(""
                 :
                 : "r"(f1), "r"(f2), "r"(i1), "r"(i2)
                 : "memory", "eax", "ecx", "edx", "esi", "edi");
    
    *f1 = *f1 * 1.5f;
    *f2 = *f2 * 2.0f;
    *i1 = *i1 * 3;
    *i2 = *i2 * 4;
}

/* Function to create register pressure */
__attribute__((noinline))
int high_pressure_path(int seed) {
    /* Declare many local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    float f1, f2, f3, f4, f5;
    
    /* Initialize with complex computations to prevent constant folding */
    v1 = seed * 1;
    v2 = seed * 2 + global_seed;
    v3 = seed * 3 - global_seed;
    v4 = seed * 4 ^ global_seed;
    v5 = seed * 5 | global_seed;
    v6 = seed * 6 & global_seed;
    v7 = seed * 7 + v1;
    v8 = seed * 8 + v2;
    v9 = seed * 9 + v3;
    v10 = seed * 10 + v4;
    
    v11 = v1 * v2 - v3;
    v12 = v4 * v5 - v6;
    v13 = v7 * v8 - v9;
    v14 = v10 * v1 - v2;
    v15 = v3 * v4 - v5;
    v16 = v6 * v7 - v8;
    v17 = v9 * v10 - v1;
    v18 = v2 * v3 - v4;
    v19 = v5 * v6 - v7;
    v20 = v8 * v9 - v10;
    
    /* Float computations for floating point register pressure */
    f1 = (float)v1 * 1.1f;
    f2 = (float)v2 * 1.2f;
    f3 = (float)v3 * 1.3f;
    f4 = (float)v4 * 1.4f;
    f5 = (float)v5 * 1.5f;
    
    /* Complex computation keeping many values live */
    f1 = f1 + f2 * f3 - f4 / f5;
    f2 = f2 + f3 * f4 - f5 / f1;
    f3 = f3 + f4 * f5 - f1 / f2;
    
    /* Call that clobbers registers - many variables are live across this call */
    clobber_callee(&v1, &v2, &v3, &v4);
    
    /* More computations after call - variables must be restored */
    v5 = v1 + v2 + v3 + v4;
    v6 = v5 * v11 - v12;
    v7 = v6 * v13 - v14;
    v8 = v7 * v15 - v16;
    v9 = v8 * v17 - v18;
    v10 = v9 * v19 - v20;
    
    /* Another call with different register types */
    clobber_callee2(&f1, &f2, &v5, &v6);
    
    /* Final computation using all variables */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                 (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5;
    
    return result;
}

/* Low pressure path for contrast */
__attribute__((noinline))
int low_pressure_path(int seed) {
    int a = seed * 2;
    int b = seed * 3;
    int c = a + b;
    return c * seed;
}

/* Main function with complex control flow */
int main(int argc, char *argv[]) {
    int i, result = 0;
    
    /* Use argc as seed for deterministic but input-dependent behavior */
    int seed = argc;
    
    /* Loop to create multiple call sites */
    for (i = 0; i < 10; i++) {
        /* Complex condition creating different basic blocks */
        if ((seed + i) % 3 == 0) {
            /* Path 1: High register pressure with call at block end */
            result += high_pressure_path(seed + i);
        } 
        else if ((seed + i) % 3 == 1) {
            /* Path 2: Medium pressure with nested condition */
            int a = seed * i + 1;
            int b = seed * i + 2;
            int c = seed * i + 3;
            int d = seed * i + 4;
            
            /* Nested if to create more complex CFG */
            if (a > b) {
                clobber_callee(&a, &b, &c, &d);
                result += a + b + c + d;
            } else {
                result += a * b - c * d;
            }
        }
        else {
            /* Path 3: Low pressure */
            result += low_pressure_path(seed + i);
        }
        
        /* Mix in some I/O to prevent reordering */
        if (i % 4 == 0) {
            /* getchar() creates a call with side effects */
            int ch = getchar();
            if (ch != EOF) {
                seed += ch;
            }
        }
    }
    
    /* Use volatile to force computation */
    volatile int final_result = result;
    
    printf("Result: %d\n", final_result);
    return final_result % 256;
}
