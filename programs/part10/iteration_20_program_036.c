/* test_ddg.c - Program to trigger GCC's Data Dependency Graph edge creation */
#include <stdio.h>
#include <stdlib.h>

/* Non-inlined function containing complex dependency patterns */
__attribute__((noinline, noclone))
int compute_loop(int* arr, double* darr, float* farr, int size) {
    int i;
    int temp_int = 0;
    double temp_double = 0.0;
    float temp_float = 0.0f;
    int* ptr = arr;
    
    /* Complex loop with multiple carried dependencies */
    for (i = 1; i < size; i++) {
        /* 1. True Data Dependency (RAW) - Integer chain */
        int prev = arr[i-1];              /* Read */
        arr[i] = prev + i + temp_int;     /* Write depending on previous read */
        temp_int = arr[i] & 0xFF;         /* Another RAW dependency */
        
        /* 2. Anti-dependency (WAR) - Floating point */
        double old_val = darr[i];         /* Read */
        darr[i] = old_val * 1.5 + i;      /* Write to same location */
        
        /* 3. Output Dependency (WAW) - Mixed types */
        farr[i] = temp_float + i;         /* Write 1 */
        farr[i] = farr[i] * 0.5f;         /* Write 2 to same location (WAW) */
        
        /* 4. Memory aliasing dependencies */
        *ptr = *ptr + 1;                  /* Pointer aliasing with arr */
        ptr = &arr[i % 4];                /* Change pointer target */
        
        /* 5. Cross-type dependencies */
        temp_double = (double)arr[i] / 2.0; /* Integer to float conversion */
        temp_float = (float)temp_double * 2.0f;
        
        /* 6. Complex expression with multiple dependencies */
        arr[(i + 1) % size] = (arr[i] * 3 + arr[i-1]) / 2;
        
        /* 7. Control-like dependency through conditional */
        if (arr[i] > 1000) {
            temp_int = arr[i] - 1000;
        } else {
            temp_int = arr[i] + 1000;
        }
    }
    
    /* Final computation that depends on all iterations */
    int result = arr[size-1] + (int)temp_double + (int)temp_float;
    return result;
}

/* Another function with nested loops for additional DDG complexity */
__attribute__((noinline, noclone))
void nested_loop_deps(int* matrix, int rows, int cols) {
    int i, j;
    
    /* Nested loops with carried dependencies in both dimensions */
    for (i = 1; i < rows; i++) {
        for (j = 1; j < cols; j++) {
            /* 2D stencil computation with multiple dependencies */
            int top = matrix[(i-1)*cols + j];      /* RAW from previous row */
            int left = matrix[i*cols + (j-1)];     /* RAW from previous column */
            int diag = matrix[(i-1)*cols + (j-1)]; /* RAW from diagonal */
            
            /* Multiple writes creating WAW and WAR */
            int temp = matrix[i*cols + j];         /* Read current (WAR source) */
            matrix[i*cols + j] = (top + left + diag) / 3; /* Write 1 (WAR dest) */
            matrix[i*cols + j] += temp * 2;        /* Write 2 (WAW) */
            
            /* Cross-iteration dependency through accumulation */
            matrix[0] += matrix[i*cols + j] % 7;   /* Output dependency on element 0 */
        }
    }
}

/* Function with pointer chasing creating memory dependencies */
__attribute__((noinline, noclone))
int pointer_chase_deps(int* data, int size) {
    int sum = 0;
    int* current = data;
    int* next = NULL;
    
    for (int i = 0; i < size - 1; i++) {
        /* Pointer-based dependencies */
        next = current + 1;                /* Pointer arithmetic */
        int val = *current;                /* Read through pointer */
        *next = val + *next + i;           /* Write creating RAW through pointer */
        sum += *current;                   /* Read again (WAR potential) */
        
        /* Complex addressing with potential aliasing */
        data[i % 8] = data[(i+1) % 8] + data[(i+2) % 8];
        
        current = next;                    /* Move pointer */
    }
    
    return sum + *current;
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
    
    /* Initialize with non-zero values */
    for (int i = 0; i < SIZE; i++) {
        int_array[i] = i + 1;
        double_array[i] = (double)i * 0.5;
        float_array[i] = (float)i * 0.25f;
    }
    
    for (int i = 0; i < ROWS * COLS; i++) {
        matrix[i] = i % 100;
    }
    
    /* Volatile to prevent optimization of results */
    volatile int result1, result2, result3;
    
    /* Execute functions with complex dependencies */
    result1 = compute_loop(int_array, double_array, float_array, SIZE);
    nested_loop_deps(matrix, ROWS, COLS);
    result2 = pointer_chase_deps(int_array, SIZE);
    
    /* Final computation using all results */
    result3 = result1 + result2 + matrix[ROWS*COLS - 1];
    
    /* Print results to prevent dead code elimination */
    printf("Results: %d, %d, %d\n", result1, result2, result3);
    
    /* Cleanup */
    free(int_array);
    free(double_array);
    free(float_array);
    free(matrix);
    
    return 0;
}
