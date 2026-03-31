/* caller-save-test.c
 * Designed to trigger specific uncovered lines in GCC's caller-save pass
 * Compile with: gcc -O3 -m32 -fno-inline -fno-ipa-ra -fno-omit-frame-pointer caller-save-test.c -o caller-save-test
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent optimizations */
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

/* Callee that clobbers many registers */
void __attribute__((noinline, noclone)) clobber_callee(int *p1, int *p2, int *p3, int *p4) {
    /* Inline asm to clobber registers and memory */
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3), "r"(p4) : CLOBBER_LIST);
    
    /* Modify through pointers to create side effects */
    if (p1) *p1 += 1;
    if (p2) *p2 += 2;
    if (p3) *p3 += 3;
    if (p4) *p4 += 4;
}

/* Another clobbering function with different signature */
void __attribute__((noinline, noclone)) clobber_callee2(int *arr, int n) {
    asm volatile("" : : "r"(arr), "r"(n) : CLOBBER_LIST);
    for (int i = 0; i < n && i < 4; i++) {
        arr[i] += i;
    }
}

int main(int argc, char **argv) {
    /* Use argc as volatile seed to create input-dependent behavior */
    volatile int seed = argc + global_seed;
    
    /* Declare MANY local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* Initialize with complex arithmetic to prevent constant folding */
    v1 = seed * 1;
    v2 = seed * 2 + argc;
    v3 = seed * 3 - argc;
    v4 = seed * 4 ^ argc;
    v5 = seed * 5 | argc;
    v6 = seed * 6 & argc;
    v7 = seed * 7 + (argc << 2);
    v8 = seed * 8 - (argc >> 1);
    v9 = seed * 9 ^ (argc * 3);
    v10 = seed * 10 | (argc + 1);
    
    /* More computations creating data dependencies */
    v11 = v1 + v2;
    v12 = v3 - v4;
    v13 = v5 * v6;
    v14 = v7 ^ v8;
    v15 = v9 | v10;
    v16 = v11 + v12;
    v17 = v13 - v14;
    v18 = v15 * v16;
    v19 = v17 ^ v18;
    v20 = v19 + seed;
    
    /* Additional variables with more complex computations */
    v21 = (v1 * v2) / (seed + 1);
    v22 = (v3 ^ v4) & (seed | 0xFF);
    v23 = (v5 + v6) * (v7 - v8);
    v24 = (v9 | v10) ^ (v11 & v12);
    v25 = v13 + v14 + v15 + v16;
    v26 = v17 * v18 - v19;
    v27 = v20 ^ v21 | v22;
    v28 = v23 + v24 * v25;
    v29 = v26 - v27 + v28;
    v30 = v29 * seed;
    
    /* Create conditional branch where one path has high register pressure */
    if (seed % 3 == 0) {
        /* High register pressure path - call with many live variables */
        
        /* Use volatile read to prevent reordering */
        volatile int barrier = global_seed;
        
        /* Call clobbering function with addresses of live variables */
        /* This makes v1-v8 live across the call */
        clobber_callee(&v1, &v2, &v3, &v4);
        
        /* More computations between calls to create multiple basic blocks */
        v5 = v1 + v2 + v3 + v4 + barrier;
        v6 = v5 * 2 - barrier;
        
        /* Another call with different live variables */
        clobber_callee(&v5, &v6, &v7, &v8);
        
        /* More live variables across calls */
        v9 = v5 + v6 + v7 + v8;
        v10 = v9 * 3;
        
        /* Call with array of live values */
        int live_arr[4] = {v21, v22, v23, v24};
        clobber_callee2(live_arr, 4);
        v21 = live_arr[0];
        v22 = live_arr[1];
        
        /* Complex computation using many variables */
        v25 = (v1 + v2) * (v3 - v4) / (v5 + 1);
        v26 = (v6 ^ v7) | (v8 & v9);
        v27 = v10 + v21 + v22 + v25 + v26;
        
    } else if (seed % 3 == 1) {
        /* Medium pressure path */
        v1 = v2 + v3;
        v4 = v5 * v6;
        clobber_callee(&v1, &v4, &v7, &v8);
        v9 = v1 + v4 + v7 + v8;
    } else {
        /* Low pressure path - no call, just computations */
        v1 = v2 + v3 + v4 + v5;
        v6 = v7 * v8 - v9;
        v10 = v1 ^ v6;
    }
    
    /* Loop to create multiple caller-save opportunities */
    int sum = 0;
    for (int i = 0; i < 3; i++) {
        /* Variables live across loop iterations */
        int loop_v1 = v1 + i;
        int loop_v2 = v2 * (i + 1);
        int loop_v3 = v3 - i;
        int loop_v4 = v4 ^ i;
        
        /* Conditional inside loop with call */
        if (i % 2 == 0) {
            /* Call at what might be end of basic block */
            clobber_callee(&loop_v1, &loop_v2, &loop_v3, &loop_v4);
            
            /* Use results */
            v1 += loop_v1;
            v2 += loop_v2;
        } else {
            /* Different computation path */
            loop_v1 = loop_v1 * 2;
            loop_v2 = loop_v2 / 2;
        }
        
        /* Accumulate to prevent dead code elimination */
        sum += loop_v1 + loop_v2 + loop_v3 + loop_v4;
    }
    
    /* Use all variables in final computation to keep them live */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                 v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30 + sum;
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d (seed: %d)\n", result, seed);
    
    /* Additional test with switch statement */
    switch (seed % 4) {
        case 0: {
            /* Case with call at potential block end */
            int case_v1 = result * 2;
            int case_v2 = result / 2;
            clobber_callee(&case_v1, &case_v2, &v1, &v2);
            result += case_v1 + case_v2;
            break;
        }
        case 1: {
            /* Different case */
            result = result ^ 0xABCD;
            break;
        }
        case 2: {
            /* Another case with call */
            int arr[3] = {v3, v4, v5};
            clobber_callee2(arr, 3);
            result += arr[0] + arr[1] + arr[2];
            break;
        }
        default:
            result = ~result;
    }
    
    printf("Final result: %d\n", result);
    return result != 0;
}
