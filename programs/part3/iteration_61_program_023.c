/* ddg_test.c - Test program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 128

/* Simple PRNG to generate data without external dependencies */
static unsigned int seed = 12345;
static inline unsigned int rand_int(void) {
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
        dst[i] = temp + 1;  /* Anti-dependency if src and dst alias */
    }
}

/* Output dependency (OUTPUT_DEP) - multiple writes to same variable */
int test_output_dep(int *arr, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        int t = arr[i] * 2;
        result = t;  /* Output dependency on 'result' across iterations */
    }
    return result;
}

/* Control dependency (CONTROL_DEP) - conditional inside loop */
void test_control_dep(int *a, int *b, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > 0) {     /* Control dependency on a[i] */
            b[i] = 1;
        } else {
            b[i] = 0;
        }
    }
}

/* Multiple flow dependencies - more complex pattern */
int test_multi_flow(int *a, int *b, int n) {
    int sum_a = 0, sum_b = 0;
    for (int i = 0; i < n; i++) {
        sum_a += a[i];  /* Flow dep 1 */
        sum_b += b[i];  /* Flow dep 2 */
    }
    return sum_a + sum_b;
}

/* Nested loops with inner loop dependencies */
void test_nested_loops(int matrix[M][N], int total[M]) {
    for (int j = 0; j < M; j++) {
        int acc = 0;
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];  /* Flow dependency in inner loop */
        }
        total[j] = acc;
    }
}

/* Loop with both flow and anti dependencies */
void test_mixed_deps(int *a, int *b, int n) {
    int prev = a[0];
    for (int i = 1; i < n; i++) {
        b[i] = prev + a[i];   /* Flow dep on prev, anti dep on a[i] if a==b */
        prev = a[i];          /* Output dep on prev */
    }
}

/* Loop-carried dependency with distance > 1 */
int test_distance_dep(int *arr, int n) {
    int sum = 0;
    for (int i = 2; i < n; i++) {
        sum += arr[i] * arr[i-2];  /* Dependency distance of 2 */
    }
    return sum;
}

/* Initialize arrays with pseudo-random data */
void init_arrays(int *a, int *b, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (int)(rand_int() % 100);
        b[i] = (int)(rand_int() % 100);
    }
}

int main(int argc, char *argv[]) {
    /* Allocate and initialize test data */
    int *arr1 = (int *)malloc(N * sizeof(int));
    int *arr2 = (int *)malloc(N * sizeof(int));
    int matrix[M][N];
    int total[M];
    
    if (!arr1 || !arr2) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    init_arrays(arr1, arr2, N);
    
    /* Initialize matrix */
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = (int)(rand_int() % 100);
        }
    }
    
    int test_to_run = 0;
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    /* Run selected test or all tests */
    switch (test_to_run) {
        case 0:
            printf("Running all DDG dependency tests...\n");
            /* Fall through to run all */
        case 1:
            printf("Test 1 - Flow dependency: %d\n", 
                   test_flow_dep(arr1, N));
            if (test_to_run > 0) break;
        case 2:
            test_anti_dep(arr1, arr2, N);
            printf("Test 2 - Anti dependency completed\n");
            if (test_to_run > 0) break;
        case 3:
            printf("Test 3 - Output dependency: %d\n",
                   test_output_dep(arr1, N));
            if (test_to_run > 0) break;
        case 4:
            test_control_dep(arr1, arr2, N);
            printf("Test 4 - Control dependency completed\n");
            if (test_to_run > 0) break;
        case 5:
            printf("Test 5 - Multiple flow dependencies: %d\n",
                   test_multi_flow(arr1, arr2, N));
            if (test_to_run > 0) break;
        case 6:
            test_nested_loops(matrix, total);
            printf("Test 6 - Nested loops completed\n");
            if (test_to_run > 0) break;
        case 7:
            test_mixed_deps(arr1, arr2, N);
            printf("Test 7 - Mixed dependencies completed\n");
            if (test_to_run > 0) break;
        case 8:
            printf("Test 8 - Distance dependency: %d\n",
                   test_distance_dep(arr1, N));
            if (test_to_run > 0) break;
        default:
            printf("Unknown test number\n");
    }
    
    /* Prevent dead code elimination */
    volatile int dummy = arr1[0] + arr2[0];
    (void)dummy;
    
    free(arr1);
    free(arr2);
    
    return 0;
}
