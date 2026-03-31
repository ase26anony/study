/* test_ddg.c - Program to trigger DDG edge creation in GCC */
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
        dst[i] = temp + 1;  /* Anti-dependency if dst aliases src */
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

/* 5. MULTIPLE DEPENDENCY TYPES COMBINED */
int mixed_dependencies(int *a, int *b, int n) {
    int acc = 0;
    int temp;
    
    for (int i = 0; i < n; i++) {
        /* Flow dependency on acc */
        acc += a[i];
        
        /* Anti dependency through temp */
        temp = b[i];
        a[i] = temp * 2;
        
        /* Control dependency */
        if (acc > 1000) {
            b[i] = 0;
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
        totals[j] = acc;
        
        /* Anti dependency in second inner loop */
        for (int i = 1; i < N; i++) {
            int prev = matrix[j][i-1];
            matrix[j][i] = prev + 1;
        }
    }
}

/* 7. LOOP-CARRIED DEPENDENCY with distance > 1 */
void distance_2_dependency(int *a, int *b, int n) {
    for (int i = 2; i < n; i++) {
        /* Flow dependency with distance 2 */
        a[i] = a[i-2] + b[i];
    }
}

/* 8. VOLATILE VARIABLE to prevent optimization */
int volatile_dependency(volatile int *arr, int n) {
    volatile int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

/* Main driver that exercises all patterns */
int main(int argc, char **argv) {
    int *array1, *array2, *array3;
    int matrix[M][N];
    int totals[M];
    
    /* Allocate and initialize data */
    array1 = (int*)malloc(N * sizeof(int));
    array2 = (int*)malloc(N * sizeof(int));
    array3 = (int*)malloc(N * sizeof(int));
    
    if (!array1 || !array2 || !array3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(array1, array2, array3, N);
    
    /* Initialize matrix */
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = (int)(lcg_rand() % 100);
        }
    }
    
    printf("Testing DDG edge creation patterns...\n");
    
    /* Execute each dependency pattern */
    int result;
    
    result = flow_dependency(array1, N);
    printf("Flow dependency result: %d\n", result);
    
    anti_dependency(array1, array2, N);
    printf("Anti dependency completed\n");
    
    result = output_dependency(array1, N);
    printf("Output dependency result: %d\n", result);
    
    control_dependency(array1, array3, N);
    printf("Control dependency completed\n");
    
    result = mixed_dependencies(array1, array2, N);
    printf("Mixed dependencies result: %d\n", result);
    
    nested_loop_dependencies(matrix, totals);
    printf("Nested loop dependencies completed\n");
    
    distance_2_dependency(array1, array2, N);
    printf("Distance-2 dependency completed\n");
    
    result = volatile_dependency(array1, N);
    printf("Volatile dependency result: %d\n", result);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    
    printf("All DDG patterns executed successfully\n");
    return 0;
}
