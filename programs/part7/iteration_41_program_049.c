#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1000

/* External function to prevent optimization */
extern int use_result(int);

/* Complex loop with mixed operations and dependencies */
void process_arrays(int n, int *arr1, int *arr2, float *farr1, float *farr2) {
    int acc = 0;
    volatile int limit = n; /* Prevent constant propagation */
    
    /* Outer loop to give scheduler repeated region */
    for (int repeat = 0; repeat < 3; ++repeat) {
        /* Main computational loop with loop-carried dependency */
        for (int i = 1; i < limit; ++i) {
            /* Loop-carried dependency on acc */
            acc += arr1[i] * arr2[i-1];
            
            /* Mixed integer operations with bitwise */
            int temp = (arr1[i] & 0xFF) | (arr2[i] << 2);
            
            /* Floating-point computation */
            float fval = farr1[i] * 2.5f + sinf(farr2[i-1]);
            
            /* Conditional control flow with data-dependent condition */
            if (fval > 0.5f && (temp % 3) == 0) {
                farr2[i] = sqrtf(fabs(fval));
                arr1[i] = temp + (int)(farr2[i] * 100.0f);
            } else {
                farr2[i] = fval * 0.8f;
                arr1[i] = temp - (int)(farr2[i] * 50.0f);
            }
            
            /* Additional dependency chain */
            arr2[i] = (arr1[i-1] + arr2[i]) ^ 0xAA;
            
            /* More floating-point operations */
            farr1[i] = farr2[i] * farr1[i-1] + 1.0f;
            
            /* Complex array indexing */
            int idx = (i * 7) % n;
            if (idx > 0) {
                farr1[i] += farr2[idx] * 0.3f;
            }
        }
        
        /* Cross-iteration dependency */
        arr1[0] = acc % 100;
        farr1[0] = (float)(acc % 1000) / 100.0f;
    }
    
    /* Use result to prevent dead code elimination */
    use_result(acc);
}

/* Another complex function with different pattern */
void process_arrays2(int n, int *arr1, int *arr2, float *farr1, float *farr2) {
    float f_acc = 0.0f;
    volatile int start = 1;
    
    for (int i = start; i < n - 1; ++i) {
        /* Multiple memory accesses with different patterns */
        int a = arr1[i] + arr2[i+1];
        int b = arr1[i-1] - arr2[i];
        
        /* Floating-point with conversion */
        float fa = (float)a * 0.25f;
        float fb = (float)b * 0.75f;
        
        /* Data-dependent conditional */
        if ((a ^ b) & 0xF) {
            farr1[i] = fa * fb + cosf(farr2[i]);
            f_acc += farr1[i];
        } else {
            farr1[i] = fa / (fb + 0.1f) - sinf(farr2[i-1]);
            f_acc -= farr1[i];
        }
        
        /* Integer computation with shift */
        arr2[i] = (arr1[i] << 2) | (arr2[i] >> 1);
        
        /* Complex condition with floating comparison */
        if (farr1[i] > 1.0f || f_acc < -10.0f) {
            arr1[i] = (int)(farr1[i] * 10.0f) & 0x3FF;
        }
        
        /* Periodic operation */
        if (i % 5 == 0) {
            farr2[i] = tanhf(farr1[i] * 0.1f);
        }
    }
    
    use_result((int)f_acc);
}

int main(int argc, char *argv[]) {
    /* Use argc to make loop bound non-constant */
    int n = (argc > 1) ? atoi(argv[1]) : 500;
    if (n > SIZE) n = SIZE;
    if (n < 10) n = 10;
    
    /* Initialize with random data */
    srand(time(NULL));
    
    int *arr1 = malloc(SIZE * sizeof(int));
    int *arr2 = malloc(SIZE * sizeof(int));
    float *farr1 = malloc(SIZE * sizeof(float));
    float *farr2 = malloc(SIZE * sizeof(float));
    
    for (int i = 0; i < SIZE; ++i) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)(rand() % 1000) / 1000.0f;
        farr2[i] = (float)(rand() % 1000) / 1000.0f;
    }
    
    /* Call processing functions multiple times */
    for (int iter = 0; iter < 2; ++iter) {
        process_arrays(n, arr1, arr2, farr1, farr2);
        process_arrays2(n, arr1, arr2, farr1, farr2);
    }
    
    /* Compute checksum to ensure computation isn't eliminated */
    int checksum = 0;
    float fchecksum = 0.0f;
    for (int i = 0; i < n; ++i) {
        checksum += arr1[i] + arr2[i];
        fchecksum += farr1[i] + farr2[i];
    }
    
    printf("Checksum: int=%d, float=%.2f\n", checksum, fchecksum);
    
    free(arr1);
    free(arr2);
    free(farr1);
    free(farr2);
    
    return 0;
}

/* Prevent the compiler from optimizing away use_result */
int use_result(int value) {
    static volatile int sink;
    sink = value;
    return sink;
}
