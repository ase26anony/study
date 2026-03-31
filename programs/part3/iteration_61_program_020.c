/* test_ddg.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 128

/* Prevent optimization from removing loops */
static volatile int sink;

/* Simple PRNG to generate data without external dependencies */
static unsigned int seed = 12345;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Flow dependency (TRUE_DEP) - classic accumulator pattern */
void flow_dependency(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];  /* Flow dependency on 'sum' across iterations */
    }
    sink = sum; /* Prevent dead code elimination */
}

/* Anti-dependency (ANTI_DEP) - read then write pattern */
void anti_dependency(int *src, int *dst, int n) {
    int temp;
    for (int i = 0; i < n; i++) {
        temp = src[i];    /* Read from src[i] */
        dst[i] = temp + 1; /* Write to dst[i] - anti-dependency if temp reused */
        
        /* Force reuse of register/variable */
        if (temp > 100) {
            dst[i] += temp;
        }
    }
    sink = dst[n-1];
}

/* Output dependency (OUTPUT_DEP) - multiple writes to same location */
void output_dependency(int *arr, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        int t = arr[i] * 2;
        result = t;  /* Output dependency on 'result' across iterations */
        
        /* Use result to prevent elimination */
        if (result > 1000) {
            arr[i] = result % 256;
        }
    }
    sink = result;
}

/* Control dependency (CONTROL_DEP) - conditional inside loop */
void control_dependency(int *a, int *b, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > 0) {  /* Control dependency on a[i] */
            b[i] = 1;
        } else {
            b[i] = 0;
        }
    }
    sink = b[n/2];
}

/* Mixed dependencies with loop-carried flow */
void mixed_dependencies(int *a, int *b, int *c, int n) {
    int acc = 0;
    for (int i = 0; i < n; i++) {
        int read_val = a[i];      /* Anti-dep if register reused */
        acc += read_val;          /* Flow dep on acc */
        b[i] = acc;               /* Output dep on b[i] if unrolled */
        if (acc > 100) {          /* Control dep on acc */
            c[i] = read_val * 2;
        }
    }
    sink = acc + b[n-1] + c[n-1];
}

/* Nested loops - DDG often built for inner loops */
void nested_loop_dependencies(int matrix[M][N], int result[M]) {
    for (int j = 0; j < M; j++) {
        int acc = 0;
        /* Inner loop with flow dependency */
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];  /* Flow dependency in inner loop */
        }
        result[j] = acc;
        
        /* Anti-dependency in outer loop */
        int temp = result[j];
        if (j > 0) {
            result[j-1] = temp + matrix[j][0];
        }
    }
    sink = result[M-1];
}

/* Loop with distance > 1 for distance vector in DDG edge */
void distance_vector_dependency(int *a, int *b, int n) {
    /* b[i] depends on a[i+2] - creates edge with distance */
    for (int i = 0; i < n - 2; i++) {
        b[i] = a[i+2] + 1;
    }
    sink = b[n-3];
}

/* Initialize arrays with pseudo-random data */
void init_arrays(int *arr1, int *arr2, int *arr3, int size) {
    for (int i = 0; i < size; i++) {
        arr1[i] = (int)(lcg_rand() % 1000);
        arr2[i] = (int)(lcg_rand() % 1000);
        arr3[i] = 0;
    }
}

int main(int argc, char *argv[]) {
    /* Allocate arrays on heap to avoid stack overflow */
    int *arr1 = malloc(N * sizeof(int));
    int *arr2 = malloc(N * sizeof(int));
    int *arr3 = malloc(N * sizeof(int));
    
    int matrix[M][N];
    int result[M];
    
    if (!arr1 || !arr2 || !arr3) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize with different patterns */
    init_arrays(arr1, arr2, arr3, N);
    
    /* Initialize matrix */
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = (int)(lcg_rand() % 500);
        }
    }
    
    /* Run all dependency patterns */
    printf("Testing DDG edge creation patterns...\n");
    
    /* Select specific test based on command line or run all */
    int run_all = (argc == 1);
    int test_num = (argc > 1) ? atoi(argv[1]) : 0;
    
    if (run_all || test_num == 1) {
        flow_dependency(arr1, N);
        printf("Flow dependency test completed\n");
    }
    
    if (run_all || test_num == 2) {
        anti_dependency(arr1, arr2, N);
        printf("Anti-dependency test completed\n");
    }
    
    if (run_all || test_num == 3) {
        output_dependency(arr1, N);
        printf("Output dependency test completed\n");
    }
    
    if (run_all || test_num == 4) {
        control_dependency(arr1, arr2, N);
        printf("Control dependency test completed\n");
    }
    
    if (run_all || test_num == 5) {
        mixed_dependencies(arr1, arr2, arr3, N);
        printf("Mixed dependencies test completed\n");
    }
    
    if (run_all || test_num == 6) {
        nested_loop_dependencies(matrix, result);
        printf("Nested loop dependencies test completed\n");
    }
    
    if (run_all || test_num == 7) {
        distance_vector_dependency(arr1, arr2, N);
        printf("Distance vector dependency test completed\n");
    }
    
    /* Use results to prevent dead code elimination */
    int total = sink + arr1[N/2] + arr2[N/3] + arr3[N/4];
    printf("Final checksum: %d\n", total);
    
    free(arr1);
    free(arr2);
    free(arr3);
    
    return 0;
}
