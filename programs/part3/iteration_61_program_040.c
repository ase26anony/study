/* test_ddg.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 128

/* Prevent aggressive optimization */
static volatile int sink;

/* Flow dependency (TRUE_DEP) - classic accumulator */
int flow_dep(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];  /* Flow dependency on sum across iterations */
    }
    sink = sum; /* Prevent dead code elimination */
    return sum;
}

/* Anti-dependency (ANTI_DEP) - read then write pattern */
void anti_dep(int *src, int *dst, int n) {
    int temp;
    for (int i = 0; i < n; i++) {
        temp = src[i];      /* Read src[i] */
        dst[i] = temp + 1;  /* Write to dst[i] - anti-dep on temp reuse */
    }
    sink = dst[n-1];
}

/* Output dependency (OUTPUT_DEP) - multiple writes to same variable */
int output_dep(int *arr, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        int t = arr[i] * 2;  /* Computation */
        result = t;          /* Output dependency on result across iterations */
    }
    sink = result;
    return result;
}

/* Control dependency (CONTROL_DEP) - conditional inside loop */
void control_dep(int *a, int *b, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > 0) {      /* Control dependency on a[i] */
            b[i] = 1;
        } else {
            b[i] = 0;
        }
    }
    sink = b[n-1];
}

/* Mixed dependencies - complex pattern */
void mixed_deps(int *a, int *b, int *c, int n) {
    int acc = 0;
    for (int i = 1; i < n; i++) {
        /* Flow dep on acc */
        acc += a[i];
        
        /* Anti-dep: read b[i-1], then write b[i] */
        int tmp = b[i-1];
        b[i] = tmp + acc;
        
        /* Control dep on acc */
        if (acc > 100) {
            c[i] = 1;
        }
    }
    sink = acc + b[n-1] + c[n-1];
}

/* Nested loops with inner loop dependencies */
void nested_loop_deps(int matrix[M][N], int *total) {
    for (int j = 0; j < M; j++) {
        int acc = 0;
        /* Inner loop with flow dependency */
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];
        }
        total[j] = acc;
        
        /* Anti-dependency in second inner loop */
        for (int i = 1; i < N; i++) {
            int tmp = matrix[j][i-1];
            matrix[j][i] = tmp + 1;
        }
    }
    sink = total[M-1];
}

/* Loop with carried dependency and distance > 1 */
void distance_dep(int *a, int *b, int n) {
    for (int i = 4; i < n; i++) {
        /* Flow dependency with distance 4 */
        b[i] = a[i] + b[i-4];
    }
    sink = b[n-1];
}

/* Initialize arrays with pseudo-random values */
void init_array(int *arr, int n, int seed) {
    unsigned int r = seed;
    for (int i = 0; i < n; i++) {
        r = r * 1103515245 + 12345;
        arr[i] = (r >> 16) & 0x7FFF;
    }
}

/* Initialize matrix */
void init_matrix(int matrix[M][N], int seed) {
    unsigned int r = seed;
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            r = r * 1103515245 + 12345;
            matrix[j][i] = (r >> 16) & 0xFF;
        }
    }
}

int main(int argc, char *argv[]) {
    /* Allocate arrays */
    int *arr1 = (int *)malloc(N * sizeof(int));
    int *arr2 = (int *)malloc(N * sizeof(int));
    int *arr3 = (int *)malloc(N * sizeof(int));
    int *arr4 = (int *)malloc(N * sizeof(int));
    int matrix[M][N];
    int total[M];
    
    /* Initialize data */
    init_array(arr1, N, 1);
    init_array(arr2, N, 2);
    init_array(arr3, N, 3);
    init_array(arr4, N, 4);
    init_matrix(matrix, 5);
    
    /* Run all dependency patterns */
    printf("Testing DDG edge creation patterns...\n");
    
    /* Select test based on command line or run all */
    int test_all = (argc > 1) ? atoi(argv[1]) : 1;
    
    if (test_all == 1 || test_all == 0) {
        printf("1. Flow dependency test...\n");
        int sum = flow_dep(arr1, N);
        printf("   Result: %d\n", sum);
    }
    
    if (test_all == 2 || test_all == 0) {
        printf("2. Anti-dependency test...\n");
        anti_dep(arr1, arr2, N);
        printf("   Completed\n");
    }
    
    if (test_all == 3 || test_all == 0) {
        printf("3. Output dependency test...\n");
        int res = output_dep(arr1, N);
        printf("   Result: %d\n", res);
    }
    
    if (test_all == 4 || test_all == 0) {
        printf("4. Control dependency test...\n");
        control_dep(arr1, arr3, N);
        printf("   Completed\n");
    }
    
    if (test_all == 5 || test_all == 0) {
        printf("5. Mixed dependencies test...\n");
        mixed_deps(arr1, arr2, arr3, N);
        printf("   Completed\n");
    }
    
    if (test_all == 6 || test_all == 0) {
        printf("6. Nested loops test...\n");
        nested_loop_deps(matrix, total);
        printf("   Completed\n");
    }
    
    if (test_all == 7 || test_all == 0) {
        printf("7. Distance > 1 dependency test...\n");
        distance_dep(arr1, arr4, N);
        printf("   Completed\n");
    }
    
    printf("All tests completed.\n");
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    
    return 0;
}
