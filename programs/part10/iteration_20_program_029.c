/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */
#include <stdio.h>
#include <stdlib.h>

/* Non-inlined function containing complex dependency patterns */
__attribute__((noinline, noclone))
int compute_loop(int* arr, double* darr, float* farr, int size) {
    int i;
    int sum = 0;
    double dsum = 0.0;
    float fsum = 0.0f;
    
    /* Complex loop with multiple carried dependencies */
    for (i = 1; i < size; i++) {
        /* 1. TRUE DATA DEPENDENCY (RAW) - Integer chain */
        int temp_int = arr[i-1];           /* Read arr[i-1] */
        arr[i] = temp_int + i * 2;         /* Write arr[i] depends on arr[i-1] */
        
        /* 2. ANTI-DEPENDENCY (WAR) - Reusing same array location */
        float old_float = farr[i];         /* Read farr[i] */
        farr[i] = old_float * 1.5f + i;    /* Write farr[i] after read */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - Multiple writes to same location */
        darr[i] = dsum * 0.5;              /* First write to darr[i] */
        darr[i] = darr[i] * 2.0 + i;       /* Second write to darr[i] */
        
        /* 4. MEMORY DEPENDENCY with pointer aliasing */
        int* ptr1 = &arr[i];
        int* ptr2 = &arr[i-1] + 1;         /* Potentially aliases arr[i] */
        *ptr1 = *ptr1 + *ptr2;             /* Compiler must assume dependency */
        
        /* 5. MIXED DATA TYPES with different latencies */
        sum += arr[i];                     /* Integer add (low latency) */
        dsum += darr[i] * 3.14159;         /* FP multiply-add (higher latency) */
        fsum = fsum * 0.9f + farr[i];      /* FP operations */
        
        /* 6. CONTROL DEPENDENCY (via conditional) */
        if (sum > 1000) {
            arr[i] = arr[i] / 2;           /* Creates control flow */
        }
    }
    
    /* Combine results to prevent elimination */
    volatile int final_result = sum + (int)dsum + (int)fsum;
    return final_result;
}

/* Another function with nested loops for more complex DDG */
__attribute__((noinline, noclone))
int nested_loop_deps(int* matrix, int rows, int cols) {
    int i, j;
    int total = 0;
    
    /* Nested loop with carried dependencies in both dimensions */
    for (i = 1; i < rows; i++) {
        for (j = 1; j < cols; j++) {
            /* 2D stencil computation - multiple dependencies */
            int idx = i * cols + j;
            int idx_up = (i-1) * cols + j;
            int idx_left = i * cols + (j-1);
            
            /* True dependencies from previous row and column */
            matrix[idx] = matrix[idx_up] + matrix[idx_left] + matrix[idx];
            
            /* Anti-dependency through temporary */
            int old_val = matrix[idx];
            matrix[idx] = old_val * 3 - j;
            
            total += matrix[idx];
        }
    }
    
    return total;
}

/* Function with pointer chasing creating memory dependencies */
__attribute__((noinline, noclone))
int pointer_chasing_loop(int* data, int size) {
    int* current = data;
    int sum = 0;
    int i;
    
    for (i = 0; i < size - 1; i++) {
        /* Pointer-based access with carried dependency */
        int val = *current;                /* Read through pointer */
        current++;                         /* Pointer arithmetic */
        *current = val + *current + i;     /* Write with dependency on previous */
        
        sum += *current;
        
        /* Create output dependency with indirect write */
        int* write_ptr = &data[i % 10];    /* Limited set of addresses */
        *write_ptr = sum % 100;            /* Potential WAW dependencies */
    }
    
    return sum;
}

int main() {
    const int SIZE = 512;
    int i;
    
    /* Allocate and initialize arrays with different data types */
    int* int_array = (int*)malloc(SIZE * sizeof(int));
    double* double_array = (double*)malloc(SIZE * sizeof(double));
    float* float_array = (float*)malloc(SIZE * sizeof(float));
    int* matrix = (int*)malloc(SIZE * SIZE * sizeof(int));
    
    if (!int_array || !double_array || !float_array || !matrix) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (i = 0; i < SIZE; i++) {
        int_array[i] = i;
        double_array[i] = i * 0.5;
        float_array[i] = i * 0.25f;
    }
    
    for (i = 0; i < SIZE * SIZE; i++) {
        matrix[i] = i % 100;
    }
    
    printf("Starting computation...\n");
    
    /* Call functions with different dependency patterns */
    int result1 = compute_loop(int_array, double_array, float_array, SIZE);
    int result2 = nested_loop_deps(matrix, 64, 64);  /* 64x64 matrix */
    int result3 = pointer_chasing_loop(int_array, SIZE);
    
    /* Use results to prevent dead code elimination */
    volatile int final = result1 + result2 + result3;
    printf("Results: %d, %d, %d (Final: %d)\n", result1, result2, result3, final);
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(float_array);
    free(matrix);
    
    return 0;
}
