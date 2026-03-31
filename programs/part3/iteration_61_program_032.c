/* ddg_test.c - Test program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 128

/* Volatile helper to prevent optimization */
static volatile int volatile_sink;

/* Flow dependency (TRUE_DEP) - classic accumulator */
int flow_dependency(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];  /* Flow dependency on sum across iterations */
    }
    volatile_sink = sum; /* Prevent dead code elimination */
    return sum;
}

/* Anti-dependency (ANTI_DEP) - read then write pattern */
void anti_dependency(int *src, int *dst, int n) {
    int temp;
    for (int i = 0; i < n; i++) {
        temp = src[i];      /* Read src[i] */
        dst[i] = temp + 1;  /* Write to dst[i] - anti-dependency on temp */
        /* temp is read then effectively "written" (reassigned) next iteration */
    }
    volatile_sink = dst[n-1];
}

/* Output dependency (OUTPUT_DEP) - multiple writes to same variable */
int output_dependency(int *arr, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        int t = arr[i] * 2;  /* Computation */
        result = t;          /* Output dependency on result across iterations */
        /* result is written each iteration */
    }
    volatile_sink = result;
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
    volatile_sink = b[n-1];
}

/* Mixed dependencies - complex pattern */
int mixed_dependencies(int *arr, int n) {
    int acc1 = 0, acc2 = 0;
    int prev = arr[0];
    
    for (int i = 1; i < n; i++) {
        /* Flow dependency on acc1 */
        acc1 += arr[i];
        
        /* Anti-dependency through prev */
        int curr = arr[i];
        acc2 += prev;      /* Read prev */
        prev = curr;       /* Write prev - anti-dependency */
        
        /* Control dependency */
        if (acc1 > acc2) {
            arr[i] = acc1;  /* Output dependency on arr[i] */
        }
    }
    
    volatile_sink = acc1 + acc2;
    return acc1;
}

/* Nested loops with inner loop dependencies */
void nested_loop_dependencies(int matrix[M][N], int result[M]) {
    for (int j = 0; j < M; j++) {
        int acc = 0;
        /* Inner loop with flow dependency */
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];  /* Flow dependency on acc */
        }
        result[j] = acc;
        
        /* Second inner loop with anti-dependency */
        int temp = 0;
        for (int i = 1; i < N; i++) {
            temp = matrix[j][i-1];      /* Read */
            matrix[j][i] = temp + 1;    /* Write - anti-dependency */
        }
    }
    volatile_sink = result[M-1];
}

/* Loop-carried dependency with distance > 1 */
int distance_gt_one(int *arr, int n) {
    int sum = 0;
    /* Pattern that might create edges with distance > 0 */
    for (int i = 4; i < n; i++) {
        sum += arr[i] * arr[i-4];  /* Distance 4 flow dependency */
    }
    volatile_sink = sum;
    return sum;
}

/* Initialize array with pseudo-random values */
void init_array(int *arr, int n, int seed) {
    /* Simple LCG to avoid library dependencies */
    int val = seed;
    for (int i = 0; i < n; i++) {
        val = (1103515245 * val + 12345) & 0x7fffffff;
        arr[i] = (val % 100) - 50;  /* Values between -50 and 49 */
    }
}

/* Initialize matrix */
void init_matrix(int matrix[M][N], int seed) {
    int val = seed;
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            val = (1103515245 * val + 12345) & 0x7fffffff;
            matrix[j][i] = (val % 100) - 50;
        }
    }
}

int main(int argc, char *argv[]) {
    /* Allocate and initialize data */
    int *arr1 = (int*)malloc(N * sizeof(int));
    int *arr2 = (int*)malloc(N * sizeof(int));
    int *arr3 = (int*)malloc(N * sizeof(int));
    int matrix[M][N];
    int result[M];
    
    init_array(arr1, N, 42);
    init_array(arr2, N, 123);
    init_array(arr3, N, 789);
    init_matrix(matrix, 456);
    
    /* Execute all dependency patterns */
    printf("Testing DDG edge creation patterns...\n");
    
    /* Test each pattern multiple times to ensure execution */
    for (int iter = 0; iter < 3; iter++) {
        int r1 = flow_dependency(arr1, N);
        anti_dependency(arr1, arr2, N);
        int r2 = output_dependency(arr1, N);
        control_dependency(arr1, arr2, N);
        int r3 = mixed_dependencies(arr3, N);
        nested_loop_dependencies(matrix, result);
        int r4 = distance_gt_one(arr1, N);
        
        /* Use results to prevent optimization */
        volatile_sink = r1 + r2 + r3 + r4 + result[0];
    }
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    
    printf("DDG test completed.\n");
    return 0;
}
