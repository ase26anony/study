#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define SIZE 1000

/* External function to prevent optimization */
extern int use_result(int);

/* Complex loop with mixed operations and dependencies */
void process_arrays(int n, int* arr1, int* arr2, float* farr1, float* farr2) {
    int acc = 0;
    float f_acc = 0.0f;
    
    /* Outer loop to give scheduler repeated region */
    for (int outer = 0; outer < 3; outer++) {
        /* Main computational loop with loop-carried dependencies */
        for (int i = 1; i < n; i++) {
            /* Loop-carried integer dependency */
            acc += arr1[i] * arr2[i-1];
            
            /* Mixed integer operations with bitwise */
            int temp = (arr1[i] & 0xFF) | (arr2[i] << 2);
            
            /* Floating-point computation with dependency */
            f_acc = farr1[i] * 1.5f + farr2[i-1];
            
            /* Conditional control flow - data dependent */
            if (farr1[i] > 0.5f) {
                /* Complex floating-point operation */
                farr2[i] = sqrtf(fabsf(farr1[i])) + f_acc;
                
                /* Integer operation inside conditional */
                arr1[i] = temp + (int)(farr2[i] * 100.0f);
            } else {
                /* Alternative path with different operations */
                farr2[i] = farr1[i] * farr1[i-1];
                arr1[i] = temp - (int)(farr2[i] * 50.0f);
            }
            
            /* Additional floating-point operation */
            farr1[i] = sinf(farr2[i] * 0.01f) + cosf(f_acc * 0.02f);
            
            /* Another loop-carried dependency */
            arr2[i] = arr2[i-1] + (int)(farr1[i] * 10.0f) + acc % 256;
        }
        
        /* Cross-iteration dependency reset with variation */
        acc = (acc + outer) & 0xFFFF;
        f_acc = f_acc * 0.9f;
    }
    
    /* Use result to prevent dead code elimination */
    use_result(acc + (int)f_acc);
}

int main(int argc, char** argv) {
    /* Use argc to make loop bound non-constant */
    volatile int n = (argc > 1) ? atoi(argv[1]) : 500;
    if (n > SIZE) n = SIZE;
    if (n < 10) n = 10;
    
    /* Initialize arrays with pseudo-random data */
    int arr1[SIZE], arr2[SIZE];
    float farr1[SIZE], farr2[SIZE];
    
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr1[i] = (float)rand() / RAND_MAX;
        farr2[i] = (float)rand() / RAND_MAX;
    }
    
    /* Process arrays multiple times with different sizes */
    for (int iter = 0; iter < 2; iter++) {
        volatile int current_n = n - iter * 50;
        if (current_n < 20) current_n = 20;
        
        process_arrays(current_n, arr1, arr2, farr1, farr2);
    }
    
    /* Calculate checksum to ensure computation isn't eliminated */
    int checksum = 0;
    float fchecksum = 0.0f;
    for (int i = 0; i < n; i++) {
        checksum += arr1[i] + arr2[i];
        fchecksum += farr1[i] + farr2[i];
    }
    
    printf("Checksum: int=%d, float=%.2f\n", checksum, fchecksum);
    return 0;
}

/* Dummy function definition to satisfy external reference */
int use_result(int val) {
    return val & 1;
}
