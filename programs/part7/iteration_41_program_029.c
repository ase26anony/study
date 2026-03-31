#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1000

/* External function to prevent optimization */
extern void use_result(int, float);

/* Complex loop with mixed operations and dependencies */
void compute_kernel(int *arr1, int *arr2, float *farr1, float *farr2, 
                    volatile int n, int threshold) {
    int acc = 0;                    /* Loop-carried dependency */
    float f_acc = 0.0f;             /* Floating-point accumulator */
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; ++outer) {
        /* Main computational loop with complex dependencies */
        for (int i = 1; i < n; ++i) {
            /* Multiple memory accesses with non-linear indexing */
            int idx1 = i;
            int idx2 = i - 1;                     /* Creates dependency on previous iteration */
            int idx3 = (2 * i) % n;               /* Non-linear access pattern */
            
            /* Integer operations with loop-carried dependency */
            int temp = arr1[idx1] * arr2[idx2];   /* RAW dependency on arr2[idx2] */
            acc += temp;                          /* Loop-carried dependency */
            
            /* Floating-point operations */
            float f_temp = farr1[idx1] * 1.5f;
            f_acc = f_temp - farr2[idx2];         /* Another loop-carried dependency */
            
            /* Conditional control flow - data dependent */
            if (acc > threshold) {                /* Unpredictable at compile time */
                /* Complex floating-point operation */
                farr2[idx1] = sqrtf(fabs(f_temp)) + f_acc;
                
                /* Bitwise operations mixed with type conversion */
                arr1[idx3] = (arr1[idx3] & 0xFF) | ((int)farr2[idx1] & 0xFF00);
            } else {
                /* Alternative path with different operations */
                farr2[idx1] = f_temp * 0.5f;
                arr1[idx3] = arr1[idx3] ^ arr2[idx2];
            }
            
            /* More mixed operations */
            float f_cond = (farr1[idx1] > 0.0f) ? farr1[idx1] : -farr1[idx1];
            arr2[i] = (int)(f_cond * 100.0f) + (arr1[idx1] >> 2);
            
            /* Additional dependency chain */
            if (i % 4 == 0) {
                farr1[i] = sinf(farr2[idx2]) * cosf(farr1[idx1]);
            }
        }
        
        /* Small inner loop with different pattern */
        for (int j = n - 1; j > 0; --j) {
            int k = j * 3 % n;
            arr1[j] = arr1[j] + arr2[k] - acc;
            farr1[j] = farr1[j] * 0.99f + f_acc;
        }
    }
    
    /* Use results to prevent dead code elimination */
    use_result(acc, f_acc);
}

/* Another complex function with different pattern */
void compute_kernel2(int *arr1, float *farr1, volatile int m) {
    float local_acc = 0.0f;
    
    for (int i = 0; i < m; ++i) {
        /* Complex addressing with multiple arrays */
        int idx_a = i;
        int idx_b = (i * 7 + 3) % m;
        int idx_c = (i * 13 + 5) % m;
        
        /* Mixed integer/float operations */
        float f_val = farr1[idx_a] * 2.0f + (float)arr1[idx_b];
        
        /* Data-dependent branching */
        if ((arr1[idx_c] & 0x3) == 0) {
            f_val = powf(f_val, 1.5f);
            arr1[idx_a] = (int)f_val ^ arr1[idx_b];
        } else if ((arr1[idx_c] & 0x3) == 1) {
            f_val = logf(fabs(f_val) + 1.0f);
            arr1[idx_a] = (int)(f_val * 10.0f);
        } else {
            f_val = f_val / 3.14159f;
            arr1[idx_a] = arr1[idx_a] << 2;
        }
        
        /* Update loop-carried dependency */
        local_acc += f_val;
        
        /* More operations to increase scheduling complexity */
        farr1[idx_b] = sinf(local_acc) * cosf(f_val);
        arr1[idx_c] = arr1[idx_c] + (int)(local_acc * 100.0f);
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    volatile int n = (argc > 1) ? atoi(argv[1]) : 500;
    if (n > SIZE) n = SIZE;
    if (n < 10) n = 10;
    
    /* Initialize with random data */
    srand(time(NULL));
    
    int *arr1 = (int*)malloc(SIZE * sizeof(int));
    int *arr2 = (int*)malloc(SIZE * sizeof(int));
    float *farr1 = (float*)malloc(SIZE * sizeof(float));
    float *farr2 = (float*)malloc(SIZE * sizeof(float));
    
    for (int i = 0; i < SIZE; ++i) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / RAND_MAX * 2.0f - 1.0f;
        farr2[i] = (float)rand() / RAND_MAX * 2.0f - 1.0f;
    }
    
    /* Threshold from random to prevent compile-time prediction */
    int threshold = rand() % 1000;
    
    /* Call computational kernels */
    compute_kernel(arr1, arr2, farr1, farr2, n, threshold);
    
    volatile int m = n / 2 + 1;
    compute_kernel2(arr1, farr1, m);
    
    /* Calculate checksum to ensure computation isn't eliminated */
    long long checksum = 0;
    float fchecksum = 0.0f;
    for (int i = 0; i < n; ++i) {
        checksum += arr1[i] + arr2[i];
        fchecksum += farr1[i] + farr2[i];
    }
    
    printf("Checksum: %lld, Float checksum: %f\n", checksum, fchecksum);
    
    free(arr1);
    free(arr2);
    free(farr1);
    free(farr2);
    
    return 0;
}

/* Dummy function to prevent optimization */
void use_result(int val, float fval) {
    /* Use volatile to ensure calls aren't optimized away */
    volatile int dummy = val;
    volatile float fdummy = fval;
    (void)dummy;
    (void)fdummy;
}
