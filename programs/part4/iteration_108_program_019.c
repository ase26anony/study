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
                 : "memory", "eax", "ecx", "edx", "esi", "edi");
    
    /* Force memory side effect */
    if (p1) *p1 ^= 1;
    if (p2) *p2 ^= 1;
    if (p3) *p3 ^= 1;
    if (p4) *p4 ^= 1;
}

/* Another clobbering function with different signature */
__attribute__((noinline, noclone))
void clobber_more(int *arr, int n) {
    asm volatile(""
                 :
                 : "r"(arr), "r"(n)
                 : "memory", "eax", "ebx", "ecx", "edx");
    
    for (int i = 0; i < n && i < 4; i++) {
        arr[i] += global_seed;
    }
}

int main(int argc, char **argv) {
    /* Use argc as volatile seed to create input-dependent behavior */
    volatile int seed = argc + global_seed;
    
    /* Declare MANY local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* Initialize with complex, non-removable computations */
    v1 = seed * 1;
    v2 = seed * 2 + argc;
    v3 = v1 + v2 * 3;
    v4 = v2 - v1 / 4;
    v5 = (v3 << 2) | (v4 >> 3);
    v6 = v5 ^ v3 ^ v4;
    v7 = v6 * 7 + seed;
    v8 = v7 - v6 + argc * 8;
    v9 = v8 * v7 / 9;
    v10 = v9 | v8 & v7;
    
    v11 = v10 + 11;
    v12 = v11 * 12 - argc;
    v13 = v12 ^ v11;
    v14 = v13 + v12 * 14;
    v15 = v14 - v13 / 15;
    v16 = v15 << 4;
    v17 = v16 >> 2;
    v18 = v17 * v16 + 18;
    v19 = v18 ^ v17 ^ seed;
    v20 = v19 - v18 * 20;
    
    v21 = v20 + 21;
    v22 = v21 * argc;
    v23 = v22 ^ v21 ^ v20;
    v24 = v23 + 24;
    v25 = v24 * 25 - seed;
    v26 = v25 ^ v24;
    v27 = v26 << 3;
    v28 = v27 >> 1;
    v29 = v28 * v27 + 29;
    v30 = v29 - v28 / 30;
    
    int result = 0;
    
    /* Complex conditional to create different basic blocks */
    for (int iteration = 0; iteration < 3; iteration++) {
        /* Different condition each iteration to vary control flow */
        if ((seed + iteration) % 3 == 0) {
            /* HIGH REGISTER PRESSURE PATH - many live variables across call */
            
            /* More computations to increase live range */
            v1 = v30 + v1 * iteration;
            v2 = v29 + v2 / (iteration + 1);
            v3 = v28 ^ v3;
            v4 = v27 | v4;
            v5 = v26 & v5;
            v6 = v25 + v6 * 6;
            v7 = v24 - v7;
            v8 = v23 * v8 + iteration;
            v9 = v22 ^ v9 ^ seed;
            v10 = v21 + v10 - argc;
            
            /* Call that clobbers registers - variables v1-v10 are live */
            clobber_callee(&v1, &v2, &v3, &v4);
            
            /* More computations keeping variables live */
            v11 = v1 + v2 + v3 + v4 + v5;
            v12 = v6 * v7 - v8 + v9 - v10;
            v13 = v11 ^ v12 ^ iteration;
            
            /* Another call with different arguments */
            clobber_more(&v11, 3);
            
            /* Use results */
            result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
            result += v11 + v12 + v13;
            
        } else if ((seed + iteration) % 3 == 1) {
            /* MEDIUM PRESSURE PATH */
            
            /* Different set of live variables */
            v14 = v15 + v16 * iteration;
            v15 = v17 - v18 / (iteration + 2);
            v16 = v19 ^ v20;
            
            clobber_callee(&v14, &v15, &v16, NULL);
            
            v17 = v14 + v15 + v16;
            result += v17 * 2;
            
        } else {
            /* LOW PRESSURE PATH - no call, simpler computations */
            v21 = v22 + v23 * iteration;
            v22 = v24 - v25;
            result += v21 + v22;
        }
        
        /* Mix in some volatile operations to prevent optimization */
        seed = global_seed + iteration;
        
        /* Loop-carried dependency */
        v30 = result ^ v30;
    }
    
    /* Additional call site at block end with different pressure */
    if (argc > 2) {
        /* Many variables live */
        int t1 = v1 + v2;
        int t2 = v3 + v4;
        int t3 = v5 + v6;
        int t4 = v7 + v8;
        int t5 = v9 + v10;
        
        /* Call at what might be block end */
        clobber_callee(&t1, &t2, &t3, &t4);
        
        result += t1 + t2 + t3 + t4 + t5;
    }
    
    /* Switch statement to create more complex control flow */
    switch (seed % 4) {
        case 0: {
            /* Another high pressure block */
            int w1 = v11 + v12;
            int w2 = v13 + v14;
            int w3 = v15 + v16;
            int w4 = v17 + v18;
            int w5 = v19 + v20;
            int w6 = v21 + v22;
            
            clobber_callee(&w1, &w2, &w3, &w4);
            clobber_more(&w5, 2);
            
            result += w1 + w2 + w3 + w4 + w5 + w6;
            break;
        }
        case 1:
            result += v23 + v24;
            break;
        case 2:
            result += v25 + v26;
            /* Intentional fallthrough */
        case 3:
            result += v27 + v28;
            if (result > 1000) {
                /* Nested condition with call */
                clobber_callee(&v29, &v30, &result, NULL);
            }
            break;
    }
    
    /* Use all variables in final computation to keep them live */
    result += v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10;
    result += v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20;
    result += v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30;
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return result & 0xFF;
}
