/* ddg_test.c - Test program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 128

/* Prevent aggressive optimization */
static volatile int sink;

/* Flow dependency (TRUE_DEP) - classic accumulator */
int flow_dep(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];  /* Flow dependency on 'sum' across iterations */
    }
    sink = sum; /* Prevent dead code elimination */
    return sum;
}

/* Anti-dependency (ANTI_DEP) - read then write pattern */
void anti_dep(int *src, int *dst, int n) {
    int temp;
    for (int i = 0; i < n; i++) {
        temp = src[i];      /* Read from src[i] */
        dst[i] = temp + 1;  /* Write to dst[i] - anti-dep on 'temp' if reused */
        /* Force reuse of temp variable across iterations */
        if (i % 2 == 0) {
            temp = 0;  /* This creates anti-dependency on temp */
        }
    }
    sink = dst[n-1];
}

/* Output dependency (OUTPUT_DEP) - multiple writes to same variable */
int output_dep(int *arr, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        int t = arr[i] * 2;  /* Computation */
        result = t;          /* Output dependency on 'result' across iterations */
        /* Use result to prevent elimination */
        if (result > 1000) result = 1000;
    }
    sink = result;
    return result;
}

/* Control dependency (CONTROL_DEP) - conditional inside loop */
void control_dep(int *a, int *b, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > 0) {      /* Control dependency on a[i] */
            b[i] = 1;
        } else {
            b[i] = 0;
        }
    }
    sink = b[n-1];
}

/* Nested loops with inner loop dependencies */
int nested_flow_dep(int matrix[M][N]) {
    int total[M];
    for (int j = 0; j < M; j++) {
        int acc = 0;
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];  /* Flow dependency in inner loop */
        }
        total[j] = acc;
    }
    
    int final_sum = 0;
    for (int j = 0; j < M; j++) {
        final_sum += total[j];
    }
    sink = final_sum;
    return final_sum;
}

/* Complex loop with multiple dependency types */
void mixed_dependencies(int *a, int *b, int *c, int n) {
    int accum = 0;
    int temp;
    
    for (int i = 0; i < n; i++) {
        /* Flow dependency on accum */
        accum += a[i];
        
        /* Anti-dependency on temp */
        temp = b[i];
        c[i] = temp * 2;
        
        /* Control dependency */
        if (accum > 100) {
            c[i] += 1;
        }
        
        /* Potential output dependency through function calls */
        accum = (accum > 1000) ? 1000 : accum;
    }
    
    sink = accum;
}

/* Initialize arrays with pseudo-random values */
void init_array(int *arr, int n, int seed) {
    unsigned int r = seed;
    for (int i = 0; i < n; i++) {
        r = r * 1103515245 + 12345;
        arr[i] = (r >> 16) & 0x7FFF;
    }
}

/* Initialize 2D array */
void init_matrix(int matrix[M][N], int seed) {
    unsigned int r = seed;
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            r = r * 1103515245 + 12345;
            matrix[j][i] = (r >> 16) & 0xFF;
        }
    }
}

int main(int argc, char *argv[]) {
    /* Allocate and initialize data */
    int *arr1 = (int *)malloc(N * sizeof(int));
    int *arr2 = (int *)malloc(N * sizeof(int));
    int *arr3 = (int *)malloc(N * sizeof(int));
    int matrix[M][N];
    
    if (!arr1 || !arr2 || !arr3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_array(arr1, N, 42);
    init_array(arr2, N, 123);
    init_array(arr3, N, 456);
    init_matrix(matrix, 789);
    
    /* Execute all dependency patterns */
    printf("Testing DDG edge creation patterns...\n");
    
    /* Test 1: Flow dependency */
    int sum1 = flow_dep(arr1, N);
    printf("Flow dependency test: %d\n", sum1);
    
    /* Test 2: Anti-dependency */
    anti_dep(arr1, arr2, N);
    printf("Anti-dependency test completed\n");
    
    /* Test 3: Output dependency */
    int sum2 = output_dep(arr1, N);
    printf("Output dependency test: %d\n", sum2);
    
    /* Test 4: Control dependency */
    control_dep(arr1, arr3, N);
    printf("Control dependency test completed\n");
    
    /* Test 5: Nested loops */
    int sum3 = nested_flow_dep(matrix);
    printf("Nested loop test: %d\n", sum3);
    
    /* Test 6: Mixed dependencies */
    mixed_dependencies(arr1, arr2, arr3, N);
    printf("Mixed dependencies test completed\n");
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    
    printf("All tests completed successfully\n");
    return 0;
}
