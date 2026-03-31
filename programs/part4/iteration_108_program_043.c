/* caller-save-test.c
 * Designed to trigger uncovered lines 905-913 in caller-save.cc
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
    /* Force register clobbering with inline asm */
    #ifdef __i386__
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4) 
                 : "eax", "ebx", "ecx", "edx", "esi", "edi", "memory");
    #elif __x86_64__
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4) 
                 : "rax", "rbx", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory");
    #else
    /* Generic memory clobber */
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4) : "memory");
    #endif
    
    /* Do some actual work to prevent removal */
    *p1 = *p1 ^ *p2;
    *p3 = *p3 + *p4;
}

/* Another clobbering function with different signature */
__attribute__((noinline, noclone))
void clobber_callee2(float *f1, float *f2, int *i1) {
    #ifdef __i386__
    asm volatile("" : : "r"(f1), "r"(f2), "r"(i1) 
                 : "eax", "ecx", "edx", "st", "st(1)", "st(2)", "st(3)", "memory");
    #else
    asm volatile("" : : "r"(f1), "r"(f2), "r"(i1) : "memory");
    #endif
    
    *f1 = *f1 * 2.0f;
    *f2 = *f2 / 3.0f;
    *i1 = *i1 + 1;
}

/* Function with complex control flow and high register pressure */
__attribute__((noinline))
int high_pressure_path(int seed) {
    /* Declare many local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    float f1, f2, f3, f4, f5;
    
    /* Initialize with complex, non-optimizable computations */
    v1 = seed * 3;
    v2 = seed + global_seed;  /* Use volatile global */
    v3 = v1 ^ v2;
    v4 = v2 - v1;
    v5 = v3 * v4;
    v6 = v5 / (seed | 1);     /* Avoid division by zero */
    v7 = v6 << 3;
    v8 = v7 >> 1;
    v9 = v8 | v7;
    v10 = v9 & v8;
    
    /* More computations creating data dependencies */
    v11 = v10 + v9;
    v12 = v11 - v8;
    v13 = v12 * v7;
    v14 = v13 ^ v6;
    v15 = v14 + v5;
    v16 = v15 - v4;
    v17 = v16 * v3;
    v18 = v17 ^ v2;
    v19 = v18 + v1;
    v20 = v19 - seed;
    
    /* Float computations to use FP registers */
    f1 = (float)v1 * 1.5f;
    f2 = (float)v2 * 2.5f;
    f3 = f1 + f2;
    f4 = f1 * f2;
    f5 = f3 / f4;
    
    /* Call that clobbers registers - many variables are live across this call */
    clobber_callee(&v10, &v11, &v12, &v13);
    
    /* More computations after call, using live variables */
    v14 = v10 + v11 + v12 + v13;
    v15 = v14 * v10;
    
    /* Another call with different register types */
    clobber_callee2(&f1, &f2, &v15);
    
    /* Final computation using all variables */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                 (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5;
    
    return result;
}

/* Low pressure path for contrast */
int low_pressure_path(int seed) {
    int a = seed * 2;
    int b = a + 1;
    return a ^ b;
}

/* Main function with complex control flow */
int main(int argc, char **argv) {
    int i, result = 0;
    
    /* Use argc as seed for deterministic but input-dependent behavior */
    int seed = argc;
    
    /* Loop to create multiple call sites */
    for (i = 0; i < 100; i++) {
        /* Complex condition creating different basic blocks */
        if ((seed + i) % 7 < 4) {
            /* High pressure path - call at end of basic block */
            result += high_pressure_path(seed + i);
            
            /* Additional volatile operation to prevent reordering */
            global_seed = (global_seed * 1103515245 + 12345) & 0x7fffffff;
        } else {
            /* Low pressure path */
            result += low_pressure_path(seed + i);
        }
        
        /* Switch statement to create more control flow complexity */
        switch ((seed + i) % 5) {
            case 0:
                result += high_pressure_path(result);
                break;
            case 1:
                result += low_pressure_path(result);
                break;
            case 2:
                /* Nested condition */
                if (result % 2) {
                    result += high_pressure_path(result * 3);
                } else {
                    result += low_pressure_path(result / 2);
                }
                break;
            default:
                /* Mixed path with moderate pressure */
                int x = result * 2;
                int y = x + global_seed;
                clobber_callee(&x, &y, &result, &seed);
                result = x ^ y;
                break;
        }
    }
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Additional printf with address-taking to create more register pressure */
    int final_check = result;
    printf("Final check: %d (address: %p)\n", final_check, (void*)&final_check);
    
    return result != 0;
}
