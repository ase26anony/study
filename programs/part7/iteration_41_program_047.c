#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1000

/* External function to prevent optimization */
extern int use_result(int);

/* Complex loop with mixed operations and dependencies */
void compute_kernel(int *arr1, int *arr2, float *farr1, float *farr2, 
                    int n, int threshold) {
    int acc = 0;                    /* Loop-carried dependency */
    float f_acc = 0.0f;             /* Floating-point accumulator */
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; ++outer) {
        /* Main computational loop with complex dependencies */
        for (int i = 1; i < n; ++i) {
            /* Multiple memory accesses with non-trivial indexing */
            int idx1 = i;
            int idx2 = i - 1;                    /* Creates dependency on previous iteration */
            int idx3 = (2 * i) % n;              /* Non-linear indexing */
            
            /* Integer operations with loop-carried dependency */
            acc += arr1[idx1] * arr2[idx2];      /* RAW dependency on arr2 from previous iteration */
            
            /* Floating-point computation */
            float temp = farr1[idx1] * 1.5f;
            f_acc = temp + farr2[idx2] * 0.7f;   /* Another loop-carried dependency */
            
            /* Conditional control flow inside loop */
            if (acc > threshold) {               /* Data-dependent, unpredictable condition */
                /* Mixed-type operation */
                arr1[idx1] = (arr1[idx1] & 0xFF) + (int)(f_acc * 100.0f);
                
                /* More floating-point with function call */
                if (farr1[idx3] > 0.5f) {
                    farr2[idx1] = sqrtf(fabsf(farr1[idx3]));
                } else {
                    farr2[idx1] = farr1[idx1] * farr1[idx1];
                }
            } else {
                /* Alternative path with bitwise operations */
                arr2[idx1] = (arr2[idx1] << 3) | (arr2[idx2] & 0x7);
                farr2[idx1] = sinf(farr1[idx1]) * cosf(farr2[idx2]);
            }
            
            /* Additional computation to increase instruction mix */
            int bit_op = arr1[idx1] ^ arr2[idx1];
            float f_mul = farr1[idx1] * farr2[idx1];
            
            /* Store results creating WAW/RAW dependencies */
            arr1[idx3] = bit_op + (int)f_mul;
            farr1[idx1] = f_mul + (float)bit_op;
        }
        
        /* Small inner loop with different pattern */
        for (int i = 0; i < n/2; ++i) {
            int j = n - i - 1;
            arr1[i] = arr1[i] + arr2[j];
            farr1[i] = farr1[j] * 2.0f - farr1[i];
        }
    }
    
    /* Use results to prevent dead code elimination */
    use_result(acc + (int)f_acc);
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bound non-constant */
    int n = (argc > 1) ? atoi(argv[1]) : 500;
    if (n > SIZE) n = SIZE;
    if (n < 10) n = 10;
    
    /* Volatile variable to prevent constant propagation */
    volatile int threshold = 1000;
    
    /* Initialize arrays with pseudo-random data */
    int *arr1 = (int*)malloc(SIZE * sizeof(int));
    int *arr2 = (int*)malloc(SIZE * sizeof(int));
    float *farr1 = (float*)malloc(SIZE * sizeof(float));
    float *farr2 = (float*)malloc(SIZE * sizeof(float));
    
    srand(time(NULL));
    for (int i = 0; i < SIZE; ++i) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / RAND_MAX;
        farr2[i] = (float)rand() / RAND_MAX;
    }
    
    /* Call computational kernel multiple times */
    for (int iter = 0; iter < 2; ++iter) {
        compute_kernel(arr1, arr2, farr1, farr2, n, threshold + iter * 500);
        
        /* Shuffle data slightly between iterations */
        for (int i = 0; i < n; i += 10) {
            arr1[i] ^= arr2[i];
            farr1[i] = farr2[i] * 0.9f;
        }
    }
    
    /* Calculate checksum to ensure computation isn't eliminated */
    long long checksum = 0;
    for (int i = 0; i < n; ++i) {
        checksum += arr1[i] + arr2[i] + (int)(farr1[i] * 100) + (int)(farr2[i] * 100);
    }
    printf("Checksum: %lld\n", checksum);
    
    free(arr1);
    free(arr2);
    free(farr1);
    free(farr2);
    
    return 0;
}

/* External function definition to prevent inlining */
int use_result(int value) {
    return value % 256;
}
