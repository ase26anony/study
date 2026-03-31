/* Test case for sel-sched-dump.cc uncovered lines */
/* Compile with: gcc -O2 -fsel-sched-pipelining -fsel-sched-pipelining-outer-loops -fdump-rtl-sched1 -fdump-rtl-sched2 -dS -mtune=generic -o test test.c */

#include <stdio.h>
#include <stdlib.h>

#define SIZE 128
#define ITERS 100000

/* Non-inlineable function to ensure selective scheduling analysis */
static void __attribute__((noinline,optimize("no-unroll-loops")))
stress_sched(int *result) {
    volatile int trigger = 0; /* Volatile to create scheduling barriers */
    int arr1[SIZE];
    int arr2[SIZE];
    float farr[SIZE];
    int i, j;
    
    /* Initialize arrays with non-trivial patterns */
    for (i = 0; i < SIZE; i++) {
        arr1[i] = i * 3 + 1;
        arr2[i] = i * 7 - 2;
        farr[i] = i * 0.5f;
    }
    
    /* Outer loop to provide sufficient iteration count */
    for (j = 0; j < ITERS; j++) {
        int temp1 = arr1[j % SIZE];
        int temp2 = arr2[j % SIZE];
        float ftemp = farr[j % SIZE];
        
        /* Complex inner loop with high ILP potential */
        for (i = 1; i < SIZE - 1; i++) {
            /* Chain of dependent arithmetic operations */
            int idx1 = (i + temp1) % SIZE;
            int idx2 = (i * 2 + temp2) % SIZE;
            
            /* Mixed integer and FP calculations */
            int calc1 = arr1[idx1] * 3 + arr2[idx2] / 7;
            int calc2 = arr1[idx2] * 5 - arr2[idx1] / 3;
            float fcalc = farr[idx1] * 2.3f - farr[idx2] * 1.7f;
            
            /* Volatile read to create scheduling barrier */
            int barrier = trigger;
            
            /* Conditional execution with side effects */
            if ((calc1 + calc2) % 17 > 8) {
                /* Branch 1: Complex calculations */
                arr1[i] = calc1 * 2 + barrier;
                arr2[i] = calc2 / 3 + (int)fcalc;
                farr[i] = fcalc * 1.5f + barrier;
                
                /* Additional dependent operations */
                int extra = arr1[(i + 3) % SIZE] + arr2[(i + 5) % SIZE];
                arr1[(i + 1) % SIZE] += extra % 31;
            } else {
                /* Branch 2: Different calculations */
                arr1[i] = calc2 * 3 - barrier;
                arr2[i] = calc1 / 2 - (int)(fcalc * 2.0f);
                farr[i] = fcalc * 0.75f - barrier;
                
                /* Different dependent operations */
                int extra = arr1[(i + 7) % SIZE] - arr2[(i + 11) % SIZE];
                arr2[(i + 2) % SIZE] += extra % 29;
            }
            
            /* Inline assembly as scheduling boundary */
            asm volatile("" ::: "memory");
            
            /* Use values computed much earlier (extending live ranges) */
            int late_use1 = arr1[idx1] + calc1;
            int late_use2 = arr2[idx2] + calc2;
            float late_usef = farr[idx1] + fcalc;
            
            /* More operations with extended live ranges */
            arr1[(i + 13) % SIZE] = late_use1 * 3 - late_use2;
            arr2[(i + 17) % SIZE] = late_use2 * 2 + late_use1;
            farr[(i + 19) % SIZE] = late_usef * 1.3f;
            
            /* Another volatile operation */
            trigger = i & 0xFF;
        }
        
        /* Accumulate results with complex addressing */
        temp1 = arr1[(j * 3) % SIZE] + arr2[(j * 5) % SIZE];
        temp2 = (int)(farr[(j * 7) % SIZE] * 100.0f);
        *result += temp1 + temp2;
    }
}

int main() {
    int result = 0;
    int i;
    
    /* Call the stress function multiple times */
    for (i = 0; i < 3; i++) {
        stress_sched(&result);
    }
    
    printf("Result: %d\n", result);
    
    /* Additional variant calls with different parameters */
    int arr[SIZE];
    for (i = 0; i < SIZE; i++) {
        arr[i] = i;
    }
    
    /* Call with array parameter */
    stress_sched(&arr[0]);
    printf("Array result: %d\n", arr[0]);
    
    return 0;
}
