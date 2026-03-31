/* test_ddg.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 128

/* Prevent aggressive optimization */
static volatile int sink;

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

/* 1. FLOW DEPENDENCY (TRUE_DEP) - Classic accumulator pattern */
int flow_dependency(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];  /* Flow dependency on 'sum' across iterations */
    }
    sink = sum; /* Prevent dead code elimination */
    return sum;
}

/* 2. ANTI DEPENDENCY - Read then write to same location */
void anti_dependency(int *src, int *dst, int n) {
    int temp;
    for (int i = 0; i < n; i++) {
        temp = src[i];      /* Read src[i] */
        dst[i] = temp + 1;  /* Anti-dependency if src and dst alias */
    }
    sink = dst[n-1];
}

/* 3. OUTPUT DEPENDENCY - Multiple writes to same variable */
int output_dependency(int *arr, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        int computed = arr[i] * 2;
        result = computed;  /* Output dependency on 'result' */
    }
    sink = result;
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
    sink = b[n-1];
}

/* 5. MULTIPLE DEPENDENCIES COMBINED */
int combined_dependencies(int *a, int *b, int n) {
    int acc = 0;
    int temp;
    
    for (int i = 0; i < n; i++) {
        temp = a[i];        /* Anti-dependency potential */
        acc += temp;        /* Flow dependency on acc */
        b[i] = acc;         /* Flow dependency on acc, anti on b[i] */
        
        if (acc > 1000) {   /* Control dependency */
            acc = 1000;
        }
    }
    sink = acc;
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
        totals[j] = acc;
        
        /* Additional computation with control dependency */
        if (acc > 10000) {
            totals[j] = 10000;
        }
    }
    sink = totals[M-1];
}

/* 7. LOOP-CARRIED DEPENDENCY with distance > 1 */
int distance_2_dependency(int *arr, int n) {
    int sum = 0;
    /* Process elements with stride 2 */
    for (int i = 2; i < n; i++) {
        sum += arr[i] + arr[i-2];  /* Dependency distance of 2 */
    }
    sink = sum;
    return sum;
}

/* 8. FLOAT DEPENDENCIES - Different data type for DDG edges */
float float_flow_dependency(float *arr, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += arr[i];  /* Flow dependency with float type */
    }
    sink = (int)sum;
    return sum;
}

/* Main driver that calls all test functions */
int main(int argc, char **argv) {
    int *array1, *array2, *array3;
    float *farray;
    int matrix[M][N];
    int totals[M];
    
    /* Allocate and initialize data */
    array1 = (int*)malloc(N * sizeof(int));
    array2 = (int*)malloc(N * sizeof(int));
    array3 = (int*)malloc(N * sizeof(int));
    farray = (float*)malloc(N * sizeof(float));
    
    if (!array1 || !array2 || !array3 || !farray) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(array1, array2, array3, N);
    
    /* Initialize float array */
    for (int i = 0; i < N; i++) {
        farray[i] = (float)(lcg_rand() % 100) / 10.0f;
    }
    
    /* Initialize matrix */
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = (int)(lcg_rand() % 100);
        }
    }
    
    /* Run all dependency tests */
    printf("Testing DDG edge creation patterns...\n");
    
    /* Select specific test if argument provided */
    int test_num = (argc > 1) ? atoi(argv[1]) : 0;
    
    if (test_num == 0 || test_num == 1) {
        printf("1. Flow dependency test: ");
        int result = flow_dependency(array1, N);
        printf("sum = %d\n", result);
    }
    
    if (test_num == 0 || test_num == 2) {
        printf("2. Anti dependency test\n");
        anti_dependency(array1, array2, N);
    }
    
    if (test_num == 0 || test_num == 3) {
        printf("3. Output dependency test: ");
        int result = output_dependency(array1, N);
        printf("result = %d\n", result);
    }
    
    if (test_num == 0 || test_num == 4) {
        printf("4. Control dependency test\n");
        control_dependency(array1, array3, N);
    }
    
    if (test_num == 0 || test_num == 5) {
        printf("5. Combined dependencies test: ");
        int result = combined_dependencies(array1, array2, N);
        printf("result = %d\n", result);
    }
    
    if (test_num == 0 || test_num == 6) {
        printf("6. Nested loop dependencies test\n");
        nested_loop_dependencies(matrix, totals);
        printf("First few totals: %d, %d, %d\n", totals[0], totals[1], totals[2]);
    }
    
    if (test_num == 0 || test_num == 7) {
        printf("7. Distance-2 dependency test: ");
        int result = distance_2_dependency(array1, N);
        printf("result = %d\n", result);
    }
    
    if (test_num == 0 || test_num == 8) {
        printf("8. Float flow dependency test: ");
        float result = float_flow_dependency(farray, N);
        printf("sum = %.2f\n", result);
    }
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(farray);
    
    printf("All tests completed.\n");
    return 0;
}
