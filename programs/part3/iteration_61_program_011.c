/* ddg_coverage.c
 * 
 * This program creates various loop patterns to trigger DDG edge creation
 * in GCC's instruction scheduler. Each function demonstrates different
 * dependency types that should cause create_ddg_dep_edge to be called.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 64

/* Simple pseudo-random generator to avoid library dependencies */
static unsigned int seed = 12345;
static inline unsigned int simple_rand(void) {
    seed = seed * 1103515245 + 12345;
    return (seed >> 16) & 0x7FFF;
}

/* Initialize arrays with pseudo-random values */
static void init_arrays(int *a, int *b, int *c, int size) {
    for (int i = 0; i < size; i++) {
        a[i] = simple_rand() % 100;
        b[i] = simple_rand() % 100;
        c[i] = 0;
    }
}

/* 1. FLOW DEPENDENCY (TRUE_DEP) - Classic accumulator pattern
 *    Creates read-after-write dependency on 'sum' across iterations
 */
int flow_dependency(int *arr) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += arr[i];  /* Flow dependency: sum read, modified, written */
    }
    return sum;
}

/* 2. ANTI DEPENDENCY - Read then write pattern
 *    Creates write-after-read dependency through temporary variable
 */
void anti_dependency(int *src, int *dst) {
    int temp;
    for (int i = 0; i < N; i++) {
        temp = src[i];      /* Read src[i] */
        dst[i] = temp + 1;  /* Write to dst[i] - anti-dependency if src==dst */
    }
}

/* 3. OUTPUT DEPENDENCY - Multiple writes to same location
 *    Creates write-after-write dependency
 */
void output_dependency(int *arr, int *result) {
    int temp;
    for (int i = 0; i < N; i++) {
        temp = arr[i] * 2;  /* Computation */
        *result = temp;     /* Output dependency: repeated write to *result */
    }
}

/* 4. CONTROL DEPENDENCY - Conditional inside loop
 *    Creates control flow dependencies
 */
void control_dependency(int *a, int *b) {
    for (int i = 0; i < N; i++) {
        if (a[i] > 50) {    /* Control dependency on a[i] */
            b[i] = 1;
        } else {
            b[i] = 0;
        }
    }
}

/* 5. MULTIPLE DEPENDENCIES COMBINED - Complex pattern
 *    Mixes flow, anti, and control dependencies
 */
int mixed_dependencies(int *a, int *b, int *c) {
    int acc = 0;
    int temp;
    
    for (int i = 1; i < N; i++) {
        /* Flow dependency on acc */
        acc += a[i];
        
        /* Anti dependency through temp */
        temp = b[i - 1];
        c[i] = temp + acc;
        
        /* Control dependency */
        if (acc > 1000) {
            c[i] = 0;
        }
    }
    return acc;
}

/* 6. NESTED LOOPS WITH INNER DEPENDENCIES
 *    Outer loop may cause DDG to be built for inner loop
 */
void nested_loop_dependencies(int matrix[M][N], int *totals) {
    for (int j = 0; j < M; j++) {
        int acc = 0;
        /* Inner loop with flow dependency */
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];
        }
        totals[j] = acc;
        
        /* Additional anti-dependency in inner loop */
        for (int i = 1; i < N; i++) {
            int temp = matrix[j][i - 1];
            matrix[j][i] = temp + 1;
        }
    }
}

/* 7. LOOP WITH DISTANCE > 1 - For distance parameter in ddg_edge
 *    Creates dependencies across multiple iterations
 */
void distance_dependency(int *a, int *b) {
    for (int i = 2; i < N; i++) {
        /* Flow dependency with distance 2 */
        b[i] = a[i - 2] + a[i - 1];
    }
}

/* 8. FLOAT DEPENDENCIES - Different data type for dt parameter
 *    May create edges with different data_type field
 */
float float_dependencies(float *arr) {
    float sum = 0.0f;
    for (int i = 0; i < N; i++) {
        sum += arr[i] * 0.5f;
    }
    return sum;
}

/* Main driver that runs all patterns */
int main(int argc, char **argv) {
    /* Allocate and initialize data */
    int *array1 = (int *)malloc(N * sizeof(int));
    int *array2 = (int *)malloc(N * sizeof(int));
    int *array3 = (int *)malloc(N * sizeof(int));
    int result = 0;
    
    float *farray = (float *)malloc(N * sizeof(float));
    int matrix[M][N];
    int totals[M];
    
    /* Initialize with deterministic pseudo-random values */
    init_arrays(array1, array2, array3, N);
    for (int i = 0; i < N; i++) {
        farray[i] = (float)(simple_rand() % 100) / 10.0f;
    }
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = simple_rand() % 100;
        }
    }
    
    /* Run different dependency patterns based on command line */
    int pattern = 0;
    if (argc > 1) {
        pattern = atoi(argv[1]);
    }
    
    switch (pattern) {
        case 0:
            /* Run all patterns */
            printf("Running all dependency patterns...\n");
            result = flow_dependency(array1);
            printf("Flow result: %d\n", result);
            
            anti_dependency(array1, array2);
            printf("Anti-dependency done\n");
            
            output_dependency(array1, &result);
            printf("Output dependency result: %d\n", result);
            
            control_dependency(array1, array2);
            printf("Control dependency done\n");
            
            result = mixed_dependencies(array1, array2, array3);
            printf("Mixed dependencies result: %d\n", result);
            
            nested_loop_dependencies(matrix, totals);
            printf("Nested loops done\n");
            
            distance_dependency(array1, array2);
            printf("Distance dependency done\n");
            
            float sumf = float_dependencies(farray);
            printf("Float dependencies result: %f\n", sumf);
            break;
            
        case 1:
            result = flow_dependency(array1);
            printf("Flow: %d\n", result);
            break;
            
        case 2:
            anti_dependency(array1, array2);
            printf("Anti done\n");
            break;
            
        case 3:
            output_dependency(array1, &result);
            printf("Output: %d\n", result);
            break;
            
        case 4:
            control_dependency(array1, array2);
            printf("Control done\n");
            break;
            
        case 5:
            result = mixed_dependencies(array1, array2, array3);
            printf("Mixed: %d\n", result);
            break;
            
        case 6:
            nested_loop_dependencies(matrix, totals);
            printf("Nested done\n");
            break;
            
        case 7:
            distance_dependency(array1, array2);
            printf("Distance done\n");
            break;
            
        case 8:
            float sumf = float_dependencies(farray);
            printf("Float: %f\n", sumf);
            break;
            
        default:
            printf("Unknown pattern\n");
            break;
    }
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(farray);
    
    return 0;
}
