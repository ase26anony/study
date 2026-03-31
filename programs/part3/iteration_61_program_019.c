/* ddg_coverage.c
 * 
 * This program creates various loop patterns that should trigger
 * the create_ddg_dep_edge function in GCC's DDG builder.
 * Compile with: gcc -O2 -fschedule-insns -fdump-rtl-ddg ddg_coverage.c -o ddg_coverage
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 64

/* Simple pseudo-random generator to avoid library dependencies */
static unsigned int seed = 12345;
static inline unsigned int simple_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Initialize arrays with pseudo-random values */
static void init_arrays(int *a, int *b, int *c, int size) {
    for (int i = 0; i < size; i++) {
        a[i] = simple_rand() % 100;
        b[i] = simple_rand() % 100;
        c[i] = 0;
    }
}

/* 1. FLOW DEPENDENCY (TRUE_DEP): Classic accumulator pattern */
int flow_dependency(int *arr) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += arr[i];  /* Flow dependency on 'sum' across iterations */
    }
    return sum;
}

/* 2. ANTI DEPENDENCY: Read then write pattern */
void anti_dependency(int *src, int *dst) {
    int temp;
    for (int i = 0; i < N; i++) {
        temp = src[i];      /* Read from src[i] */
        dst[i] = temp + 1;  /* Anti dependency if src and dst alias */
    }
}

/* 3. OUTPUT DEPENDENCY: Multiple writes to same variable */
int output_dependency(int *arr) {
    int result = 0;
    for (int i = 0; i < N; i++) {
        int computed = arr[i] * 2;
        result = computed;  /* Output dependency on 'result' */
    }
    return result;
}

/* 4. CONTROL DEPENDENCY: Conditional inside loop */
void control_dependency(int *a, int *b) {
    for (int i = 0; i < N; i++) {
        if (a[i] > 50) {    /* Control dependency on a[i] */
            b[i] = 1;
        } else {
            b[i] = 0;
        }
    }
}

/* 5. MULTIPLE DEPENDENCIES: Combined flow and anti */
int multiple_dependencies(int *a, int *b) {
    int acc = 0;
    int temp;
    for (int i = 0; i < N; i++) {
        temp = a[i];        /* Anti dependency if a and b overlap */
        acc += temp;        /* Flow dependency on acc */
        b[i] = acc;         /* Flow dependency on acc, anti on b */
    }
    return acc;
}

/* 6. NESTED LOOPS with inner loop dependencies */
void nested_loop_dependencies(int matrix[M][N], int *totals) {
    for (int j = 0; j < M; j++) {
        int acc = 0;
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];  /* Flow dependency in inner loop */
        }
        totals[j] = acc;
    }
}

/* 7. LOOP-CARRIED DEPENDENCY with distance > 1 */
void distance_dependency(int *src, int *dst) {
    for (int i = 2; i < N; i++) {
        dst[i] = src[i-1] + src[i-2];  /* Flow dependencies with distances 1 and 2 */
    }
}

/* 8. VOLATILE to prevent over-optimization */
int volatile_dependency(volatile int *arr) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += arr[i];  /* Volatile access ensures dependency */
    }
    return sum;
}

/* 9. FLOAT DEPENDENCIES: Different data types */
float float_dependency(float *arr) {
    float sum = 0.0f;
    for (int i = 0; i < N; i++) {
        sum += arr[i];  /* Flow dependency with floats */
    }
    return sum;
}

/* Main driver that runs all patterns */
int main(int argc, char **argv) {
    /* Allocate and initialize data */
    int *array1 = (int *)malloc(N * sizeof(int));
    int *array2 = (int *)malloc(N * sizeof(int));
    int *array3 = (int *)malloc(N * sizeof(int));
    int matrix[M][N];
    float *farrays = (float *)malloc(N * sizeof(float));
    volatile int *varray = (volatile int *)malloc(N * sizeof(int));
    
    /* Initialize with deterministic but non-constant values */
    init_arrays(array1, array2, array3, N);
    
    /* Initialize float array */
    for (int i = 0; i < N; i++) {
        farrays[i] = (float)(simple_rand() % 100) / 10.0f;
        varray[i] = simple_rand() % 100;
    }
    
    /* Initialize matrix */
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = simple_rand() % 100;
        }
    }
    
    int result = 0;
    
    /* Run all dependency patterns */
    result += flow_dependency(array1);
    
    anti_dependency(array1, array2);
    result += array2[0];
    
    result += output_dependency(array1);
    
    control_dependency(array1, array3);
    result += array3[0];
    
    result += multiple_dependencies(array1, array2);
    
    int totals[M];
    nested_loop_dependencies(matrix, totals);
    result += totals[0];
    
    distance_dependency(array1, array2);
    result += array2[0];
    
    result += volatile_dependency(varray);
    
    result += (int)float_dependency(farrays);
    
    /* Clean up */
    free(array1);
    free(array2);
    free(array3);
    free(farrays);
    free((void *)varray);
    
    /* Print result to prevent dead code elimination */
    printf("Result: %d\n", result);
    
    return 0;
}
