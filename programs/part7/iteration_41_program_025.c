#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1000

/* External function to prevent optimization */
extern int use_result(int);

/* Function with complex loop to engage selective scheduler */
void process_data(int n, float threshold, int* arr1, int* arr2, 
                  float* farr1, float* farr2, int* results) {
    int acc = 0;
    int bitmask = 0xFF;
    
    /* Outer loop to give scheduler repeated region */
    for (int iter = 0; iter < 3; ++iter) {
        /* Reset accumulator each iteration */
        acc = iter * 100;
        
        /* Main computational loop with dependencies */
        for (int i = 1; i < n; ++i) {
            /* Loop-carried dependency */
            int prev = arr1[i-1] + acc;
            
            /* Mixed integer operations */
            int temp = (arr1[i] & bitmask) * arr2[i];
            
            /* Floating-point computation */
            float ftemp = farr1[i] * 2.0f - farr1[i-1];
            
            /* Conditional control flow - data dependent */
            if (ftemp > threshold && (i % 7) != 0) {
                /* Complex floating-point operation */
                farr2[i] = sqrtf(fabsf(ftemp)) + sinf(farr1[i] * 0.01f);
                
                /* Integer operation using float result */
                temp += (int)(farr2[i] * 100.0f);
            } else {
                /* Alternative computation path */
                farr2[i] = ftemp * 0.5f;
                temp -= arr2[i-1];
            }
            
            /* More mixed operations */
            int idx = (i * 3) % n;
            arr1[i] = prev + temp + arr2[idx];
            
            /* Update accumulator with loop-carried dependency */
            acc += arr1[i] - arr2[i];
            
            /* Bitwise operation */
            results[i] = (results[i-1] ^ temp) | (arr1[i] & 0x7F);
            
            /* Additional floating-point operation */
            if (i % 5 == 0) {
                farr1[i] = cosf(farr2[i] * 0.1f) * 2.0f;
            }
        }
        
        /* Cross-iteration dependency */
        arr1[0] = acc % 1000;
    }
}

int main(int argc, char** argv) {
    /* Use argc to prevent compile-time constant propagation */
    volatile int n = (argc > 1) ? atoi(argv[1]) : 500;
    if (n > SIZE) n = SIZE;
    if (n < 10) n = 10;
    
    /* Initialize arrays */
    int* arr1 = (int*)malloc(SIZE * sizeof(int));
    int* arr2 = (int*)malloc(SIZE * sizeof(int));
    float* farr1 = (float*)malloc(SIZE * sizeof(float));
    float* farr2 = (float*)malloc(SIZE * sizeof(float));
    int* results = (int*)malloc(SIZE * sizeof(int));
    
    /* Seed RNG */
    srand(time(NULL));
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < SIZE; ++i) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / RAND_MAX * 2.0f - 1.0f;
        farr2[i] = 0.0f;
        results[i] = 0;
    }
    
    /* Volatile threshold to prevent optimization */
    volatile float threshold = 0.25f + (argc * 0.01f);
    
    /* Process data multiple times */
    for (int repeat = 0; repeat < 2; ++repeat) {
        process_data(n, threshold, arr1, arr2, farr1, farr2, results);
        
        /* Modify threshold slightly each iteration */
        threshold += 0.05f;
    }
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    float fchecksum = 0.0f;
    for (int i = 0; i < n; ++i) {
        checksum ^= arr1[i] + results[i];
        fchecksum += farr1[i] + farr2[i];
        
        /* Prevent vectorization from simplifying too much */
        if (i % 8 == 0) {
            checksum += (int)(fchecksum * 10.0f);
        }
    }
    
    /* Use results to prevent optimization */
    printf("Checksums: int=%d, float=%.2f\n", checksum, fchecksum);
    
    /* Free memory */
    free(arr1);
    free(arr2);
    free(farr1);
    free(farr2);
    free(results);
    
    return checksum % 100;
}
