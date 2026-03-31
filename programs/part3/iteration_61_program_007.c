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
        temp = src[i];      /* Read src[i] */
        dst[i] = temp + 1;  /* Anti-dependency: write after read */
    }
    prevent_opt = temp;  /* Use temp to prevent dead code elimination */
}

/* Output dependency (OUTPUT_DEP) - multiple writes to same variable */
int output_dependency(int *arr, int size) {
    int result = 0;
    for (int i = 0; i < size; i++) {
        int t = arr[i] * 2;  /* Compute temporary */
        result = t;          /* Output dependency on result */
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

/* Mixed dependencies - complex pattern */
int mixed_dependencies(int *arr, int size) {
    int acc1 = 0, acc2 = 0;
    int temp;
    
    for (int i = 1; i < size; i++) {
        /* Flow dependency on acc1 */
        acc1 += arr[i];
        
        /* Anti-dependency through temp */
        temp = arr[i - 1];
        arr[i] = temp + acc1;
        
        /* Control dependency */
        if (acc1 > 1000) {
            acc2 += 1;
        }
    }
    return acc1 + acc2 + prevent_opt;
}

/* Nested loops with inner loop dependencies */
void nested_loop_deps(int matrix[M][N], int result[M]) {
    for (int j = 0; j < M; j++) {
        int acc = 0;
        /* Inner loop with flow dependency */
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];  /* Flow dependency on acc */
        }
        result[j] = acc;
        
        /* Anti-dependency in second inner loop */
        for (int i = 1; i < N; i++) {
            int temp = matrix[j][i - 1];
            matrix[j][i] = temp + 1;  /* Anti-dependency */
        }
    }
}

/* Loop with distance > 1 for distance vector in DDG edge */
int distance_vector_dep(int *arr, int size) {
    int sum = 0;
    /* Loop with stride to create distance > 1 dependencies */
    for (int i = 2; i < size; i++) {
        arr[i] = arr[i - 2] + 1;  /* Distance 2 flow dependency */
        sum += arr[i];
    }
    return sum + prevent_opt;
}

/* Initialize arrays with pseudo-random values */
void init_array(int *arr, int size, int seed) {
    int val = seed;
    for (int i = 0; i < size; i++) {
        val = (val * 1103515245 + 12345) & 0x7fffffff;
        arr[i] = (val % 100) - 50;  /* Values between -50 and 49 */
    }
}

int main(int argc, char *argv[]) {
    int *array1, *array2, *result;
    int matrix[M][N];
    
    /* Allocate and initialize data */
    array1 = (int *)malloc(N * sizeof(int));
    array2 = (int *)malloc(N * sizeof(int));
    result = (int *)malloc(M * sizeof(int));
    
    if (!array1 || !array2 || !result) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_array(array1, N, 42);
    init_array(array2, N, 123);
    
    /* Initialize matrix */
    for (int j = 0; j < M; j++) {
        init_array(matrix[j], N, j * 7919);
    }
    
    /* Execute all dependency patterns */
    int total = 0;
    
    printf("Testing DDG edge creation patterns...\n");
    
    /* Test 1: Flow dependency */
    total += flow_dependency(array1, N);
    
    /* Test 2: Anti-dependency */
    anti_dependency(array1, array2, N);
    
    /* Test 3: Output dependency */
    total += output_dependency(array1, N);
    
    /* Test 4: Control dependency */
    control_dependency(array1, array2, N);
    
    /* Test 5: Mixed dependencies */
    total += mixed_dependencies(array1, N);
    
    /* Test 6: Nested loops */
    nested_loop_deps(matrix, result);
    for (int i = 0; i < M; i++) {
        total += result[i];
    }
    
    /* Test 7: Distance vector dependency */
    total += distance_vector_dep(array2, N);
    
    /* Use results to prevent dead code elimination */
    printf("Total (prevents optimization): %d\n", total);
    
    /* Clean up */
    free(array1);
    free(array2);
    free(result);
    
    return 0;
}
