/* ddg_test.c - Test program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 128

/* Simple PRNG to generate data without external dependencies */
static unsigned int seed = 12345;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return (seed >> 16) & 0x7FFF;
}

/* Flow (TRUE_DEP) dependency: accumulator pattern */
int test_flow_dep(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];  /* Flow dependency on 'sum' across iterations */
    }
    return sum;
}

/* Anti-dependency: read then write to same location */
void test_anti_dep(int *src, int *dst, int n) {
    int temp;
    for (int i = 0; i < n; i++) {
        temp = src[i];      /* Read from src[i] */
        dst[i] = temp + 1;  /* Write to dst[i] - anti-dep if src==dst */
    }
}

/* Output dependency: multiple writes to same variable */
int test_output_dep(int *arr, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        int computed = arr[i] * 2;
        result = computed;  /* Output dependency on 'result' */
    }
    return result;
}

/* Control dependency: conditional inside loop */
void test_control_dep(int *a, int *b, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > 0) {     /* Control dependency on a[i] */
            b[i] = 1;
        } else {
            b[i] = 0;
        }
    }
}

/* Mixed dependencies in nested loops */
int test_nested_mixed(int matrix[M][N]) {
    int total = 0;
    for (int j = 0; j < M; j++) {
        int row_sum = 0;
        /* Inner loop with flow dependency */
        for (int i = 0; i < N; i++) {
            row_sum += matrix[j][i];
        }
        total += row_sum;  /* Flow dependency on total */
    }
    return total;
}

/* Complex loop with multiple dependency types */
void test_complex_deps(int *a, int *b, int *c, int n) {
    int acc = 0;
    for (int i = 1; i < n; i++) {
        /* Flow: acc depends on previous iteration */
        acc = acc + a[i];
        
        /* Anti: read b[i-1], then write to b[i] */
        int prev = b[i-1];
        b[i] = prev + acc;
        
        /* Control: depends on computed value */
        if (acc > 100) {
            c[i] = 1;
        } else {
            c[i] = 0;
        }
    }
}

/* Prevent optimization by using volatile */
volatile int vol_counter = 0;

/* Main driver that runs all tests */
int main(int argc, char **argv) {
    /* Allocate and initialize arrays */
    int *arr1 = (int*)malloc(N * sizeof(int));
    int *arr2 = (int*)malloc(N * sizeof(int));
    int *arr3 = (int*)malloc(N * sizeof(int));
    int matrix[M][N];
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < N; i++) {
        arr1[i] = lcg_rand() % 100;
        arr2[i] = lcg_rand() % 100;
        arr3[i] = 0;
    }
    
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = lcg_rand() % 50;
        }
    }
    
    /* Run tests based on command line argument or all */
    int test_to_run = (argc > 1) ? atoi(argv[1]) : 0;
    
    switch (test_to_run) {
        case 1:
            printf("Flow dep result: %d\n", test_flow_dep(arr1, N));
            break;
        case 2:
            test_anti_dep(arr1, arr2, N);
            printf("Anti-dep completed\n");
            break;
        case 3:
            printf("Output dep result: %d\n", test_output_dep(arr1, N));
            break;
        case 4:
            test_control_dep(arr1, arr3, N);
            printf("Control dep completed\n");
            break;
        case 5:
            printf("Nested mixed result: %d\n", test_nested_mixed(matrix));
            break;
        case 6:
            test_complex_deps(arr1, arr2, arr3, N);
            printf("Complex deps completed\n");
            break;
        default:
            /* Run all tests */
            int sum1 = test_flow_dep(arr1, N);
            test_anti_dep(arr1, arr2, N);
            int sum2 = test_output_dep(arr1, N);
            test_control_dep(arr1, arr3, N);
            int sum3 = test_nested_mixed(matrix);
            test_complex_deps(arr1, arr2, arr3, N);
            
            /* Use results to prevent dead code elimination */
            vol_counter = sum1 + sum2 + sum3;
            printf("All tests completed (volatile: %d)\n", vol_counter);
    }
    
    free(arr1);
    free(arr2);
    free(arr3);
    
    return 0;
}
