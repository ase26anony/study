/* ddg_test.c - Test program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 64

/* Simple PRNG to generate data without external dependencies */
static unsigned int seed = 12345;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Initialize arrays with pseudo-random data */
static void init_data(int *arr, int size) {
    for (int i = 0; i < size; i++) {
        arr[i] = (int)(lcg_rand() % 100);
    }
}

/* FLOW DEPENDENCY (TRUE_DEP): Classic accumulator pattern */
int test_flow_dep(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];  /* Flow dependency on 'sum' across iterations */
    }
    return sum;
}

/* ANTI DEPENDENCY: Read then write pattern */
void test_anti_dep(int *src, int *dst, int n) {
    int temp;
    for (int i = 0; i < n; i++) {
        temp = src[i];      /* Read from src[i] */
        dst[i] = temp + 1;  /* Anti dependency if src and dst alias */
    }
}

/* OUTPUT DEPENDENCY: Multiple writes to same variable */
int test_output_dep(int *arr, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        int t = arr[i] * 2;
        result = t;  /* Output dependency on 'result' across iterations */
    }
    return result;
}

/* CONTROL DEPENDENCY: Conditional inside loop */
void test_control_dep(int *a, int *b, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > 50) {   /* Control dependency on a[i] */
            b[i] = 1;
        } else {
            b[i] = 0;
        }
    }
}

/* MULTIPLE DEPENDENCIES: Combined flow and anti dependencies */
int test_mixed_deps(int *a, int *b, int n) {
    int acc = 0;
    int temp;
    
    for (int i = 0; i < n; i++) {
        temp = a[i];        /* Anti dependency potential */
        acc = acc + temp;   /* Flow dependency on 'acc' */
        b[i] = acc;         /* Flow dependency on 'acc' */
    }
    return acc;
}

/* NESTED LOOPS with inner loop dependencies */
void test_nested_loops(int matrix[M][N], int *total) {
    for (int j = 0; j < M; j++) {
        int acc = 0;
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];  /* Flow dependency in inner loop */
        }
        total[j] = acc;
    }
}

/* LOOP-CARRIED DEPENDENCY with distance > 1 */
void test_distance_dep(int *src, int *dst, int n, int distance) {
    for (int i = distance; i < n; i++) {
        dst[i] = src[i - distance] + dst[i - 1];  /* Multiple dependencies */
    }
}

/* VOLATILE variables to prevent optimization */
int test_volatile_dep(volatile int *arr, int n) {
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Main driver that exercises all patterns */
int main(int argc, char **argv) {
    /* Allocate and initialize data */
    int *array1 = (int *)malloc(N * sizeof(int));
    int *array2 = (int *)malloc(N * sizeof(int));
    int *array3 = (int *)malloc(N * sizeof(int));
    int matrix[M][N];
    int totals[M];
    
    if (!array1 || !array2 || !array3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_data(array1, N);
    init_data(array2, N);
    init_data(array3, N);
    
    /* Initialize matrix */
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = (int)(lcg_rand() % 100);
        }
    }
    
    int test_id = (argc > 1) ? atoi(argv[1]) : 0;
    int result = 0;
    
    /* Execute selected test or all tests */
    switch (test_id) {
        case 0:  /* Run all tests */
        case 1:
            result = test_flow_dep(array1, N);
            printf("Flow dep result: %d\n", result);
            if (test_id != 0) break;
            
        case 2:
            test_anti_dep(array1, array2, N);
            printf("Anti dep completed\n");
            if (test_id != 0) break;
            
        case 3:
            result = test_output_dep(array1, N);
            printf("Output dep result: %d\n", result);
            if (test_id != 0) break;
            
        case 4:
            test_control_dep(array1, array2, N);
            printf("Control dep completed\n");
            if (test_id != 0) break;
            
        case 5:
            result = test_mixed_deps(array1, array2, N);
            printf("Mixed deps result: %d\n", result);
            if (test_id != 0) break;
            
        case 6:
            test_nested_loops(matrix, totals);
            printf("Nested loops completed\n");
            if (test_id != 0) break;
            
        case 7:
            test_distance_dep(array1, array2, N, 3);
            printf("Distance dep completed\n");
            if (test_id != 0) break;
            
        case 8:
            result = test_volatile_dep(array3, N);
            printf("Volatile dep result: %d\n", result);
            if (test_id != 0) break;
            
        default:
            printf("Invalid test ID\n");
    }
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}
