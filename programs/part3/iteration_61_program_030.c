/* ddg_test.c - Test program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 128

/* Simple PRNG to generate data without external dependencies */
static unsigned int seed = 12345;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Flow dependency (TRUE_DEP) - classic accumulator pattern */
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
        dst[i] = temp + 1;  /* Write to dst[i] - anti-dep on temp reuse */
    }
}

/* Output dependency (OUTPUT_DEP) - multiple writes to same variable */
int test_output_dep(int *arr, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        int t = arr[i] * 2;  /* Computation */
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

/* Mixed dependencies in nested loops */
void test_nested_mixed(int matrix[M][N], int total[M]) {
    for (int j = 0; j < M; j++) {
        int acc = 0;
        /* Inner loop with flow dependency */
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];  /* Flow dep on acc */
        }
        total[j] = acc;
        
        /* Anti-dependency in same inner loop */
        int prev = matrix[j][0];
        for (int i = 1; i < N; i++) {
            int curr = matrix[j][i];
            matrix[j][i-1] = prev + curr;  /* Anti-dep through prev */
            prev = curr;
        }
    }
}

/* Loop with distance > 1 for vectorization/scheduling analysis */
void test_distance_dep(int *a, int *b, int n) {
    for (int i = 4; i < n; i++) {
        /* Flow dependency with distance 4 */
        a[i] = a[i-4] + b[i];
    }
}

/* Complex pattern with multiple dependency types */
int test_complex_pattern(int *arr1, int *arr2, int n) {
    int sum1 = 0, sum2 = 0;
    int temp[N];
    
    /* First loop: flow and anti dependencies */
    for (int i = 0; i < n; i++) {
        int x = arr1[i];
        temp[i] = x * 2;      /* Anti-dep if x reused? Actually separate var */
        sum1 += temp[i];      /* Flow dep on sum1 */
    }
    
    /* Second loop: control and output dependencies */
    for (int i = 0; i < n; i++) {
        if (temp[i] > 100) {  /* Control dep */
            arr2[i] = sum1;   /* Output dep through multiple writes? */
        } else {
            arr2[i] = sum2;
            sum2++;           /* Flow dep on sum2 */
        }
    }
    
    return sum1 + sum2;
}

/* Volatile variables to prevent over-optimization */
void test_volatile_deps(void) {
    volatile int v1 = 0, v2 = 0;
    int arr[10];
    
    for (int i = 0; i < 10; i++) {
        arr[i] = v1;      /* Read volatile */
        v2 = arr[i] + 1;  /* Write volatile - creates edges */
    }
}

/* Main driver that runs all tests */
int main(int argc, char **argv) {
    /* Allocate and initialize data */
    int *arr1 = (int*)malloc(N * sizeof(int));
    int *arr2 = (int*)malloc(N * sizeof(int));
    int *arr3 = (int*)malloc(N * sizeof(int));
    int matrix[M][N];
    int total[M];
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < N; i++) {
        arr1[i] = (int)(lcg_rand() % 100);
        arr2[i] = (int)(lcg_rand() % 100);
        arr3[i] = 0;
    }
    
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = (int)(lcg_rand() % 100);
        }
    }
    
    int test_to_run = 0;
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    /* Run selected test or all tests */
    switch (test_to_run) {
        case 0: /* Run all tests */
        case 1:
            printf("Test 1 - Flow dependencies: %d\n", 
                   test_flow_dep(arr1, N));
        if (test_to_run != 0) break;
        
        case 2:
            test_anti_dep(arr1, arr2, N);
            printf("Test 2 - Anti dependencies completed\n");
        if (test_to_run != 0) break;
        
        case 3:
            printf("Test 3 - Output dependencies: %d\n",
                   test_output_dep(arr1, N));
        if (test_to_run != 0) break;
        
        case 4:
            test_control_dep(arr1, arr3, N);
            printf("Test 4 - Control dependencies completed\n");
        if (test_to_run != 0) break;
        
        case 5:
            test_nested_mixed(matrix, total);
            printf("Test 5 - Nested mixed dependencies completed\n");
        if (test_to_run != 0) break;
        
        case 6:
            test_distance_dep(arr1, arr2, N);
            printf("Test 6 - Distance dependencies completed\n");
        if (test_to_run != 0) break;
        
        case 7:
            printf("Test 7 - Complex pattern: %d\n",
                   test_complex_pattern(arr1, arr2, N));
        if (test_to_run != 0) break;
        
        case 8:
            test_volatile_deps();
            printf("Test 8 - Volatile dependencies completed\n");
        if (test_to_run != 0) break;
        
        default:
            printf("Running all dependency tests...\n");
            test_flow_dep(arr1, N);
            test_anti_dep(arr1, arr2, N);
            test_output_dep(arr1, N);
            test_control_dep(arr1, arr3, N);
            test_nested_mixed(matrix, total);
            test_distance_dep(arr1, arr2, N);
            test_complex_pattern(arr1, arr2, N);
            test_volatile_deps();
            printf("All tests completed\n");
    }
    
    /* Use results to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += arr1[i] + arr2[i] + arr3[i];
    }
    for (int j = 0; j < M; j++) {
        checksum += total[j];
    }
    
    free(arr1);
    free(arr2);
    free(arr3);
    
    printf("Final checksum: %d\n", checksum);
    return 0;
}
