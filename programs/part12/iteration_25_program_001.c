#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define INNER_SIZE 128

/* Prevent optimization and create dependencies */
volatile int global_seed = 42;

/* Functions to prevent optimization */
__attribute__((noinline)) 
int use_value(int val) {
    return val ^ global_seed;
}

__attribute__((noinline))
void modify_array(int* arr, int idx, int val) {
    arr[idx] = val ^ global_seed;
}

/* Test 1: Flow dependency (RAW) with carried dependency */
__attribute__((noinline))
int test_flow_dependency(int* arr, int n) {
    int sum = arr[0];
    volatile int temp = sum;  /* Prevent optimization */
    
    for (int i = 1; i < n; i++) {
        /* Flow dependency: sum from previous iteration used in current */
        sum = sum + arr[i];
        /* Anti-dependency: arr[i] read, then modified through pointer */
        modify_array(arr, i, sum);
    }
    
    return use_value(sum);
}

/* Test 2: Anti-dependency (WAR) within same iteration */
__attribute__((noinline))
int test_anti_dependency(int* arr, int n) {
    int result = 0;
    
    for (int i = 0; i < n - 1; i++) {
        int read_val = arr[i];          /* Read arr[i] */
        arr[i] = arr[i + 1] * 2;        /* Overwrite arr[i] - anti-dependency */
        result += read_val + arr[i];    /* Use both values */
    }
    
    return use_value(result);
}

/* Test 3: Output dependency (WAW) and mixed operations */
__attribute__((noinline))
int test_output_dependency(float* farr, int* iarr, int n) {
    float accum = 0.0f;
    
    for (int i = 0; i < n; i++) {
        /* Multiple writes to same location - output dependency */
        farr[i] = (float)iarr[i] * 1.5f;
        farr[i] = farr[i] * 2.0f;       /* Second write to farr[i] */
        
        /* Flow dependency through accum */
        accum += farr[i];
        
        /* Third write with dependency on accum */
        farr[i] = accum / (i + 1);
    }
    
    return (int)accum;
}

/* Test 4: Nested loops with cross-iteration dependencies */
__attribute__((noinline))
int test_nested_dependency(int** matrix, int rows, int cols) {
    int total = 0;
    
    for (int i = 1; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            /* Flow dependency on previous row */
            matrix[i][j] = matrix[i-1][j] + matrix[i][j] * 3;
            
            /* Anti-dependency within same row */
            int temp = matrix[i][j];
            matrix[i][j] = (temp * 2) % 100;
            total += temp;
        }
    }
    
    return use_value(total);
}

/* Test 5: Control flow with dependencies */
__attribute__((noinline))
int test_control_flow_dependency(int* arr, int n) {
    int sum_even = 0;
    int sum_odd = arr[0];
    
    for (int i = 1; i < n; i++) {
        if (i % 2 == 0) {
            /* Flow dependency on sum_even */
            sum_even = sum_even + arr[i];
            /* Output dependency on arr[i] */
            arr[i] = sum_even;
        } else {
            /* Flow dependency on sum_odd */
            sum_odd = sum_odd * 2 + arr[i];
            /* Anti-dependency: read arr[i], then modify */
            int old = arr[i];
            arr[i] = sum_odd % 256;
            sum_odd += old;
        }
    }
    
    return use_value(sum_even + sum_odd);
}

/* Test 6: Complex dependency chain with different data types */
__attribute__((noinline))
int test_mixed_dependencies(double* darr, int* iarr, char* carr, int n) {
    double dsum = 0.0;
    int isum = 0;
    
    for (int i = 1; i < n; i++) {
        /* Flow dependency through dsum with type conversion */
        dsum += (double)iarr[i-1] * 0.5;
        
        /* Output dependency on darr[i] */
        darr[i] = dsum;
        darr[i] = darr[i] * 1.1;  /* Second write */
        
        /* Anti-dependency on iarr */
        int temp = iarr[i];
        iarr[i] = (int)darr[i] + temp;
        
        /* Flow dependency through isum */
        isum += iarr[i] * carr[i];
        
        /* Complex dependency chain */
        carr[i] = (char)((isum + temp) % 128);
    }
    
    return (int)dsum + isum;
}

int main() {
    /* Initialize data with some values */
    int* arr1 = (int*)malloc(SIZE * sizeof(int));
    int* arr2 = (int*)malloc(SIZE * sizeof(int));
    float* farr = (float*)malloc(SIZE * sizeof(float));
    double* darr = (double*)malloc(SIZE * sizeof(double));
    char* carr = (char*)malloc(SIZE * sizeof(char));
    
    /* Create a matrix for nested test */
    int rows = 64, cols = 64;
    int** matrix = (int**)malloc(rows * sizeof(int*));
    for (int i = 0; i < rows; i++) {
        matrix[i] = (int*)malloc(cols * sizeof(int));
        for (int j = 0; j < cols; j++) {
            matrix[i][j] = (i * cols + j) % 100;
        }
    }
    
    /* Initialize arrays */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        farr[i] = (float)(rand() % 1000) / 10.0f;
        darr[i] = (double)(rand() % 1000) / 10.0;
        carr[i] = (char)(rand() % 128);
    }
    
    int result = 0;
    
    /* Execute all tests to trigger various DDG edge creations */
    result += test_flow_dependency(arr1, SIZE);
    result += test_anti_dependency(arr2, SIZE);
    result += test_output_dependency(farr, arr1, SIZE / 2);
    result += test_nested_dependency(matrix, rows, cols);
    result += test_control_flow_dependency(arr1, SIZE);
    result += test_mixed_dependencies(darr, arr2, carr, SIZE);
    
    /* Use results to prevent dead code elimination */
    printf("Final checksum: %d\n", result);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(farr);
    free(darr);
    free(carr);
    for (int i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);
    
    return 0;
}
