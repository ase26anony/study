/* test_ddg.c - Program to trigger DDG edge creation in GCC scheduler */

#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure loop body remains intact for DDG analysis */
__attribute__((noinline, noclone))
int compute_loop(int* arr, double* darr, float* farr, int size) {
    int i;
    int sum = 0;
    int temp;
    double dtemp;
    float ftemp;
    
    /* Initialize with some values */
    for (i = 0; i < size; i++) {
        arr[i] = i % 100;
        darr[i] = i * 0.5;
        farr[i] = i * 0.25f;
    }
    
    /* Main loop with complex dependency patterns */
    for (i = 1; i < size - 1; i++) {
        /* 1. TRUE DEPENDENCY (RAW) - carried across iterations */
        arr[i] = arr[i-1] + arr[i];  /* RAW: read arr[i-1], write arr[i] */
        
        /* 2. ANTI-DEPENDENCY (WAR) - within same iteration */
        temp = arr[i];               /* WAR: read arr[i] */
        arr[i] = temp * 2;           /* WAR: write arr[i] after read */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - within same iteration */
        arr[i] = arr[i] + 1;         /* WAW: write arr[i] again */
        
        /* 4. Memory aliasing with pointer arithmetic */
        int* ptr = &arr[i];
        *ptr = *ptr + *(ptr-1);      /* Potential memory dependency */
        
        /* 5. Floating-point dependencies with different latencies */
        dtemp = darr[i];             /* Load with potential FP latency */
        darr[i] = dtemp * 1.5;       /* FP multiply with different latency */
        
        /* 6. Mixed-type dependencies */
        ftemp = farr[i];
        farr[i] = ftemp + (float)darr[i];  /* Mixed float/double */
        
        /* 7. Control dependency-like pattern */
        if (arr[i] > 100) {
            arr[i] = arr[i] % 100;   /* Creates control flow */
        }
        
        /* 8. Another carried dependency with different distance */
        if (i > 2) {
            arr[i] += arr[i-2];      /* Distance-2 carried dependency */
        }
        
        /* 9. Complex expression with multiple operations */
        sum += arr[i] * 3 + (int)darr[i] + (int)farr[i];
    }
    
    /* Prevent dead code elimination */
    volatile int keep = sum;
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
            /* 2D stencil computation with multiple dependencies */
            int idx = i * cols + j;
            
            /* Dependencies from previous row and column */
            matrix[idx] = matrix[idx - 1] +           /* Same row, previous col */
                          matrix[idx - cols] +        /* Previous row, same col */
                          matrix[idx - cols - 1];     /* Diagonal */
            
            /* Anti-dependency pattern */
            int old_val = matrix[idx];
            matrix[idx] = (old_val * 3) / 2;
            
            /* Output dependency */
            matrix[idx] = matrix[idx] + i + j;
            
            total += matrix[idx];
        }
    }
    
    return total;
}

/* Function with pointer aliasing to create memory dependencies */
__attribute__((noinline, noclone))
int pointer_aliasing_loop(int* a, int* b, int size) {
    int i;
    int result = 0;
    
    /* Create potential aliasing */
    int* p1 = a;
    int* p2 = b;
    
    for (i = 1; i < size; i++) {
        /* The compiler can't be sure p1 and p2 don't alias */
        *p1 = *p1 + *p2 + i;
        p1++;
        p2++;
        
        /* Another memory operation that might alias */
        a[i] = b[i-1] + a[i];
        
        result += a[i];
    }
    
    return result;
}

int main() {
    const int SIZE = 1024;
    const int ROWS = 64;
    const int COLS = 64;
    
    /* Allocate arrays with different data types */
    int* arr = (int*)malloc(SIZE * sizeof(int));
    double* darr = (double*)malloc(SIZE * sizeof(double));
    float* farr = (float*)malloc(SIZE * sizeof(float));
    int* matrix = (int*)malloc(ROWS * COLS * sizeof(int));
    
    if (!arr || !darr || !farr || !matrix) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    /* Call functions with different dependency patterns */
    int result1 = compute_loop(arr, darr, farr, SIZE);
    int result2 = nested_loop_compute(matrix, ROWS, COLS);
    int result3 = pointer_aliasing_loop(arr, matrix, SIZE < ROWS*COLS ? SIZE : ROWS*COLS);
    
    /* Use results to prevent optimization */
    volatile int final_result = result1 + result2 + result3;
    
    printf("Results: %d, %d, %d\n", result1, result2, result3);
    printf("Final: %d\n", final_result);
    
    /* Cleanup */
    free(arr);
    free(darr);
    free(farr);
    free(matrix);
    
    return 0;
}
