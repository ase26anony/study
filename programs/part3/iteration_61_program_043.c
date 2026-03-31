/* ddg_test.c - Test program to trigger DDG edge creation in GCC */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define M 128

/* Simple PRNG to generate data without external dependencies */
static unsigned int seed = 12345;
static inline unsigned int lcg_rand(void) {
    seed = seed * 1103515245 + 12345;
    return seed;
}

/* Flow dependency (TRUE_DEP) - accumulator pattern */
int flow_dependency(int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];  /* Flow dependency on 'sum' across iterations */
    }
    return sum;
}

/* Anti-dependency (ANTI_DEP) - read then write pattern */
void anti_dependency(int *src, int *dst, int n) {
    int temp;
    for (int i = 0; i < n; i++) {
        temp = src[i];      /* Read from src[i] */
        dst[i] = temp + 1;  /* Anti-dependency if src and dst alias */
    }
}

/* Output dependency (OUTPUT_DEP) - multiple writes to same location */
int output_dependency(int *arr, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        int t = arr[i] * 2;
        result = t;  /* Output dependency on 'result' across iterations */
    }
    return result;
}

/* Control dependency (CONTROL_DEP) - conditional inside loop */
void control_dependency(int *a, int *b, int n) {
    for (int i = 0; i < n; i++) {
        if (a[i] > 0) {     /* Control dependency on a[i] */
            b[i] = 1;
        } else {
            b[i] = 0;
        }
    }
}

/* Mixed dependencies in nested loops */
int nested_mixed_deps(int matrix[M][N]) {
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
    return final_sum;
}

/* Complex loop with multiple dependency types */
void complex_dependencies(int *a, int *b, int *c, int n) {
    int prev = a[0];
    for (int i = 1; i < n; i++) {
        int curr = a[i];           /* Flow from prev iteration via 'prev' */
        b[i] = prev + curr;        /* Anti if b aliases a */
        prev = curr;               /* Output on 'prev' */
        
        if (b[i] > 100) {          /* Control dependency */
            c[i] = b[i] * 2;
        } else {
            c[i] = b[i];
        }
    }
}

/* Initialize arrays with pseudo-random data */
void init_arrays(int *arr1, int *arr2, int *arr3, int size) {
    for (int i = 0; i < size; i++) {
        arr1[i] = (int)(lcg_rand() % 1000);
        arr2[i] = (int)(lcg_rand() % 1000);
        arr3[i] = 0;
    }
}

/* Main test driver */
int main(int argc, char **argv) {
    int *array1 = malloc(N * sizeof(int));
    int *array2 = malloc(N * sizeof(int));
    int *array3 = malloc(N * sizeof(int));
    
    /* Initialize test data */
    init_arrays(array1, array2, array3, N);
    
    /* Matrix for nested loop test */
    int matrix[M][N];
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = (int)(lcg_rand() % 100);
        }
    }
    
    /* Run all dependency tests */
    int result;
    
    printf("Testing flow dependency...\n");
    result = flow_dependency(array1, N);
    printf("Flow result: %d\n", result);
    
    printf("Testing anti dependency...\n");
    anti_dependency(array1, array2, N);
    printf("Anti dependency done\n");
    
    printf("Testing output dependency...\n");
    result = output_dependency(array1, N);
    printf("Output result: %d\n", result);
    
    printf("Testing control dependency...\n");
    control_dependency(array1, array3, N);
    printf("Control dependency done\n");
    
    printf("Testing nested mixed dependencies...\n");
    result = nested_mixed_deps(matrix);
    printf("Nested result: %d\n", result);
    
    printf("Testing complex dependencies...\n");
    complex_dependencies(array1, array2, array3, N);
    printf("Complex dependencies done\n");
    
    /* Clean up */
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}
