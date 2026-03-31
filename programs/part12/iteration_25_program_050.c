/* test_ddg_coverage.c - Program to trigger DDG edge creation in GCC */

#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define INNER_SIZE 128

/* Prevent optimization of dependencies */
volatile int volatile_var = 0;
int global_sum = 0;

/* Functions with noinline to prevent inlining and preserve dependencies */
__attribute__((noinline)) int* get_array_ptr(int* arr, int i) {
    volatile_var++;
    return &arr[i];
}

__attribute__((noinline)) void modify_value(int* val) {
    *val += volatile_var;
}

/* Test 1: Flow dependency (RAW) with carried dependency across iterations */
__attribute__((noinline)) int test_flow_dependency(int* arr) {
    int sum = 0;
    
    /* Classic flow dependency: sum depends on previous iteration */
    for (int i = 0; i < SIZE; i++) {
        sum += arr[i];  /* Flow dependency on sum */
        arr[i] = sum;   /* Flow dependency on arr[i] from previous iteration */
    }
    
    /* Additional flow dependency with distance > 0 */
    int temp = 0;
    for (int i = 2; i < SIZE; i++) {
        arr[i] = arr[i-2] + arr[i-1];  /* Distance 1 and 2 flow dependencies */
        temp += arr[i];
    }
    
    return sum + temp;
}

/* Test 2: Anti-dependency (WAR) */
__attribute__((noinline)) int test_anti_dependency(int* arr, int* brr) {
    int result = 0;
    
    for (int i = 0; i < SIZE; i++) {
        int temp = arr[i];      /* Read arr[i] */
        arr[i] = brr[i] + i;    /* Write arr[i] - anti-dependency with previous read */
        brr[i] = temp * 2;      /* Write brr[i] */
        result += arr[i] + brr[i];
    }
    
    /* Anti-dependency within same iteration with control flow */
    for (int i = 1; i < SIZE - 1; i++) {
        int read_val = arr[i];
        if (i % 3 == 0) {
            arr[i] = read_val * 3;  /* Anti-dependency */
        } else {
            arr[i] = read_val / 2;  /* Anti-dependency */
        }
        result += arr[i];
    }
    
    return result;
}

/* Test 3: Output dependency (WAW) */
__attribute__((noinline)) int test_output_dependency(int* arr) {
    int result = 0;
    
    /* Multiple writes to same location */
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;              /* Write 1 */
        arr[i] = arr[i] * 2;     /* Write 2 - output dependency */
        arr[i] = arr[i] + 1;     /* Write 3 - output dependency */
        result += arr[i];
    }
    
    /* WAW with different conditions */
    for (int i = 0; i < SIZE; i++) {
        if (i % 2 == 0) {
            arr[i] = i * 10;     /* Write under condition */
        } else {
            arr[i] = i * 20;     /* Alternative write - output dependency */
        }
        
        /* Another write creating WAW */
        modify_value(&arr[i]);   /* Function call prevents optimization */
        result += arr[i];
    }
    
    return result;
}

/* Test 4: Nested loops with outer-loop dependencies */
__attribute__((noinline)) int test_nested_dependency(int arr[][INNER_SIZE]) {
    int total = 0;
    
    /* Nested loop with flow dependency on outer loop */
    for (int i = 1; i < SIZE/INNER_SIZE; i++) {
        for (int j = 0; j < INNER_SIZE; j++) {
            /* Flow dependency on arr[i-1][j] from previous outer iteration */
            arr[i][j] = arr[i-1][j] + (i * j);
            total += arr[i][j];
        }
    }
    
    /* More complex nested dependencies */
    for (int i = 0; i < SIZE/INNER_SIZE; i++) {
        int row_sum = 0;
        for (int j = 1; j < INNER_SIZE; j++) {
            /* Anti-dependency within inner loop */
            int temp = arr[i][j];
            arr[i][j] = arr[i][j-1] + temp;  /* Flow + anti dependencies */
            row_sum += arr[i][j];
        }
        total += row_sum;
    }
    
    return total;
}

/* Test 5: Mixed data types and operations */
__attribute__((noinline)) int test_mixed_types(float* farr, double* darr, int* iarr) {
    float fsum = 0.0f;
    double dsum = 0.0;
    int isum = 0;
    
    /* Different data types with dependencies */
    for (int i = 1; i < SIZE; i++) {
        /* Flow dependency with float */
        farr[i] = farr[i-1] * 1.1f + (float)i;
        fsum += farr[i];
        
        /* Anti-dependency with double */
        double old_val = darr[i];
        darr[i] = darr[i-1] / 2.0;
        dsum += darr[i] + old_val;
        
        /* Output dependency with int */
        iarr[i] = i * 2;
        iarr[i] = iarr[i] + (int)farr[i];  /* WAW with flow from float */
        isum += iarr[i];
    }
    
    return (int)(fsum + dsum) + isum;
}

/* Test 6: Complex control flow with dependencies */
__attribute__((noinline)) int test_control_flow_dependency(int* arr, int* brr) {
    int sum = 0;
    
    for (int i = 1; i < SIZE; i++) {
        /* Complex control flow creating different dependency patterns */
        if (arr[i-1] > 0) {
            /* Flow dependency through condition */
            brr[i] = arr[i-1] + brr[i-1];
            sum += brr[i];
        } else if (arr[i-1] < 0) {
            /* Alternative path with anti-dependency */
            int temp = brr[i];
            brr[i] = -arr[i-1];
            arr[i] = temp * 2;  /* Anti-dependency */
            sum += brr[i] + arr[i];
        } else {
            /* Output dependency path */
            brr[i] = 0;
            brr[i] = brr[i] + i;  /* WAW */
            sum += brr[i];
        }
        
        /* Always executed, creating flow dependency */
        arr[i] = sum % 1000;
    }
    
    return sum;
}

int main() {
    /* Initialize arrays with different patterns */
    int* arr1 = (int*)malloc(SIZE * sizeof(int));
    int* arr2 = (int*)malloc(SIZE * sizeof(int));
    int* arr3 = (int*)malloc(SIZE * sizeof(int));
    float* farr = (float*)malloc(SIZE * sizeof(float));
    double* darr = (double*)malloc(SIZE * sizeof(double));
    int* iarr = (int*)malloc(SIZE * sizeof(int));
    
    /* 2D array for nested test */
    int rows = SIZE / INNER_SIZE;
    int (*arr2d)[INNER_SIZE] = (int(*)[INNER_SIZE])malloc(rows * INNER_SIZE * sizeof(int));
    
    srand(time(NULL));
    
    /* Initialize with random values */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        arr3[i] = rand() % 100;
        farr[i] = (float)(rand() % 100) / 10.0f;
        darr[i] = (double)(rand() % 100) / 5.0;
        iarr[i] = rand() % 100;
    }
    
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < INNER_SIZE; j++) {
            arr2d[i][j] = rand() % 100;
        }
    }
    
    /* Run all tests to trigger DDG construction */
    int result = 0;
    
    result += test_flow_dependency(arr1);
    result += test_anti_dependency(arr2, arr3);
    result += test_output_dependency(arr1);
    result += test_nested_dependency(arr2d);
    result += test_mixed_types(farr, darr, iarr);
    result += test_control_flow_dependency(arr2, arr3);
    
    /* Use volatile to prevent optimization */
    volatile_var = result % 1000;
    
    /* Print result to prevent dead code elimination */
    printf("Final result: %d\n", result);
    printf("Volatile var: %d\n", volatile_var);
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(farr);
    free(darr);
    free(iarr);
    free(arr2d);
    
    return 0;
}
