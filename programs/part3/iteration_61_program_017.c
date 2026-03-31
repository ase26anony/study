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
int test_flow_dep(int *arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];  /* Flow dependency on 'sum' across iterations */
    }
    return sum;
}

/* Anti-dependency (ANTI_DEP) - read then write pattern */
void test_anti_dep(int *src, int *dst, int size) {
    int temp;
    for (int i = 0; i < size; i++) {
        temp = src[i];      /* Read from src[i] */
        dst[i] = temp + 1;  /* Write to dst[i] - anti-dep on temp if reused */
    }
}

/* Output dependency (OUTPUT_DEP) - multiple writes to same variable */
int test_output_dep(int *arr, int size) {
    int result = 0;
    for (int i = 0; i < size; i++) {
        int computed = arr[i] * 2;
        result = computed;  /* Output dependency on 'result' across iterations */
    }
    return result;
}

/* Control dependency (CONTROL_DEP) - conditional inside loop */
void test_control_dep(int *a, int *b, int size) {
    for (int i = 0; i < size; i++) {
        if (a[i] > 0) {     /* Control dependency on a[i] */
            b[i] = 1;
        } else {
            b[i] = 0;
        }
    }
}

/* Mixed dependencies in single loop */
int test_mixed_deps(int *a, int *b, int size) {
    int acc = 0;
    int prev = 0;
    
    for (int i = 0; i < size; i++) {
        int temp = a[i];          /* Anti-dep potential if temp reused */
        acc += temp;              /* Flow dep on acc */
        b[i] = prev;              /* Flow dep on prev (carried across iterations) */
        prev = temp;              /* Output dep on prev */
        if (acc > 1000) {         /* Control dep on acc */
            b[i] = 0;
        }
    }
    return acc;
}

/* Nested loops with inner loop dependencies */
void test_nested_loops(int matrix[M][N], int *totals) {
    for (int j = 0; j < M; j++) {
        int acc = 0;
        /* Inner loop with flow dependency */
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];  /* Flow dependency on acc */
        }
        totals[j] = acc;
        
        /* Second inner loop with anti-dependency */
        for (int i = 1; i < N; i++) {
            int temp = matrix[j][i-1];  /* Read */
            matrix[j][i] = temp + 1;    /* Write - anti-dep */
        }
    }
}

/* Loop with pointer aliasing (creates ambiguous dependencies) */
void test_pointer_aliasing(int *a, int *b, int *c, int size) {
    for (int i = 1; i < size - 1; i++) {
        a[i] = b[i-1] + c[i+1];  /* Multiple flow dependencies */
        b[i] = a[i] * 2;         /* Anti-dep on a[i] if a and b alias */
    }
}

/* Prevent optimization with volatile */
volatile int sink;

int main(int argc, char **argv) {
    /* Allocate and initialize data */
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
    
    /* Test different dependency patterns */
    int total = 0;
    
    /* Flow dependency test */
    total += test_flow_dep(arr1, N);
    
    /* Anti-dependency test */
    test_anti_dep(arr1, arr2, N);
    
    /* Output dependency test */
    total += test_output_dep(arr2, N);
    
    /* Control dependency test */
    test_control_dep(arr1, arr3, N);
    
    /* Mixed dependencies test */
    total += test_mixed_deps(arr1, arr2, N);
    
    /* Nested loops test */
    int totals[M];
    test_nested_loops(matrix, totals);
    
    /* Pointer aliasing test */
    test_pointer_aliasing(arr1, arr2, arr3, N);
    
    /* Use results to prevent dead code elimination */
    sink = total + totals[0] + arr3[N-1];
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    
    printf("DDG test completed (sink = %d)\n", sink);
    return 0;
}
