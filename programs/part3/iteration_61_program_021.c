/* test_ddg.c - Test program to trigger ddg_edge creation in GCC's DDG builder */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 128

/* Prevent aggressive optimization */
static volatile int sink;

/* Simple PRNG to generate data without external dependencies */
static unsigned int lcg_seed = 12345;
static inline unsigned int lcg_rand(void) {
    lcg_seed = lcg_seed * 1103515245 + 12345;
    return lcg_seed;
}

/* Flow dependency (TRUE_DEP) - classic accumulator pattern */
int test_flow_dep(int *arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];  /* Flow dependency on 'sum' across iterations */
    }
    sink = sum; /* Prevent dead code elimination */
    return sum;
}

/* Anti-dependency (ANTI_DEP) - read then write pattern */
void test_anti_dep(int *src, int *dst, int size) {
    int temp;
    for (int i = 0; i < size; i++) {
        temp = src[i];      /* Read from src[i] */
        dst[i] = temp + 1;  /* Write to dst[i] - anti-dep if temp reused */
        /* Force reuse of temp variable */
        if (dst[i] > 100) {
            temp = dst[i];  /* Another write to temp */
        }
    }
    sink = dst[size-1];
}

/* Output dependency (OUTPUT_DEP) - multiple writes to same variable */
int test_output_dep(int *arr, int size) {
    int result = 0;
    for (int i = 0; i < size; i++) {
        int t = arr[i] * 2;  /* Compute temporary */
        result = t;          /* Output dependency on 'result' across iterations */
        if (i % 2 == 0) {
            result += 1;     /* Another write to result */
        }
    }
    sink = result;
    return result;
}

/* Control dependency (CONTROL_DEP) - conditional inside loop */
void test_control_dep(int *a, int *b, int size) {
    for (int i = 0; i < size; i++) {
        if (a[i] > 0) {      /* Control dependency on a[i] */
            b[i] = 1;
        } else {
            b[i] = 0;
        }
    }
    sink = b[size-1];
}

/* Mixed dependencies in single loop */
void test_mixed_deps(int *a, int *b, int *c, int size) {
    int acc = 0;
    for (int i = 0; i < size; i++) {
        /* Flow dependency on acc */
        acc += a[i];
        
        /* Anti dependency through temp variable */
        int temp = b[i];
        c[i] = temp + acc;
        
        /* Control dependency */
        if (c[i] > 100) {
            acc -= 1;
        }
    }
    sink = acc;
}

/* Nested loops with inner loop dependencies */
void test_nested_loops(int matrix[M][N], int *total) {
    for (int j = 0; j < M; j++) {
        int acc = 0;
        /* Inner loop with flow dependency */
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];
        }
        total[j] = acc;
        
        /* Anti-dependency in second inner loop */
        int prev = total[j];
        for (int i = 0; i < N; i++) {
            int curr = matrix[j][i];
            matrix[j][i] = prev + curr;
            prev = curr;
        }
    }
    sink = total[M-1];
}

/* Loop with pointer-based dependencies */
void test_pointer_deps(int *arr, int size) {
    int *p = arr;
    int *end = arr + size;
    int sum = 0;
    
    while (p < end) {
        sum += *p;      /* Flow dependency on sum */
        *p = sum;       /* Anti/output dependency through *p */
        p++;
    }
    sink = sum;
}

/* Initialize arrays with pseudo-random data */
void init_data(int *arr, int size) {
    for (int i = 0; i < size; i++) {
        arr[i] = (int)(lcg_rand() % 1000);
    }
}

/* Initialize 2D array */
void init_matrix(int matrix[M][N]) {
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = (int)(lcg_rand() % 1000);
        }
    }
}

int main(int argc, char *argv[]) {
    /* Allocate arrays */
    int *arr1 = (int*)malloc(N * sizeof(int));
    int *arr2 = (int*)malloc(N * sizeof(int));
    int *arr3 = (int*)malloc(N * sizeof(int));
    int *arr4 = (int*)malloc(N * sizeof(int));
    int matrix[M][N];
    int *total = (int*)malloc(M * sizeof(int));
    
    if (!arr1 || !arr2 || !arr3 || !arr4 || !total) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize data */
    init_data(arr1, N);
    init_data(arr2, N);
    init_data(arr3, N);
    init_data(arr4, N);
    init_matrix(matrix);
    
    printf("Testing DDG edge creation patterns...\n");
    
    /* Test each dependency pattern */
    int result;
    
    result = test_flow_dep(arr1, N);
    printf("Flow dependency test: %d\n", result);
    
    test_anti_dep(arr1, arr2, N);
    printf("Anti dependency test completed\n");
    
    result = test_output_dep(arr1, N);
    printf("Output dependency test: %d\n", result);
    
    test_control_dep(arr1, arr2, N);
    printf("Control dependency test completed\n");
    
    test_mixed_deps(arr1, arr2, arr3, N);
    printf("Mixed dependencies test completed\n");
    
    test_nested_loops(matrix, total);
    printf("Nested loops test completed\n");
    
    test_pointer_deps(arr4, N);
    printf("Pointer dependencies test completed\n");
    
    /* Clean up */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    free(total);
    
    printf("All tests completed successfully\n");
    return 0;
}
