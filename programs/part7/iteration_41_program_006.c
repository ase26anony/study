#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

/* Complex loop with mixed operations and dependencies */
void compute_kernel(int *arr1, int *arr2, float *farr1, float *farr2, 
                    int n, int threshold) {
    int acc = 0;  /* Loop-carried dependency */
    float f_acc = 0.0f;
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; ++outer) {
        /* Main computational loop with complex dependencies */
        for (int i = 1; i < n; ++i) {
            /* Multiple memory accesses with non-trivial indexing */
            int idx1 = i;
            int idx2 = i - 1;  /* Creates dependency chain */
            int idx3 = 2 * i;
            
            /* Integer operations with loop-carried dependency */
            acc += arr1[idx1] * arr2[idx2];
            
            /* Floating-point operations */
            float temp = farr1[idx1] * 1.5f - farr2[idx2];
            f_acc = f_acc * 0.99f + temp;
            
            /* Conditional control flow - data dependent */
            if (farr1[idx1] > 0.5f && (arr1[idx1] & 0x3) == 0) {
                /* Complex operation inside conditional */
                farr2[idx1] = sqrtf(fabsf(farr1[idx1])) + f_acc;
                
                /* Mixed integer/float operations */
                arr1[idx1] = (arr1[idx1] & 0xFF) + (int)(farr2[idx1] * 100.0f);
            } else {
                /* Alternative path */
                farr2[idx1] = farr1[idx1] * farr1[idx2];
                arr1[idx1] = arr1[idx2] ^ arr1[idx1];
            }
            
            /* More operations with bitwise and arithmetic */
            arr2[idx1] = (arr2[idx2] + acc) | (arr1[idx1] & 0xFFFF);
            
            /* Additional floating-point operation */
            if (idx3 < n) {
                farr1[idx3] = sinf(farr2[idx1] * 0.01f);
            }
            
            /* Another conditional with threshold */
            if (acc > threshold) {
                acc = acc / 2;  /* Reset accumulator partially */
                farr1[idx1] = farr1[idx1] * 0.5f;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to prevent compile-time constant propagation */
    int n = 500;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n < 100) n = 100;
        if (n > 1000) n = 1000;
    }
    
    /* Volatile variable to prevent optimization */
    volatile int threshold = 10000;
    
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
        farr1[i] = (float)rand() / RAND_MAX;
        farr2[i] = (float)rand() / RAND_MAX;
    }
    
    /* Perform computation */
    compute_kernel(arr1, arr2, farr1, farr2, n, threshold);
    
    /* Calculate checksum to prevent dead code elimination */
    long long checksum = 0;
    float f_checksum = 0.0f;
    for (int i = 0; i < n; ++i) {
        checksum += arr1[i] + arr2[i];
        f_checksum += farr1[i] + farr2[i];
    }
    
    printf("Checksum: int=%lld float=%f\n", checksum, f_checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr1);
    free(farr2);
    
    return 0;
}
