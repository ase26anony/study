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
        temp = src[i];      /* Read src[i] */
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

/* Mixed dependencies - more complex pattern */
void test_mixed_deps(int *a, int *b, int *c, int size) {
    int acc = 0;
    for (int i = 1; i < size; i++) {
        /* Flow dep on acc, anti-dep on b[i-1] */
        acc = acc + a[i];
        b[i-1] = acc;       /* Write after read in next iteration */
        c[i] = b[i-1] * 2;  /* Flow dep on b[i-1] */
    }
}

/* Nested loops with inner loop dependencies */
void test_nested_loops(int matrix[M][N], int *totals) {
    for (int j = 0; j < M; j++) {
        int acc = 0;
        /* Inner loop with flow dependency */
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];
        }
        totals[j] = acc;
        
        /* Second inner loop with anti-dependency */
        int prev = matrix[j][0];
        for (int i = 1; i < N; i++) {
            int curr = matrix[j][i];
            matrix[j][i-1] = prev * 2;  /* Write after read in next iteration */
            prev = curr;
        }
        matrix[j][N-1] = prev * 2;
    }
}

/* Loop with pointer aliasing to create ambiguous dependencies */
void test_pointer_aliasing(int *a, int *b, int *c, int size) {
    for (int i = 1; i < size; i++) {
        a[i] = b[i-1] + c[i];   /* Flow dep on b[i-1] */
        b[i] = a[i] * 2;        /* Flow dep on a[i] */
    }
}

/* Prevent optimization by using volatile */
int test_volatile_dep(volatile int *arr, int size) {
    volatile int sum = 0;
    for (volatile int i = 0; i < size; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Initialize arrays with pseudo-random data */
void init_arrays(int *a, int *b, int *c, int size) {
    for (int i = 0; i < size; i++) {
        a[i] = (int)(lcg_rand() % 100);
        b[i] = (int)(lcg_rand() % 100);
        c[i] = (int)(lcg_rand() % 100);
    }
}

void init_matrix(int matrix[M][N]) {
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = (int)(lcg_rand() % 100);
        }
    }
}

int main(int argc, char *argv[]) {
    /* Allocate arrays on heap to avoid stack overflow */
    int *arr1 = (int*)malloc(N * sizeof(int));
    int *arr2 = (int*)malloc(N * sizeof(int));
    int *arr3 = (int*)malloc(N * sizeof(int));
    int *results = (int*)malloc(M * sizeof(int));
    int (*matrix)[N] = (int(*)[N])malloc(M * N * sizeof(int));
    
    if (!arr1 || !arr2 || !arr3 || !results || !matrix) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    init_arrays(arr1, arr2, arr3, N);
    init_matrix(matrix);
    
    /* Run all dependency tests */
    printf("Testing various dependency patterns for DDG edge creation...\n");
    
    /* Test 1: Flow dependencies */
    int sum1 = test_flow_dep(arr1, N);
    printf("Flow dependency test result: %d\n", sum1);
    
    /* Test 2: Anti-dependencies */
    test_anti_dep(arr1, arr2, N);
    printf("Anti-dependency test completed\n");
    
    /* Test 3: Output dependencies */
    int sum2 = test_output_dep(arr1, N);
    printf("Output dependency test result: %d\n", sum2);
    
    /* Test 4: Control dependencies */
    test_control_dep(arr1, arr2, N);
    printf("Control dependency test completed\n");
    
    /* Test 5: Mixed dependencies */
    test_mixed_deps(arr1, arr2, arr3, N);
    printf("Mixed dependencies test completed\n");
    
    /* Test 6: Nested loops */
    test_nested_loops(matrix, results);
    printf("Nested loops test completed\n");
    
    /* Test 7: Pointer aliasing */
    test_pointer_aliasing(arr1, arr2, arr3, N);
    printf("Pointer aliasing test completed\n");
    
    /* Test 8: Volatile dependencies */
    int sum3 = test_volatile_dep(arr1, N);
    printf("Volatile dependency test result: %d\n", sum3);
    
    /* Clean up */
    free(arr1);
    free(arr2);
    free(arr3);
    free(results);
    free(matrix);
    
    printf("All tests completed successfully\n");
    return 0;
}
