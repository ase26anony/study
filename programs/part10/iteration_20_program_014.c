/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */
#include <stdio.h>
#include <stdlib.h>

/* Non-inlined function containing complex dependency patterns */
__attribute__((noinline, noclone))
double compute_loop(double* arr, int* indices, int size) {
    double sum = 0.0;
    double temp = 0.0;
    double* ptr = arr;
    int i;
    
    /* Complex loop with multiple dependency types */
    for (i = 1; i < size; i++) {
        /* 1. TRUE DEPENDENCY (RAW) - carried across iterations */
        double prev = arr[i-1];           /* Read */
        arr[i] = prev * 1.5 + i;          /* Write depending on previous read */
        
        /* 2. ANTI-DEPENDENCY (WAR) - reuse of same location */
        temp = arr[indices[i]];           /* Read from array at computed index */
        arr[indices[i]] = temp * 0.75;    /* Write to same location */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - multiple writes to same location */
        arr[i % 16] = prev * 2.0;         /* First write */
        arr[i % 16] = arr[i % 16] + 1.0;  /* Second write to same location */
        
        /* 4. MEMORY DEPENDENCY - pointer aliasing creates ambiguous deps */
        *ptr = *ptr + arr[i];             /* ptr may alias arr */
        ptr = &arr[i % 8];                /* Change pointer target */
        
        /* 5. MIXED DATA TYPES - integer and floating point operations */
        int int_val = (int)arr[i];        /* Type conversion */
        double fp_val = int_val * 3.14159;
        arr[i % 32] = fp_val / (i + 1);
        
        /* 6. COMPLEX ADDRESS CALCULATION - creates memory deps */
        double* addr = &arr[(i * 13 + 7) % size];
        *addr = *addr * 0.99;
        
        /* Accumulate result to prevent elimination */
        sum += arr[i] + temp + fp_val;
    }
    
    return sum;
}

/* Another function with nested loops for additional DDG complexity */
__attribute__((noinline, noclone))
void nested_loop_deps(float* farr, int* iarr, int n) {
    int i, j;
    
    for (i = 1; i < n; i++) {
        /* Loop-carried dependency in inner loop */
        float acc = farr[i-1];
        for (j = 0; j < 8; j++) {
            /* Multiple dependency patterns in inner loop */
            iarr[j] = iarr[j] + i;        /* Integer RAW */
            farr[j] = farr[j] * acc;      /* Float RAW with carried value */
            acc = farr[j] * 0.5f;         /* Update accumulator */
            
            /* Memory dependency through different arrays */
            if (j > 0) {
                farr[j-1] = iarr[j] * 0.1f;  /* Cross-type dependency */
            }
        }
        
        /* Output dependency chain */
        farr[i % 4] = acc;
        farr[i % 4] = farr[i % 4] + 1.0f;
        farr[i % 4] = farr[i % 4] * 2.0f;
    }
}

int main() {
    const int SIZE = 1024;
    double* arr = (double*)malloc(SIZE * sizeof(double));
    int* indices = (int*)malloc(SIZE * sizeof(int));
    float* farr = (float*)malloc(SIZE * sizeof(float));
    int* iarr = (int*)malloc(SIZE * sizeof(int));
    
    /* Initialize arrays with pattern */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = (i % 100) * 0.1;
        indices[i] = (i * 7) % SIZE;
        farr[i] = (i % 50) * 0.2f;
        iarr[i] = i;
    }
    
    /* Volatile to prevent dead code elimination */
    volatile double result1 = 0.0;
    volatile float result2 = 0.0f;
    
    /* Execute loops with complex dependencies */
    result1 = compute_loop(arr, indices, SIZE);
    nested_loop_deps(farr, iarr, SIZE / 4);
    
    /* Compute verification results */
    double check_sum = 0.0;
    for (int i = 0; i < SIZE; i++) {
        check_sum += arr[i];
    }
    
    /* Use results to prevent optimization */
    printf("Result1: %f\n", result1);
    printf("Check sum: %f\n", check_sum);
    printf("Array[0]: %f, Array[%d]: %f\n", arr[0], SIZE-1, arr[SIZE-1]);
    
    free(arr);
    free(indices);
    free(farr);
    free(iarr);
    
    return 0;
}
