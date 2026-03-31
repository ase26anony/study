/* test_ddg.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 128

/* Prevent aggressive optimization */
static volatile int sink;

/* Flow dependency (TRUE_DEP) - accumulator pattern */
int flow_dependency(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];  /* Flow dependency on 'sum' across iterations */
    }
    sink = sum; /* Prevent dead code elimination */
    return sum;
}

/* Anti-dependency (ANTI_DEP) - read then write pattern */
void anti_dependency(int *src, int *dst, int n) {
    int temp;
    for (int i = 0; i < n; i++) {
        temp = src[i];      /* Read from src[i] */
        dst[i] = temp + 1;  /* Write to dst[i] - anti-dependency if src==dst */
    }
    sink = dst[n-1];
}

/* Output dependency (OUTPUT_DEP) - multiple writes to same variable */
int output_dependency(int *arr, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        int t = arr[i] * 2;
        result = t;  /* Output dependency on 'result' across iterations */
    }
    sink = result;
    return result;
}

/* Control dependency (CONTROL_DEP) - conditional inside loop */
void control_dependency(int *a, int *b, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > 0) {      /* Control dependency on a[i] */
            b[i] = 1;
        } else {
            b[i] = 0;
        }
    }
    sink = b[n-1];
}

/* Mixed dependencies - complex pattern */
void mixed_dependencies(int *a, int *b, int *c, int n) {
    int acc = 0;
    for (int i = 1; i < n; i++) {
        int prev = a[i-1];           /* Flow from previous iteration via a[i-1] */
        acc += prev;                 /* Flow dependency on acc */
        b[i] = acc;                  /* Anti-dependency if b overlaps with a */
        if (acc > 100) {             /* Control dependency */
            c[i] = b[i] * 2;         /* Flow from b[i] */
        } else {
            c[i] = b[i];             /* Output dependency on c[i] across iterations */
        }
    }
    sink = c[n-1];
}

/* Nested loops with inner loop dependencies */
void nested_loop_dependencies(int matrix[M][N], int *total) {
    for (int j = 0; j < M; j++) {
        int acc = 0;
        /* Inner loop with flow dependency */
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];  /* Flow dependency on acc */
        }
        total[j] = acc;  /* Anti-dependency if total overlaps with matrix */
        
        /* Second inner loop with different pattern */
        for (int i = 1; i < N; i++) {
            matrix[j][i] = matrix[j][i-1] + 1;  /* Flow dependency through array */
        }
    }
    sink = total[M-1];
}

/* Loop with pointer aliasing (creates ambiguous dependencies) */
void pointer_aliasing(int *a, int *b, int n) {
    int *p = a;
    int *q = b;
    
    for (int i = 0; i < n; i++) {
        *p = *q + 1;      /* May create flow/anti dependencies if p and q alias */
        p++;
        q++;
    }
    sink = a[n-1];
}

/* Initialize arrays with pseudo-random values */
void init_array(int *arr, int n, int seed) {
    int val = seed;
    for (int i = 0; i < n; i++) {
        val = (val * 1103515245 + 12345) & 0x7fffffff;
        arr[i] = (val % 100) - 50;  /* Values between -50 and 49 */
    }
}

/* Main driver that runs all dependency patterns */
int main(int argc, char **argv) {
    /* Allocate and initialize data */
    int *arr1 = (int*)malloc(N * sizeof(int));
    int *arr2 = (int*)malloc(N * sizeof(int));
    int *arr3 = (int*)malloc(N * sizeof(int));
    int *arr4 = (int*)malloc(N * sizeof(int));
    
    int matrix[M][N];
    int total[M];
    
    /* Initialize with different seeds for variety */
    init_array(arr1, N, 1);
    init_array(arr2, N, 2);
    init_array(arr3, N, 3);
    init_array(arr4, N, 4);
    
    /* Initialize matrix */
    for (int j = 0; j < M; j++) {
        init_array(matrix[j], N, j + 10);
    }
    
    printf("Running dependency tests...\n");
    
    /* Run each dependency pattern */
    int result;
    
    result = flow_dependency(arr1, N);
    printf("Flow dependency result: %d\n", result);
    
    anti_dependency(arr1, arr2, N);
    printf("Anti-dependency completed\n");
    
    result = output_dependency(arr1, N);
    printf("Output dependency result: %d\n", result);
    
    control_dependency(arr1, arr3, N);
    printf("Control dependency completed\n");
    
    mixed_dependencies(arr1, arr2, arr3, N);
    printf("Mixed dependencies completed\n");
    
    nested_loop_dependencies(matrix, total);
    printf("Nested loop dependencies completed\n");
    
    pointer_aliasing(arr1, arr4, N);
    printf("Pointer aliasing completed\n");
    
    /* Clean up */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    
    printf("All tests completed successfully\n");
    return 0;
}
