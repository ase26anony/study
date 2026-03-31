/* Test case to trigger selective scheduling RTL dumps */
#include <stdio.h>
#include <stdlib.h>

#define SIZE 128
#define ITERS 100000

/* Non-inlineable function to ensure selective scheduling analysis */
static void __attribute__((noinline,noipa))
stress_sched(int *result) {
    volatile int seed = 42; /* volatile to prevent optimization */
    int arr1[SIZE];
    int arr2[SIZE];
    float farr[SIZE];
    int i, j;
    
    /* Initialize arrays with chaotic but deterministic values */
    for (i = 0; i < SIZE; i++) {
        arr1[i] = (i * 37 + seed) % 7919;
        arr2[i] = (i * 73 + seed) % 7919;
        farr[i] = (float)((i * 19 + seed) % 7919) * 0.1f;
    }
    
    /* Outer loop - provides enough iterations for scheduling */
    for (j = 0; j < ITERS; j++) {
        int temp1, temp2, temp3;
        float ftemp1, ftemp2;
        
        /* Complex inner loop with high ILP potential */
        for (i = 1; i < SIZE - 1; i++) {
            /* Chain of dependent integer operations creating register pressure */
            temp1 = arr1[i-1] * 3 + arr2[i+1];
            temp2 = temp1 ^ (arr1[i] << 2);
            temp3 = temp2 - arr2[i-1] * 7;
            
            /* Floating-point operations mixed in */
            ftemp1 = farr[i] * 2.5f + (float)temp3 * 0.01f;
            ftemp2 = ftemp1 / (farr[i-1] + 1.0f);
            
            /* Conditional with side effects in both branches */
            if ((temp3 & 0x7F) > 64) {
                /* Branch 1: complex operations */
                arr1[i] = (temp1 * 11 + temp2 * 13) & 0xFFFF;
                arr2[i] = (temp3 * 17 - arr1[i+1]) & 0xFFFF;
                farr[i] = ftemp1 * ftemp2 + (float)(i * j);
            } else {
                /* Branch 2: different complex operations */
                arr1[i] = (temp2 * 19 - temp1 * 23) & 0xFFFF;
                arr2[i] = (arr1[i-1] * 29 + temp3 * 31) & 0xFFFF;
                farr[i] = ftemp2 * 3.14159f - (float)(i + j);
            }
            
            /* Inline assembly as scheduling barrier */
            asm volatile("" ::: "memory");
            
            /* Late use of early-computed values extending live ranges */
            arr1[i-1] = arr1[i-1] + (temp1 & 0xFF) - (temp2 & 0xFF);
            arr2[i+1] = arr2[i+1] ^ (temp3 & 0xFF);
            
            /* More floating point operations */
            farr[i] = farr[i] + (float)(arr1[i] + arr2[i]) * 0.001f;
        }
        
        /* Cross-iteration dependency to prevent loop unrolling from simplifying */
        arr1[0] = arr1[SIZE-1];
        arr2[0] = arr2[SIZE-1];
        farr[0] = farr[SIZE-1];
    }
    
    /* Compute checksum result */
    int sum = 0;
    for (i = 0; i < SIZE; i++) {
        sum += arr1[i] + arr2[i] + (int)farr[i];
    }
    *result = sum;
}

int main() {
    int result1, result2;
    
    /* Call multiple times to ensure warmup and consistent scheduling */
    stress_sched(&result1);
    stress_sched(&result2);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d %d (checksum: %d)\n", 
           result1, result2, result1 + result2);
    
    return 0;
}
