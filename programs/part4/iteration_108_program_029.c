/* caller-save-test.c
 * Designed to trigger uncovered lines in GCC's caller-save.cc
 * Compile with: gcc -O3 -m32 -fno-inline -fno-ipa-ra -fno-omit-frame-pointer caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent optimizations */
volatile int global_seed;

/* Function that clobbers many registers - x86 specific */
__attribute__((noinline, noclone))
void clobber_callee_x86(int *p1, int *p2, int *p3, int *p4) {
    /* Inline asm to clobber call-clobbered registers on x86 */
    asm volatile (
        "# Clobbering eax, ecx, edx\n"
        "movl $0, %%eax\n"
        "movl $0, %%ecx\n"
        "movl $0, %%edx\n"
        : /* no outputs */
        : "r"(p1), "r"(p2), "r"(p3), "r"(p4)
        : "eax", "ecx", "edx", "memory"
    );
    *p1 += 1;
    *p2 += 2;
    *p3 += 3;
    *p4 += 4;
}

/* Alternative for non-x86 architectures */
__attribute__((noinline, noclone))
void clobber_callee_generic(int *p1, int *p2, int *p3, int *p4) {
    /* Use volatile asm to prevent optimization */
    asm volatile ("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4) : "memory");
    *p1 += 1;
    *p2 += 2;
    *p3 += 3;
    *p4 += 4;
}

/* Another clobbering function with different signature */
__attribute__((noinline, noclone))
void clobber_more(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] += i;
    }
    /* Memory clobber to force spills */
    asm volatile ("" : : : "memory");
}

int main(int argc, char **argv) {
    /* Use argc as volatile seed to create input-dependent behavior */
    volatile int seed = argc + (int)time(NULL);
    global_seed = seed;
    
    int i, result = 0;
    
    /* Loop to create multiple call sites */
    for (int outer = 0; outer < 3; outer++) {
        /* Declare MANY local variables to create register pressure */
        int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
        int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
        int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
        
        /* Initialize with complex arithmetic to prevent optimization */
        v1 = seed + outer * 1;
        v2 = seed + outer * 2;
        v3 = seed + outer * 3;
        v4 = seed + outer * 4;
        v5 = seed + outer * 5;
        v6 = seed + outer * 6;
        v7 = seed + outer * 7;
        v8 = seed + outer * 8;
        v9 = seed + outer * 9;
        v10 = seed + outer * 10;
        v11 = seed + outer * 11;
        v12 = seed + outer * 12;
        v13 = seed + outer * 13;
        v14 = seed + outer * 14;
        v15 = seed + outer * 15;
        v16 = seed + outer * 16;
        v17 = seed + outer * 17;
        v18 = seed + outer * 18;
        v19 = seed + outer * 19;
        v20 = seed + outer * 20;
        v21 = seed + outer * 21;
        v22 = seed + outer * 22;
        v23 = seed + outer * 23;
        v24 = seed + outer * 24;
        v25 = seed + outer * 25;
        v26 = seed + outer * 26;
        v27 = seed + outer * 27;
        v28 = seed + outer * 28;
        v29 = seed + outer * 29;
        v30 = seed + outer * 30;
        
        /* Perform computations that create data dependencies */
        for (i = 0; i < 5; i++) {
            v1 = v1 * 3 + v2;
            v2 = v2 * 5 + v3;
            v3 = v3 * 7 + v4;
            v4 = v4 * 11 + v5;
            v5 = v5 * 13 + v6;
            v6 = v6 * 17 + v7;
            v7 = v7 * 19 + v8;
            v8 = v8 * 23 + v9;
            v9 = v9 * 29 + v10;
            v10 = v10 * 31 + v11;
            v11 = v11 * 37 + v12;
            v12 = v12 * 41 + v13;
            v13 = v13 * 43 + v14;
            v14 = v14 * 47 + v15;
            v15 = v15 * 53 + v16;
            v16 = v16 * 59 + v17;
            v17 = v17 * 61 + v18;
            v18 = v18 * 67 + v19;
            v19 = v19 * 71 + v20;
            v20 = v20 * 73 + v21;
            v21 = v21 * 79 + v22;
            v22 = v22 * 83 + v23;
            v23 = v23 * 89 + v24;
            v24 = v24 * 97 + v25;
            v25 = v25 * 101 + v26;
            v26 = v26 * 103 + v27;
            v27 = v27 * 107 + v28;
            v28 = v28 * 109 + v29;
            v29 = v29 * 113 + v30;
            v30 = v30 * 127 + v1;
        }
        
        /* Complex conditional to create different basic blocks */
        if ((seed + outer) % 3 == 0) {
            /* High register pressure path - call clobbering function
             * Many variables are live across this call */
            
            /* Use volatile read to prevent moving computations */
            volatile int barrier = global_seed;
            v1 += barrier;
            
            /* Call that clobbers registers - variables v1-v8 are live */
#ifdef __i386__
            clobber_callee_x86(&v1, &v2, &v3, &v4);
#else
            clobber_callee_generic(&v1, &v2, &v3, &v4);
#endif
            
            /* More computations to keep variables live */
            v5 = v1 + v2 + v3 + v4;
            v6 = v5 * 2 - v1;
            
            /* Another call with different live variables */
            int arr[4] = {v7, v8, v9, v10};
            clobber_more(arr, 4);
            v7 = arr[0];
            v8 = arr[1];
            v9 = arr[2];
            v10 = arr[3];
            
        } else if ((seed + outer) % 3 == 1) {
            /* Medium pressure path */
            v1 = v1 * 2;
            v2 = v2 * 3;
            
            /* Call with fewer live variables */
            int arr[2] = {v3, v4};
            clobber_more(arr, 2);
            v3 = arr[0];
            v4 = arr[1];
            
        } else {
            /* Low pressure path - no calls */
            v1 = v1 + v2;
            v2 = v2 + v3;
        }
        
        /* Use all variables after conditional to keep them live */
        result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
        result += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
        result += v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
        
        /* Mix in some I/O to prevent optimization */
        if (outer == 0) {
            printf("Intermediate: %d\n", result);
        }
    }
    
    /* Final result depends on all computations */
    printf("Final result: %d\n", result);
    
    return result % 256;
}
