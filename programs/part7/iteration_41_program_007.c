#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/* Complex loop with mixed operations to engage selective scheduler */
void compute_kernel(int *arr1, int *arr2, float *farr1, float *farr2, 
                    int n, int threshold) {
    int acc = 0;  /* Loop-carried dependency */
    float f_acc = 0.0f;
    
    /* Outer loop to give scheduler repeated region */
    for (int iter = 0; iter < 3; ++iter) {
        /* Main computational loop with complex dependencies */
        for (int i = 1; i < n; ++i) {
            /* Multiple memory accesses with non-trivial indexing */
            int idx1 = i;
            int idx2 = 2 * i - 1;
            int idx3 = i % 32;
            
            /* Loop-carried integer dependency */
            acc += arr1[idx1] * arr2[idx2];
            
            /* Floating-point computation */
            float temp = farr1[idx1] * 1.5f + f_acc;
            
            /* Conditional control flow - data dependent */
            if (temp > threshold * 0.5f) {
                /* Mixed integer/float operations */
                farr2[idx1] = sqrtf(fabsf(temp)) + (float)(acc & 0xFF);
                
                /* Bitwise operation */
                arr1[idx1] = (arr1[idx1] ^ arr2[idx2]) & 0x7FFFFFFF;
            } else {
                /* Alternative computation path */
                farr2[idx1] = temp * 0.8f;
                arr1[idx1] = (arr1[idx1] | 0x1F) - idx3;
            }
            
            /* Additional floating-point dependency chain */
            f_acc = farr2[idx1] * 0.9f + sinf((float)i * 0.01f);
            
            /* Cross-type conversion with dependency */
            arr2[i] = (int)(f_acc * 100.0f) + arr1[i-1];  /* i-1 creates dependency */
            
            /* Complex conditional with short-circuit evaluation */
            if (i > 10 && (arr1[i] % 7 == 0 || farr1[i] < -0.3f)) {
                arr2[i] = ~arr2[i];
                farr1[i] = copysignf(farr1[i], f_acc);
            }
        }
        
        /* Slight variation between outer iterations */
        threshold = (threshold + iter) % 256;
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bound non-constant */
    int n = 500;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 100) n = 100;
        if (n > 1000) n = 1000;
    }
    
    /* Volatile to prevent compile-time optimization */
    volatile int threshold = 50;
    
    /* Allocate arrays */
    int *arr1 = (int*)malloc(n * sizeof(int));
    int *arr2 = (int*)malloc(n * sizeof(int));
    float *farr1 = (float*)malloc(n * sizeof(float));
    float *farr2 = (float*)malloc(n * sizeof(float));
    
    if (!arr1 || !arr2 || !farr1 || !farr2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    srand(time(NULL));
    for (int i = 0; i < n; ++i) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)(rand() % 2000 - 1000) / 500.0f;
        farr2[i] = 0.0f;
    }
    
    /* Call computational kernel multiple times */
    for (int repeat = 0; repeat < 2; ++repeat) {
        compute_kernel(arr1, arr2, farr1, farr2, n, threshold + repeat);
        
        /* Modify threshold slightly between calls */
        threshold = (threshold * 13 + 7) % 97;
    }
    
    /* Compute checksum to prevent dead code elimination */
    long long int_sum = 0;
    float float_sum = 0.0f;
    for (int i = 0; i < n; ++i) {
        int_sum += arr1[i] + arr2[i];
        float_sum += farr1[i] + farr2[i];
    }
    
    printf("Checksum: int=%lld float=%f\n", int_sum, float_sum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr1);
    free(farr2);
    
    return 0;
}
