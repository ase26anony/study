/* test_ddg.c - Test program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 128

/* Simple PRNG to generate data without external dependencies */
static unsigned int seed = 12345;
static inline unsigned int simple_rand(void) {
    seed = seed * 1103515245 + 12345;
    return (seed >> 16) & 0x7FFF;
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
        dst[i] = temp + 1;  /* Anti-dependency if 'temp' reused (though compiler may optimize) */
        
        /* Force anti-dependency by reusing variable */
        src[i] = dst[i] * 2; /* Write to src[i] after reading it earlier */
    }
}

/* Output dependency (OUTPUT_DEP) - multiple writes to same location */
int test_output_dep(int *arr, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        int computed = arr[i] * 2 + 1;
        result = computed;  /* Output dependency on 'result' across iterations */
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

/* Mixed dependencies in single loop */
void test_mixed_deps(int *a, int *b, int *c, int n) {
    int acc = 0;
    for (int i = 0; i < n; i++) {
        /* Flow dependency on acc */
        acc += a[i];
        
        /* Anti-dependency on b[i] */
        int temp = b[i];
        b[i] = acc + temp;
        
        /* Control dependency */
        if (acc > 1000) {
            c[i] = 1;
        } else {
            c[i] = 0;
        }
    }
}

/* Nested loops with inner loop dependencies */
int test_nested_loops(int matrix[M][N]) {
    int total[M];
    for (int j = 0; j < M; j++) {
        int acc = 0;
        /* Inner loop with flow dependency */
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];
        }
        total[j] = acc;
    }
    
    /* Sum results */
    int final_sum = 0;
    for (int j = 0; j < M; j++) {
        final_sum += total[j];
    }
    return final_sum;
}

/* Loop with carried dependency and distance > 0 */
void test_distance_dep(int *a, int *b, int n) {
    for (int i = 2; i < n; i++) {
        /* Flow dependency with distance 2 */
        a[i] = a[i-2] + b[i];
    }
}

/* Prevent optimization by using volatile */
int test_volatile_dep(volatile int *arr, int n) {
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Initialize arrays with pseudo-random data */
void init_arrays(int *arr1, int *arr2, int *arr3, int n) {
    for (int i = 0; i < n; i++) {
        arr1[i] = (int)simple_rand() % 100;
        arr2[i] = (int)simple_rand() % 100;
        arr3[i] = 0;
    }
}

/* Initialize matrix */
void init_matrix(int matrix[M][N]) {
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = (int)simple_rand() % 50;
        }
    }
}

int main(int argc, char *argv[]) {
    /* Allocate arrays on heap to avoid stack overflow */
    int *arr1 = (int*)malloc(N * sizeof(int));
    int *arr2 = (int*)malloc(N * sizeof(int));
    int *arr3 = (int*)malloc(N * sizeof(int));
    int matrix[M][N];
    
    if (!arr1 || !arr2 || !arr3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    init_arrays(arr1, arr2, arr3, N);
    init_matrix(matrix);
    
    printf("Testing DDG edge creation patterns...\n");
    
    /* Test 1: Flow dependency */
    int sum1 = test_flow_dep(arr1, N);
    printf("Flow dependency test: sum = %d\n", sum1);
    
    /* Test 2: Anti-dependency */
    test_anti_dep(arr1, arr2, N);
    printf("Anti-dependency test completed\n");
    
    /* Test 3: Output dependency */
    int result = test_output_dep(arr1, N);
    printf("Output dependency test: result = %d\n", result);
    
    /* Test 4: Control dependency */
    test_control_dep(arr1, arr3, N);
    printf("Control dependency test completed\n");
    
    /* Test 5: Mixed dependencies */
    test_mixed_deps(arr1, arr2, arr3, N);
    printf("Mixed dependencies test completed\n");
    
    /* Test 6: Nested loops */
    int matrix_sum = test_nested_loops(matrix);
    printf("Nested loops test: matrix sum = %d\n", matrix_sum);
    
    /* Test 7: Distance dependency */
    test_distance_dep(arr1, arr2, N);
    printf("Distance dependency test completed\n");
    
    /* Test 8: Volatile dependency */
    int volatile_sum = test_volatile_dep(arr1, N);
    printf("Volatile dependency test: sum = %d\n", volatile_sum);
    
    /* Clean up */
    free(arr1);
    free(arr2);
    free(arr3);
    
    printf("All DDG tests completed successfully\n");
    return 0;
}
