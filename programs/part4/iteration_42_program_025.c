/* Test case to trigger selective scheduling RTL dumps in sel-sched-dump.cc */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 128
#define ITERS 100000

/* Target function with complex scheduling requirements */
static void __attribute__((noinline)) 
stress_sched(int *result) {
    volatile int seed = 42;  /* volatile to prevent optimization */
    int arr1[SIZE];
    int arr2[SIZE];
    float farr[SIZE];
    int i, j;
    
    /* Initialize arrays with chaotic but deterministic values */
    for (i = 0; i < SIZE; i++) {
        arr1[i] = (i * 1103515245 + 12345) & 0x7fffffff;
        arr2[i] = (i * 1664525 + 1013904223) & 0x7fffffff;
        farr[i] = (float)i * 0.12345f;
    }
    
    /* Outer loop - provides enough iterations for scheduling */
    for (j = 0; j < ITERS; j++) {
        int idx = j & (SIZE - 1);
        int a = arr1[idx];
        int b = arr2[idx];
        float f = farr[idx];
        
        /* Complex dependency chain with mixed operations */
        int t1 = a * 3 + b;
        int t2 = b / 7 - a;
        float ft1 = f * 2.5f;
        float ft2 = f / 1.7f;
        
        /* Volatile read creates scheduling barrier */
        int barrier = seed;
        
        /* Conditional with side effects in both branches */
        if ((t1 ^ t2) & 0x100) {
            /* Branch 1: Integer-heavy operations */
            t1 = t1 * 5 + barrier;
            t2 = t2 ^ (barrier << 3);
            ft1 = ft1 + (float)t1 * 0.01f;
            
            /* Inline assembly as scheduling boundary */
            asm volatile("" ::: "memory");
            
            /* More operations after assembly */
            t1 = t1 + (t2 >> 4);
            arr1[idx] = t1 ^ t2;
        } else {
            /* Branch 2: Float-heavy operations */
            ft2 = ft2 - (float)t2 * 0.02f;
            t1 = t1 & 0xffff;
            t2 = t2 | 0xff00;
            
            /* Another inline assembly barrier */
            asm volatile("" ::: "memory");
            
            /* Operations with cross-type dependencies */
            ft1 = ft1 * ft2;
            arr2[idx] = t1 + (int)ft1;
        }
        
        /* Live range extension: use values computed much earlier */
        int final1 = t1 * 11 + (int)ft1;
        int final2 = t2 * 13 + (int)ft2;
        
        /* Complex index calculation with dependencies */
        int idx2 = (final1 ^ final2) & (SIZE - 1);
        
        /* Cross-array operations extending live ranges */
        arr1[idx2] = arr1[idx2] + final1;
        arr2[idx2] = arr2[idx2] ^ final2;
        farr[idx2] = farr[idx2] + (float)final1 - (float)final2;
        
        /* Periodic complex operation every 8 iterations */
        if ((j & 7) == 0) {
            int sum = 0;
            for (int k = 0; k < 8; k++) {
                sum += arr1[(idx + k) & (SIZE - 1)];
                sum -= arr2[(idx + k) & (SIZE - 1)];
            }
            arr1[idx] = sum;
        }
    }
    
    /* Compute final result checksum */
    int checksum = 0;
    for (i = 0; i < SIZE; i++) {
        checksum += arr1[i] + arr2[i] + (int)farr[i];
    }
    *result = checksum;
}

/* Secondary function to increase compilation unit complexity */
static int __attribute__((noinline))
helper_func(int x, int y) {
    int z = x * y;
    z = (z << 3) | (z >> 29);  /* rotate */
    z = z ^ (z * 0x5bd1e995);
    return z;
}

int main() {
    int result1, result2;
    
    /* Call target function multiple times */
    stress_sched(&result1);
    
    /* Call again with different memory pattern */
    result2 = helper_func(result1, 12345);
    stress_sched(&result2);
    
    /* Mix results to ensure both are used */
    int final_result = result1 ^ result2;
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", final_result);
    
    /* Additional system call might affect scheduling */
    fflush(stdout);
    
    return (final_result == 0) ? 0 : 1;
}
