#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1000

/* External function to prevent optimization */
extern int use_result(int);

/* Complex loop with mixed operations and dependencies */
void process_data(int n, int* arr1, int* arr2, float* farr1, float* farr2) {
    int acc = 0;
    float f_acc = 0.0f;
    
    /* Loop with multiple dependencies and conditions */
    for (int i = 1; i < n; ++i) {
        /* Loop-carried dependency on acc */
        acc += arr1[i] * arr2[i-1];
        
        /* Mixed integer/float operations */
        float temp = (float)acc * 0.01f;
        
        /* Conditional with unpredictable branch */
        if (farr1[i] > 0.5f) {
            farr2[i] = sqrtf(farr1[i] + temp);
        } else {
            farr2[i] = farr1[i] * farr1[i-1];
        }
        
        /* More mixed operations */
        arr1[i] = (arr1[i] & 0xFF) + (int)(farr2[i] * 100.0f);
        
        /* Another loop-carried dependency */
        f_acc += farr2[i] - farr2[i-1];
        
        /* Bitwise operation with floating point conversion */
        arr2[i] = (arr2[i] ^ (arr1[i] << 2)) + (int)f_acc;
    }
    
    /* Use results to prevent dead code elimination */
    use_result(acc + (int)f_acc);
}

/* Another complex loop with different pattern */
void process_data2(int n, int* arr1, int* arr2, float* farr1, float* farr2) {
    int sum = 0;
    
    for (int i = 2; i < n; ++i) {
        /* Complex indexing with multiple dependencies */
        int idx1 = i % 100;
        int idx2 = (i * 3) % 100;
        
        /* Multiple memory accesses with dependencies */
        int val1 = arr1[idx1] + arr2[idx2];
        int val2 = arr1[i-1] - arr2[i-2];
        
        /* Floating point with condition */
        float f_val = farr1[i] * 2.0f;
        if (val1 > val2) {
            f_val = f_val / 1.5f;
        }
        
        /* Store with conversion */
        farr2[i] = f_val + (float)(val1 & 0xF);
        
        /* Update array with mixed operations */
        arr1[i] = (arr1[i] * 3 + val2) | 0x7F;
        
        /* Running sum with conditional */
        sum += (arr1[i] > 1000) ? arr1[i] : -arr1[i];
    }
    
    use_result(sum);
}

int main(int argc, char** argv) {
    /* Use argc to make loop bound non-constant */
    volatile int n = (argc > 1) ? atoi(argv[1]) : 500;
    if (n > SIZE) n = SIZE;
    if (n < 10) n = 10;
    
    /* Allocate and initialize arrays */
    int* arr1 = (int*)malloc(SIZE * sizeof(int));
    int* arr2 = (int*)malloc(SIZE * sizeof(int));
    float* farr1 = (float*)malloc(SIZE * sizeof(float));
    float* farr2 = (float*)malloc(SIZE * sizeof(float));
    
    /* Seed RNG */
    srand(time(NULL));
    
    /* Initialize with random data */
    for (int i = 0; i < SIZE; ++i) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / (float)RAND_MAX;
        farr2[i] = 0.0f;
    }
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; ++outer) {
        /* Call both processing functions */
        process_data(n, arr1, arr2, farr1, farr2);
        process_data2(n, arr1, arr2, farr1, farr2);
        
        /* Shuffle data slightly between iterations */
        for (int i = 1; i < n; i += 2) {
            arr1[i] ^= 0xAA;
            farr1[i] += 0.1f;
        }
    }
    
    /* Final checksum to ensure computation isn't eliminated */
    int checksum = 0;
    for (int i = 0; i < n; ++i) {
        checksum += arr1[i] + (int)farr2[i];
    }
    
    printf("Result checksum: %d\n", checksum);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr1);
    free(farr2);
    
    return 0;
}

/* Implementation of external function */
int use_result(int val) {
    static int total = 0;
    total += val;
    return total;
}
