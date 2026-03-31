/* ddg_test.c - Test program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 128

/* Prevent aggressive optimization */
static volatile int sink;

/* Flow dependency (TRUE_DEP) - accumulator pattern */
int flow_dep(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];  /* Flow dependency on 'sum' across iterations */
    }
    sink = sum;  /* Prevent dead code elimination */
    return sum;
}

/* Anti-dependency (ANTI_DEP) - read then write pattern */
void anti_dep(int *src, int *dst, int n) {
    int temp;
    for (int i = 0; i < n; i++) {
        temp = src[i];    /* Read from src[i] */
        dst[i] = temp + 1; /* Write to dst[i] - anti-dep on temp if reused */
    }
    sink = dst[n-1];
}

/* Output dependency (OUTPUT_DEP) - multiple writes to same variable */
void output_dep(int *arr, int *res, int n) {
    int tmp;
    for (int i = 0; i < n; i++) {
        tmp = arr[i] * 2;     /* Write to tmp */
        res[i] = tmp;         /* Write to res[i] */
        /* tmp is written each iteration - output dependency if compiler reuses register */
    }
    sink = res[n-1];
}

/* Control dependency (CONTROL_DEP) - conditional inside loop */
void control_dep(int *a, int *b, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > 0) {        /* Control dependency on a[i] */
            b[i] = 1;
        } else {
            b[i] = 0;
        }
    }
    sink = b[n-1];
}

/* Mixed dependencies - complex pattern */
int mixed_deps(int *a, int *b, int n) {
    int acc = 0;
    int prev = 0;
    
    for (int i = 0; i < n; i++) {
        int temp = a[i];          /* Anti-dep potential */
        b[i] = prev + temp;       /* Flow dep on prev, output dep on b[i] */
        prev = b[i];              /* Flow dep on b[i] */
        if (temp > 0) {           /* Control dep */
            acc += temp;          /* Flow dep on acc */
        }
    }
    sink = acc;
    return acc;
}

/* Nested loops with inner loop dependencies */
void nested_loop(int matrix[M][N], int total[M]) {
    for (int j = 0; j < M; j++) {
        int acc = 0;
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];  /* Flow dependency in inner loop */
        }
        total[j] = acc;           /* Output dependency on total[j] across outer iterations */
    }
    sink = total[M-1];
}

/* Loop-carried dependency with distance > 1 */
void distance_dep(int *src, int *dst, int n) {
    for (int i = 2; i < n; i++) {
        dst[i] = src[i-2] + src[i-1];  /* Flow dep with distance 1 and 2 */
    }
    sink = dst[n-1];
}

/* Initialize arrays with pseudo-random values (LCG) */
void init_array(int *arr, int n, int seed) {
    unsigned int state = seed;
    for (int i = 0; i < n; i++) {
        state = state * 1103515245 + 12345;
        arr[i] = (state >> 16) & 0x7FFF;
    }
}

/* Initialize 2D array */
void init_matrix(int matrix[M][N], int seed) {
    unsigned int state = seed;
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            state = state * 1103515245 + 12345;
            matrix[j][i] = (state >> 16) & 0x7FFF;
        }
    }
}

int main(int argc, char *argv[]) {
    /* Allocate arrays on heap to avoid stack overflow */
    int *arr1 = (int*)malloc(N * sizeof(int));
    int *arr2 = (int*)malloc(N * sizeof(int));
    int *arr3 = (int*)malloc(N * sizeof(int));
    int *arr4 = (int*)malloc(N * sizeof(int));
    int (*matrix)[N] = (int(*)[N])malloc(M * N * sizeof(int));
    int *total = (int*)malloc(M * sizeof(int));
    
    if (!arr1 || !arr2 || !arr3 || !arr4 || !matrix || !total) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with different seeds */
    init_array(arr1, N, 42);
    init_array(arr2, N, 123);
    init_array(arr3, N, 456);
    init_array(arr4, N, 789);
    init_matrix(matrix, 999);
    
    printf("Testing DDG edge creation patterns...\n");
    
    /* Test each dependency pattern */
    int result;
    
    result = flow_dep(arr1, N);
    printf("Flow dependency test: %d\n", result);
    
    anti_dep(arr1, arr2, N);
    printf("Anti-dependency test completed\n");
    
    output_dep(arr1, arr3, N);
    printf("Output dependency test completed\n");
    
    control_dep(arr1, arr4, N);
    printf("Control dependency test completed\n");
    
    result = mixed_deps(arr1, arr2, N);
    printf("Mixed dependencies test: %d\n", result);
    
    nested_loop(matrix, total);
    printf("Nested loop test completed\n");
    
    distance_dep(arr1, arr2, N);
    printf("Distance dependency test completed\n");
    
    /* Clean up */
    free(arr1);
    free(arr2);
    free(arr3);
    free(arr4);
    free(matrix);
    free(total);
    
    printf("All tests completed successfully\n");
    return 0;
}
