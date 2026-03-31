#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/* Function to prevent dead code elimination */
static void escape(void* p) {
    asm volatile("" : : "g"(p) : "memory");
}

/* Complex loop with mixed operations and dependencies */
void process_data(int n, float threshold, int* arr1, int* arr2, 
                  float* farr1, float* farr2) {
    int acc = 0;
    volatile int vlimit = n; /* Prevent constant propagation */
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; ++outer) {
        /* Main computational loop with loop-carried dependency */
        for (int i = 1; i < vlimit; ++i) {
            /* Loop-carried dependency: acc depends on previous iteration */
            acc += arr1[i] * arr2[i-1];
            
            /* Mixed integer operations with bitwise */
            int temp = (arr1[i] & 0xFF) | (arr2[i] << 2);
            
            /* Floating-point computation */
            farr1[i] = farr1[i-1] * 1.01f + sinf((float)i * 0.1f);
            
            /* Conditional control flow with data-dependent condition */
            if (farr1[i] > threshold) {
                /* Complex floating-point operation */
                farr2[i] = sqrtf(fabsf(farr1[i])) + cosf((float)temp * 0.01f);
                
                /* Integer operation inside conditional */
                arr1[i] = (temp ^ 0x55) + (int)(farr2[i] * 100.0f);
            } else {
                /* Alternative path */
                farr2[i] = farr1[i] * 0.5f;
                arr1[i] = (temp & 0xAA) - (int)farr2[i];
            }
            
            /* Additional dependency chain */
            arr2[i] = arr2[i-1] + (int)(farr2[i] * 10.0f) + (acc & 0xFF);
            
            /* Cross-type operation */
            farr1[i] += (float)(arr1[i] % 100) * 0.01f;
        }
        
        /* Small perturbation between outer iterations */
        threshold *= 0.95f;
        acc = (acc >> 1) | (acc << 31); /* Rotate bits */
    }
    
    /* Use results to prevent elimination */
    escape(&acc);
    escape(arr1);
    escape(arr2);
    escape(farr1);
    escape(farr2);
}

int main(int argc, char** argv) {
    /* Use command line argument for non-constant loop bound */
    int n = 500;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 10) n = 500;
        if (n > 10000) n = 10000;
    }
    
    /* Initialize with volatile to prevent compile-time evaluation */
    volatile int seed = time(NULL);
    srand(seed);
    
    /* Allocate arrays with mixed types */
    int* arr1 = (int*)malloc(n * sizeof(int));
    int* arr2 = (int*)malloc(n * sizeof(int));
    float* farr1 = (float*)malloc(n * sizeof(float));
    float* farr2 = (float*)malloc(n * sizeof(float));
    
    if (!arr1 || !arr2 || !farr1 || !farr2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < n; ++i) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)(rand() % 1000) / 1000.0f;
        farr2[i] = 0.0f;
    }
    
    /* Data-dependent threshold */
    float threshold = (float)(rand() % 500) / 1000.0f + 0.3f;
    
    /* Process data multiple times */
    for (int iter = 0; iter < 2; ++iter) {
        process_data(n, threshold, arr1, arr2, farr1, farr2);
        threshold += 0.05f; /* Change threshold slightly */
    }
    
    /* Calculate checksum to ensure computation isn't eliminated */
    long long checksum = 0;
    for (int i = 0; i < n; ++i) {
        checksum += arr1[i] + arr2[i] + (long long)(farr1[i] * 1000.0f);
    }
    
    printf("Checksum: %lld\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr1);
    free(farr2);
    
    return 0;
}
