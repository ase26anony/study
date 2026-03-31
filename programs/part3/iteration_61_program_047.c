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

/* Flow dependency (TRUE_DEP) - classic accumulator pattern */
int flow_dependency(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];  /* Flow dependency on 'sum' across iterations */
    }
    return sum;
}

/* Anti-dependency (ANTI_DEP) - read then write pattern */
void anti_dependency(int *src, int *dst, int n) {
    int temp;
    for (int i = 0; i < n; i++) {
        temp = src[i];      /* Read from src[i] */
        dst[i] = temp + 1;  /* Write to dst[i] - anti-dependency if temp reused */
        
        /* Force anti-dependency by reusing temp in next iteration */
        if (i < n - 1) {
            src[i + 1] = temp * 2;  /* Write to src[i+1] after reading src[i] */
        }
    }
}

/* Output dependency (OUTPUT_DEP) - multiple writes to same location */
int output_dependency(int *arr, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        int computed = arr[i] * 2 + 1;
        result = computed;  /* Output dependency on 'result' across iterations */
    }
    return result;
}

/* Control dependency (CONTROL_DEP) - conditional inside loop */
void control_dependency(int *a, int *b, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > 0) {      /* Control dependency on a[i] */
            b[i] = 1;
        } else {
            b[i] = 0;
        }
    }
}

/* Mixed dependencies - combines multiple dependency types */
int mixed_dependencies(int *a, int *b, int *c, int n) {
    int acc = 0;
    int temp;
    
    for (int i = 0; i < n; i++) {
        /* Flow dependency on acc */
        acc += a[i];
        
        /* Anti-dependency through temp */
        temp = b[i];
        c[i] = temp * 2;
        
        /* Control dependency */
        if (acc > 1000) {
            c[i] = 0;
        }
        
        /* Potential output dependency through function return */
    }
    return acc;
}

/* Nested loops with inner loop dependencies */
void nested_loop_dependencies(int matrix[M][N], int *col_sums, int *row_sums) {
    for (int j = 0; j < M; j++) {
        int row_acc = 0;
        
        /* Inner loop with flow dependency */
        for (int i = 0; i < N; i++) {
            row_acc += matrix[j][i];  /* Flow dependency on row_acc */
        }
        row_sums[j] = row_acc;
    }
    
    for (int i = 0; i < N; i++) {
        int col_acc = 0;
        
        /* Another inner loop with different access pattern */
        for (int j = 0; j < M; j++) {
            col_acc += matrix[j][i];  /* Flow dependency on col_acc */
        }
        col_sums[i] = col_acc;
    }
}

/* Loop with distance > 0 dependencies for software pipelining */
void distance_vector_dependencies(int *a, int *b, int n) {
    /* Loop-carried dependency with distance 2 */
    for (int i = 2; i < n; i++) {
        a[i] = a[i - 2] + b[i];  /* Flow dependency with distance 2 */
    }
}

/* Main test driver */
int main(int argc, char **argv) {
    /* Allocate and initialize arrays */
    int *arr1 = (int *)malloc(N * sizeof(int));
    int *arr2 = (int *)malloc(N * sizeof(int));
    int *arr3 = (int *)malloc(N * sizeof(int));
    int matrix[M][N];
    int col_sums[N];
    int row_sums[M];
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < N; i++) {
        arr1[i] = (int)lcg_rand() % 100;
        arr2[i] = (int)lcg_rand() % 100;
        arr3[i] = 0;
    }
    
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = (int)lcg_rand() % 50;
        }
    }
    
    /* Run all dependency tests */
    int test_num = (argc > 1) ? atoi(argv[1]) : 0;
    
    switch (test_num) {
        case 0:
            /* Run all tests */
            printf("Running all dependency tests...\n");
            printf("Flow dependency result: %d\n", flow_dependency(arr1, N));
            anti_dependency(arr1, arr3, N);
            printf("Output dependency result: %d\n", output_dependency(arr2, N));
            control_dependency(arr1, arr3, N);
            printf("Mixed dependencies result: %d\n", mixed_dependencies(arr1, arr2, arr3, N));
            nested_loop_dependencies(matrix, col_sums, row_sums);
            distance_vector_dependencies(arr1, arr2, N);
            break;
            
        case 1:
            printf("Flow dependency test\n");
            printf("Result: %d\n", flow_dependency(arr1, N));
            break;
            
        case 2:
            printf("Anti-dependency test\n");
            anti_dependency(arr1, arr3, N);
            break;
            
        case 3:
            printf("Output dependency test\n");
            printf("Result: %d\n", output_dependency(arr2, N));
            break;
            
        case 4:
            printf("Control dependency test\n");
            control_dependency(arr1, arr3, N);
            break;
            
        case 5:
            printf("Mixed dependencies test\n");
            printf("Result: %d\n", mixed_dependencies(arr1, arr2, arr3, N));
            break;
            
        case 6:
            printf("Nested loop dependencies test\n");
            nested_loop_dependencies(matrix, col_sums, row_sums);
            break;
            
        case 7:
            printf("Distance vector dependencies test\n");
            distance_vector_dependencies(arr1, arr2, N);
            break;
            
        default:
            printf("Invalid test number. Use 0-7\n");
            break;
    }
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    
    return 0;
}
