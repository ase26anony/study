/* test_ddg.c - Program to trigger DDG edge creation in GCC scheduler */

#include <stdio.h>
#include <stdlib.h>

/* Non-inlined function to ensure loop body remains intact for DDG analysis */
__attribute__((noinline, noclone))
int compute_loop(int* arr, double* darr, float* farr, int size) {
    int i;
    int sum = 0;
    double dsum = 0.0;
    float fsum = 0.0f;
    
    /* Initialize with some values */
    for (i = 0; i < size; i++) {
        arr[i] = i;
        darr[i] = i * 0.5;
        farr[i] = i * 0.25f;
    }
    
    /* Main loop with complex dependency patterns */
    for (i = 1; i < size; i++) {
        /* 1. TRUE DEPENDENCY (RAW) - Integer chain */
        int temp1 = arr[i-1];          /* Read arr[i-1] */
        arr[i] = temp1 + i * 2;        /* Write arr[i] depends on arr[i-1] */
        
        /* 2. ANTI-DEPENDENCY (WAR) - Reusing same location */
        double temp2 = darr[i];        /* Read darr[i] */
        darr[i] = darr[i-1] * 1.5;     /* Write darr[i] after read */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - Multiple writes */
        farr[i] = farr[i-1] + 0.1f;    /* First write to farr[i] */
        farr[i] = farr[i] * 2.0f;      /* Second write to farr[i] */
        
        /* 4. MIXED DATA TYPES with dependencies */
        /* Integer to float conversion dependency */
        float ftemp = (float)arr[i];
        fsum += ftemp * 0.5f;
        
        /* Floating-point with different latencies */
        dsum += darr[i] * darr[i-1];   /* FP multiply dependency */
        
        /* 5. POINTER ALIASING for memory dependencies */
        int* ptr1 = &arr[i];
        int* ptr2 = &arr[i-1] + 1;     /* Could alias with &arr[i] */
        *ptr1 = *ptr1 + *ptr2;         /* Potential memory dependency */
        
        /* 6. LOOP-CARRIED DEPENDENCY with distance > 1 */
        if (i >= 3) {
            arr[i] += arr[i-3];        /* Dependency distance 3 */
        }
        
        /* 7. CONTROL DEPENDENCY */
        if (arr[i] > 100) {
            darr[i] /= 2.0;            /* Control-dependent operation */
        }
        
        /* 8. ACCUMULATOR with true dependency */
        sum += arr[i];                 /* sum has loop-carried dependency */
    }
    
    /* Final computation mixing all dependencies */
    volatile int final_result = sum + (int)dsum + (int)fsum;
    return final_result;
}

/* Another function with nested loops for more complex DDG */
__attribute__((noinline, noclone))
int nested_loop_compute(int* matrix, int n) {
    int i, j;
    int total = 0;
    
    /* Nested loops with cross-iteration dependencies */
    for (i = 1; i < n; i++) {
        for (j = 1; j < n; j++) {
            /* 2D stencil computation with multiple dependencies */
            int up = matrix[(i-1)*n + j];      /* RAW dependency */
            int left = matrix[i*n + (j-1)];    /* RAW dependency */
            int diag = matrix[(i-1)*n + (j-1)]; /* RAW dependency */
            
            /* Multiple writes to same location (WAW) */
            matrix[i*n + j] = up + left;
            matrix[i*n + j] = matrix[i*n + j] - diag;
            
            /* Anti-dependency (WAR) */
            int old_val = matrix[i*n + j];
            matrix[i*n + j] = old_val * 2;
            
            /* Loop-carried dependency in inner loop */
            total += matrix[i*n + j];
        }
    }
    
    return total;
}

/* Function with pointer chasing for complex memory dependencies */
__attribute__((noinline, noclone))
int pointer_chase_loop(int* data, int size) {
    int* current = data;
    int sum = 0;
    
    for (int i = 0; i < size - 1; i++) {
        /* Pointer chasing creates memory dependency chain */
        int val = *current;            /* Read through pointer */
        current = data + val % size;   /* Next pointer depends on read value */
        sum += val;
        
        /* Write to create output dependency */
        *data = sum % 256;
    }
    
    return sum;
}

int main() {
    const int SIZE = 256;
    
    /* Allocate arrays with different data types */
    int* int_array = (int*)malloc(SIZE * sizeof(int));
    double* double_array = (double*)malloc(SIZE * sizeof(double));
    float* float_array = (float*)malloc(SIZE * sizeof(float));
    int* matrix = (int*)malloc(SIZE * SIZE * sizeof(int));
    
    if (!int_array || !double_array || !float_array || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize matrix */
    for (int i = 0; i < SIZE * SIZE; i++) {
        matrix[i] = i % 100;
    }
    
    printf("Starting computation...\n");
    
    /* Call functions with different dependency patterns */
    int result1 = compute_loop(int_array, double_array, float_array, SIZE);
    printf("Result 1: %d\n", result1);
    
    int result2 = nested_loop_compute(matrix, 16);  /* Smaller size for matrix */
    printf("Result 2: %d\n", result2);
    
    int result3 = pointer_chase_loop(int_array, SIZE);
    printf("Result 3: %d\n", result3);
    
    /* Final validation to prevent optimization */
    volatile int final_check = result1 + result2 + result3;
    printf("Final check: %d\n", final_check);
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(float_array);
    free(matrix);
    
    return 0;
}
