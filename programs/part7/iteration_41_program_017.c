#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1000

/* External function to prevent optimization */
extern int use_result(int);

/* Complex loop with mixed operations and dependencies */
void process_loop(int n, int* arr1, int* arr2, float* farr1, float* farr2) {
    volatile int limit = n;  /* Prevent constant propagation */
    int acc = 0;
    float f_acc = 0.0f;
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; outer++) {
        /* Main computational loop with loop-carried dependencies */
        for (int i = 1; i < limit; i++) {
            /* Loop-carried integer dependency */
            acc += arr1[i] * arr2[i-1];
            
            /* Mixed integer/floating-point operations */
            float temp = (float)acc * 0.01f;
            f_acc = f_acc * 0.95f + temp;
            
            /* Conditional control flow with data-dependent condition */
            if (farr1[i] > 0.5f) {
                /* Complex floating-point operation */
                farr2[i] = sqrtf(fabsf(farr1[i] * f_acc));
                
                /* Integer operation with bitwise manipulation */
                arr1[i] = (arr1[i] & 0xFF) | ((int)farr2[i] << 8);
            } else {
                /* Alternative computation path */
                farr2[i] = sinf(farr1[i] * 3.14159f);
                arr1[i] = (arr1[i] >> 4) ^ (int)(farr2[i] * 100.0f);
            }
            
            /* Additional memory access pattern */
            arr2[i] = arr1[i] + arr2[i-1] + (int)(farr1[i] * 10.0f);
            
            /* Complex indexing with non-linear pattern */
            int idx = (i * 7) % SIZE;
            if (idx > 0) {
                farr1[idx] = farr1[idx] * 0.9f + farr2[i] * 0.1f;
            }
        }
        
        /* Small variation in limit to prevent complete optimization */
        limit = n - (outer % 2);
    }
    
    /* Use results to prevent dead code elimination */
    use_result(acc + (int)f_acc);
}

/* Another complex function with different patterns */
void process_loop2(int n, int* arr1, int* arr2, float* farr1, float* farr2) {
    volatile int start = rand() % 10;
    int sum = 0;
    
    for (int i = start; i < n - 1; i += 2) {
        /* Multiple interleaved dependencies */
        int t1 = arr1[i] ^ arr2[i+1];
        int t2 = arr1[i+1] & arr2[i];
        
        /* Floating-point with conditional */
        float f1 = farr1[i] * 2.0f;
        float f2 = farr2[i+1] / 3.0f;
        
        if ((t1 + t2) % 7 == 0) {
            farr1[i] = f1 + f2;
            arr1[i] = t1 | t2;
        } else {
            farr1[i] = f1 - f2;
            arr1[i] = t1 & t2;
        }
        
        /* Cross-iteration floating dependency */
        farr2[i] = farr1[i-1] * 0.8f + farr2[i] * 0.2f;
        
        sum += arr1[i] + (int)farr1[i];
    }
    
    use_result(sum);
}

int main(int argc, char** argv) {
    /* Use command line argument for variable loop bound */
    int n = (argc > 1) ? atoi(argv[1]) : 500;
    if (n > SIZE) n = SIZE;
    if (n < 10) n = 10;
    
    /* Initialize with random data */
    srand(time(NULL));
    
    int* arr1 = (int*)malloc(SIZE * sizeof(int));
    int* arr2 = (int*)malloc(SIZE * sizeof(int));
    float* farr1 = (float*)malloc(SIZE * sizeof(float));
    float* farr2 = (float*)malloc(SIZE * sizeof(float));
    
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / RAND_MAX;
        farr2[i] = (float)rand() / RAND_MAX;
    }
    
    /* Call processing functions multiple times */
    for (int iter = 0; iter < 2; iter++) {
        process_loop(n, arr1, arr2, farr1, farr2);
        process_loop2(n, arr1, arr2, farr1, farr2);
    }
    
    /* Final checksum to ensure computation isn't eliminated */
    int checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += arr1[i] + arr2[i] + (int)(farr1[i] * 100) + (int)(farr2[i] * 100);
    }
    
    printf("Result checksum: %d\n", checksum);
    
    free(arr1);
    free(arr2);
    free(farr1);
    free(farr2);
    
    return 0;
}

/* Prevent the compiler from optimizing away use_result */
int use_result(int value) {
    volatile int sink = value;
    return sink;
}
