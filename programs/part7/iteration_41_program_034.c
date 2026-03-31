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
    volatile int limit = n;  /* Prevent compile-time optimization */
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; outer++) {
        /* Main computational loop with complex dependencies */
        for (int i = 1; i < limit; i++) {
            /* Loop-carried dependency */
            acc += arr1[i] * arr2[i-1];
            
            /* Mixed integer operations with bitwise */
            int temp = (arr1[i] & 0xFF) | (arr2[i] << 2);
            
            /* Floating-point computation */
            float ftemp = farr1[i] * 2.5f - farr2[i-1];
            
            /* Conditional control flow with data-dependent condition */
            if (ftemp > 0.0f && (temp % 7) > 3) {
                /* Complex floating-point operation */
                farr2[i] = sqrtf(fabsf(ftemp)) + sinf(farr1[i] * 0.01f);
                
                /* Integer operation dependent on float result */
                arr1[i] = temp + (int)(farr2[i] * 100.0f);
            } else {
                /* Alternative computation path */
                farr2[i] = ftemp * ftemp;
                arr1[i] = temp - (int)(farr2[i] * 50.0f);
            }
            
            /* Additional loop-carried dependency */
            arr2[i] = arr2[i-1] + (int)(farr1[i] * 10.0f);
            
            /* Memory access with non-linear indexing */
            if (i * 2 < limit) {
                farr1[i*2] = farr1[i] * 0.5f + farr2[i];
            }
        }
        
        /* Small variation in each outer iteration */
        limit = (limit > 10) ? limit - 1 : n;
    }
    
    /* Use result to prevent dead code elimination */
    use_result(acc);
}

/* Another complex function with different pattern */
void process_arrays2(int n, int *arr1, int *arr2, float *farr1, float *farr2) {
    float acc_f = 0.0f;
    volatile int start = 2;
    
    for (int i = start; i < n; i++) {
        /* Complex addressing pattern */
        int idx1 = (i * 3) % n;
        int idx2 = (i * 5) % n;
        
        /* Multiple dependencies */
        int val1 = arr1[idx1] + arr2[idx2];
        float val2 = farr1[idx1] * farr2[idx2];
        
        /* Conditional with floating comparison */
        if (val2 > 100.0f || val1 < 0) {
            arr1[i] = val1 >> 2;
            farr1[i] = val2 * 0.25f;
        } else {
            arr1[i] = val1 << 1;
            farr1[i] = val2 * 4.0f;
        }
        
        /* Cross-type dependency */
        arr2[i] = (int)(farr1[i] * 1000.0f) ^ arr1[i];
        
        /* Accumulator with type conversion */
        acc_f += (float)arr1[i] * 0.01f + farr2[i];
    }
    
    /* Prevent optimization */
    if (acc_f > 0.0f) {
        use_result((int)acc_f);
    }
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
    
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / RAND_MAX * 10.0f;
        farr2[i] = (float)rand() / RAND_MAX * 10.0f;
    }
    
    /* Call processing functions multiple times */
    for (int iter = 0; iter < 2; iter++) {
        process_arrays(n, arr1, arr2, farr1, farr2);
        process_arrays2(n, arr1, arr2, farr1, farr2);
        
        /* Modify data slightly between iterations */
        for (int i = 0; i < n; i += 10) {
            arr1[i] += iter;
            farr1[i] += (float)iter * 0.1f;
        }
    }
    
    /* Final checksum to ensure computation isn't eliminated */
    int checksum = 0;
    for (int i = 0; i < n; i++) {
        checksum += arr1[i] + arr2[i] + (int)farr1[i] + (int)farr2[i];
    }
    printf("Result checksum: %d\n", checksum);
    
    free(arr1);
    free(arr2);
    free(farr1);
    free(farr2);
    
    return 0;
}

/* Implementation of external function */
int use_result(int val) {
    return val & 0xFF;
}
