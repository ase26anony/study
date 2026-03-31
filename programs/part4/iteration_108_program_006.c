/* caller-save-test.c
 * Designed to trigger specific uncovered lines in GCC's caller-save pass
 * Compile with: gcc -O3 -m32 -fno-inline -fno-ipa-ra -fno-omit-frame-pointer caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent optimizations */
volatile int global_seed;

/* Function that clobbers many registers - marked noinline to prevent optimization */
void __attribute__((noinline, noclone)) clobber_callee(int *p1, int *p2, int *p3, int *p4) {
    /* Inline asm to explicitly clobber registers on x86 */
    asm volatile("" 
                 : 
                 : "r"(p1), "r"(p2), "r"(p3), "r"(p4)
                 : "memory", "eax", "ebx", "ecx", "edx", "esi", "edi");
    
    /* Opaque memory operation */
    if (p1) *p1 += 1;
    if (p2) *p2 += 2;
    if (p3) *p3 += 3;
    if (p4) *p4 += 4;
}

/* Another clobbering function with different signature */
void __attribute__((noinline, noclone)) clobber_callee2(float *f1, float *f2) {
    asm volatile("" 
                 : 
                 : "r"(f1), "r"(f2)
                 : "memory", "eax", "ecx", "edx", "xmm0", "xmm1", "xmm2", "xmm3");
    
    if (f1) *f1 += 1.5f;
    if (f2) *f2 += 2.5f;
}

int main(int argc, char **argv) {
    /* Use argc as volatile seed to create input-dependent behavior */
    global_seed = argc;
    
    /* Declare MANY local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    float f1, f2, f3, f4, f5, f6;
    
    /* Initialize with complex arithmetic to prevent constant folding */
    v1 = global_seed * 1;
    v2 = global_seed * 2 + argc;
    v3 = global_seed * 3 - argc;
    v4 = global_seed * 4 ^ argc;
    v5 = global_seed * 5 | argc;
    v6 = global_seed * 6 & argc;
    v7 = global_seed * 7 + (argc << 2);
    v8 = global_seed * 8 - (argc >> 1);
    v9 = global_seed * 9 ^ (argc * 3);
    v10 = global_seed * 10 | (argc + 1);
    
    v11 = v1 + v2;
    v12 = v3 * v4;
    v13 = v5 ^ v6;
    v14 = v7 | v8;
    v15 = v9 - v10;
    v16 = v11 * v12;
    v17 = v13 + v14;
    v18 = v15 ^ v16;
    v19 = v17 | v18;
    v20 = v19 - global_seed;
    
    /* Float computations */
    f1 = (float)v1 * 1.1f;
    f2 = (float)v2 * 1.2f;
    f3 = (float)v3 * 1.3f;
    f4 = (float)v4 * 1.4f;
    f5 = (float)v5 * 1.5f;
    f6 = (float)v6 * 1.6f;
    
    /* Create conditional branch where one path has high register pressure */
    if (global_seed & 1) {
        /* High register pressure path - many live variables across call */
        
        /* More computations to increase live range */
        v1 = v20 + v19;
        v2 = v18 * v17;
        v3 = v16 ^ v15;
        v4 = v14 | v13;
        
        f1 = f6 * 2.0f + f5;
        f2 = f4 * 3.0f - f3;
        
        /* Call that clobbers registers - variables v1-v20, f1-f6 are live */
        clobber_callee(&v1, &v2, &v3, &v4);
        
        /* More computations after call using live variables */
        v5 = v1 + v2 + v3 + v4;
        v6 = v5 * v20;
        
        /* Another call with float registers live */
        clobber_callee2(&f1, &f2);
        
        f3 = f1 + f2 + f5 + f6;
        
        /* Use all variables to prevent dead code elimination */
        v7 = v5 + v6 + v19 + v18;
        v8 = v7 * v17;
    } else {
        /* Simpler path with less register pressure */
        v1 = v2 + v3;
        v4 = v5 * v6;
        v7 = v8 ^ v9;
    }
    
    /* Loop to create multiple call sites with varying conditions */
    int i;
    int loop_sum = 0;
    for (i = 0; i < 10; i++) {
        /* Varying condition creates different basic block structures */
        if ((global_seed + i) & 2) {
            /* Another high pressure call site inside loop */
            int t1 = v1 + i;
            int t2 = v2 * i;
            int t3 = v3 ^ i;
            int t4 = v4 | i;
            
            /* These variables are live across the call */
            float ft1 = f1 + (float)i;
            float ft2 = f2 * (float)i;
            
            clobber_callee(&t1, &t2, &t3, &t4);
            clobber_callee2(&ft1, &ft2);
            
            loop_sum += t1 + t2 + t3 + t4 + (int)ft1 + (int)ft2;
        } else {
            /* Simpler loop path */
            loop_sum += v5 + v6 + i;
        }
        
        /* Modify some variables to create data flow across iterations */
        v1 += i;
        v2 ^= i;
        f1 += (float)i;
    }
    
    /* Switch statement to create more complex control flow */
    int switch_result = 0;
    switch (global_seed % 4) {
        case 0:
            /* Call at end of this case block */
            v10 = v9 + v8;
            v11 = v7 * v6;
            clobber_callee(&v10, &v11, &v12, &v13);
            switch_result = v10 + v11;
            break;
            
        case 1:
            /* Different call pattern */
            f4 = f3 * 2.0f;
            f5 = f2 + 1.0f;
            clobber_callee2(&f4, &f5);
            switch_result = (int)(f4 + f5);
            break;
            
        case 2:
            /* Nested condition with call */
            if (v1 > v2) {
                v14 = v15 + v16;
                clobber_callee(&v14, &v17, &v18, &v19);
                switch_result = v14;
            } else {
                switch_result = v20;
            }
            break;
            
        default:
            /* Call with many arguments - more register pressure */
            clobber_callee(&v1, &v2, &v3, &v4);
            clobber_callee(&v5, &v6, &v7, &v8);
            switch_result = v1 + v5;
            break;
    }
    
    /* Final computation using all variables to prevent optimization */
    int final_result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                      v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                      (int)f1 + (int)f2 + (int)f3 + (int)f4 + (int)f5 + (int)f6 +
                      loop_sum + switch_result;
    
    printf("Result: %d (seed: %d)\n", final_result, global_seed);
    
    return final_result & 0xFF;
}
