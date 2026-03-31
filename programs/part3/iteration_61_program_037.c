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

/* Flow dependency (TRUE_DEP) - accumulator pattern */
int test_flow_dep(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];  /* Flow dependency on 'sum' across iterations */
    }
    return sum;
}

/* Anti-dependency (ANTI_DEP) - read then write pattern */
void test_anti_dep(int *src, int *dst, int n) {
    int temp;
    for (int i = 0; i < n; i++) {
        temp = src[i];      /* Read from src[i] */
        dst[i] = temp + 1;  /* Write to dst[i] - anti-dep if temp reused in reg */
    }
}

/* Output dependency (OUTPUT_DEP) - multiple writes to same variable */
int test_output_dep(int *arr, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        int t = arr[i] * 2;  /* Different computation each iteration */
        result = t;          /* Output dependency on 'result' */
    }
    return result;
}

/* Control dependency (CONTROL_DEP) - conditional inside loop */
void test_control_dep(int *a, int *b, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > 0) {      /* Control dependency on a[i] */
            b[i] = 1;
        } else {
            b[i] = 0;
        }
    }
}

/* Mixed dependencies - complex pattern */
void test_mixed_deps(int *a, int *b, int *c, int n) {
    int acc = 0;
    for (int i = 1; i < n; i++) {
        /* Flow dep on acc, anti dep on b[i-1] */
        acc = acc + a[i];
        b[i] = acc;
        
        /* Control dep on acc */
        if (acc > 100) {
            c[i] = b[i-1];  /* Flow dep on b[i-1] */
        } else {
            c[i] = 0;
        }
    }
}

/* Nested loops with inner loop dependencies */
void test_nested_loops(int matrix[M][N], int total[M]) {
    for (int j = 0; j < M; j++) {
        int acc = 0;
        /* Inner loop with flow dependency */
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];
        }
        total[j] = acc;
    }
}

/* Loop with pointer aliasing (creates memory dependencies) */
void test_pointer_aliasing(int *p, int *q, int n) {
    for (int i = 0; i < n - 1; i++) {
        p[i] = q[i + 1] + p[i];  /* Potential flow and anti dependencies */
    }
}

/* Main driver that runs all tests */
int main(int argc, char **argv) {
    /* Allocate and initialize data */
    int *arr1 = (int *)malloc(N * sizeof(int));
    int *arr2 = (int *)malloc(N * sizeof(int));
    int *arr3 = (int *)malloc(N * sizeof(int));
    int matrix[M][N];
    int total[M];
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < N; i++) {
        arr1[i] = (int)lcg_rand() % 100;
        arr2[i] = 0;
        arr3[i] = 0;
    }
    
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = (int)lcg_rand() % 50;
        }
    }
    
    /* Run tests based on command line or all by default */
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
            test_control_dep(arr1, arr2, N);
            printf("Control dep completed\n");
            break;
        case 5:
            test_mixed_deps(arr1, arr2, arr3, N);
            printf("Mixed deps completed\n");
            break;
        case 6:
            test_nested_loops(matrix, total);
            printf("Nested loops completed\n");
            break;
        case 7:
            test_pointer_aliasing(arr1, arr2, N);
            printf("Pointer aliasing completed\n");
            break;
        default:
            /* Run all tests */
            printf("Running all DDG tests...\n");
            printf("Flow dep: %d\n", test_flow_dep(arr1, N));
            test_anti_dep(arr1, arr2, N);
            printf("Output dep: %d\n", test_output_dep(arr1, N));
            test_control_dep(arr1, arr3, N);
            test_mixed_deps(arr1, arr2, arr3, N/2);
            test_nested_loops(matrix, total);
            test_pointer_aliasing(arr1, arr2, N);
            printf("All tests completed\n");
    }
    
    /* Use results to prevent dead code elimination */
    volatile int sink = arr1[0] + arr2[0] + arr3[0] + total[0];
    (void)sink;
    
    free(arr1);
    free(arr2);
    free(arr3);
    
    return 0;
}
