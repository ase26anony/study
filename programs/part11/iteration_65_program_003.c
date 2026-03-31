#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 100
#define M 50

/* Test 1: Loop-carried flow dependence with distance > 0 */
__attribute__((always_inline))
static inline void test_flow_dependence(int *a, int *b, int n) {
    /* Flow dependence with distance 2 */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i];
    }
    
    /* Flow dependence with distance 1, different data type */
    volatile float *vf = (volatile float*)a;
    for (int i = 0; i < n - 1; i++) {
        vf[i + 1] = vf[i] + 1.5f;
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
        c[i] = c[i - 1] * 2;    /* Flow: read c[i-1], Output: write c[i] */
    }
}

/* Test 3: Indirect addressing with potential aliasing */
static void test_indirect_addressing(int *p, int *q, int n) {
    /* No restrict - compiler must assume aliasing */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n];  /* Potential flow dependence */
    }
}

/* With restrict - compiler can assume no aliasing */
static void test_indirect_noalias(int *restrict p, int *restrict q, int n) {
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 13) % n];  /* Still self-dependence */
    }
}

/* Test 4: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
static inline void test_nested_dependence(int arr[M][N]) {
    /* Inner loop has carried dependence with distance 2 */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 4
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + 1;
        }
    }
}

/* Test 5: Volatile memory accesses to force dependencies */
static void test_volatile_dependence(void) {
    volatile int varr[100];
    
    /* Simple volatile flow dependence */
    for (int i = 1; i < 100; i++) {
        varr[i] = varr[i - 1] + i;
    }
    
    /* Volatile anti-dependence */
    volatile int temp;
    for (int i = 0; i < 99; i++) {
        temp = varr[i];     /* Read */
        varr[i] = varr[i + 1]; /* Write to same location read in next iteration? */
        varr[i + 1] = temp;
    }
}

/* Test 6: Complex pointer arithmetic with potential aliasing */
static void test_pointer_aliasing(int *base, int offset1, int offset2, int n) {
    int *p1 = base + offset1;
    int *p2 = base + offset2;
    
    /* Compiler can't easily analyze these pointers */
    for (int i = 0; i < n; i++) {
        p1[i] = p2[i * 2 % n] + base[i];
    }
}

/* Test 7: Different data types for different edge data_type fields */
static void test_multiple_datatypes(void) {
    int int_arr[SIZE];
    float float_arr[SIZE];
    double double_arr[SIZE];
    
    /* Integer flow dependence */
    for (int i = 1; i < SIZE; i++) {
        int_arr[i] = int_arr[i - 1] + i;
    }
    
    /* Float anti-dependence */
    for (int i = 0; i < SIZE - 1; i++) {
        float temp = float_arr[i];
        float_arr[i] = float_arr[i + 1];
        float_arr[i + 1] = temp;
    }
    
    /* Double output dependence */
    for (int i = 0; i < SIZE; i++) {
        double_arr[i] = i * 0.5;
        double_arr[i] = double_arr[i] * 2.0; /* WAW on same location */
    }
}

int main(void) {
    /* Initialize with some data */
    int *a = malloc(SIZE * sizeof(int));
    int *b = malloc(SIZE * sizeof(int));
    int *c = malloc(SIZE * sizeof(int));
    int arr_2d[M][N];
    
    srand(time(NULL));
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr_2d[i][j] = rand() % 100;
        }
    }
    
    volatile int checksum = 0;
    
    /* Execute all tests to trigger various DDG edge creations */
    test_flow_dependence(a, b, SIZE);
    
    for (int i = 0; i < SIZE; i++) checksum += a[i];
    
    test_mixed_dependences(a, b, c, SIZE);
    
    for (int i = 0; i < SIZE; i++) checksum += a[i] + c[i];
    
    test_indirect_addressing(a, b, SIZE);
    test_indirect_noalias(b, c, SIZE);
    
    for (int i = 0; i < SIZE; i++) checksum += b[i];
    
    test_nested_dependence(arr_2d);
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            checksum += arr_2d[i][j];
        }
    }
    
    test_volatile_dependence();
    
    test_pointer_aliasing(a, 10, 20, SIZE - 30);
    
    for (int i = 0; i < SIZE; i++) checksum += a[i];
    
    test_multiple_datatypes();
    
    printf("Final checksum: %d\n", checksum);
    
    free(a);
    free(b);
    free(c);
    
    return 0;
}
