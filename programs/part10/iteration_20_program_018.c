/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure loop body remains intact for DDG analysis */
__attribute__((noinline, noclone))
double compute_loop(double* arr, int* indices, int size, int* counter) {
    double sum = 0.0;
    double temp1, temp2;
    volatile double mem_barrier; /* Prevent reordering */
    
    /* Complex loop with multiple dependency types */
    for (int i = 1; i < size; i++) {
        /* 1. TRUE DEPENDENCY (RAW) - carried across iterations */
        arr[i] = arr[i-1] * 1.5 + (double)i;
        
        /* 2. ANTI-DEPENDENCY (WAR) - read then write same location */
        temp1 = arr[i];           /* Read arr[i] */
        arr[i] = temp1 + arr[i-1]; /* Write arr[i] - anti-dep with previous read */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - multiple writes to same location */
        arr[i] = arr[i] * 0.9;    /* Second write to arr[i] - output dep */
        
        /* 4. MEMORY DEPENDENCY with pointer aliasing */
        double* ptr = &arr[indices[i] % size];
        *ptr = *ptr + 2.0;        /* Potential memory dep via pointer */
        
        /* 5. MIXED DATA TYPES creating different latency edges */
        int int_val = (int)arr[i]; /* Float to int conversion */
        double* dbl_ptr = &arr[i % 8];
        
        /* 6. CONTROL DEPENDENCY via conditional */
        if (int_val % 3 == 0) {
            temp2 = *dbl_ptr * 3.14;
            arr[i] += temp2;
        } else {
            temp2 = *dbl_ptr / 3.14;
            arr[i] -= temp2;
        }
        
        /* 7. INDUCTION VARIABLE with carried dependency */
        *counter += i * 2;         /* Integer carried dependency */
        
        /* 8. FLOATING POINT with different latency */
        sum += arr[i] * 0.123456; /* FP multiply-add dependency chain */
        
        /* Memory barrier to prevent over-optimization */
        mem_barrier = arr[i];
    }
    
    return sum;
}

/* Another function with nested loops for more complex DDG */
__attribute__((noinline, noclone))
void nested_loop_deps(float* farr, int* iarr, int n) {
    volatile float vsum = 0.0f;
    
    /* Nested loops with cross-iteration dependencies */
    for (int i = 1; i < n; i++) {
        for (int j = 1; j < n; j++) {
            /* 2D stencil computation with multiple deps */
            float center = farr[i*n + j];
            float left = farr[i*n + (j-1)];
            float top = farr[(i-1)*n + j];
            
            /* True dependencies in both dimensions */
            farr[i*n + j] = (center + left + top) * 0.333f;
            
            /* Integer dependency chain in parallel */
            iarr[i*n + j] = iarr[(i-1)*n + j] + iarr[i*n + (j-1)];
            
            /* Mixed-type operation */
            vsum += farr[i*n + j] + (float)iarr[i*n + j];
        }
    }
}

int main() {
    const int SIZE = 256;
    double array[SIZE];
    int indices[SIZE];
    int counter = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        array[i] = (double)(i % 10);
        indices[i] = (i * 7) % SIZE; /* Create non-linear access pattern */
    }
    
    /* Call the compute function - this should trigger DDG construction */
    double result = compute_loop(array, indices, SIZE, &counter);
    
    /* Second test with nested loops */
    const int N = 64;
    float farr[N*N];
    int iarr[N*N];
    
    for (int i = 0; i < N*N; i++) {
        farr[i] = (float)(i % 5);
        iarr[i] = i % 7;
    }
    
    nested_loop_deps(farr, iarr, N);
    
    /* Use results to prevent dead code elimination */
    volatile double print_me = result + (double)counter + (double)farr[N*N/2];
    printf("Result: %f\n", print_me);
    
    return 0;
}
