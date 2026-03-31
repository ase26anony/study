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

/* Initialize arrays with pseudo-random values */
static void init_arrays(int *a, int *b, int *c, int size) {
    for (int i = 0; i < size; i++) {
        a[i] = (int)(lcg_rand() % 100);
        b[i] = (int)(lcg_rand() % 100);
        c[i] = 0;
    }
}

/* Test 1: Flow dependency (TRUE_DEP) - accumulator pattern */
int test_flow_dep(int *arr, int n) {
    int sum = 0;
    /* Creates flow dependency on 'sum' across iterations */
    for (int i = 0; i < n; i++) {
        sum += arr[i];  /* Read sum, modify, write back */
    }
    return sum;
}

/* Test 2: Anti-dependency (ANTI_DEP) - read then write pattern */
void test_anti_dep(int *src, int *dst, int n) {
    int temp;
    /* Creates anti-dependency through temporary variable */
    for (int i = 0; i < n; i++) {
        temp = src[i];    /* Read from src */
        dst[i] = temp + 1; /* Write to dst - anti-dep on temp reuse */
    }
}

/* Test 3: Output dependency (OUTPUT_DEP) - multiple writes to same variable */
int test_output_dep(int *arr, int n) {
    int result = 0;
    /* Creates output dependency on 'result' */
    for (int i = 0; i < n; i++) {
        int computed = arr[i] * 2;
        result = computed;  /* Multiple writes to result across iterations */
    }
    return result;
}

/* Test 4: Control dependency (CONTROL_DEP) - conditional inside loop */
void test_control_dep(int *a, int *b, int n) {
    /* Creates control dependency through conditional branch */
    for (int i = 0; i < n; i++) {
        if (a[i] > 50) {      /* Loop-variant condition */
            b[i] = a[i] * 2;  /* Control-dependent store */
        } else {
            b[i] = a[i];
        }
    }
}

/* Test 5: Complex flow dependency - array shift/copy */
void test_array_shift(int *src, int *dst, int n) {
    /* Multiple dependencies: flow, anti, and output */
    dst[0] = src[0];
    for (int i = 1; i < n; i++) {
        dst[i] = src[i-1] + src[i];  /* Flow dep on src[i-1], anti on dst[i] */
    }
}

/* Test 6: Nested loops with inner loop dependencies */
int test_nested_loops(int matrix[M][N]) {
    int total[M];
    int grand_total = 0;
    
    /* Outer loop */
    for (int j = 0; j < M; j++) {
        int acc = 0;
        /* Inner loop with flow dependency */
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];  /* Flow dependency on acc */
        }
        total[j] = acc;
        grand_total += acc;  /* Another flow dependency in outer loop */
    }
    return grand_total;
}

/* Test 7: Mixed dependencies in single loop */
void test_mixed_deps(int *a, int *b, int *c, int n) {
    int prev = 0;
    for (int i = 0; i < n; i++) {
        int temp = a[i];          /* Anti-dep potential if temp reused */
        b[i] = prev + temp;       /* Flow dep on prev */
        prev = b[i];              /* Output dep on prev, flow to next iter */
        c[i] = (temp > 50) ? 1 : 0; /* Control dep through conditional */
    }
}

/* Test 8: Loop with volatile to prevent optimization */
int test_volatile_dep(volatile int *arr, int n) {
    volatile int sum = 0;
    /* Volatile ensures dependencies aren't optimized away */
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Test 9: Pointer-chasing loop for complex flow analysis */
int test_pointer_chase(int *arr, int n) {
    int *p = arr;
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += *p;
        p++;  /* Flow dependency through pointer increment */
    }
    return sum;
}

/* Test 10: Reduction with multiple accumulators */
int test_multi_accumulator(int *arr, int n) {
    int sum1 = 0, sum2 = 0, sum3 = 0;
    /* Multiple independent flow dependencies */
    for (int i = 0; i < n; i++) {
        sum1 += arr[i];
        sum2 += arr[i] * 2;
        sum3 += arr[i] * 3;
    }
    return sum1 + sum2 + sum3;
}

/* Main driver that runs all tests */
int main(int argc, char *argv[]) {
    /* Allocate and initialize test data */
    int *array1 = (int*)malloc(N * sizeof(int));
    int *array2 = (int*)malloc(N * sizeof(int));
    int *array3 = (int*)malloc(N * sizeof(int));
    int matrix[M][N];
    
    if (!array1 || !array2 || !array3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with pseudo-random data */
    init_arrays(array1, array2, array3, N);
    
    /* Initialize matrix */
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = (int)(lcg_rand() % 100);
        }
    }
    
    int test_to_run = -1;
    if (argc > 1) {
        test_to_run = atoi(argv[1]);
    }
    
    /* Run selected test or all tests */
    if (test_to_run == -1 || test_to_run == 1) {
        int result = test_flow_dep(array1, N);
        printf("Test 1 (Flow): %d\n", result);
    }
    
    if (test_to_run == -1 || test_to_run == 2) {
        test_anti_dep(array1, array2, N);
        printf("Test 2 (Anti): array2[0]=%d\n", array2[0]);
    }
    
    if (test_to_run == -1 || test_to_run == 3) {
        int result = test_output_dep(array1, N);
        printf("Test 3 (Output): %d\n", result);
    }
    
    if (test_to_run == -1 || test_to_run == 4) {
        test_control_dep(array1, array3, N);
        printf("Test 4 (Control): array3[0]=%d\n", array3[0]);
    }
    
    if (test_to_run == -1 || test_to_run == 5) {
        test_array_shift(array1, array2, N);
        printf("Test 5 (Array shift): array2[1]=%d\n", array2[1]);
    }
    
    if (test_to_run == -1 || test_to_run == 6) {
        int result = test_nested_loops(matrix);
        printf("Test 6 (Nested): %d\n", result);
    }
    
    if (test_to_run == -1 || test_to_run == 7) {
        test_mixed_deps(array1, array2, array3, N);
        printf("Test 7 (Mixed): array2[0]=%d, array3[0]=%d\n", array2[0], array3[0]);
    }
    
    if (test_to_run == -1 || test_to_run == 8) {
        int result = test_volatile_dep(array1, N);
        printf("Test 8 (Volatile): %d\n", result);
    }
    
    if (test_to_run == -1 || test_to_run == 9) {
        int result = test_pointer_chase(array1, N);
        printf("Test 9 (Pointer chase): %d\n", result);
    }
    
    if (test_to_run == -1 || test_to_run == 10) {
        int result = test_multi_accumulator(array1, N);
        printf("Test 10 (Multi-acc): %d\n", result);
    }
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}
