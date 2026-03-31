#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1000

/* External function to prevent optimization */
extern int use_result(int);

/* Complex loop with mixed operations and dependencies */
void compute_kernel(int *arr1, int *arr2, float *farr1, float *farr2, 
                    int n, float threshold) {
    int acc = 0;  /* Loop-carried dependency */
    int temp;
    float ftemp;
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; outer++) {
        /* Main computational loop with complex dependencies */
        for (int i = 1; i < n; i++) {
            /* Multiple memory accesses with non-trivial indexing */
            int idx1 = i;
            int idx2 = i - 1;  /* Creates dependency on previous iteration */
            int idx3 = (2 * i) % n;
            
            /* Integer operations with loop-carried dependency */
            acc += arr1[idx1] * arr2[idx2];  /* Cross-iteration dependency */
            
            /* Mixed integer/float operations */
            ftemp = (float)arr1[idx3] * 0.5f;
            
            /* Conditional control flow inside loop */
            if (farr1[i] > threshold) {
                /* Floating-point operation */
                farr2[i] = sqrtf(fabsf(farr1[i]));  /* sqrt requires FPU */
                
                /* Bitwise and arithmetic mix */
                temp = (arr1[i] & 0xFF) | (arr2[i] << 8);
                arr1[i] = temp + (int)(farr2[i] * 100.0f);
            } else {
                /* Alternative computation path */
                farr2[i] = farr1[i] * farr1[i-1];  /* Another cross-iteration dependency */
                arr1[i] = (arr1[i] ^ arr2[i]) + acc;  /* Uses accumulated value */
            }
            
            /* More mixed operations */
            float fval = (float)(arr1[i] % 256) / 255.0f;
            farr1[i] = fval * threshold + (float)(i % 10);
            
            /* Additional dependency chain */
            arr2[i] = arr2[idx2] + (int)(sinf(fval) * 100.0f);
        }
        
        /* Small variation in threshold for outer loop */
        threshold *= 0.9f;
    }
    
    /* Use result to prevent dead code elimination */
    use_result(acc);
}

int main(int argc, char **argv) {
    /* Use argc to make loop bound non-constant */
    int n = (argc > 1) ? atoi(argv[1]) : 500;
    if (n > SIZE) n = SIZE;
    if (n < 10) n = 10;
    
    /* Volatile to prevent compile-time optimization */
    volatile int volatile_n = n;
    
    /* Initialize arrays with pseudo-random data */
    int *arr1 = malloc(SIZE * sizeof(int));
    int *arr2 = malloc(SIZE * sizeof(int));
    float *farr1 = malloc(SIZE * sizeof(float));
    float *farr2 = malloc(SIZE * sizeof(float));
    
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / RAND_MAX;
        farr2[i] = 0.0f;
    }
    
    /* Non-constant threshold using math function */
    float threshold = sinf((float)n) * 0.5f + 0.5f;
    
    /* Perform computation */
    compute_kernel(arr1, arr2, farr1, farr2, volatile_n, threshold);
    
    /* Calculate checksum to ensure computation isn't eliminated */
    int checksum = 0;
    float fchecksum = 0.0f;
    for (int i = 0; i < volatile_n; i++) {
        checksum += arr1[i] + arr2[i];
        fchecksum += farr1[i] + farr2[i];
    }
    
    printf("Checksum: int=%d, float=%.2f\n", checksum, fchecksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr1);
    free(farr2);
    
    return 0;
}

/* External function definition */
int use_result(int val) {
    return val % 256;
}
