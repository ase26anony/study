/* test_ddg.c - Program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 64

/* Prevent aggressive optimization */
static volatile int force_keep = 0;

/* Flow dependency (TRUE_DEP) - accumulator pattern */
int flow_dependency(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];  /* Flow dependency on 'sum' across iterations */
    }
    force_keep = sum;  /* Prevent dead code elimination */
    return sum;
}

/* Anti-dependency (ANTI_DEP) - read then write pattern */
void anti_dependency(int *src, int *dst, int n) {
    int temp;
    for (int i = 0; i < n; i++) {
        temp = src[i];      /* Read from src[i] */
        dst[i] = temp + 1;  /* Write to dst[i] - anti-dependency on temp if reused */
        
        /* Force reuse of temp to create anti-dependency */
        if (i % 2 == 0) {
            temp = dst[i];  /* This creates anti-dependency with previous read */
        }
    }
    force_keep = dst[n-1];
}

/* Output dependency (OUTPUT_DEP) - multiple writes to same variable */
int output_dependency(int *arr, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        int t = arr[i] * 2;  /* Different computation each iteration */
        result = t;          /* Output dependency on 'result' across iterations */
        
        /* Use result to prevent elimination */
        if (result > 1000) {
            force_keep = i;
        }
    }
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
    force_keep = b[n-1];
}

/* Mixed dependencies - complex pattern */
void mixed_dependencies(int *src, int *dst, int n) {
    int acc = 0;
    int prev = src[0];
    
    for (int i = 1; i < n; i++) {
        /* Flow dependency on acc */
        acc += src[i];
        
        /* Anti-dependency on prev */
        int curr = src[i];
        dst[i-1] = prev + acc;  /* Read prev, write dst */
        prev = curr;            /* Write prev - anti-dependency with read above */
        
        /* Control dependency */
        if (acc > 100) {
            dst[i] = 0;         /* Control-dependent write */
        }
    }
    dst[n-1] = prev;
    force_keep = acc;
}

/* Nested loops with inner loop dependencies */
void nested_loop_dependencies(int matrix[M][N], int *total) {
    for (int j = 0; j < M; j++) {
        int acc = 0;
        /* Inner loop with flow dependency */
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];  /* Flow dependency on acc */
        }
        total[j] = acc;
        
        /* Anti-dependency pattern in second inner loop */
        int temp = 0;
        for (int i = 0; i < N; i++) {
            int val = matrix[j][i];  /* Read */
            temp = val * 2;          /* Write - anti-dependency if temp reused */
            matrix[j][i] = temp;     /* Write */
        }
    }
    force_keep = total[M-1];
}

/* Loop with carried dependencies and distance > 0 */
void distance_gt_zero(int *a, int *b, int n, int distance) {
    for (int i = distance; i < n; i++) {
        /* Flow dependency with distance */
        b[i] = a[i - distance] + b[i - 1];
    }
    force_keep = b[n-1];
}

/* Simple PRNG for generating test data without external dependencies */
static unsigned int seed = 123456789;
static unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

static void init_array(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        arr[i] = (int)(lcg_rand() % 1000);
    }
}

static void init_matrix(int matrix[M][N]) {
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = (int)(lcg_rand() % 1000);
        }
    }
}

int main(int argc, char *argv[]) {
    /* Allocate test data */
    int *arr1 = (int*)malloc(N * sizeof(int));
    int *arr2 = (int*)malloc(N * sizeof(int));
    int *arr3 = (int*)malloc(N * sizeof(int));
    int matrix[M][N];
    int total[M];
    
    /* Initialize with pseudo-random data */
    init_array(arr1, N);
    init_array(arr2, N);
    init_array(arr3, N);
    init_matrix(matrix);
    
    printf("Testing DDG edge creation patterns...\n");
    
    /* Test each dependency pattern */
    int result;
    
    result = flow_dependency(arr1, N);
    printf("Flow dependency test: %d\n", result);
    
    anti_dependency(arr1, arr2, N);
    printf("Anti-dependency test completed\n");
    
    result = output_dependency(arr1, N);
    printf("Output dependency test: %d\n", result);
    
    control_dependency(arr1, arr2, N);
    printf("Control dependency test completed\n");
    
    mixed_dependencies(arr1, arr2, N);
    printf("Mixed dependencies test completed\n");
    
    nested_loop_dependencies(matrix, total);
    printf("Nested loop dependencies test completed\n");
    
    distance_gt_zero(arr1, arr2, N, 3);
    printf("Distance > 0 test completed\n");
    
    /* Cleanup */
    free(arr1);
    free(arr2);
    free(arr3);
    
    printf("All tests completed. Force keep: %d\n", force_keep);
    
    return 0;
}
