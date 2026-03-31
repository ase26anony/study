/* ddg_test.c - Test program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 128

/* Prevent aggressive optimization */
static volatile int prevent_opt = 0;

/* Flow dependency (TRUE_DEP) - classic accumulator */
int flow_dependency(int *arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];  /* Flow dependency on sum across iterations */
    }
    return sum + prevent_opt;
}

/* Anti-dependency (ANTI_DEP) - read then write pattern */
void anti_dependency(int *src, int *dst, int size) {
    int temp;
    for (int i = 0; i < size; i++) {
        temp = src[i];      /* Read from src */
        dst[i] = temp + 1;  /* Write to dst - creates anti-dependency on temp */
        /* Force temp to be reused to ensure anti-dependency */
        if (prevent_opt) temp = 0;
    }
}

/* Output dependency (OUTPUT_DEP) - multiple writes to same variable */
int output_dependency(int *arr, int size) {
    int result = 0;
    for (int i = 0; i < size; i++) {
        int t = arr[i] * 2;  /* Computation */
        result = t;          /* Output dependency on result across iterations */
    }
    return result + prevent_opt;
}

/* Control dependency (CONTROL_DEP) - conditional inside loop */
void control_dependency(int *a, int *b, int size) {
    for (int i = 0; i < size; i++) {
        if (a[i] > 0) {      /* Control dependency on a[i] */
            b[i] = 1;
        } else {
            b[i] = 0;
        }
    }
}

/* Mixed dependencies - more complex pattern */
int mixed_dependencies(int *a, int *b, int size) {
    int acc = 0;
    int prev = 0;
    
    for (int i = 0; i < size; i++) {
        int temp = a[i];
        
        /* Flow dependency on acc */
        acc += temp;
        
        /* Anti-dependency through temp reuse */
        b[i] = prev + temp;
        prev = temp;  /* Creates flow dependency on prev */
        
        /* Control dependency */
        if (acc > 1000) {
            acc -= 500;
        }
    }
    return acc + prevent_opt;
}

/* Nested loops with inner loop dependencies */
void nested_loop_deps(int matrix[M][N], int *totals) {
    for (int j = 0; j < M; j++) {
        int acc = 0;
        /* Inner loop with flow dependency */
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];
        }
        totals[j] = acc;
        
        /* Anti-dependency in second inner loop */
        int prev = 0;
        for (int i = 0; i < N; i++) {
            int temp = matrix[j][i];
            matrix[j][i] = prev + temp;
            prev = temp;
        }
    }
}

/* Loop with distance > 1 dependencies */
void distance_dependency(int *src, int *dst, int size, int distance) {
    for (int i = distance; i < size; i++) {
        /* Flow dependency with distance */
        dst[i] = src[i - distance] + dst[i - 1];
    }
}

/* Initialize arrays with pseudo-random values */
void init_array(int *arr, int size, int seed) {
    for (int i = 0; i < size; i++) {
        /* Simple LCG to avoid library calls */
        seed = (1103515245 * seed + 12345) & 0x7fffffff;
        arr[i] = (seed >> 16) & 0xFF;
    }
}

/* Initialize matrix */
void init_matrix(int matrix[M][N], int seed) {
    for (int j = 0; j < M; j++) {
        init_array(matrix[j], N, seed + j);
    }
}

int main(int argc, char **argv) {
    /* Allocate arrays */
    int *arr1 = (int*)malloc(N * sizeof(int));
    int *arr2 = (int*)malloc(N * sizeof(int));
    int *arr3 = (int*)malloc(N * sizeof(int));
    int matrix[M][N];
    int totals[M];
    
    /* Initialize data */
    init_array(arr1, N, 42);
    init_array(arr2, N, 123);
    init_array(arr3, N, 456);
    init_matrix(matrix, 789);
    
    /* Run all dependency tests */
    int test = (argc > 1) ? atoi(argv[1]) : 0;
    
    switch (test) {
        case 0:
            /* Run all tests */
            printf("Flow result: %d\n", flow_dependency(arr1, N));
            anti_dependency(arr1, arr2, N);
            printf("Output result: %d\n", output_dependency(arr1, N));
            control_dependency(arr1, arr2, N);
            printf("Mixed result: %d\n", mixed_dependencies(arr1, arr2, N));
            nested_loop_deps(matrix, totals);
            distance_dependency(arr1, arr3, N, 2);
            break;
            
        case 1:
            printf("Flow: %d\n", flow_dependency(arr1, N));
            break;
            
        case 2:
            anti_dependency(arr1, arr2, N);
            printf("Anti done\n");
            break;
            
        case 3:
            printf("Output: %d\n", output_dependency(arr1, N));
            break;
            
        case 4:
            control_dependency(arr1, arr2, N);
            printf("Control done\n");
            break;
            
        case 5:
            printf("Mixed: %d\n", mixed_dependencies(arr1, arr2, N));
            break;
            
        case 6:
            nested_loop_deps(matrix, totals);
            printf("Nested done\n");
            break;
            
        case 7:
            distance_dependency(arr1, arr3, N, 2);
            printf("Distance done\n");
            break;
            
        default:
            printf("Invalid test\n");
            break;
    }
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    
    return 0;
}
