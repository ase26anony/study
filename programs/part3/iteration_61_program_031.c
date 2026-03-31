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

/* Anti-dependency (ANTI_DEP) - read then write pattern */
void test_anti_dep(int *src, int *dst, int n) {
    int temp;
    for (int i = 0; i < n; i++) {
        temp = src[i];      /* Read from src[i] */
        dst[i] = temp + 1;  /* Write to dst[i] - anti-dep on 'temp' if reused */
    }
}

/* Output dependency (OUTPUT_DEP) - multiple writes to same variable */
void test_output_dep(int *result, int n) {
    int temp;
    for (int i = 0; i < n; i++) {
        temp = i * i;       /* Compute value */
        result[i] = temp;   /* Output dependency if 'result' is same location */
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
int test_nested_loops(int matrix[M][N]) {
    int total[M];
    int grand_total = 0;
    
    for (int j = 0; j < M; j++) {
        int acc = 0;
        /* Inner loop with flow dependency */
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];  /* Flow dependency on 'acc' */
        }
        total[j] = acc;
        grand_total += acc;       /* Another flow dependency */
    }
    return grand_total;
}

/* Mixed dependencies in single loop */
void test_mixed_deps(int *a, int *b, int *c, int n) {
    int accum = 0;
    int prev = 0;
    
    for (int i = 0; i < n; i++) {
        /* Flow dependency on accum */
        accum += a[i];
        
        /* Anti-dependency: read b[i], then write to c[i] */
        int temp = b[i];
        c[i] = temp + accum;
        
        /* Control dependency */
        if (accum > prev) {
            a[i] = accum;
        }
        prev = accum;
    }
}

/* Prevent optimization by using volatile */
volatile int vol_counter = 0;

/* Main test driver */
int main(int argc, char **argv) {
    /* Allocate and initialize arrays */
    int *arr1 = (int *)malloc(N * sizeof(int));
    int *arr2 = (int *)malloc(N * sizeof(int));
    int *arr3 = (int *)malloc(N * sizeof(int));
    int matrix[M][N];
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < N; i++) {
        arr1[i] = (int)(lcg_rand() % 100);
        arr2[i] = (int)(lcg_rand() % 100);
        arr3[i] = 0;
    }
    
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = (int)(lcg_rand() % 50);
        }
    }
    
    /* Run all dependency tests */
    int result;
    
    result = test_flow_dep(arr1, N);
    vol_counter = result;  /* Use volatile to prevent dead code elimination */
    
    test_anti_dep(arr1, arr2, N);
    vol_counter = arr2[N-1];
    
    test_output_dep(arr3, N);
    vol_counter = arr3[N/2];
    
    test_control_dep(arr1, arr3, N);
    vol_counter = arr3[0];
    
    result = test_nested_loops(matrix);
    vol_counter = result;
    
    test_mixed_deps(arr1, arr2, arr3, N);
    vol_counter = arr1[N-1] + arr3[N-1];
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    
    printf("DDG test completed. vol_counter = %d\n", vol_counter);
    return 0;
}
