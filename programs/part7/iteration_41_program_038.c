#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1000

/* External function to prevent inlining and create more complex RTL */
extern int external_helper(int x);

/* Function with complex loop to trigger selective scheduling */
void process_arrays(int n, int *arr1, int *arr2, float *farr1, float *farr2) {
    int acc = 0;
    volatile int threshold = 100;  /* Prevent constant propagation */
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; outer++) {
        /* Main computational loop with loop-carried dependency */
        for (int i = 1; i < n; i++) {
            /* Loop-carried dependency: current depends on previous */
            int prev_val = arr1[i-1] + arr2[i-1];
            
            /* Mixed integer operations with bitwise */
            int temp = (arr1[i] & 0xFF) | (arr2[i] << 2);
            temp = temp * prev_val / 256;
            
            /* Floating-point computation */
            float ftemp = farr1[i] * 1.5f + farr2[i-1];
            
            /* Conditional control flow with data-dependent condition */
            if (ftemp > 0.5f && (temp % 7) > 3) {
                /* Complex floating-point operation */
                farr2[i] = sqrtf(fabsf(ftemp)) + sinf(farr1[i] * 0.01f);
                
                /* Integer operation inside conditional */
                arr1[i] = temp + (int)(farr2[i] * 10.0f);
            } else {
                /* Alternative path */
                farr2[i] = ftemp * 0.8f;
                arr1[i] = temp - external_helper(i);
            }
            
            /* More mixed operations */
            arr2[i] = (arr2[i] ^ arr1[i]) + (i & 0xF);
            
            /* Accumulator with loop-carried dependency */
            acc += arr1[i] + (int)(farr2[i] * 100.0f);
            
            /* Additional floating-point operation */
            farr1[i] = farr1[i] * 0.99f + acc * 0.001f;
        }
        
        /* Small computation between outer iterations */
        if (outer < 2) {
            for (int i = 0; i < 10; i++) {
                arr1[i] = arr1[i] + arr2[n-i-1];
                farr2[i] = farr1[n-i-1] * 2.0f;
            }
        }
    }
    
    /* Use the result to prevent dead code elimination */
    volatile int result = acc;
    printf("Accumulator result: %d\n", result);
}

/* Simple external helper function */
int external_helper(int x) {
    return (x * 37) & 0xFF;
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bound non-constant */
    int n = (argc > 1) ? atoi(argv[1]) : 500;
    if (n > SIZE) n = SIZE;
    if (n < 10) n = 10;
    
    /* Initialize with random data */
    srand(time(NULL));
    
    int *arr1 = malloc(SIZE * sizeof(int));
    int *arr2 = malloc(SIZE * sizeof(int));
    float *farr1 = malloc(SIZE * sizeof(float));
    float *farr2 = malloc(SIZE * sizeof(float));
    
    if (!arr1 || !arr2 || !farr1 || !farr2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Fill arrays with pseudo-random data */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / RAND_MAX * 2.0f - 1.0f;
        farr2[i] = (float)rand() / RAND_MAX;
    }
    
    /* Process arrays - this is where selective scheduling should trigger */
    process_arrays(n, arr1, arr2, farr1, farr2);
    
    /* Final checksum to ensure computation isn't optimized away */
    long long checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += arr1[i] + arr2[i] + (long long)(farr1[i] * 1000) + (long long)(farr2[i] * 1000);
    }
    printf("Final checksum: %lld\n", checksum);
    
    free(arr1);
    free(arr2);
    free(farr1);
    free(farr2);
    
    return 0;
}
