/* test_ddg.c - Test program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 64

/* Prevent aggressive optimization */
static volatile int sink;

/* Simple PRNG to generate data without external dependencies */
static unsigned int seed = 12345;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Flow (TRUE_DEP) dependency: accumulator pattern */
void test_flow_dep(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];  /* Flow dependency on 'sum' across iterations */
    }
    sink = sum;  /* Prevent dead code elimination */
}

/* Anti (ANTI_DEP) dependency: read then write pattern */
void test_anti_dep(int *src, int *dst, int n) {
    int temp;
    for (int i = 0; i < n; i++) {
        temp = src[i];      /* Read from src[i] */
        dst[i] = temp + 1;  /* Write to dst[i] - anti-dep if temp reused */
    }
    sink = dst[n-1];
}

/* Output (OUTPUT_DEP) dependency: multiple writes to same variable */
void test_output_dep(int *arr, int *result, int n) {
    int temp;
    for (int i = 0; i < n; i++) {
        temp = arr[i] * 2;  /* Computation */
        *result = temp;     /* Output dependency on *result across iterations */
    }
    sink = *result;
}

/* Control (CONTROL_DEP) dependency: conditional inside loop */
void test_control_dep(int *a, int *b, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > 0) {     /* Control dependency on a[i] */
            b[i] = 1;
        } else {
            b[i] = 0;
        }
    }
    sink = b[n-1];
}

/* Multiple flow dependencies with different distances */
void test_multi_flow(int *arr, int n) {
    int sum1 = 0, sum2 = 0;
    for (int i = 2; i < n; i++) {
        sum1 += arr[i];     /* Flow dep distance 1 */
        sum2 += arr[i-2];   /* Flow dep distance 2 */
    }
    sink = sum1 + sum2;
}

/* Nested loops with inner loop dependencies */
void test_nested_loops(int matrix[M][N], int *total) {
    for (int j = 0; j < M; j++) {
        int acc = 0;
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];  /* Flow dependency in inner loop */
        }
        total[j] = acc;
    }
    sink = total[M-1];
}

/* Complex pattern with mixed dependencies */
void test_mixed_deps(int *a, int *b, int *c, int n) {
    int prev = a[0];
    for (int i = 1; i < n; i++) {
        int curr = a[i];          /* Anti-dep through 'prev' reuse */
        b[i] = prev + curr;       /* Flow dep on 'prev' */
        c[i-1] = b[i] * 2;        /* Flow dep on 'b[i]' with distance */
        prev = curr;              /* Output dep on 'prev' */
    }
    sink = c[n-2];
}

/* Initialize arrays with pseudo-random data */
void init_arrays(int *arr1, int *arr2, int *arr3, int n) {
    for (int i = 0; i < n; i++) {
        arr1[i] = (int)(lcg_rand() % 100);
        arr2[i] = (int)(lcg_rand() % 100);
        arr3[i] = (int)(lcg_rand() % 100);
    }
}

int main(int argc, char **argv) {
    /* Allocate arrays */
    int *arr1 = malloc(N * sizeof(int));
    int *arr2 = malloc(N * sizeof(int));
    int *arr3 = malloc(N * sizeof(int));
    int result = 0;
    
    /* Initialize with random data */
    init_arrays(arr1, arr2, arr3, N);
    
    /* Test different dependency patterns */
    printf("Testing DDG edge creation patterns...\n");
    
    /* Select test based on command line argument if provided */
    int test_num = (argc > 1) ? atoi(argv[1]) : 0;
    
    switch (test_num) {
        case 1:
            test_anti_dep(arr1, arr2, N);
            break;
        case 2:
            test_output_dep(arr1, &result, N);
            break;
        case 3:
            test_control_dep(arr1, arr2, N);
            break;
        case 4:
            test_multi_flow(arr1, N);
            break;
        case 5:
            test_mixed_deps(arr1, arr2, arr3, N);
            break;
        case 6: {
            int matrix[M][N];
            int total[M];
            for (int j = 0; j < M; j++) {
                for (int i = 0; i < N; i++) {
                    matrix[j][i] = (int)(lcg_rand() % 100);
                }
            }
            test_nested_loops(matrix, total);
            break;
        }
        default:  /* Run all tests */
            test_flow_dep(arr1, N);
            test_anti_dep(arr1, arr2, N);
            test_output_dep(arr1, &result, N);
            test_control_dep(arr1, arr2, N);
            test_multi_flow(arr1, N);
            test_mixed_deps(arr1, arr2, arr3, N);
            break;
    }
    
    printf("Tests completed.\n");
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    
    return 0;
}
