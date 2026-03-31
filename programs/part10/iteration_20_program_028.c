/* test_ddg.c - Program to trigger DDG edge creation in GCC scheduler */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure loop body remains intact for DDG analysis */
__attribute__((noinline, noclone))
int compute_loop(int* arr, double* darr, float* farr, int size) {
    int i;
    int temp_int;
    double temp_double;
    float temp_float;
    int* ptr = arr;
    int sum = 0;
    
    /* Complex loop with multiple dependency types */
    for (i = 1; i < size; i++) {
        /* 1. TRUE DEPENDENCY (RAW) - Integer chain */
        arr[i] = arr[i-1] + i * 2;          /* True dep on arr[i-1] */
        
        /* 2. ANTI-DEPENDENCY (WAR) - Reuse of same location */
        temp_int = arr[i];                  /* Read arr[i] */
        arr[i] = temp_int * 3 - i;          /* Write arr[i] - anti-dep on previous read */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - Multiple writes */
        darr[i] = (double)arr[i] * 1.5;     /* First write to darr[i] */
        darr[i] = darr[i] * 0.75 + 2.0;     /* Second write to darr[i] - output dep */
        
        /* 4. MEMORY DEPENDENCY with pointer aliasing */
        *ptr = *ptr + 1;                    /* Could alias with arr elements */
        ptr = &arr[i % 4];                  /* Change pointer target */
        
        /* 5. MIXED DATA TYPES with dependencies */
        temp_float = (float)darr[i];        /* Float from double */
        farr[i] = temp_float * 0.5f;        /* Float operation */
        
        /* 6. FLOATING POINT dependency chain */
        temp_double = darr[i] * 1.25;       /* FP multiply */
        darr[i] = temp_double / 2.0;        /* FP divide - true dep on temp_double */
        
        /* 7. ADDRESS CALCULATION dependency */
        int idx = (arr[i] + i) % size;      /* Complex address calc */
        farr[idx] = farr[idx] + 1.0f;       /* Memory dep through idx */
        
        /* 8. ACCUMULATOR with loop-carried dependency */
        sum = sum + arr[i] + (int)darr[i];  /* True dep on sum from previous iteration */
        
        /* 9. CONDITIONAL with dependencies */
        if (sum % 5 == 0) {
            arr[i] = arr[i] + 100;          /* Control + data dependency */
        }
    }
    
    return sum;
}

/* Another function with nested loops for more complex DDG */
__attribute__((noinline, noclone))
int nested_loop_compute(int* matrix, int rows, int cols) {
    int i, j;
    int total = 0;
    
    /* Nested loop with cross-iteration dependencies */
    for (i = 1; i < rows; i++) {
        for (j = 1; j < cols; j++) {
            /* True dependency on previous row and column */
            matrix[i * cols + j] = 
                matrix[(i-1) * cols + j] +    /* Dependency on previous row */
                matrix[i * cols + (j-1)] -    /* Dependency on previous column */
                matrix[(i-1) * cols + (j-1)]; /* Dependency on diagonal */
            
            /* Anti-dependency through temporary */
            int old_val = matrix[i * cols + j];
            matrix[i * cols + j] = old_val * 2 - j;
            
            /* Loop-carried dependency through total */
            total = total + matrix[i * cols + j];
        }
    }
    
    return total;
}

/* Function with pointer chasing to create memory dependencies */
__attribute__((noinline, noclone))
int pointer_chasing_loop(int* data, int size) {
    int* current = data;
    int sum = 0;
    
    for (int i = 0; i < size - 1; i++) {
        /* Pointer chase with dependency */
        int val = *current;
        current = data + (val % size);
        sum += val;
        
        /* Create output dependency */
        *current = sum % 256;
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
    
    /* Initialize data */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i;
        double_array[i] = i * 0.5;
        float_array[i] = i * 0.25f;
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        matrix[i] = i % 100;
    }
    
    /* Volatile to prevent dead code elimination */
    volatile int result1, result2, result3;
    
    /* Call functions with complex loops */
    result1 = compute_loop(int_array, double_array, float_array, SIZE);
    result2 = nested_loop_compute(matrix, ROWS, COLS);
    result3 = pointer_chasing_loop(int_array, SIZE);
    
    /* Use results to prevent optimization */
    printf("Results: %d, %d, %d\n", result1, result2, result3);
    
    /* Final computation using all arrays to ensure they're used */
    int final_check = 0;
    for (int i = 0; i < SIZE; i++) {
        final_check += int_array[i] + (int)double_array[i] + (int)float_array[i];
    }
    printf("Final check: %d\n", final_check);
    
    free(int_array);
    free(double_array);
    free(float_array);
    free(matrix);
    
    return 0;
}
