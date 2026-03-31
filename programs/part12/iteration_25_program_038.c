#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define INNER_SIZE 128

/* Prevent optimization and create dependencies */
volatile int global_seed = 42;

/* Noinline functions to prevent optimization */
__attribute__((noinline)) int use_value(int x) {
    return x ^ global_seed;
}

__attribute__((noinline)) void modify_value(int *x) {
    *x += global_seed;
}

/* Test 1: Flow dependency (RAW) with carried dependency */
__attribute__((noinline)) int test_flow_dependency(int *arr) {
    int sum = 0;
    /* Create flow dependency: sum from previous iteration used in current */
    for (int i = 0; i < SIZE; i++) {
        sum += arr[i];
        arr[i] = sum;  /* Write depends on previous read+write of sum */
    }
    return sum;
}

/* Test 2: Anti-dependency (WAR) */
__attribute__((noinline)) int test_anti_dependency(int *a, int *b) {
    int temp = 0;
    for (int i = 0; i < SIZE; i++) {
        temp = a[i];      /* Read a[i] */
        a[i] = b[i];      /* Overwrite a[i] - anti-dependency with previous read */
        b[i] = temp;      /* Write b[i] */
    }
    return a[SIZE-1] + b[SIZE-1];
}

/* Test 3: Output dependency (WAW) */
__attribute__((noinline)) int test_output_dependency(int *arr) {
    int result = 0;
    for (int i = 0; i < SIZE; i++) {
        arr[i] = i;               /* First write */
        arr[i] = arr[i] * 2;      /* Second write to same location - output dependency */
        arr[i] = arr[i] + 1;      /* Third write to same location */
        result += arr[i];
    }
    return result;
}

/* Test 4: Nested loops with cross-iteration dependencies */
__attribute__((noinline)) int test_nested_dependency(int (*matrix)[INNER_SIZE]) {
    int total = 0;
    /* Outer loop dependency carried to inner loop */
    for (int i = 1; i < SIZE/INNER_SIZE; i++) {
        for (int j = 0; j < INNER_SIZE; j++) {
            /* Flow dependency across outer loop iterations */
            matrix[i][j] = matrix[i-1][j] + matrix[i][j];
            total += matrix[i][j];
        }
    }
    return total;
}

/* Test 5: Mixed data types and operations */
__attribute__((noinline)) float test_mixed_types(float *farr, double *darr) {
    float fsum = 0.0f;
    double dsum = 0.0;
    
    for (int i = 1; i < SIZE; i++) {
        /* Flow dependency with different data types */
        farr[i] = farr[i-1] * 1.5f + farr[i];
        darr[i] = darr[i-1] / 2.0 + darr[i];
        
        /* Anti-dependency between different types */
        float ftemp = farr[i];
        double dtemp = darr[i];
        
        farr[i] = (float)dtemp;
        darr[i] = (double)ftemp;
        
        fsum += farr[i];
        dsum += darr[i];
    }
    return fsum + (float)dsum;
}

/* Test 6: Control flow with dependencies */
__attribute__((noinline)) int test_control_flow_dependency(int *arr, int *brr) {
    int sum = 0;
    for (int i = 1; i < SIZE; i++) {
        if (i % 3 == 0) {
            /* Flow dependency in one branch */
            arr[i] = arr[i-1] + brr[i];
            sum += arr[i];
        } else if (i % 3 == 1) {
            /* Anti-dependency in another branch */
            int temp = brr[i];
            brr[i] = arr[i];
            arr[i] = temp;
            sum += temp;
        } else {
            /* Output dependency in third branch */
            arr[i] = i;
            arr[i] = arr[i] * arr[i-1];
            sum += arr[i];
        }
    }
    return sum;
}

/* Test 7: Pointer aliasing creating complex dependencies */
__attribute__((noinline)) int test_pointer_aliasing(int *arr, int *alias1, int *alias2) {
    int sum = 0;
    /* Create potential aliasing */
    alias1 = arr + 1;
    alias2 = arr + 2;
    
    for (int i = 2; i < SIZE - 2; i++) {
        /* Multiple dependencies through potentially aliased pointers */
        arr[i] = *alias1 + *alias2;  /* Flow from previous iteration via aliases */
        sum += arr[i];
        
        /* Update aliases - creating anti-dependencies */
        *alias1 = arr[i] + 1;
        *alias2 = arr[i] + 2;
        
        /* Move aliases */
        alias1++;
        alias2++;
    }
    return sum;
}

/* Test 8: Reduction with multiple dependency types */
__attribute__((noinline)) int test_reduction_mixed(int *arr) {
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    for (int i = 0; i < SIZE; i++) {
        /* Three parallel reductions with dependencies */
        sum1 += arr[i];           /* Flow in sum1 */
        sum2 = sum1 + sum2;       /* Flow from sum1 to sum2 */
        arr[i] = sum2;            /* Output dependency on arr[i] */
        sum3 = arr[i] + sum3;     /* Flow from arr[i] to sum3 */
        
        /* Anti-dependency through function call */
        modify_value(&arr[i]);
    }
    return sum1 + sum2 + sum3;
}

int main() {
    /* Initialize arrays with different patterns */
    int *arr1 = (int*)malloc(SIZE * sizeof(int));
    int *arr2 = (int*)malloc(SIZE * sizeof(int));
    float *farr = (float*)malloc(SIZE * sizeof(float));
    double *darr = (double*)malloc(SIZE * sizeof(double));
    int (*matrix)[INNER_SIZE] = (int(*)[INNER_SIZE])malloc((SIZE/INNER_SIZE) * INNER_SIZE * sizeof(int));
    
    srand(time(NULL));
    
    /* Initialize with random values */
    for (int i = 0; i < SIZE; i++) {
        arr1[i] = rand() % 100;
        arr2[i] = rand() % 100;
        farr[i] = (float)(rand() % 100) / 10.0f;
        darr[i] = (double)(rand() % 100) / 10.0;
    }
    
    for (int i = 0; i < SIZE/INNER_SIZE; i++) {
        for (int j = 0; j < INNER_SIZE; j++) {
            matrix[i][j] = rand() % 50;
        }
    }
    
    int total_result = 0;
    
    /* Execute all tests to trigger various DDG edge creations */
    total_result += test_flow_dependency(arr1);
    total_result += test_anti_dependency(arr1, arr2);
    total_result += test_output_dependency(arr1);
    total_result += test_nested_dependency(matrix);
    
    float float_result = test_mixed_types(farr, darr);
    total_result += (int)float_result;
    
    total_result += test_control_flow_dependency(arr1, arr2);
    total_result += test_pointer_aliasing(arr1, arr1 + 1, arr1 + 2);
    total_result += test_reduction_mixed(arr1);
    
    /* Use results to prevent dead code elimination */
    printf("Final checksum: %d\n", total_result);
    printf("Sample values: arr1[0]=%d, arr2[0]=%d\n", arr1[0], arr2[0]);
    printf("Float sample: farr[0]=%.2f, Double sample: darr[0]=%.2f\n", farr[0], darr[0]);
    
    free(arr1);
    free(arr2);
    free(farr);
    free(darr);
    free(matrix);
    
    return total_result != 0 ? 0 : 1;
}
