/* Compile with: gcc -O3 -m32 -fno-inline -fno-ipa-ra -fno-omit-frame-pointer caller_save_test.c -o caller_save_test */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent optimizations */
volatile int global_seed;

/* Function that clobbers many registers - x86 specific */
__attribute__((noinline, noclone))
void clobber_callee_x86(int *p1, int *p2, int *p3) {
    /* Inline asm to clobber caller-saved registers on x86 */
    asm volatile("" 
                 : 
                 : "r"(p1), "r"(p2), "r"(p3)
                 : "memory", "eax", "ecx", "edx", "esi", "edi", "ebx");
    *p1 += 1;
    *p2 += 2;
    *p3 += 3;
}

/* Alternative for non-x86 architectures */
__attribute__((noinline, noclone))
void clobber_callee_generic(int *p1, int *p2, int *p3) {
    /* Use memory clobber and dummy asm to prevent optimization */
    asm volatile("" : : : "memory");
    *p1 += 1;
    *p2 += 2;
    *p3 += 3;
}

/* Another clobbering function with different signature */
__attribute__((noinline, noclone))
void clobber_more(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] += i;
    }
    asm volatile("" : : : "memory");
}

int main(int argc, char **argv) {
    /* Use argc as volatile seed to create input-dependent behavior */
    volatile int seed = argc + (int)time(NULL);
    global_seed = seed;
    
    int result = 0;
    
    /* Loop to create multiple call sites */
    for (int iter = 0; iter < 3; iter++) {
        /* Declare MANY local variables to create register pressure */
        int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
        int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
        
        /* Initialize with complex arithmetic to prevent constant folding */
        v1 = seed + iter * 1;
        v2 = seed * 2 - iter;
        v3 = (seed << 3) | (iter & 0xFF);
        v4 = ~seed + iter;
        v5 = seed ^ (iter * 0x1234);
        v6 = seed % 17 + iter;
        v7 = (seed & 0xFFFF) * iter;
        v8 = seed / (iter + 1) + 123;
        v9 = seed | (iter << 16);
        v10 = seed & (0xFFFFFFFF << iter);
        
        /* Chain computations to create data dependencies */
        v11 = v1 + v2 * v3;
        v12 = v4 - v5 / (v6 + 1);
        v13 = v7 | v8 & v9;
        v14 = v10 ^ v1 * v2;
        v15 = v3 + v4 - v5;
        v16 = v6 * v7 / (v8 + 1);
        v17 = v9 & v10 | v1;
        v18 = v2 ^ v3 * v4;
        v19 = v5 + v6 - v7;
        v20 = v8 * v9 / (v10 + 1);
        
        /* More computations */
        v21 = v11 + v12 - v13;
        v22 = v14 * v15 / (v16 + 1);
        v23 = v17 | v18 & v19;
        v24 = v20 ^ v11 * v12;
        v25 = v13 + v14 - v15;
        v26 = v16 * v17 / (v18 + 1);
        v27 = v19 & v20 | v11;
        v28 = v12 ^ v13 * v14;
        v29 = v15 + v16 - v17;
        v30 = v18 * v19 / (v20 + 1);
        
        /* Read volatile global to create a barrier */
        int barrier = global_seed;
        
        /* Complex conditional to create different basic blocks */
        if ((seed ^ iter) & 0x1) {
            /* High register pressure path - call with many live variables */
            
            /* Use all variables in computation before call */
            int sum1 = v1 + v3 + v5 + v7 + v9 + v11 + v13 + v15 + v17 + v19;
            int sum2 = v2 + v4 + v6 + v8 + v10 + v12 + v14 + v16 + v18 + v20;
            int sum3 = v21 + v23 + v25 + v27 + v29;
            int sum4 = v22 + v24 + v26 + v28 + v30;
            
            /* Call clobbering function - many registers will be live across this call */
            #ifdef __i386__
            clobber_callee_x86(&sum1, &sum2, &sum3);
            #else
            clobber_callee_generic(&sum1, &sum2, &sum3);
            #endif
            
            /* Use results after call - keeping variables live */
            v1 = sum1 + barrier;
            v2 = sum2 - barrier;
            v3 = sum3 * barrier;
            v4 = sum4 / (barrier + 1);
            
            /* Another call with different arguments */
            int arr[5] = {v5, v6, v7, v8, v9};
            clobber_more(arr, 5);
            v5 = arr[0];
            v6 = arr[1];
            
        } else {
            /* Lower pressure path - still use variables to prevent optimization */
            v1 = v1 ^ v2;
            v3 = v3 | v4;
            v5 = v5 & v6;
            v7 = v7 + v8;
            v9 = v9 - v10;
        }
        
        /* Use variables after conditional to keep them live */
        int checksum = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                      v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                      v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
        
        /* Mix in volatile read */
        checksum ^= global_seed;
        
        /* Use checksum to prevent dead code elimination */
        result += checksum;
        
        /* Modify seed for next iteration */
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    printf("Result: %d\n", result);
    return 0;
}
