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
    for (int repeat = 0; repeat < 3; ++repeat) {
        /* Main computational loop with complex dependencies */
        for (int i = 1; i < n; ++i) {
            /* Multiple memory accesses with non-linear indexing */
            int idx1 = i;
            int idx2 = 2 * i;
            int idx3 = i - 1;
            
            /* Loop-carried integer dependency */
            acc += arr1[idx1] * arr2[idx3];
            
            /* Mixed integer/float operations */
            float temp = (float)arr2[idx2 % n] * 0.5f;
            farr1[idx1] = temp + f_acc;
            
            /* Conditional control flow with data-dependent condition */
            if (farr1[idx1] > threshold) {
                /* Complex floating-point operation */
                farr2[idx1] = sqrtf(fabsf(farr1[idx1]));
                
                /* Bitwise operation combined with float conversion */
                arr1[idx1] = (arr1[idx1] & 0xFF) + (int)(farr2[idx1] * 100.0f);
            } else {
                /* Alternative path with different operations */
                farr2[idx1] = powf(farr1[idx1], 1.5f);
                arr1[idx1] = (arr1[idx1] | 0x7F) - (int)farr2[idx1];
            }
            
            /* Additional floating-point accumulation */
            f_acc = farr2[idx1] * 0.3f + f_acc * 0.7f;
            
            /* Cross-array dependency */
            arr2[i] = arr1[idx3] + (arr2[i] >> 2);
        }
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to prevent compile-time constant propagation */
    int n = 500;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0 || n > 1000) n = 500;
    }
    
    /* Volatile variable to prevent optimization */
    volatile int threshold = 50;
    
    /* Initialize arrays with random data */
    int *arr1 = (int*)malloc(n * sizeof(int));
    int *arr2 = (int*)malloc(n * sizeof(int));
    float *farr1 = (float*)malloc(n * sizeof(float));
    float *farr2 = (float*)malloc(n * sizeof(float));
    
    srand(time(NULL));
    for (int i = 0; i < n; ++i) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)(rand() % 1000) / 1000.0f;
        farr2[i] = 0.0f;
    }
    
    /* Perform computation - this should trigger selective scheduling */
    compute_kernel(arr1, arr2, farr1, farr2, n, threshold);
    
    /* Calculate checksum to prevent dead code elimination */
    long long checksum = 0;
    float f_checksum = 0.0f;
    for (int i = 0; i < n; ++i) {
        checksum += arr1[i] + arr2[i];
        f_checksum += farr1[i] + farr2[i];
    }
    
    printf("Checksum: %lld, Float checksum: %f\n", checksum, f_checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr1);
    free(farr2);
    
    return 0;
}
