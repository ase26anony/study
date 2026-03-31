/* caller-save-test.c */
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent optimizations */
volatile int global_seed;

/* Function that clobbers many registers - prevent inlining */
#ifdef __x86_64__
#define CLOBBER_LIST "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11"
#elif __i386__
#define CLOBBER_LIST "eax", "ecx", "edx", "esi", "edi"
#elif __riscv
#define CLOBBER_LIST "t0", "t1", "t2", "t3", "t4", "t5", "t6", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7"
#else
#define CLOBBER_LIST "memory"
#endif

void __attribute__((noinline, noclone)) 
clobber_callee(int *p1, int *p2, int *p3, int *p4) {
    /* Opaque assembly to clobber registers */
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4) : CLOBBER_LIST);
    *p1 += 1;
    *p2 += 2;
    *p3 += 3;
    *p4 += 4;
}

/* Another clobbering function with different signature */
void __attribute__((noinline, noclone))
clobber_callee2(float *f1, float *f2, int *i1) {
    asm volatile("" : : "r"(f1), "r"(f2), "r"(i1) : CLOBBER_LIST);
    *f1 = *f1 * 2.0f;
    *f2 = *f2 * 3.0f;
    *i1 = *i1 * 5;
}

int main(int argc, char **argv) {
    /* Use argc as seed for deterministic but input-dependent behavior */
    int seed = argc > 1 ? atoi(argv[1]) : time(NULL);
    global_seed = seed;
    
    int result = 0;
    
    /* Loop to create multiple call sites */
    for (int iter = 0; iter < 3; iter++) {
        /* Declare MANY local variables to create register pressure */
        int v1 = seed + iter * 1;
        int v2 = seed + iter * 2;
        int v3 = seed + iter * 3;
        int v4 = seed + iter * 4;
        int v5 = seed + iter * 5;
        int v6 = seed + iter * 6;
        int v7 = seed + iter * 7;
        int v8 = seed + iter * 8;
        int v9 = seed + iter * 9;
        int v10 = seed + iter * 10;
        int v11 = seed + iter * 11;
        int v12 = seed + iter * 12;
        int v13 = seed + iter * 13;
        int v14 = seed + iter * 14;
        int v15 = seed + iter * 15;
        int v16 = seed + iter * 16;
        
        /* Float variables for floating point register pressure */
        float f1 = (float)v1 * 0.5f;
        float f2 = (float)v2 * 0.25f;
        float f3 = (float)v3 * 0.125f;
        float f4 = (float)v4 * 0.0625f;
        
        /* Complex computation that cannot be optimized away */
        v1 = v1 * v2 + global_seed;
        v2 = v2 * v3 - global_seed;
        v3 = v3 * v4 / (global_seed | 1);
        v4 = v4 ^ v5 ^ global_seed;
        v5 = (v5 + v6) * (v7 - v8);
        v6 = v6 & v7 | v8;
        v7 = v7 + v9 * v10;
        v8 = v8 - v11 + v12;
        v9 = v9 * 13 + v13;
        v10 = v10 / 7 + v14;
        v11 = v11 % 19 + v15;
        v12 = v12 << 2 + v16;
        
        f1 = f1 * f2 + (float)global_seed;
        f2 = f2 - f3 * (float)(global_seed & 0xFF);
        f3 = f3 / (f4 + 1.0f);
        f4 = f4 * 3.14159f - f1;
        
        /* Conditional to create different basic blocks */
        if ((seed + iter) % 3 == 0) {
            /* High register pressure path - call at end of block */
            
            /* More computations to increase live ranges */
            v13 = v1 + v2 + v3;
            v14 = v4 * v5 - v6;
            v15 = v7 ^ v8 ^ v9;
            v16 = v10 + v11 + v12;
            
            f1 = f1 + (float)v13;
            f2 = f2 * (float)v14;
            
            /* Call that clobbers registers - many variables are live */
            clobber_callee(&v1, &v2, &v3, &v4);
            
            /* This is the end of the basic block containing the call */
            /* The caller-save pass may need to insert saves before the call */
        } 
        else if ((seed + iter) % 3 == 1) {
            /* Different path with different register pressure */
            v1 = v1 ^ v5;
            v2 = v2 + v6;
            v3 = v3 * v7;
            
            /* Another clobbering call with different types */
            clobber_callee2(&f1, &f2, &v1);
            
            /* More computations after call */
            v4 = v4 - v8;
            v5 = v5 & v9;
        }
        else {
            /* Path without calls - simpler computation */
            v1 = v1 + 1;
            v2 = v2 - 1;
            v3 = v3 * 2;
        }
        
        /* Use all variables after conditional to keep them live */
        int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 
                     + v11 + v12 + v13 + v14 + v15 + v16
                     + (int)f1 + (int)f2 + (int)f3 + (int)f4;
        
        /* Mix in volatile read to prevent optimization */
        checksum ^= global_seed;
        
        result += checksum;
        
        /* Another call site in loop with different live variables */
        if (iter % 2 == 0) {
            int temp1 = v1 + v2;
            int temp2 = v3 + v4;
            clobber_callee(&temp1, &temp2, &v5, &v6);
            result += temp1 + temp2;
        }
    }
    
    printf("Result: %d\n", result);
    return result & 0xFF;
}
