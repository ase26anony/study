/* test_ddg.c - Program to trigger GCC Data Dependency Graph edge creation */

#include <stdio.h>
#include <stdlib.h>

/* Prevent optimization that would eliminate dependencies */
#define NOINLINE __attribute__((noinline, noclone))

/* Complex loop with multiple dependency types */
NOINLINE
double compute_loop(double* arr, int* indices, float* farr, int size) {
    double sum = 0.0;
    double temp = 0.0;
    int i;
    
    /* Loop with multiple carried dependencies */
    for (i = 1; i < size; i++) {
        /* 1. TRUE DEPENDENCY (RAW) - carried across iterations */
        double prev = arr[i-1];           /* Read arr[i-1] */
        arr[i] = prev * 1.5 + i;         /* Write arr[i] depends on arr[i-1] */
        
        /* 2. ANTI-DEPENDENCY (WAR) - reuse of same location */
        temp = farr[i];                   /* Read farr[i] */
        farr[i] = temp * 0.5f + i;       /* Write farr[i] after read */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - multiple writes to same location */
        arr[i] = arr[i] + 0.1;           /* Second write to arr[i] */
        
        /* 4. MEMORY DEPENDENCY with pointer aliasing */
        double* ptr = &arr[indices[i]];  /* Potential alias */
        *ptr = *ptr * 2.0;               /* Could alias with arr[i] */
        
        /* 5. MIXED DATA TYPES creating different latency edges */
        int int_val = (int)arr[i];       /* Type conversion */
        float float_val = (float)int_val * 1.2f;
        double double_val = (double)float_val * 2.5;
        
        /* 6. COMPLEX DEPENDENCY CHAIN with different operations */
        sum += double_val * (i % 3);     /* Accumulator with varying multiplier */
        
        /* 7. CONTROL DEPENDENCY-like pattern */
        if (sum > 1000.0) {
            sum = sum * 0.9;             /* Creates control flow */
        }
    }
    
    /* Additional output dependency */
    arr[0] = sum;                        /* WAW with potential earlier arr[0] access */
    
    return sum;
}

/* Another function with nested loops for more complex DDG */
NOINLINE
int nested_loop_deps(int* matrix, int rows, int cols) {
    int total = 0;
    int i, j;
    
    /* Nested loop with carried dependencies in both dimensions */
    for (i = 1; i < rows; i++) {
        for (j = 1; j < cols; j++) {
            /* True dependency in i dimension */
            int up = matrix[(i-1)*cols + j];
            
            /* True dependency in j dimension */
            int left = matrix[i*cols + (j-1)];
            
            /* Anti-dependency */
            int current = matrix[i*cols + j];
            
            /* Complex computation with mixed dependencies */
            matrix[i*cols + j] = (up + left) * 2 - current;
            
            /* Output dependency */
            matrix[i*cols + j] += (i * j) % 7;
            
            total += matrix[i*cols + j];
        }
    }
    
    return total;
}

/* Function with pointer chasing creating memory dependencies */
NOINLINE
void pointer_chasing(int** ptr_array, int size) {
    int i;
    int val = 0;
    
    for (i = 0; i < size - 1; i++) {
        /* True dependency through pointer dereference */
        val = *ptr_array[i];
        
        /* Write with potential aliasing */
        *ptr_array[i+1] = val + i;
        
        /* Anti-dependency with the same memory location */
        int temp = *ptr_array[i];
        *ptr_array[i] = temp * 2;
    }
}

int main() {
    const int SIZE = 512;
    const int ROWS = 64;
    const int COLS = 64;
    
    /* Allocate and initialize arrays */
    double* arr = (double*)malloc(SIZE * sizeof(double));
    float* farr = (float*)malloc(SIZE * sizeof(float));
    int* indices = (int*)malloc(SIZE * sizeof(int));
    int* matrix = (int*)malloc(ROWS * COLS * sizeof(int));
    int** ptr_array = (int**)malloc(SIZE * sizeof(int*));
    
    int i;
    
    /* Initialize data */
    for (i = 0; i < SIZE; i++) {
        arr[i] = (double)i * 0.5;
        farr[i] = (float)i * 0.3f;
        indices[i] = (i * 7) % SIZE;
        ptr_array[i] = &indices[i];
    }
    
    for (i = 0; i < ROWS * COLS; i++) {
        matrix[i] = i % 100;
    }
    
    /* Volatile to prevent optimization of results */
    volatile double result1;
    volatile int result2;
    
    /* Execute loops to trigger DDG construction */
    result1 = compute_loop(arr, indices, farr, SIZE);
    result2 = nested_loop_deps(matrix, ROWS, COLS);
    pointer_chasing(ptr_array, SIZE);
    
    /* Use results to prevent dead code elimination */
    printf("Result 1: %f\n", (double)result1);
    printf("Result 2: %d\n", result2);
    printf("Final arr[0]: %f\n", arr[0]);
    
    /* Cleanup */
    free(arr);
    free(farr);
    free(indices);
    free(matrix);
    free(ptr_array);
    
    return 0;
}
