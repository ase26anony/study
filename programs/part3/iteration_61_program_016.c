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
        temp = src[i];      /* Read from src[i] */
        dst[i] = temp + 1;  /* Write to dst[i] - anti-dependency if temp reused */
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
        int prev = a[i-1];          /* Flow from previous iteration via a[i-1] */
        acc += prev;                /* Flow dependency on acc */
        b[i] = acc;                 /* Anti-dependency if b aliases with a */
        if (acc > 100) {            /* Control dependency */
            c[i] = acc;
        }
    }
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
            matrix[j][i] = temp + 1;    /* Write - anti-dependency */
        }
    }
}

/* Loop with pointer aliasing to create ambiguous dependencies */
void test_pointer_aliasing(int *p, int *q, int size) {
    /* Assume p and q may alias */
    for (int i = 1; i < size; i++) {
        p[i] = p[i-1] + q[i];  /* Potential flow and anti dependencies */
    }
}

/* Main test driver */
int main(int argc, char **argv) {
    /* Allocate and initialize test data */
    int *arr1 = (int*)malloc(N * sizeof(int));
    int *arr2 = (int*)malloc(N * sizeof(int));
    int *arr3 = (int*)malloc(N * sizeof(int));
    int matrix[M][N];
    int totals[M];
    
    /* Initialize with pseudo-random data */
    for (int i = 0; i < N; i++) {
        arr1[i] = (int)(lcg_rand() % 100);
        arr2[i] = (int)(lcg_rand() % 100);
        arr3[i] = 0;
    }
    
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = (int)(lcg_rand() % 100);
        }
    }
    
    /* Run all dependency tests */
    printf("Testing DDG edge creation patterns...\n");
    
    /* Test 1: Flow dependencies */
    int sum1 = test_flow_dep(arr1, N);
    printf("Flow dep test: sum = %d\n", sum1);
    
    /* Test 2: Anti-dependencies */
    test_anti_dep(arr1, arr2, N);
    printf("Anti-dep test completed\n");
    
    /* Test 3: Output dependencies */
    int result = test_output_dep(arr1, N);
    printf("Output dep test: result = %d\n", result);
    
    /* Test 4: Control dependencies */
    test_control_dep(arr1, arr3, N);
    printf("Control dep test completed\n");
    
    /* Test 5: Mixed dependencies */
    test_mixed_deps(arr1, arr2, arr3, N);
    printf("Mixed deps test completed\n");
    
    /* Test 6: Nested loops */
    test_nested_loops(matrix, totals);
    printf("Nested loops test completed\n");
    
    /* Test 7: Pointer aliasing */
    test_pointer_aliasing(arr1, arr2, N);
    printf("Pointer aliasing test completed\n");
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    
    return 0;
}
