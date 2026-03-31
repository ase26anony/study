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
        a[i] = lcg_rand() % 100;
        b[i] = lcg_rand() % 100;
        c[i] = 0;
    }
}

/* 1. FLOW DEPENDENCY (TRUE_DEP) - Classic accumulator pattern */
int flow_dependency(const int *arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];  /* Flow dependency on 'sum' across iterations */
    }
    return sum;
}

/* 2. ANTI DEPENDENCY - Read then write to same location */
void anti_dependency(int *src, int *dst, int size) {
    int temp;
    for (int i = 0; i < size; i++) {
        temp = src[i];      /* Read src[i] */
        dst[i] = temp + 1;  /* Anti-dependency if src and dst alias */
    }
}

/* 3. OUTPUT DEPENDENCY - Multiple writes to same variable */
int output_dependency(const int *arr, int size) {
    int result = 0;
    for (int i = 0; i < size; i++) {
        int computed = arr[i] * 2;
        result = computed;  /* Output dependency on 'result' */
    }
    return result;
}

/* 4. CONTROL DEPENDENCY - Conditional inside loop */
void control_dependency(const int *a, int *b, int size) {
    for (int i = 0; i < size; i++) {
        if (a[i] > 50) {    /* Control dependency on a[i] */
            b[i] = 1;
        } else {
            b[i] = 0;
        }
    }
}

/* 5. MULTIPLE DEPENDENCIES COMBINED */
int combined_dependencies(int *a, int *b, int size) {
    int acc = 0;
    int temp;
    
    for (int i = 1; i < size; i++) {
        temp = a[i-1];      /* Flow from previous iteration */
        a[i] = temp + b[i]; /* Anti if a and b overlap */
        acc += a[i];        /* Flow on acc */
        
        if (acc > 1000) {   /* Control dependency */
            acc = 1000;
        }
    }
    return acc;
}

/* 6. NESTED LOOPS with inner loop dependencies */
void nested_loop_deps(int matrix[M][N], int *totals) {
    for (int j = 0; j < M; j++) {
        int acc = 0;
        /* Inner loop with flow dependency */
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];
        }
        totals[j] = acc;
        
        /* Anti-dependency in second inner loop */
        for (int i = 1; i < N; i++) {
            int prev = matrix[j][i-1];
            matrix[j][i] = prev + 1;
        }
    }
}

/* 7. LOOP-CARRIED DEPENDENCY WITH DISTANCE > 1 */
int distance_2_dependency(const int *arr, int size) {
    int sum = 0;
    /* Process in steps of 2, creating distance-2 dependencies */
    for (int i = 2; i < size; i++) {
        sum += arr[i] + arr[i-2];  /* Dependency distance = 2 */
    }
    return sum;
}

/* 8. FLOAT DEPENDENCIES (different data type in DDG) */
float float_flow_dependency(const float *arr, int size) {
    float sum = 0.0f;
    for (int i = 0; i < size; i++) {
        sum += arr[i];  /* Flow dependency with floats */
    }
    return sum;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    int *array1, *array2, *array3;
    float *farray;
    int matrix[M][N];
    int totals[M];
    
    /* Allocate and initialize arrays */
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
            matrix[j][i] = lcg_rand() % 100;
        }
    }
    
    /* Run all dependency tests */
    printf("Testing DDG edge creation patterns...\n");
    
    /* Test 1: Flow dependency */
    int sum1 = flow_dependency(array1, N);
    printf("Flow dependency test: %d\n", sum1);
    
    /* Test 2: Anti-dependency (with potential aliasing) */
    anti_dependency(array1, array2, N);
    printf("Anti-dependency test completed\n");
    
    /* Test 3: Output dependency */
    int sum2 = output_dependency(array1, N);
    printf("Output dependency test: %d\n", sum2);
    
    /* Test 4: Control dependency */
    control_dependency(array1, array3, N);
    printf("Control dependency test completed\n");
    
    /* Test 5: Combined dependencies */
    int sum3 = combined_dependencies(array1, array2, N);
    printf("Combined dependencies test: %d\n", sum3);
    
    /* Test 6: Nested loops */
    nested_loop_deps(matrix, totals);
    printf("Nested loop test completed\n");
    
    /* Test 7: Distance > 1 dependency */
    int sum4 = distance_2_dependency(array1, N);
    printf("Distance-2 dependency test: %d\n", sum4);
    
    /* Test 8: Float dependencies */
    float fsum = float_flow_dependency(farray, N);
    printf("Float flow dependency test: %.2f\n", fsum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(farray);
    
    printf("All DDG tests completed successfully\n");
    return 0;
}
