/* ddg_test.c - Test program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 128

/* Simple pseudo-random generator to avoid library dependencies */
static unsigned int seed = 12345;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return (seed >> 16) & 0x7FFF;
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
int flow_dependency(int *arr, int n) {
    int sum = 0;
    /* Creates flow dependency on 'sum' across iterations */
    for (int i = 0; i < n; i++) {
        sum += arr[i];  /* Read sum, modify, write back */
    }
    return sum;
}

/* 2. ANTI DEPENDENCY - Read then write pattern */
void anti_dependency(int *src, int *dst, int n) {
    int temp;
    /* Creates anti-dependency through temporary variable */
    for (int i = 0; i < n; i++) {
        temp = src[i];      /* Read from src[i] */
        dst[i] = temp + 1;  /* Write to dst[i] - anti-dep on temp */
    }
}

/* 3. OUTPUT DEPENDENCY - Multiple writes to same variable */
int output_dependency(int *arr, int n) {
    int result = 0;
    /* Creates output dependency through repeated writes */
    for (int i = 0; i < n; i++) {
        int computed = arr[i] * 2;
        result = computed;  /* Multiple writes to 'result' */
    }
    return result;
}

/* 4. CONTROL DEPENDENCY - Conditional inside loop */
void control_dependency(int *a, int *b, int n) {
    /* Creates control dependency through condition */
    for (int i = 0; i < n; i++) {
        if (a[i] > 50) {    /* Loop-variant condition */
            b[i] = 1;
        } else {
            b[i] = 0;
        }
    }
}

/* 5. MULTIPLE DEPENDENCY TYPES COMBINED */
int combined_dependencies(int *a, int *b, int n) {
    int sum = 0;
    int temp;
    
    for (int i = 0; i < n; i++) {
        /* Flow dependency on sum */
        sum += a[i];
        
        /* Anti dependency through temp */
        temp = b[i];
        a[i] = temp * 2;
        
        /* Control dependency */
        if (sum > 1000) {
            b[i] = 0;
        }
    }
    return sum;
}

/* 6. NESTED LOOPS with inner loop dependencies */
void nested_loop_dependencies(int matrix[M][N], int *totals) {
    /* Outer loop */
    for (int j = 0; j < M; j++) {
        int acc = 0;
        
        /* Inner loop with flow dependency */
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];  /* Flow dep on acc */
        }
        
        totals[j] = acc;
        
        /* Anti dependency in second inner loop */
        for (int i = 1; i < N; i++) {
            int prev = matrix[j][i-1];  /* Read */
            matrix[j][i] = prev + 1;    /* Write - anti dep */
        }
    }
}

/* 7. DISTANCE > 0 DEPENDENCY - Loop-carried across iterations */
void distance_dependency(int *a, int *b, int n) {
    /* b[i] depends on a[i-2] - creates edge with distance > 0 */
    for (int i = 2; i < n; i++) {
        b[i] = a[i-2] + a[i-1];  /* Depends on previous iterations */
    }
}

/* 8. FLOAT DEPENDENCIES - Different data type */
float float_flow_dependency(float *arr, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += arr[i];  /* Flow dependency on float sum */
    }
    return sum;
}

/* Main test driver */
int main(int argc, char **argv) {
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
    
    /* Initialize with pseudo-random data */
    init_arrays(array1, array2, array3, N);
    for (int i = 0; i < N; i++) {
        farray[i] = (float)(lcg_rand() % 100) / 10.0f;
    }
    
    /* Initialize matrix */
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = lcg_rand() % 100;
        }
    }
    
    /* Execute all dependency patterns */
    printf("Testing DDG edge creation patterns...\n");
    
    /* Use volatile to prevent dead code elimination */
    volatile int result;
    
    /* Test 1: Flow dependency */
    result = flow_dependency(array1, N);
    printf("Flow dependency test: %d\n", result);
    
    /* Test 2: Anti dependency */
    anti_dependency(array1, array2, N);
    printf("Anti dependency test completed\n");
    
    /* Test 3: Output dependency */
    result = output_dependency(array1, N);
    printf("Output dependency test: %d\n", result);
    
    /* Test 4: Control dependency */
    control_dependency(array1, array3, N);
    printf("Control dependency test completed\n");
    
    /* Test 5: Combined dependencies */
    result = combined_dependencies(array1, array2, N);
    printf("Combined dependencies test: %d\n", result);
    
    /* Test 6: Nested loops */
    nested_loop_dependencies(matrix, totals);
    printf("Nested loop test completed\n");
    
    /* Test 7: Distance dependency */
    distance_dependency(array1, array2, N);
    printf("Distance dependency test completed\n");
    
    /* Test 8: Float dependencies */
    volatile float fresult = float_flow_dependency(farray, N);
    printf("Float flow dependency test: %f\n", fresult);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(farray);
    
    printf("All DDG tests completed successfully\n");
    return 0;
}
