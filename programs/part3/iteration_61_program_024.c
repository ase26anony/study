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

/* Flow dependency (TRUE_DEP) - classic accumulator pattern */
int flow_dependency(int *arr, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += arr[i];  /* Flow dependency on 'sum' across iterations */
    }
    return sum;
}

/* Anti-dependency (ANTI_DEP) - read then write pattern */
void anti_dependency(int *src, int *dst, int size) {
    int temp;
    for (int i = 0; i < size; i++) {
        temp = src[i];      /* Read from src[i] */
        dst[i] = temp + 1;  /* Anti-dependency if src and dst alias */
    }
}

/* Output dependency (OUTPUT_DEP) - multiple writes to same variable */
int output_dependency(int *arr, int size) {
    int result = 0;
    for (int i = 0; i < size; i++) {
        int t = arr[i] * 2;
        result = t;  /* Output dependency on 'result' across iterations */
    }
    return result;
}

/* Control dependency (CONTROL_DEP) - conditional inside loop */
void control_dependency(int *a, int *b, int size) {
    for (int i = 0; i < size; i++) {
        if (a[i] > 0) {    /* Control dependency on a[i] */
            b[i] = 1;
        } else {
            b[i] = 0;
        }
    }
}

/* Complex flow dependency with multiple accumulators */
int multi_flow_dependency(int *arr, int size) {
    int sum1 = 0, sum2 = 0;
    for (int i = 0; i < size; i++) {
        sum1 += arr[i];
        sum2 += sum1;  /* Flow dependency on sum1, anti on sum2 */
    }
    return sum1 + sum2;
}

/* Nested loops with inner loop dependencies */
void nested_loop_dependency(int matrix[M][N], int *totals) {
    for (int j = 0; j < M; j++) {
        int acc = 0;
        for (int i = 0; i < N; i++) {
            acc += matrix[j][i];  /* Flow dependency in inner loop */
        }
        totals[j] = acc;
    }
}

/* Loop with mixed dependencies */
void mixed_dependencies(int *a, int *b, int *c, int size) {
    int prev = 0;
    for (int i = 0; i < size; i++) {
        int read = a[i];          /* Anti-dependency potential */
        b[i] = read + prev;       /* Flow dependency on prev */
        prev = b[i];              /* Output dependency on prev */
        if (read > 0) {           /* Control dependency */
            c[i] = 1;
        }
    }
}

/* Initialize arrays with pseudo-random data */
void init_arrays(int *arr1, int *arr2, int *arr3, int size) {
    for (int i = 0; i < size; i++) {
        arr1[i] = (int)(lcg_rand() % 100);
        arr2[i] = (int)(lcg_rand() % 100);
        arr3[i] = 0;
    }
}

/* Initialize matrix */
void init_matrix(int matrix[M][N]) {
    for (int j = 0; j < M; j++) {
        for (int i = 0; i < N; i++) {
            matrix[j][i] = (int)(lcg_rand() % 50);
        }
    }
}

/* Main test driver */
int main(int argc, char *argv[]) {
    int arr1[N], arr2[N], arr3[N];
    int matrix[M][N];
    int totals[M];
    
    /* Initialize data */
    init_arrays(arr1, arr2, arr3, N);
    init_matrix(matrix);
    
    printf("Testing DDG edge creation patterns...\n");
    
    /* Test 1: Flow dependency */
    int sum1 = flow_dependency(arr1, N);
    printf("Flow dependency test: sum = %d\n", sum1);
    
    /* Test 2: Anti-dependency */
    anti_dependency(arr1, arr2, N);
    printf("Anti-dependency test completed\n");
    
    /* Test 3: Output dependency */
    int result = output_dependency(arr1, N);
    printf("Output dependency test: result = %d\n", result);
    
    /* Test 4: Control dependency */
    control_dependency(arr1, arr3, N);
    printf("Control dependency test completed\n");
    
    /* Test 5: Multiple flow dependencies */
    int sum2 = multi_flow_dependency(arr1, N);
    printf("Multi-flow dependency test: sum = %d\n", sum2);
    
    /* Test 6: Nested loops */
    nested_loop_dependency(matrix, totals);
    printf("Nested loop dependency test completed\n");
    
    /* Test 7: Mixed dependencies */
    mixed_dependencies(arr1, arr2, arr3, N);
    printf("Mixed dependencies test completed\n");
    
    /* Prevent dead code elimination */
    volatile int sink = sum1 + sum2 + result + totals[0] + arr3[0];
    (void)sink;
    
    return 0;
}
