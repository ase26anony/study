/* test_ddg.c - Program to trigger DDG edge creation in GCC scheduler */
#include <stdio.h>
#include <stdlib.h>

/* Non-inlined function containing complex dependency patterns */
__attribute__((noinline, noclone))
int compute_loop(int* arr, float* farr, double* darr, int size) {
    int i;
    int temp_int = 0;
    float temp_float = 1.0f;
    double temp_double = 2.0;
    int* ptr = arr;
    volatile int vol_result = 0;  /* Prevent dead code elimination */
    
    /* Complex loop with multiple carried dependencies */
    for (i = 1; i < size; i++) {
        /* 1. TRUE DEPENDENCY (RAW) with integer */
        int prev = arr[i-1];              /* Read */
        arr[i] = prev + i + temp_int;     /* Write depending on previous read */
        temp_int = arr[i] & 0xFF;         /* Anti-dependency on arr[i] */
        
        /* 2. ANTI-DEPENDENCY (WAR) with float */
        float fprev = farr[i];            /* Read before write */
        farr[i] = fprev * 1.5f + temp_float; /* Write to same location */
        temp_float = farr[i] * 0.8f;      /* Use result */
        
        /* 3. OUTPUT DEPENDENCY (WAW) with double */
        darr[i] = temp_double * i;        /* First write */
        darr[i] = darr[i] + darr[i-1];    /* Second write to same location */
        temp_double = darr[i] / 2.0;      /* Read after write */
        
        /* 4. MEMORY ALIASING with pointers (ambiguous dependencies) */
        *ptr = *ptr + 1;                  /* Pointer may alias with arrays */
        ptr = &arr[i % 4];                /* Change pointer target */
        
        /* 5. MIXED TYPE OPERATIONS creating different latency edges */
        arr[i] = arr[i] + (int)(farr[i] * 2.0f);  /* Float->int conversion */
        farr[i] = (float)(arr[i] % 17) + 0.5f;    /* Int->float conversion */
        
        /* 6. CONTROL DEPENDENCY simulation */
        if (arr[i] > 1000) {
            temp_int = temp_int / 2;      /* Creates control flow */
        }
        
        /* Accumulate for result verification */
        vol_result += arr[i] + (int)farr[i];
    }
    
    return vol_result + (int)temp_double;
}

/* Another function with nested loops for more complex DDG */
__attribute__((noinline, noclone))
void nested_loop_deps(int* matrix, int rows, int cols) {
    int i, j;
    volatile int check = 0;
    
    /* Nested loop with carried dependencies in both dimensions */
    for (i = 1; i < rows; i++) {
        for (j = 1; j < cols; j++) {
            /* 2D true dependency (depends on previous row and column) */
            int up = matrix[(i-1)*cols + j];
            int left = matrix[i*cols + (j-1)];
            
            /* Multiple writes creating output dependencies */
            matrix[i*cols + j] = up + left;
            matrix[i*cols + j] = matrix[i*cols + j] * 2;  /* WAW */
            
            /* Anti-dependency through temporary */
            int temp = matrix[i*cols + j];                /* Read */
            matrix[i*cols + j] = temp + i + j;            /* Write to same */
            
            check += matrix[i*cols + j];
        }
    }
    
    /* Use volatile to prevent optimization */
    *(volatile int*)&check = check;
}

int main() {
    const int SIZE = 256;
    const int ROWS = 64;
    const int COLS = 64;
    
    /* Allocate and initialize arrays with different data types */
    int* int_array = (int*)malloc(SIZE * sizeof(int));
    float* float_array = (float*)malloc(SIZE * sizeof(float));
    double* double_array = (double*)malloc(SIZE * sizeof(double));
    int* matrix = (int*)malloc(ROWS * COLS * sizeof(int));
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i % 37;
        float_array[i] = (float)i * 0.5f;
        double_array[i] = (double)i * 0.25;
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        matrix[i] = i % 19;
    }
    
    /* Call functions with complex dependency patterns */
    int result1 = compute_loop(int_array, float_array, double_array, SIZE);
    nested_loop_deps(matrix, ROWS, COLS);
    
    /* Compute verification result (prevents dead code elimination) */
    int final_check = result1;
    for (int i = 0; i < SIZE; i++) {
        final_check += int_array[i] % 7;
    }
    for (int i = 0; i < ROWS * COLS; i += 16) {
        final_check += matrix[i] % 11;
    }
    
    printf("Result: %d\n", final_check);
    
    /* Cleanup */
    free(int_array);
    free(float_array);
    free(double_array);
    free(matrix);
    
    return 0;
}
