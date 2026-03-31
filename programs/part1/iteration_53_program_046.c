/* test-annotate-expr.c */
#include <stdio.h>
#include <stdlib.h>

/* Dummy function to prevent optimization */
extern void use(int val);
void use(int val) {
    volatile int sink = val;
    (void)sink;
}

/* Function with ivdep annotation (no-vector) */
void test_no_vector(int *a, int *b, int n) {
    int i;
    /* Potential dependency that ivdep overrides */
    #pragma GCC ivdep
    for (i = 1; i < n; i++) {
        a[i] = a[i-1] + b[i];  /* Appears to have dependency */
    }
    use(a[n-1]);
}

/* Function with vector annotation */
void test_vector(float *x, float *y, int n) {
    int i;
    #pragma GCC vector
    for (i = 0; i < n; i++) {
        x[i] = y[i] * 2.0f + 1.0f;
    }
    use((int)x[n-1]);
}

/* Function with parallel annotation */
void test_parallel(int *data, int n) {
    int i;
    int local_sum = 0;
    #pragma GCC parallel
    for (i = 0; i < n; i++) {
        local_sum += data[i];  /* Independent iterations */
    }
    use(local_sum);
}

/* Function with unroll annotation (maybe-infinite) */
void test_maybe_infinite(int *arr, int n) {
    int i;
    int prod = 1;
    #pragma GCC unroll
    for (i = 0; i < n; i++) {
        prod *= arr[i] + 1;
    }
    use(prod);
}

/* Nested loops with annotations */
void test_nested(int *mat, int rows, int cols) {
    int i, j;
    
    /* Outer loop with ivdep */
    #pragma GCC ivdep
    for (i = 0; i < rows; i++) {
        /* Inner loop with vector hint */
        #pragma GCC vector
        for (j = 0; j < cols; j++) {
            mat[i*cols + j] += i + j;
        }
    }
    use(mat[(rows-1)*cols + (cols-1)]);
}

/* Annotated loop in conditional context */
void test_conditional(int *arr, int n, int flag) {
    if (flag) {
        int i;
        #pragma GCC parallel
        for (i = 0; i < n; i++) {
            arr[i] = arr[i] * 3;
        }
        use(arr[n-1]);
    } else {
        int i;
        #pragma GCC ivdep
        for (i = n-1; i >= 0; i--) {
            arr[i] = arr[i] / 2;
        }
        use(arr[0]);
    }
}

/* Main function with mixed annotated loops */
int main(void) {
    const int N = 100;
    const int M = 50;
    int i, result = 0;
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(N * sizeof(int));
    int *b = (int*)malloc(N * sizeof(int));
    float *x = (float*)malloc(N * sizeof(float));
    float *y = (float*)malloc(N * sizeof(float));
    int *data = (int*)malloc(N * sizeof(int));
    int *arr = (int*)malloc(M * sizeof(int));
    int *mat = (int*)malloc(N * N * sizeof(int));
    
    if (!a || !b || !x || !y || !data || !arr || !mat) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i;
        x[i] = (float)i;
        y[i] = (float)(i * 2);
        data[i] = i % 10;
    }
    for (i = 0; i < M; i++) {
        arr[i] = (i % 5) + 1;
    }
    for (i = 0; i < N * N; i++) {
        mat[i] = i % 100;
    }
    
    /* Execute all test functions */
    test_no_vector(a, b, N);
    test_vector(x, y, N);
    test_parallel(data, N);
    test_maybe_infinite(arr, M);
    test_nested(mat, 10, 10);
    test_conditional(a, N, 1);
    
    /* Compute final checksum */
    for (i = 0; i < N; i++) {
        result += a[i] + b[i] + (int)x[i] + data[i];
    }
    for (i = 0; i < M; i++) {
        result += arr[i];
    }
    for (i = 0; i < 100; i++) {
        result += mat[i];
    }
    
    printf("Result: %d\n", result);
    
    /* Cleanup */
    free(a); free(b); free(x); free(y);
    free(data); free(arr); free(mat);
    
    return 0;
}
