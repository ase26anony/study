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
        double prev = arr[i-1];           /* Read arr[i-1] */
        arr[i] = prev * 1.5 + i;         /* Write arr[i] depends on arr[i-1] */
        
        /* 2. ANTI-DEPENDENCY (WAR) - reuse of same location */
        temp = arr[indices[i] % size];   /* Read arr[some_index] */
        arr[indices[i] % size] = temp * 0.5; /* Write to same location */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - multiple writes to same location */
        double* alias_ptr = &arr[i % 8]; /* Potential aliasing */
        *alias_ptr = i * 2.0;            /* Write 1 */
        *alias_ptr = *alias_ptr + 1.0;   /* Write 2 to same location */
        
        /* 4. Memory dependencies with pointer arithmetic */
        ptr = &arr[i];
        *ptr = *ptr * (*ptr + 1.0);      /* Read-modify-write with pointer */
        
        /* 5. Mixed data types in dependency chain */
        int int_val = (int)arr[i];       /* Convert double to int */
        double dbl_val = int_val * 3.14159; /* Back to double */
        arr[i % 4] = dbl_val;            /* Write with modulo */
        
        /* 6. Complex expression with varying operation latencies */
        sum += arr[i] * arr[i-1] / (i + 1.0); /* FP multiply, divide, add */
        
        /* 7. Control-like dependency through conditional */
        if (sum > 1000.0) {
            sum = sum * 0.99;            /* Creates data flow */
        }
    }
    
    /* Additional loop with different patterns */
    for (i = size - 1; i > 0; i--) {
        /* Reverse carried dependency */
        arr[i-1] = arr[i] * 0.8 + arr[i-1] * 0.2;
        
        /* Memory dependency with offset */
        double* p1 = &arr[i];
        double* p2 = &arr[(i + 3) % size];
        *p1 = *p1 + *p2;
    }
    
    return sum;
}

/* Another function with nested loops */
__attribute__((noinline, noclone))
void nested_loop_pattern(float* farr, int size) {
    int i, j;
    
    for (i = 1; i < size; i++) {
        /* Outer loop carried dependency */
        farr[i] = farr[i-1] * 1.1f;
        
        for (j = 0; j < 4; j++) {
            /* Inner loop with dependencies */
            float tmp = farr[j];
            farr[j] = tmp + (float)(i * j);
            
            /* Cross-iteration dependency in inner loop */
            if (j > 0) {
                farr[j] = farr[j] + farr[j-1] * 0.5f;
            }
        }
    }
}

int main() {
    const int SIZE = 256;
    double* arr = (double*)malloc(SIZE * sizeof(double));
    int* indices = (int*)malloc(SIZE * sizeof(int));
    float* farr = (float*)malloc(SIZE * sizeof(float));
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = (double)(i + 1) * 0.5;
        indices[i] = (i * 13 + 7) % SIZE;  /* Pseudo-random pattern */
        farr[i] = (float)i * 0.25f;
    }
    
    /* Force compiler to consider all dependencies */
    volatile double result = 0.0;
    
    /* Call functions with complex dependency patterns */
    result = compute_loop(arr, indices, SIZE);
    nested_loop_pattern(farr, SIZE);
    
    /* Use results to prevent dead code elimination */
    printf("Result: %f\n", result);
    printf("Sample values: arr[10]=%f, farr[20]=%f\n", arr[10], farr[20]);
    
    /* Additional computation to increase scheduling complexity */
    double checksum = 0.0;
    for (int i = 0; i < SIZE; i++) {
        checksum += arr[i] + farr[i];
    }
    printf("Checksum: %f\n", checksum);
    
    free(arr);
    free(indices);
    free(farr);
    
    return 0;
}
