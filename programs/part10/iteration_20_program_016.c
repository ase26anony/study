/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure loop body remains intact for DDG analysis */
__attribute__((noinline, noclone))
int compute_loop(int* arr, double* darr, float* farr, int size) {
    int i;
    int temp_int;
    double temp_double;
    float temp_float;
    int* ptr;
    int sum = 0;
    
    /* Initialize pointers for aliasing */
    int* alias_ptr = arr + (size / 2);
    
    /* Complex loop with multiple dependency types */
    for (i = 1; i < size - 1; i++) {
        /* 1. TRUE DEPENDENCY (RAW) - Integer chain */
        arr[i] = arr[i - 1] + i * 2;          /* RAW on arr[i-1] */
        
        /* 2. ANTI-DEPENDENCY (WAR) - Reuse of arr[i] */
        temp_int = arr[i];                    /* Read arr[i] */
        arr[i] = temp_int * 3 - i;            /* Write arr[i] - WAR on arr[i] */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - Multiple writes to same location */
        arr[i] = arr[i] + arr[i - 1];         /* Write arr[i] */
        arr[i] = arr[i] * 2;                  /* Another write to arr[i] - WAW */
        
        /* 4. Memory aliasing dependency */
        if (i == size / 2) {
            *alias_ptr = arr[i] + 5;          /* Potential aliasing with arr[size/2] */
        }
        
        /* 5. Floating-point RAW dependency with different latency */
        darr[i] = darr[i - 1] * 1.5 + i;      /* FP multiplication has different latency */
        
        /* 6. Mixed-type dependency chain */
        temp_double = darr[i];
        farr[i] = (float)temp_double + 0.5f;  /* Type conversion dependency */
        
        /* 7. Pointer arithmetic dependency */
        ptr = arr + i;
        temp_int = *ptr;                      /* Load through pointer */
        *ptr = temp_int + *(ptr - 1);         /* Store with pointer arithmetic */
        
        /* 8. Control dependency-like pattern */
        if (arr[i] > 1000) {
            arr[i] = arr[i] % 1000;           /* Creates control flow */
        }
        
        /* 9. Accumulator with loop-carried dependency */
        sum += arr[i] + (int)darr[i];         /* Loop-carried on sum */
        
        /* 10. Another WAW with different data type */
        farr[i] = farr[i] * 1.1f;             /* Another write to farr[i] */
    }
    
    return sum;
}

/* Another function with nested loops for more complex DDG */
__attribute__((noinline, noclone))
void nested_loop_deps(int* matrix, int rows, int cols) {
    int i, j;
    
    /* Nested loop with stride access patterns */
    for (i = 1; i < rows; i++) {
        for (j = 1; j < cols; j++) {
            /* 2D stencil computation with multiple dependencies */
            matrix[i * cols + j] = 
                matrix[(i-1) * cols + j] +    /* RAW - row above */
                matrix[i * cols + (j-1)] +    /* RAW - column left */
                matrix[(i-1) * cols + (j-1)]; /* RAW - diagonal */
            
            /* Anti-dependency in 2D */
            int old_val = matrix[i * cols + j];
            matrix[i * cols + j] = old_val * 2 - matrix[i * cols + j];
        }
    }
}

int main() {
    const int SIZE = 1024;
    const int ROWS = 64;
    const int COLS = 64;
    
    /* Allocate and initialize arrays with different data types */
    int* int_array = (int*)malloc(SIZE * sizeof(int));
    double* double_array = (double*)malloc(SIZE * sizeof(double));
    float* float_array = (float*)malloc(SIZE * sizeof(float));
    int* matrix = (int*)malloc(ROWS * COLS * sizeof(int));
    
    /* Initialize with pattern to avoid zero elimination */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i % 100;
        double_array[i] = i * 0.5;
        float_array[i] = i * 0.25f;
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        matrix[i] = i % 50;
    }
    
    /* Use volatile to prevent over-optimization of results */
    volatile int result1, result2;
    
    /* Call functions to trigger DDG construction */
    result1 = compute_loop(int_array, double_array, float_array, SIZE);
    nested_loop_deps(matrix, ROWS, COLS);
    
    /* Compute checksum to ensure all work is done */
    result2 = 0;
    for (int i = 0; i < SIZE; i++) {
        result2 += int_array[i] + (int)double_array[i];
    }
    
    /* Print results to prevent dead code elimination */
    printf("Result 1: %d\n", result1);
    printf("Result 2: %d\n", result2);
    printf("Sample values: %d, %d, %d\n", 
           int_array[SIZE/2], int_array[SIZE/3], int_array[SIZE/4]);
    
    free(int_array);
    free(double_array);
    free(float_array);
    free(matrix);
    
    return 0;
}
