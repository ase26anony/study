/* test_ddg.c - Program to trigger DDG edge creation in GCC scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Non-inlined function to ensure loop body remains intact for DDG analysis */
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
    
    /* Main loop with complex dependency patterns */
    for (i = 1; i < size - 1; i++) {
        /* 1. TRUE DEPENDENCY (RAW) - Integer chain */
        arr[i] = arr[i-1] + i * 2;           /* Read arr[i-1], write arr[i] */
        
        /* 2. ANTI-DEPENDENCY (WAR) - Float operations */
        temp_float = farr[i];                /* Read farr[i] */
        farr[i] = temp_float * 1.1f + i;     /* Write farr[i] - anti-dep on previous read */
        
        /* 3. OUTPUT DEPENDENCY (WAW) - Double operations */
        darr[i] = darr[i] * 0.9;             /* First write to darr[i] */
        darr[i] = darr[i] + temp_float;      /* Second write to darr[i] - output dep */
        
        /* 4. MEMORY DEPENDENCY with pointer aliasing */
        ptr = &arr[i];
        *ptr = *ptr + arr[i-1];              /* Potential memory dep through pointer */
        
        /* 5. MIXED DATA TYPE DEPENDENCIES */
        temp_int = (int)farr[i];             /* Float to int conversion */
        arr[i] = arr[i] + temp_int;          /* Mixed type dependency */
        
        /* 6. CROSS-ITERATION DEPENDENCIES with different latencies */
        temp_double = darr[i-1] * 2.5;       /* FP multiply - higher latency */
        darr[i] = darr[i] + temp_double;     /* FP add */
        
        /* 7. ACCUMULATOR with carried dependency */
        sum = sum + arr[i] + (int)darr[i];   /* Loop-carried dependency on sum */
        
        /* 8. CONTROL DEPENDENCY (through conditional) */
        if (sum > 1000) {
            arr[i] = arr[i] / 2;             /* Creates control dependency edge */
        }
    }
    
    return sum;
}

/* Another function with nested loops for more complex DDG */
__attribute__((noinline, noclone))
void nested_loop_deps(int* matrix, int rows, int cols) {
    int i, j;
    
    /* Nested loops with multiple dependency patterns */
    for (i = 1; i < rows; i++) {
        for (j = 1; j < cols; j++) {
            /* True dependency in i dimension */
            matrix[i*cols + j] = matrix[(i-1)*cols + j] + matrix[i*cols + (j-1)];
            
            /* Anti-dependency through temporary */
            int temp = matrix[i*cols + j];
            matrix[i*cols + j] = temp * 3 - j;
            
            /* Output dependency */
            matrix[i*cols + j] = matrix[i*cols + j] + i;
        }
    }
}

/* Function with pointer chasing for memory deps */
__attribute__((noinline, noclone))
int pointer_chasing(int* data, int size) {
    int* current = data;
    int sum = 0;
    int i;
    
    /* Create linked-list like access pattern */
    for (i = 0; i < size - 1; i++) {
        /* Read through pointer */
        int val = *current;
        
        /* Write through pointer with offset */
        *(current + 1) = val + i;
        
        /* Move pointer - creates dependency chain */
        current = current + 1;
        
        sum += val;
    }
    
    return sum;
}

int main(void) {
    const int SIZE = 1024;
    int* int_array = (int*)malloc(SIZE * sizeof(int));
    float* float_array = (float*)malloc(SIZE * sizeof(float));
    double* double_array = (double*)malloc(SIZE * sizeof(double));
    int* matrix = (int*)malloc(SIZE * SIZE * sizeof(int));
    
    /* Prevent compiler from optimizing away arrays */
    volatile int result;
    
    /* Call function with complex dependencies */
    result = compute_loop(int_array, float_array, double_array, SIZE);
    
    /* Call nested loop function */
    nested_loop_deps(matrix, 32, 32);
    
    /* Call pointer chasing function */
    result += pointer_chasing(int_array, SIZE);
    
    /* Use result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(matrix);
    
    return 0;
}
