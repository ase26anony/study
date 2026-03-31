/* test_ddg.c - Program to trigger DDG edge creation in GCC scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Prevent inlining to ensure loop body remains intact for DDG analysis */
__attribute__((noinline, noclone))
int compute_loop(int* arr, float* farr, double* darr, int size) {
    int i;
    int temp_int;
    float temp_float;
    double temp_double;
    int* ptr;
    int sum = 0;
    
    /* Initialize arrays with some values */
    for (i = 0; i < size; i++) {
        arr[i] = i;
        farr[i] = i * 0.5f;
        darr[i] = i * 0.25;
    }
    
    /* 
     * Main loop with complex dependency patterns to trigger DDG edge creation
     * This loop contains multiple types of dependencies:
     */
    for (i = 1; i < size - 1; i++) {
        /* 1. TRUE DEPENDENCY (RAW) with integer */
        temp_int = arr[i - 1];          /* Read arr[i-1] */
        arr[i] = temp_int + i;          /* Write arr[i] depends on arr[i-1] */
        
        /* 2. ANTI-DEPENDENCY (WAR) with float */
        temp_float = farr[i];           /* Read farr[i] */
        farr[i] = temp_float * 1.1f;    /* Write farr[i] - anti-dep on previous read */
        
        /* 3. OUTPUT DEPENDENCY (WAW) with double */
        darr[i] = temp_int * 0.33;      /* First write to darr[i] */
        darr[i] = darr[i] + 0.5;        /* Second write to darr[i] - output dep */
        
        /* 4. MEMORY DEPENDENCY with pointer aliasing */
        ptr = &arr[i];
        *ptr = *ptr + arr[i + 1];       /* Potential memory dep through pointer */
        
        /* 5. MIXED DATA TYPES in dependency chain */
        sum += arr[i] + (int)farr[i] + (int)darr[i];
        
        /* 6. LOOP-CARRIED DEPENDENCY with different latencies */
        /* Integer add (low latency) */
        arr[i + 1] = arr[i] + 1;
        
        /* Floating multiply (higher latency) */
        farr[i + 1] = farr[i] * 1.5f;
        
        /* Double operations (potentially different latency) */
        darr[i + 1] = darr[i] * 1.25;
    }
    
    /* Additional computation to prevent dead code elimination */
    for (i = 0; i < size; i++) {
        sum += arr[i] % 7;
    }
    
    return sum;
}

/* Another function with nested loops for more complex DDG */
__attribute__((noinline, noclone))
int nested_loop_compute(int* A, int* B, int n) {
    int i, j;
    int total = 0;
    
    /* Nested loop with carried dependencies */
    for (i = 1; i < n; i++) {
        for (j = 1; j < n; j++) {
            /* Cross-iteration dependencies in both dimensions */
            A[i * n + j] = A[(i - 1) * n + j] + A[i * n + (j - 1)];
            
            /* Anti-dependency */
            int temp = B[i * n + j];
            B[i * n + j] = temp + A[i * n + j];
            
            /* Output dependency */
            B[i * n + j] = B[i * n + j] * 2;
            
            total += A[i * n + j] + B[i * n + j];
        }
    }
    
    return total;
}

/* Function with volatile to prevent over-optimization but preserve dependencies */
__attribute__((noinline, noclone))
int volatile_loop(int* data, int size) {
    int i;
    volatile int barrier = 0;
    int result = 0;
    
    for (i = 1; i < size; i++) {
        /* True dependency chain */
        int t1 = data[i - 1];
        int t2 = t1 * 3;
        data[i] = t2 + i;
        
        /* Use volatile to create memory barrier but not break dependencies */
        if (barrier == 0) {
            result += data[i];
        }
    }
    
    return result;
}

int main() {
    const int SIZE = 256;
    int* int_array = (int*)malloc(SIZE * sizeof(int));
    float* float_array = (float*)malloc(SIZE * sizeof(float));
    double* double_array = (double*)malloc(SIZE * sizeof(double));
    int* A = (int*)malloc(SIZE * SIZE * sizeof(int));
    int* B = (int*)malloc(SIZE * SIZE * sizeof(int));
    
    if (!int_array || !float_array || !double_array || !A || !B) {
        printf("Memory allocation failed\n");
        return 1;
    }
    
    /* Call functions with different dependency patterns */
    int result1 = compute_loop(int_array, float_array, double_array, SIZE);
    int result2 = nested_loop_compute(A, B, 16);
    int result3 = volatile_loop(int_array, SIZE);
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d, %d, %d\n", result1, result2, result3);
    
    /* Additional volatile store to force memory operations */
    volatile int final_check = result1 + result2 + result3;
    
    free(int_array);
    free(float_array);
    free(double_array);
    free(A);
    free(B);
    
    return 0;
}
