/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */
#include <stdio.h>
#include <stdlib.h>

/* Non-inlined function containing the complex dependency loop */
__attribute__((noinline, noclone))
int compute_loop(int* arr, double* darr, float* farr, int size) {
    int i;
    int temp_int;
    double temp_double;
    float temp_float;
    int* ptr;
    volatile int sink; /* Prevent dead code elimination */
    
    /* Initialize some values */
    int sum = 0;
    double product = 1.0;
    
    /* Complex loop with multiple dependency types */
    for (i = 1; i < size; i++) {
        /* 1. TRUE DEPENDENCY (RAW) - Integer chain */
        arr[i] = arr[i-1] + i * 2;           /* Read-after-write dependency */
        
        /* 2. ANTI-DEPENDENCY (WAR) - Reusing same location */
        temp_int = arr[i];                   /* Read arr[i] */
        arr[i] = temp_int * 3 - i;           /* Write arr[i] - anti-dependency */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - Multiple writes */
        arr[i] = arr[i] / 2;                 /* First write */
        arr[i] = arr[i] + arr[i-1];          /* Second write - output dependency */
        
        /* 4. Floating-point true dependency with different latency */
        darr[i] = darr[i-1] * 1.5 + (double)i; /* FP multiplication has different latency */
        
        /* 5. Mixed-type dependencies */
        temp_double = darr[i];
        farr[i] = (float)temp_double * 0.5f;  /* Type conversion + FP operation */
        
        /* 6. Pointer-based memory dependency (ambiguous aliasing) */
        ptr = &arr[i % 16];                   /* Creates potential aliasing */
        *ptr = *ptr + 1;                      /* Memory dependency through pointer */
        
        /* 7. Loop-carried dependency with distance > 1 */
        if (i >= 3) {
            arr[i] = arr[i] + arr[i-3];       /* Distance-3 dependency */
        }
        
        /* 8. Control dependency-like pattern */
        temp_float = farr[i];
        if (temp_float > 100.0f) {
            temp_float = temp_float * 0.9f;   /* Conditional operation */
        }
        farr[i] = temp_float;
        
        /* Accumulate results to prevent elimination */
        sum += arr[i];
        product *= (darr[i] + 1.0);
    }
    
    /* Use volatile to ensure computation isn't optimized away */
    sink = sum;
    
    /* Return value that depends on all computations */
    return sum + (int)product;
}

/* Another function with nested loops for additional DDG complexity */
__attribute__((noinline, noclone))
int nested_loop_compute(int* matrix, int n) {
    int i, j;
    int total = 0;
    
    /* Nested loops with cross-iteration dependencies */
    for (i = 1; i < n; i++) {
        for (j = 1; j < n; j++) {
            /* 2D stencil computation with multiple dependencies */
            matrix[i*n + j] = 
                matrix[(i-1)*n + j] +      /* Vertical dependency */
                matrix[i*n + (j-1)] +      /* Horizontal dependency */
                matrix[(i-1)*n + (j-1)];   /* Diagonal dependency */
            
            /* Anti-dependency in 2D */
            int old_val = matrix[i*n + j];
            matrix[i*n + j] = old_val * 2 - (i + j);
            
            total += matrix[i*n + j];
        }
    }
    
    return total;
}

int main() {
    const int SIZE = 256;
    const int MATRIX_SIZE = 32;
    
    /* Allocate and initialize arrays with different data types */
    int* int_array = (int*)malloc(SIZE * sizeof(int));
    double* double_array = (double*)malloc(SIZE * sizeof(double));
    float* float_array = (float*)malloc(SIZE * sizeof(float));
    int* matrix = (int*)malloc(MATRIX_SIZE * MATRIX_SIZE * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i + 1;
        double_array[i] = (double)(i + 1) * 0.5;
        float_array[i] = (float)(i + 1) * 0.25f;
    }
    
    for (int i = 0; i < MATRIX_SIZE * MATRIX_SIZE; i++) {
        matrix[i] = i % 17;
    }
    
    /* Call the functions with complex dependency patterns */
    int result1 = compute_loop(int_array, double_array, float_array, SIZE);
    int result2 = nested_loop_compute(matrix, MATRIX_SIZE);
    
    /* Use results to prevent optimization */
    printf("Result 1: %d\n", result1);
    printf("Result 2: %d\n", result2);
    
    /* Final computation mixing all results */
    volatile int final_result = result1 + result2;
    printf("Final: %d\n", final_result);
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(float_array);
    free(matrix);
    
    return 0;
}
