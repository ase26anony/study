/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */
#include <stdio.h>
#include <stdlib.h>

/* Non-inlined function containing complex dependency patterns */
__attribute__((noinline, noclone))
int compute_loop(int* arr, double* darr, float* farr, int size) {
    int i;
    int temp_int = 0;
    double temp_double = 0.0;
    float temp_float = 0.0f;
    int* ptr = arr;
    volatile int vol_var = 0;  /* Prevent some optimizations */
    
    /* Complex loop with multiple dependency types */
    for (i = 1; i < size; i++) {
        /* 1. TRUE DEPENDENCY (RAW) - Integer chain */
        int prev = arr[i-1];          /* Read */
        arr[i] = prev + i + vol_var;  /* Write depends on read */
        
        /* 2. ANTI-DEPENDENCY (WAR) - Reusing same array location */
        temp_int = arr[i];            /* Read */
        arr[i] = temp_int * 2;        /* Write after read to same location */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - Multiple writes */
        arr[i] = arr[i] + 1;          /* First write */
        arr[i] = arr[i] * 3;          /* Second write to same location */
        
        /* 4. Memory aliasing with pointers - creates ambiguous dependencies */
        *ptr = *ptr + 1;              /* ptr points to arr, aliasing possible */
        ptr = &arr[i];                /* Change pointer */
        
        /* 5. Floating-point dependencies with different latencies */
        double dprev = darr[i-1];     /* FP load */
        darr[i] = dprev * 1.5;        /* FP multiply (higher latency) */
        
        /* 6. Mixed-type dependencies */
        temp_float = (float)darr[i];  /* FP conversion */
        farr[i] = temp_float + 0.5f;  /* FP addition */
        
        /* 7. Cross-type dependencies (int -> float -> double) */
        temp_double = (double)arr[i]; /* Int to double conversion */
        darr[i] = darr[i] + temp_double * 0.25;
        
        /* 8. Loop-carried dependency with distance > 1 */
        if (i >= 3) {
            arr[i] = arr[i] + arr[i-3];  /* Distance 3 dependency */
        }
        
        /* 9. Control dependency */
        if (arr[i] > 1000) {
            vol_var = 1;                /* Affects next iteration */
        }
        
        /* 10. Complex expression with multiple operations */
        arr[i] = ((arr[i] << 2) | (arr[i-1] & 0xF)) + (int)(darr[i] * farr[i]);
    }
    
    /* Final computation to prevent dead code elimination */
    int result = 0;
    for (i = 0; i < size; i++) {
        result += arr[i] + (int)darr[i] + (int)farr[i];
    }
    
    return result;
}

/* Another function with nested loops for additional DDG complexity */
__attribute__((noinline, noclone))
int nested_loop_compute(int* matrix, int rows, int cols) {
    int i, j;
    int sum = 0;
    
    /* Nested loops with carried dependencies in both dimensions */
    for (i = 1; i < rows; i++) {
        for (j = 1; j < cols; j++) {
            /* 2D carried dependencies */
            int top = matrix[(i-1)*cols + j];      /* RAW - vertical */
            int left = matrix[i*cols + (j-1)];     /* RAW - horizontal */
            
            /* Complex computation with multiple dependencies */
            matrix[i*cols + j] = (top * 2 + left * 3) / 5;
            
            /* Anti-dependency in inner loop */
            int temp = matrix[i*cols + j];         /* Read */
            matrix[i*cols + j] = temp + i + j;     /* Write after read */
            
            /* Output dependency */
            matrix[i*cols + j] = matrix[i*cols + j] << 1;
            matrix[i*cols + j] = matrix[i*cols + j] >> 1;
        }
    }
    
    /* Compute checksum */
    for (i = 0; i < rows; i++) {
        for (j = 0; j < cols; j++) {
            sum += matrix[i*cols + j];
        }
    }
    
    return sum;
}

int main() {
    const int SIZE = 256;
    const int ROWS = 32;
    const int COLS = 32;
    
    /* Allocate and initialize arrays with different data types */
    int* int_array = (int*)malloc(SIZE * sizeof(int));
    double* double_array = (double*)malloc(SIZE * sizeof(double));
    float* float_array = (float*)malloc(SIZE * sizeof(float));
    int* matrix = (int*)malloc(ROWS * COLS * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i + 1;
        double_array[i] = (double)i * 0.5;
        float_array[i] = (float)i * 0.25f;
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        matrix[i] = i % 100;
    }
    
    /* Call functions with complex dependency patterns */
    int result1 = compute_loop(int_array, double_array, float_array, SIZE);
    int result2 = nested_loop_compute(matrix, ROWS, COLS);
    
    /* Use volatile to ensure computation isn't optimized away */
    volatile int final_result = result1 + result2;
    
    /* Print result to prevent dead code elimination */
    printf("Computed results: %d, %d (Final: %d)\n", 
           result1, result2, final_result);
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(float_array);
    free(matrix);
    
    return 0;
}
