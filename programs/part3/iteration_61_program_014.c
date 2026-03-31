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

/* Initialize arrays with pseudo-random data */
static void init_arrays(int *a, int *b, int *c, int size) {
    for (int i = 0; i < size; i++) {
        a[i] = (int)(lcg_rand() % 100);
        b[i] = (int)(lcg_rand() % 100);
        c[i] = 0;
    }
}

/* 1. FLOW DEPENDENCY (TRUE_DEP) - Classic accumulator pattern */
int flow_dependency(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];  /* Flow dependency on 'sum' across iterations */
    }
    return sum;
}

/* 2. ANTI DEPENDENCY - Read then write pattern */
void anti_dependency(int *src, int *dst, int n) {
    int temp;
    for (int i = 0; i < n; i++) {
        temp = src[i];      /* Read src[i] */
        dst[i] = temp + 1;  /* Anti dependency if dst aliases src */
    }
}

/* 3. OUTPUT DEPENDENCY - Multiple writes to same variable */
int output_dependency(int *arr, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        int computed = arr[i] * 2;
        result = computed;  /* Output dependency on 'result' */
    }
    return result;
}

/* 4. CONTROL DEPENDENCY - Conditional inside loop */
void control_dependency(int *a, int *b, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > 50) {    /* Control dependency on a[i] */
            b[i] = 1;
        } else {
            b[i] = 0;
        }
    }
}

/* 5. MULTIPLE DEPENDENCIES COMBINED */
int combined_dependencies(int *a, int *b, int n) {
    int acc = 0;
    int temp;
    
    for (int i = 0; i < n; i++) {
        temp = a[i];        /* Anti dependency potential */
        acc += temp;        /* Flow dependency on acc */
        if (acc > 1000) {   /* Control dependency */
            b[i] = 1;
        } else {
            b[i] = 0;       /* Output dependency on b[i] */
        }
    }
    return acc;
}

/* 6. NESTED LOOPS with inner loop dependencies */
void nested_loop_dependencies(int matrix[M][N], int *totals) {
    for (int j = 0; j < M; j++) {
        int acc = 0;
        /* Inner loop with flow dependency */
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];
        }
        totals[j] = acc;  /* Anti dependency if totals aliases matrix */
    }
}

/* 7. LOOP-CARRIED DEPENDENCY with distance > 1 */
int distance_2_dependency(int *arr, int n) {
    int sum = 0;
    /* Process in steps of 2, creating dependencies at distance 2 */
    for (int i = 2; i < n; i++) {
        arr[i] = arr[i-2] + 1;  /* Flow dependency with distance 2 */
        sum += arr[i];
    }
    return sum;
}

/* 8. VOLATILE VARIABLE to prevent optimization */
int volatile_dependency(volatile int *arr, int n) {
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];  /* Volatile ensures dependency isn't optimized away */
    }
    return sum;
}

/* Main test driver */
int main(int argc, char **argv) {
    int *array1, *array2, *array3;
    int matrix[M][N];
    int totals[M];
    
    /* Allocate and initialize arrays */
    array1 = (int*)malloc(N * sizeof(int));
    array2 = (int*)malloc(N * sizeof(int));
    array3 = (int*)malloc(N * sizeof(int));
    
    if (!array1 || !array2 || !array3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with different patterns */
    init_arrays(array1, array2, array3, N);
    
    /* Initialize matrix */
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = (int)(lcg_rand() % 100);
        }
    }
    
    printf("Testing various dependency patterns to trigger DDG edge creation...\n");
    
    /* Test 1: Flow dependency */
    int sum1 = flow_dependency(array1, N);
    printf("Flow dependency test: sum = %d\n", sum1);
    
    /* Test 2: Anti dependency */
    anti_dependency(array1, array2, N);
    printf("Anti dependency test completed\n");
    
    /* Test 3: Output dependency */
    int sum3 = output_dependency(array1, N);
    printf("Output dependency test: result = %d\n", sum3);
    
    /* Test 4: Control dependency */
    control_dependency(array1, array3, N);
    printf("Control dependency test completed\n");
    
    /* Test 5: Combined dependencies */
    int sum5 = combined_dependencies(array1, array2, N);
    printf("Combined dependencies test: acc = %d\n", sum5);
    
    /* Test 6: Nested loops */
    nested_loop_dependencies(matrix, totals);
    printf("Nested loop test completed\n");
    
    /* Test 7: Distance > 1 dependency */
    int sum7 = distance_2_dependency(array1, N);
    printf("Distance-2 dependency test: sum = %d\n", sum7);
    
    /* Test 8: Volatile dependency */
    int sum8 = volatile_dependency(array1, N);
    printf("Volatile dependency test: sum = %d\n", sum8);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    
    printf("All tests completed successfully\n");
    return 0;
}
