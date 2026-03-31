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
            /* Multiple memory accesses with non-linear indexing */
            int idx1 = i;
            int idx2 = i - 1;                    /* Creates dependency on previous iteration */
            int idx3 = (i * 2) % n;              /* Non-linear access pattern */
            
            /* Integer operations with data dependencies */
            int temp = arr1[idx1] * arr2[idx2];  /* Cross-iteration dependency */
            acc += temp;                         /* Loop-carried accumulation */
            
            /* Floating-point operations */
            float f_temp = farr1[idx1] + farr2[idx2];
            f_acc = f_temp * 0.99f;              /* Floating-point dependency chain */
            
            /* Conditional control flow - data dependent */
            if (farr1[idx1] > 0.5f) {            /* Unpredictable branch */
                /* Mixed integer/float operations */
                farr2[idx1] = sqrtf(fabsf(f_temp));
                arr1[idx3] = (arr1[idx3] & 0xFF) + (int)(farr2[idx1] * 100.0f);
            } else {
                /* Alternative path with different operations */
                farr2[idx1] = f_temp * f_temp;
                arr1[idx3] = (arr1[idx3] | 0x1F) - (int)farr2[idx1];
            }
            
            /* More complex dependency web */
            arr2[i] = (acc & 0xFFF) + arr1[idx3] - temp;
            
            /* Bitwise operations mixed with arithmetic */
            int mask = (i << 3) | 0xF;
            arr1[i] = (arr1[i] ^ mask) + (arr2[idx2] >> 2);
            
            /* Additional floating-point operation */
            if (i % 4 == 0) {
                farr1[i] = sinf(f_acc) * cosf(farr2[idx2]);
            }
        }
        
        /* Small inner loop with different pattern */
        for (int j = n - 1; j > 0; --j) {
            /* Reverse traversal creates different access pattern */
            arr1[j] = arr1[j] + arr2[j] - arr1[j-1];
            
            /* Another conditional */
            if (arr1[j] > threshold) {
                farr2[j] = farr1[j] * 2.0f;
            }
        }
    }
    
    /* Use results to prevent dead code elimination */
    use_result(acc + (int)f_acc);
}

/* Helper function with side effects */
int use_result(int val) {
    static int total = 0;
    total += val;
    return total;
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int n = SIZE;
    if (argc > 1) {
        n = atoi(argv[1]);
        if (n <= 0 || n > SIZE) n = SIZE;
    }
    
    /* Volatile variable to prevent compile-time optimization */
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
    
    /* Call the computational kernel multiple times */
    for (int iter = 0; iter < 2; ++iter) {
        compute_kernel(arr1, arr2, farr1, farr2, n, threshold + iter);
        
        /* Modify threshold to change branch behavior */
        threshold = (threshold * 13 + 7) % 500;
    }
    
    /* Calculate checksum to ensure computation isn't eliminated */
    int checksum = 0;
    float f_checksum = 0.0f;
    for (int i = 0; i < n; ++i) {
        checksum += arr1[i] + arr2[i];
        f_checksum += farr1[i] + farr2[i];
    }
    
    printf("Checksum: int=%d, float=%.2f\n", checksum, f_checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr1);
    free(farr2);
    
    return 0;
}
