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

/* Flow dependency (TRUE_DEP) - accumulator pattern */
int test_flow_dep(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];  /* Flow dependency on 'sum' across iterations */
    }
    return sum;
}

/* Anti dependency (ANTI_DEP) - read then write pattern */
void test_anti_dep(int *src, int *dst, int n) {
    int temp;
    for (int i = 0; i < n; i++) {
        temp = src[i];      /* Read src[i] */
        dst[i] = temp + 1;  /* Write to dst[i] - anti dependency if temp reused */
        /* Force anti-dependency by reusing temp in next iteration */
        if (i < n - 1) {
            src[i + 1] = temp * 2;  /* Creates anti-dependency on src[i+1] */
        }
    }
}

/* Output dependency (OUTPUT_DEP) - multiple writes to same location */
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
        if (a[i] > 0) {  /* Control dependency on a[i] */
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
        /* Flow dependency on acc */
        acc = acc + a[i];
        
        /* Anti dependency on b[i] */
        int tmp = b[i];
        b[i - 1] = tmp * 2;
        
        /* Control dependency */
        if (acc > 100) {
            c[i] = acc;
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
        
        /* Additional anti-dependency in outer loop */
        if (j > 0) {
            total[j - 1] = acc / 2;
        }
    }
}

/* Loop with memory dependencies that prevent optimization */
void test_memory_deps(int *restrict a, int *restrict b, int *restrict c, int n) {
    for (int i = 1; i < n - 1; i++) {
        /* Flow dependency chain */
        a[i] = b[i - 1] + c[i];
        b[i] = a[i] * 2;
        c[i + 1] = b[i] - 1;
    }
}

/* Volatile variables to prevent dead code elimination */
static volatile int sink;

int main(int argc, char **argv) {
    /* Allocate and initialize arrays */
    int *arr1 = (int *)malloc(N * sizeof(int));
    int *arr2 = (int *)malloc(N * sizeof(int));
    int *arr3 = (int *)malloc(N * sizeof(int));
    int *arr4 = (int *)malloc(N * sizeof(int));
    
    int matrix[M][N];
    int total[M];
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < N; i++) {
        arr1[i] = (int)(lcg_rand() % 100);
        arr2[i] = (int)(lcg_rand() % 100);
        arr3[i] = (int)(lcg_rand() % 100);
        arr4[i] = (int)(lcg_rand() % 100);
    }
    
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = (int)(lcg_rand() % 50);
        }
    }
    
    /* Test different dependency patterns */
    int mode = (argc > 1) ? atoi(argv[1]) : 0;
    
    switch (mode) {
        case 0:
            /* Test all patterns */
            sink = test_flow_dep(arr1, N);
            test_anti_dep(arr1, arr2, N);
            sink = test_output_dep(arr3, N);
            test_control_dep(arr1, arr4, N);
            test_mixed_deps(arr1, arr2, arr3, N);
            test_nested_loops(matrix, total);
            test_memory_deps(arr1, arr2, arr3, N);
            break;
            
        case 1:
            /* Focus on flow dependencies */
            for (int iter = 0; iter < 10; iter++) {
                sink = test_flow_dep(arr1, N);
            }
            break;
            
        case 2:
            /* Focus on anti-dependencies */
            for (int iter = 0; iter < 10; iter++) {
                test_anti_dep(arr1, arr2, N);
            }
            break;
            
        case 3:
            /* Focus on control dependencies */
            for (int iter = 0; iter < 10; iter++) {
                test_control_dep(arr1, arr4, N);
            }
            break;
            
        case 4:
            /* Focus on nested loops */
            for (int iter = 0; iter < 5; iter++) {
                test_nested_loops(matrix, total);
            }
            break;
            
        default:
            /* Mixed intensive test */
            for (int iter = 0; iter < 100; iter++) {
                sink = test_flow_dep(arr1, N);
                test_anti_dep(arr1, arr2, N);
                test_mixed_deps(arr1, arr2, arr3, N);
            }
            break;
    }
    
    /* Use results to prevent dead code elimination */
    printf("Results: %d %d %d\n", sink, arr2[0], total[0]);
    
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    
    return 0;
}
