/* test_ddg.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 128

/* Simple PRNG to generate data without external dependencies */
static unsigned int seed = 12345;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return (unsigned int)(seed / 65536) % 32768;
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
        dst[i] = temp + 1;  /* Write to dst[i] - anti-dep on 'temp' if reused */
    }
}

/* Output dependency (OUTPUT_DEP) - multiple writes to same variable */
void test_output_dep(int *arr, int *result, int n) {
    int temp;
    for (int i = 0; i < n; i++) {
        temp = arr[i] * 2;  /* Computation */
        *result = temp;     /* Output dependency on '*result' across iterations */
    }
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

/* Complex pattern with multiple dependency types */
void test_mixed_deps(int *a, int *b, int *c, int n) {
    int acc = 0;
    int prev = 0;
    
    for (int i = 0; i < n; i++) {
        /* Flow dependency on 'acc' */
        acc += a[i];
        
        /* Anti-dependency: read b[i], then write to c[i] */
        int temp = b[i];
        c[i] = temp + acc;
        
        /* Output-like pattern */
        prev = c[i] % 256;
        
        /* Control dependency */
        if (acc > 1000) {
            c[i] = 0;
        }
    }
}

/* Prevent optimization by using volatile */
volatile int sink;

int main(int argc, char **argv) {
    /* Allocate and initialize arrays */
    int *arr1 = (int*)malloc(N * sizeof(int));
    int *arr2 = (int*)malloc(N * sizeof(int));
    int *arr3 = (int*)malloc(N * sizeof(int));
    int matrix[M][N];
    int total[M];
    
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
    
    /* Execute all test patterns */
    int result;
    
    /* Flow dependency test */
    result = test_flow_dep(arr1, N);
    sink = result;  /* Prevent dead code elimination */
    
    /* Anti-dependency test */
    test_anti_dep(arr1, arr2, N);
    sink = arr2[N-1];
    
    /* Output dependency test */
    test_output_dep(arr1, &result, N);
    sink = result;
    
    /* Control dependency test */
    test_control_dep(arr1, arr3, N);
    sink = arr3[N-1];
    
    /* Nested loops test */
    test_nested_loops(matrix, total);
    sink = total[M-1];
    
    /* Mixed dependencies test */
    test_mixed_deps(arr1, arr2, arr3, N);
    sink = arr3[N-1];
    
    /* Clean up */
    free(arr1);
    free(arr2);
    free(arr3);
    
    return 0;
}
