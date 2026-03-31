/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */
#include <stdio.h>
#include <stdlib.h>

/* Non-inlined function containing complex dependency patterns */
__attribute__((noinline, noclone))
double compute_loop(double* arr, int* indices, int size, int* counter) {
    double sum = 0.0;
    double temp1, temp2;
    int i;
    
    /* Complex loop with multiple dependency types */
    for (i = 1; i < size; i++) {
        /* 1. TRUE DEPENDENCY (RAW) - carried across iterations */
        arr[i] = arr[i-1] * 1.5 + (double)i;  /* FP operation with latency */
        
        /* 2. ANTI-DEPENDENCY (WAR) - read then write same location */
        temp1 = arr[i];                      /* Read arr[i] */
        arr[i] = temp1 + arr[i] * 0.1;       /* Write arr[i] - WAR with previous read */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - multiple writes to same location */
        arr[i] = arr[i] * 2.0;               /* Second write to arr[i] - WAW */
        
        /* 4. MEMORY DEPENDENCY with pointer aliasing */
        indices[i] = indices[i-1] + i;       /* Integer dependency */
        
        /* 5. Mixed data types creating different latency edges */
        temp2 = (double)indices[i] * 0.25;   /* Integer to FP conversion */
        
        /* 6. Complex expression with multiple operations */
        sum += arr[i] + temp2;               /* Accumulator with FP add latency */
        
        /* 7. Control-like dependency through conditional */
        if (indices[i] % 8 == 0) {
            (*counter)++;                    /* Pointer-based update */
        }
        
        /* 8. Additional memory dependency through pointer arithmetic */
        *(arr + (i % 16)) = *(arr + ((i-1) % 16)) * 0.99;
    }
    
    return sum;
}

/* Another function with different patterns */
__attribute__((noinline, noclone))
int nested_loop_deps(float* farr, double* darr, int n) {
    int i, j;
    float ftemp;
    double dsum = 0.0;
    
    /* Nested loops with carried dependencies */
    for (i = 1; i < n; i++) {
        /* Outer loop carried dependency */
        darr[i] = darr[i-1] * 0.9 + (double)i;
        
        for (j = 1; j < 8; j++) {
            /* Inner loop with multiple dependency types */
            ftemp = farr[j];                 /* Read */
            farr[j] = ftemp * 1.1f + (float)(i*j); /* Write - WAR */
            farr[j-1] = farr[j] * 0.5f;      /* Different index - RAW */
            
            /* Cross-type dependency */
            dsum += (double)farr[j] * darr[i];
        }
        
        /* Output dependency in outer loop */
        darr[i] = dsum * 0.01;               /* WAW with line 48 */
    }
    
    return (int)dsum;
}

int main(void) {
    const int SIZE = 512;
    double arr[SIZE];
    float farr[16];
    int indices[SIZE];
    int counter = 0;
    volatile double result;  /* volatile to prevent optimization */
    volatile int int_result;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = (double)(i % 10);
        indices[i] = i;
    }
    
    for (int i = 0; i < 16; i++) {
        farr[i] = (float)i * 0.5f;
    }
    
    /* Call function with complex dependencies */
    result = compute_loop(arr, indices, SIZE, &counter);
    
    /* Call second function with different patterns */
    int_result = nested_loop_deps(farr, arr, 64);
    
    /* Use results to prevent dead code elimination */
    printf("Result 1: %f\n", result);
    printf("Result 2: %d\n", int_result);
    printf("Counter: %d\n", counter);
    
    return 0;
}
