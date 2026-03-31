/* caller-save-test.c
 * Designed to trigger specific uncovered lines in GCC's caller-save pass
 * Lines 905-913 in caller-save.cc handle insertion of save/restore instructions
 * at block boundaries where the call is at BB_END.
 */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

/* Global volatile to prevent optimizations */
volatile int global_seed = 42;

/* Function that clobbers many registers - prevent inlining */
#ifdef __x86_64__
#define CLOBBER_LIST "rax", "rcx", "rdx", "rsi", "rdi", "r8", "r9", "r10", "r11", "memory"
#elif __i386__
#define CLOBBER_LIST "eax", "ecx", "edx", "esi", "edi", "memory"
#elif __riscv
#define CLOBBER_LIST "t0", "t1", "t2", "t3", "t4", "t5", "t6", "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7", "memory"
#else
#define CLOBBER_LIST "memory"
#endif

/* Callee that clobbers registers - different versions for different arches */
void __attribute__((noinline, noclone)) clobber_callee(int *p1, int *p2, int *p3) {
    /* Opaque assembly to clobber registers */
    asm volatile("" : : "r"(p1), "r"(p2), "r"(p3) : CLOBBER_LIST);
    
    /* Ensure the pointers are used */
    if (p1) *p1 += 1;
    if (p2) *p2 += 2;
    if (p3) *p3 += 3;
}

/* Another clobbering function with different signature */
void __attribute__((noinline, noclone)) clobber_more(int *arr, int n) {
    asm volatile("" : : "r"(arr), "r"(n) : CLOBBER_LIST);
    for (int i = 0; i < n && i < 10; i++) {
        arr[i] += i;
    }
}

/* Function with high register pressure around calls */
int __attribute__((noinline)) high_pressure_path(int seed) {
    /* Declare many local variables to create register pressure */
    int v1, v2, v3, v4, v5, v6, v7, v8, v9, v10;
    int v11, v12, v13, v14, v15, v16, v17, v18, v19, v20;
    int v21, v22, v23, v24, v25, v26, v27, v28, v29, v30;
    
    /* Initialize with complex, non-removable computations */
    volatile int init = global_seed + seed;
    
    v1 = init * 1;   v2 = init * 2;   v3 = init * 3;   v4 = init * 4;   v5 = init * 5;
    v6 = init * 6;   v7 = init * 7;   v8 = init * 8;   v9 = init * 9;   v10 = init * 10;
    v11 = init * 11; v12 = init * 12; v13 = init * 13; v14 = init * 14; v15 = init * 15;
    v16 = init * 16; v17 = init * 17; v18 = init * 18; v19 = init * 19; v20 = init * 20;
    v21 = init * 21; v22 = init * 22; v23 = init * 23; v24 = init * 24; v25 = init * 25;
    v26 = init * 26; v27 = init * 27; v28 = init * 28; v29 = init * 29; v30 = init * 30;
    
    /* Create data dependencies between variables */
    for (int i = 0; i < 5; i++) {
        v1 = v2 + v3;  v2 = v3 + v4;  v3 = v4 + v5;  v4 = v5 + v6;  v5 = v6 + v7;
        v6 = v7 + v8;  v7 = v8 + v9;  v8 = v9 + v10; v9 = v10 + v11; v10 = v11 + v12;
        v11 = v12 + v13; v12 = v13 + v14; v13 = v14 + v15; v14 = v15 + v16; v15 = v16 + v17;
    }
    
    /* Call that clobbers registers - many variables are live across this call */
    clobber_callee(&v1, &v2, &v3);
    
    /* More computations to keep variables live */
    v4 = v1 + v2 + v3;
    v5 = v4 * v6 - v7;
    v8 = v9 + v10 * v11;
    
    /* Another call with different arguments */
    int arr[5] = {v12, v13, v14, v15, v16};
    clobber_more(arr, 5);
    
    /* Use all variables in final computation to prevent elimination */
    int result = v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 + v10 +
                 v11 + v12 + v13 + v14 + v15 + v16 + v17 + v18 + v19 + v20 +
                 v21 + v22 + v23 + v24 + v25 + v26 + v27 + v28 + v29 + v30 +
                 arr[0] + arr[1] + arr[2] + arr[3] + arr[4];
    
    return result;
}

/* Low pressure path for contrast */
int __attribute__((noinline)) low_pressure_path(int seed) {
    int a = seed * 2;
    int b = seed + 5;
    int c = a * b;
    
    /* Simple call with few live variables */
    clobber_callee(&a, &b, &c);
    
    return a + b + c;
}

/* Main function with complex control flow */
int main(int argc, char **argv) {
    int result = 0;
    
    /* Use argc as seed for deterministic but input-dependent behavior */
    int seed = argc;
    
    /* Multiple conditional paths to create different basic blocks */
    for (int iteration = 0; iteration < 10; iteration++) {
        /* Vary the condition to explore different paths */
        int condition = (seed + iteration) % 7;
        
        if (condition < 3) {
            /* High register pressure path - call at end of basic block */
            result += high_pressure_path(seed + iteration);
        } 
        else if (condition < 5) {
            /* Medium pressure with nested condition */
            int x = seed * iteration;
            int y = x + 100;
            int z = y * 2;
            
            if (x > y) {
                clobber_callee(&x, &y, &z);
                result += x - y + z;
            } else {
                clobber_more(&x, 3);
                result += x + y + z;
            }
        }
        else {
            /* Low pressure path */
            result += low_pressure_path(seed + iteration);
        }
        
        /* Mix in some switch statements for varied control flow */
        switch (iteration % 4) {
            case 0: {
                int temp = result * 2;
                clobber_callee(&temp, &result, &seed);
                result = temp;
                break;
            }
            case 1:
                /* Another call site */
                result = result ^ seed;
                break;
            case 2: {
                int arr[3] = {result, seed, iteration};
                clobber_more(arr, 3);
                result += arr[0] + arr[1];
                break;
            }
            default:
                /* No call in this path */
                result += iteration;
                break;
        }
    }
    
    /* Use volatile to prevent dead code elimination */
    volatile int final_result = result;
    
    printf("Result: %d\n", final_result);
    return final_result % 100;
}
