#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1000

/* External function to prevent optimization */
extern int external_func(int);

/* Complex loop with mixed operations and dependencies */
void compute_loop(int n, int* arr1, int* arr2, float* farr1, float* farr2) {
    int acc = 0;
    float f_acc = 0.0f;
    
    /* Loop-carried dependencies with mixed operations */
    for (int i = 1; i < n; ++i) {
        /* Integer operations with dependency chain */
        int temp = arr1[i] * arr2[i-1];
        acc += temp;
        
        /* Floating-point operations */
        float f_temp = farr1[i] * 2.5f;
        f_acc += f_temp;
        
        /* Conditional control flow inside loop */
        if (farr1[i] > 0.5f) {
            /* Complex floating-point operation */
            farr2[i] = sqrtf(farr1[i]) + f_acc;
            
            /* Mixed integer/float operation */
            arr1[i] = (arr1[i] & 0xFF) + (int)(farr2[i] * 100.0f);
        } else {
            /* Alternative path with different operations */
            farr2[i] = farr1[i] * farr1[i-1];
            arr1[i] = arr1[i] | (arr2[i] << 2);
        }
        
        /* Additional memory access pattern */
        int idx = (i * 7) % n;
        arr2[idx] = arr1[i] + arr2[(i-1) % n];
        
        /* More floating-point operations */
        farr1[i] = farr2[i] / (f_temp + 1.0f);
        
        /* Bitwise operation with dependency */
        arr1[i] ^= arr1[i-1];
    }
    
    /* Prevent dead code elimination */
    volatile int result = acc + (int)f_acc;
    (void)result;
}

int main(int argc, char** argv) {
    /* Use argc to make loop bound non-constant */
    volatile int n = (argc > 1) ? atoi(argv[1]) : 500;
    if (n > SIZE) n = SIZE;
    if (n < 10) n = 10;
    
    /* Initialize arrays with random data */
    int arr1[SIZE], arr2[SIZE];
    float farr1[SIZE], farr2[SIZE];
    
    srand(time(NULL));
    for (int i = 0; i < SIZE; ++i) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / RAND_MAX;
        farr2[i] = 0.0f;
    }
    
    /* Call external function to create uncertainty */
    int seed = external_func(argc);
    srand(seed);
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; ++outer) {
        /* Modify array contents slightly each iteration */
        for (int i = 0; i < n; ++i) {
            arr1[i] ^= rand() % 256;
            farr1[i] += (float)(rand() % 100) / 1000.0f;
        }
        
        /* Main computation loop */
        compute_loop(n, arr1, arr2, farr1, farr2);
    }
    
    /* Calculate checksum to prevent elimination */
    long long checksum = 0;
    for (int i = 0; i < n; ++i) {
        checksum += arr1[i] + arr2[i] + (long long)(farr1[i] * 1000) + (long long)(farr2[i] * 1000);
    }
    
    printf("Checksum: %lld\n", checksum);
    return 0;
}

/* Simple external function definition */
int external_func(int x) {
    return x * 1103515245 + 12345;
}
