#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 100
#define M 50

/* Force memory dependencies with volatile */
volatile int volatile_array[SIZE];

/* Test 1: Loop-carried flow dependence with distance > 0 */
__attribute__((always_inline))
static inline void test_flow_dependence_distance(int *a, int *b, int n) {
    /* Flow dependence with distance 2 */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i];
    }
    
    /* Flow dependence with distance 1, different data type */
    float *fa = (float *)a;
    float *fb = (float *)b;
    for (int i = 1; i < n; i++) {
        fa[i] = fa[i - 1] + fb[i];
    }
}

/* Test 2: Multiple dependence types in single loop */
__attribute__((always_inline))
static inline void test_mixed_dependences(int *a, int *b, int *c, int n) {
    int t;
    
    /* Contains flow (RAW), anti (WAR), and output (WAW) dependences */
    for (int i = 1; i < n; i++) {
        t = a[i - 1];           /* Flow: read a[i-1] */
        a[i] = t + b[i];        /* Flow: use t, Anti: write a[i] after read in next iter? */
        c[i] = c[i - 1] * 2;    /* Flow: read c[i-1] */
        b[i] = b[i] + 1;        /* Output: write b[i] that was read earlier */
    }
}

/* Test 3: Pointer aliasing with potential self-overlap */
static void test_pointer_aliasing(int *p, int *q, int n) {
    /* No restrict - compiler must assume aliasing */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n];
    }
}

/* Test 4: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
static inline void test_nested_loops(int arr[M][N]) {
    /* Inner loop has distance 2 flow dependence */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 2
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + 1;
        }
    }
}

/* Test 5: Volatile memory dependencies */
static void test_volatile_dependence(void) {
    /* Simple volatile flow dependence */
    for (int i = 1; i < SIZE; i++) {
        volatile_array[i] = volatile_array[i - 1] + i;
    }
    
    /* More complex volatile pattern with distance */
    for (int i = 2; i < SIZE; i++) {
        volatile_array[i] = volatile_array[i - 2] * volatile_array[i - 1];
    }
}

/* Test 6: Complex loop with multiple arrays and indices */
static void test_complex_indices(int *a, int *b, int *c, int n) {
    /* Multiple interleaved dependencies */
    for (int i = 3; i < n; i++) {
        a[i] = b[i - 1] + c[i - 2];          /* Flow from b[i-1], c[i-2] */
        b[i] = a[i - 3] * 2;                 /* Flow from a[i-3], distance 3 */
        c[i] = a[i - 1] + b[i - 1];          /* Flow from a[i-1], b[i-1] */
    }
}

/* Test 7: Software pipelining candidate with reduction */
static int test_reduction_loop(int *data, int n) {
    int sum = 0;
    
    /* Reduction with carried dependence on sum */
    for (int i = 0; i < n; i++) {
        sum += data[i];
        data[i] = sum;  /* Flow dependence on sum */
    }
    
    return sum;
}

/* Test 8: Loop with if-conversion potential */
static void test_conditional_dependence(int *a, int *b, int n) {
    for (int i = 1; i < n; i++) {
        if (b[i] > 0) {
            a[i] = a[i - 1] + b[i];  /* Conditional flow dependence */
        } else {
            a[i] = a[i - 1] - b[i];  /* Also flow dependence */
        }
        b[i] = a[i] * 2;             /* Anti-dependence */
    }
}

int main(void) {
    /* Initialize with some data */
    int *array1 = (int *)malloc(SIZE * sizeof(int));
    int *array2 = (int *)malloc(SIZE * sizeof(int));
    int *array3 = (int *)malloc(SIZE * sizeof(int));
    int nested_arr[M][N];
    
    srand(time(NULL));
    
    for (int i = 0; i < SIZE; i++) {
        array1[i] = rand() % 100;
        array2[i] = rand() % 100;
        array3[i] = rand() % 100;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            nested_arr[i][j] = rand() % 100;
        }
    }
    
    /* Initialize volatile array */
    for (int i = 0; i < SIZE; i++) {
        volatile_array[i] = rand() % 100;
    }
    
    volatile int checksum = 0;
    
    /* Execute all tests to trigger various DDG edge creations */
    test_flow_dependence_distance(array1, array2, SIZE);
    checksum += array1[SIZE / 2];
    
    test_mixed_dependences(array1, array2, array3, SIZE);
    checksum += array2[SIZE / 2] + array3[SIZE / 2];
    
    test_pointer_aliasing(array1, array2, SIZE);
    checksum += array1[SIZE / 3];
    
    test_nested_loops(nested_arr);
    checksum += nested_arr[M/2][N/2];
    
    test_volatile_dependence();
    checksum += volatile_array[SIZE / 2];
    
    test_complex_indices(array1, array2, array3, SIZE);
    checksum += array1[SIZE / 4] + array2[SIZE / 4];
    
    int reduction_sum = test_reduction_loop(array1, SIZE);
    checksum += reduction_sum;
    
    test_conditional_dependence(array2, array3, SIZE);
    checksum += array2[SIZE / 2];
    
    /* Use results to prevent dead code elimination */
    printf("Final checksum: %d\n", checksum);
    
    /* Print a sample to ensure computations happened */
    printf("Sample values: %d, %d, %d\n", 
           array1[10], array2[20], array3[30]);
    
    free(array1);
    free(array2);
    free(array3);
    
    return 0;
}
