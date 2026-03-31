/* caller-save-test.c
 * Designed to trigger uncovered lines in GCC's caller-save.cc
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
    /* Inline asm to clobber specific registers on x86 */
    #ifdef __i386__
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4) : 
                  "eax", "ebx", "ecx", "edx", "esi", "edi", "memory");
    #else
    /* Generic memory clobber */
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4) : "memory");
    #endif
    
    /* Do some work to prevent removal */
    *p1 = *p1 + 1;
    *p2 = *p2 - 1;
    if (p3) *p3 = *p3 * 2;
    if (p4) *p4 = *p4 / 2;
}

/* Another clobbering function with different signature */
__attribute__((noinline, noclone))
void clobber_callee2(float *f1, float *f2, int *i1) {
    #ifdef __i386__
    asm volatile("" : : "r"(f1), "r"(f2), "r"(i1) : 
                  "eax", "ecx", "edx", "memory");
    #else
    asm volatile("" : : "r"(f1), "r"(f2), "r"(i1) : "memory");
    #endif
    
    if (f1) *f1 = *f1 + 1.0f;
    if (f2) *f2 = *f2 * 0.5f;
    if (i1) *i1 = *i1 + 100;
}

/* Function with complex control flow and high register pressure */
int high_pressure_path(int seed) {
    /* Declare many local variables to create register pressure */
    int v1 = seed + 1;
    int v2 = seed * 2;
    int v3 = seed - 5;
    int v4 = seed / 3;
    int v5 = seed % 7;
    int v6 = v1 + v2;
    int v7 = v3 * v4;
    int v8 = v5 ^ v6;
    int v9 = v7 & v8;
    int v10 = v9 | seed;
    
    /* Use volatile read to create barrier */
    int v11 = global_seed;
    int v12 = v11 + v10;
    int v13 = v12 * v9;
    int v14 = v13 - v8;
    int v15 = v14 / (v7 ? v7 : 1);
    int v16 = v15 ^ v6;
    int v17 = v16 & v5;
    int v18 = v17 | v4;
    int v19 = v18 * v3;
    int v20 = v19 + v2;
    
    /* Create data dependencies across all variables */
    v1 = v20 - v19;
    v2 = v19 - v18;
    v3 = v18 - v17;
    v4 = v17 - v16;
    v5 = v16 - v15;
    
    /* Call that clobbers registers - this should be at end of basic block */
    clobber_callee(&v1, &v2, &v3, &v4);
    
    /* Use results after call to keep them live */
    return v1 + v2 + v3 + v4 + v5 + v6 + v20;
}

/* Alternative path with less pressure */
int low_pressure_path(int seed) {
    int a = seed * 3;
    int b = seed + 10;
    return a - b;
}

/* Main test function with multiple call sites and complex control flow */
int test_function(int argc, char **argv) {
    int result = 0;
    
    /* Use argc and argv to create input-dependent but deterministic behavior */
    int seed = argc;
    if (argv[0]) {
        seed += argv[0][0];
    }
    
    /* Loop to create multiple basic blocks with calls */
    for (int i = 0; i < 10; i++) {
        /* Complex condition creating separate basic blocks */
        if ((seed + i) % 3 == 0) {
            /* Path 1: High register pressure with call at block end */
            int temp = high_pressure_path(seed + i);
            
            /* Another volatile operation */
            global_seed = temp % 100;
            
            /* Another call with different register pressure */
            int x = temp + i;
            int y = temp - i;
            float f1 = (float)x;
            float f2 = (float)y;
            clobber_callee2(&f1, &f2, &x);
            
            result += x + (int)f1 + (int)f2;
        } 
        else if ((seed + i) % 3 == 1) {
            /* Path 2: Different high pressure scenario */
            int a1 = seed * i + 1;
            int a2 = seed * i + 2;
            int a3 = seed * i + 3;
            int a4 = seed * i + 4;
            int a5 = seed * i + 5;
            int a6 = seed * i + 6;
            int a7 = seed * i + 7;
            int a8 = seed * i + 8;
            int a9 = seed * i + 9;
            int a10 = seed * i + 10;
            
            /* Create complex data web */
            a1 = a1 ^ a2;
            a2 = a2 & a3;
            a3 = a3 | a4;
            a4 = a4 + a5;
            a5 = a5 - a6;
            a6 = a6 * a7;
            a7 = a7 / (a8 ? a8 : 1);
            a8 = a8 ^ a9;
            a9 = a9 & a10;
            a10 = a10 | a1;
            
            /* Call in the middle of computations */
            clobber_callee(&a1, &a2, &a3, &a4);
            
            /* More computations after call */
            int b1 = a5 + a6;
            int b2 = a7 + a8;
            int b3 = a9 + a10;
            
            /* Another call */
            clobber_callee(&b1, &b2, &b3, &a1);
            
            result += b1 + b2 + b3 + a1;
        }
        else {
            /* Path 3: Low pressure path */
            result += low_pressure_path(seed + i);
        }
        
        /* Mix result with volatile global */
        result ^= global_seed;
        
        /* Switch statement to create more complex control flow */
        switch (i % 4) {
            case 0: {
                int s1 = result + 1;
                int s2 = result * 2;
                int s3 = result - 3;
                clobber_callee(&s1, &s2, &s3, &result);
                result = s1 + s2 + s3;
                break;
            }
            case 1:
                /* No call here - different block structure */
                result = result * 3;
                break;
            case 2: {
                float f1 = (float)result;
                float f2 = f1 * 1.5f;
                clobber_callee2(&f1, &f2, &result);
                result += (int)f1 + (int)f2;
                break;
            }
            case 3:
                /* Another path without call */
                result = result / 2;
                break;
        }
    }
    
    return result;
}

int main(int argc, char **argv) {
    /* Initialize with time for some randomness, but keep deterministic */
    int seed = time(NULL) % 1000;
    seed += argc;
    
    /* Run test multiple times to increase coverage chances */
    int total = 0;
    for (int iter = 0; iter < 5; iter++) {
        int res = test_function(argc + iter, argv);
        total += res;
        printf("Iteration %d: result = %d\n", iter, res);
        
        /* Modify global volatile to affect future iterations */
        global_seed = (global_seed + res) % 1000;
    }
    
    printf("Total: %d\n", total);
    return total % 256;
}
