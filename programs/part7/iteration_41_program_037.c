#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1000

/* External function to prevent optimization */
extern int use_result(int);

/* Function with complex loop to engage selective scheduling */
void process_data(int n, float threshold, int *arr1, int *arr2, 
                  float *farr1, float *farr2) {
    int acc = 0;
    volatile int limit = n; /* Prevent constant propagation */
    
    /* Outer loop to give scheduler repeated region */
    for (int iter = 0; iter < 3; iter++) {
        acc = 0;
        
        /* Main complex loop with mixed operations and dependencies */
        for (int i = 1; i < limit; i++) {
            /* Loop-carried dependency */
            acc += arr1[i] * arr2[i-1];
            
            /* Mixed integer/floating point operations */
            float temp = (float)acc / (i + 1);
            farr1[i] = temp * 0.75f;
            
            /* Conditional with data-dependent branch */
            if (farr1[i] > threshold) {
                /* Complex floating point operation */
                farr2[i] = sqrtf(farr1[i]) + sinf((float)i * 0.01f);
            } else {
                /* Alternative computation path */
                farr2[i] = farr1[i] * farr1[i-1];
            }
            
            /* Bitwise operations combined with floating point */
            arr1[i] = (arr1[i] & 0xFF) | ((int)(farr2[i] * 100.0f) << 8);
            
            /* Additional memory access pattern */
            arr2[i] = arr1[i] + arr2[i-1] + (int)(farr2[i] * 10.0f);
            
            /* Cross-iteration dependency with stride */
            if (i > 2) {
                farr1[i] += farr2[i-2] * 0.5f;
            }
        }
    }
    
    /* Use result to prevent dead code elimination */
    use_result(acc);
}

/* Another function with different pattern */
void process_data2(int n, int *arr1, float *farr1) {
    volatile int start = rand() % 10;
    float acc_f = 0.0f;
    
    for (int i = start; i < n; i += 2) {
        /* Complex indexing */
        int idx = (i * 3) % n;
        
        /* Mixed computations */
        float val = farr1[idx] * 2.0f;
        
        /* Conditional with unpredictable branch */
        if ((arr1[i] & 0x3) == 0) {
            val = cosf(val) + 1.0f;
        } else {
            val = val * val - 1.0f;
        }
        
        /* Update with dependency */
        farr1[i] = val + acc_f;
        acc_f = val * 0.9f;
        
        /* Integer operation with floating result */
        arr1[i] = (int)(val * 1000.0f) ^ (arr1[i] << 1);
    }
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bounds non-constant */
    int n = (argc > 1) ? atoi(argv[1]) : 500;
    if (n > SIZE) n = SIZE;
    if (n < 10) n = 10;
    
    /* Initialize with random data */
    srand(time(NULL));
    
    int *arr1 = (int*)malloc(SIZE * sizeof(int));
    int *arr2 = (int*)malloc(SIZE * sizeof(int));
    float *farr1 = (float*)malloc(SIZE * sizeof(float));
    float *farr2 = (float*)malloc(SIZE * sizeof(float));
    
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / RAND_MAX;
        farr2[i] = (float)rand() / RAND_MAX;
    }
    
    /* Varying threshold to affect branch prediction */
    float threshold = 0.3f + 0.4f * ((float)rand() / RAND_MAX);
    
    /* Call processing functions multiple times */
    for (int repeat = 0; repeat < 2; repeat++) {
        process_data(n, threshold, arr1, arr2, farr1, farr2);
        process_data2(n, arr1, farr1);
        
        /* Modify data slightly between repetitions */
        for (int i = 0; i < n; i++) {
            arr1[i] += repeat;
            farr1[i] += repeat * 0.1f;
        }
    }
    
    /* Compute checksum to ensure computation isn't eliminated */
    int checksum = 0;
    float fchecksum = 0.0f;
    for (int i = 0; i < n; i++) {
        checksum += arr1[i] + arr2[i];
        fchecksum += farr1[i] + farr2[i];
    }
    
    printf("Checksums: int=%d float=%.2f\n", checksum, fchecksum);
    
    free(arr1);
    free(arr2);
    free(farr1);
    free(farr2);
    
    return 0;
}

/* External function definition */
int use_result(int val) {
    return val % 100;
}
