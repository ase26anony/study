/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */

#include <stdio.h>
#include <stdlib.h>

/* Non-inlined function containing the complex dependency loop */
__attribute__((noinline, noclone))
double compute_loop(double* arr, int* int_arr, float* flt_arr, int size) {
    volatile double sum = 0.0;  /* Prevent optimization of final result */
    double temp1, temp2;
    int i;
    
    /* Complex loop with multiple carried dependencies */
    for (i = 1; i < size; i++) {
        /* 1. TRUE DEPENDENCY (RAW) with floating point */
        temp1 = arr[i-1] * 1.5;          /* Read arr[i-1] */
        arr[i] = temp1 + (double)i;      /* Write arr[i] depends on temp1 */
        
        /* 2. ANTI-DEPENDENCY (WAR) with integer */
        temp2 = (double)int_arr[i];      /* Read int_arr[i] */
        int_arr[i] = (int)(temp2) + i;   /* Write int_arr[i] after read */
        
        /* 3. OUTPUT DEPENDENCY (WAW) with float */
        flt_arr[i] = (float)temp1 * 0.5f; /* First write to flt_arr[i] */
        flt_arr[i] = (float)temp2 * 0.3f; /* Second write to same location */
        
        /* 4. MEMORY DEPENDENCY with pointer aliasing potential */
        /* Create ambiguous memory references */
        double* ptr1 = &arr[i];
        double* ptr2 = &arr[i-1] + 1;  /* Could alias with ptr1 */
        *ptr1 = *ptr1 + *ptr2 * 0.1;
        
        /* 5. MIXED DATA TYPE OPERATIONS with different latencies */
        /* Integer operation (low latency) */
        int_arr[i-1] = int_arr[i-1] * 2 + 1;
        
        /* Floating point operation (higher latency) */
        arr[i] = arr[i] / 1.41421356;
        
        /* 6. ACCUMULATOR with true dependency chain */
        sum = sum + arr[i] + (double)int_arr[i] + (double)flt_arr[i];
    }
    
    return sum;
}

/* Another function with nested loops for additional DDG complexity */
__attribute__((noinline, noclone))
void nested_loop_deps(int* matrix, int rows, int cols) {
    int i, j;
    
    /* Nested loops with carried dependencies in both dimensions */
    for (i = 1; i < rows; i++) {
        for (j = 1; j < cols; j++) {
            /* True dependency along rows */
            matrix[i*cols + j] = matrix[(i-1)*cols + j] + matrix[i*cols + (j-1)];
            
            /* Anti-dependency through temporary */
            int temp = matrix[i*cols + j];
            matrix[i*cols + j] = temp * 2 - 1;
            
            /* Output dependency */
            matrix[i*cols + j] = matrix[i*cols + j] * 3;
            matrix[i*cols + j] = matrix[i*cols + j] / 2;
        }
    }
}

/* Function with pointer chasing creating memory dependencies */
__attribute__((noinline, noclone))
double pointer_chasing_loop(double* data, int* indices, int size) {
    double result = 0.0;
    int i;
    
    for (i = 0; i < size - 1; i++) {
        /* Pointer chasing with true dependency */
        double* current = &data[indices[i]];
        double* next = &data[indices[i+1]];
        
        /* Memory dependency chain */
        *next = *current * 1.61803399;  /* Golden ratio */
        
        /* Anti-dependency with the same memory location */
        double temp = *next;
        *next = temp + (double)i;
        
        result += *next;
    }
    
    return result;
}

int main(void) {
    const int SIZE = 1024;
    const int ROWS = 64;
    const int COLS = 64;
    
    /* Allocate and initialize arrays with different data types */
    double* double_arr = (double*)malloc(SIZE * sizeof(double));
    int* int_arr = (int*)malloc(SIZE * sizeof(int));
    float* float_arr = (float*)malloc(SIZE * sizeof(float));
    int* matrix = (int*)malloc(ROWS * COLS * sizeof(int));
    int* indices = (int*)malloc(SIZE * sizeof(int));
    
    if (!double_arr || !int_arr || !float_arr || !matrix || !indices) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        double_arr[i] = (double)i * 0.5;
        int_arr[i] = i * 2;
        float_arr[i] = (float)i * 0.25f;
        indices[i] = i % (SIZE/2);  /* Create some aliasing */
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        matrix[i] = i;
    }
    
    volatile double result1, result2, result3;
    
    /* Call functions to trigger DDG construction */
    result1 = compute_loop(double_arr, int_arr, float_arr, SIZE);
    nested_loop_deps(matrix, ROWS, COLS);
    result2 = pointer_chasing_loop(double_arr, indices, SIZE);
    
    /* Use results to prevent dead code elimination */
    result3 = result1 + result2 + (double)matrix[ROWS*COLS-1];
    
    printf("Result: %f\n", (double)result3);
    
    /* Cleanup */
    free(double_arr);
    free(int_arr);
    free(float_arr);
    free(matrix);
    free(indices);
    
    return 0;
}
