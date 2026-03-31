#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 256
#define N 100
#define M 50

/* Test 1: Loop-carried flow dependence with distance > 0 */
__attribute__((always_inline))
inline void test_flow_dependence(int *a, int *b, int n) {
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
inline void test_mixed_dependences(int *a, int *b, int *c, int n) {
    int t;
    /* Contains flow (RAW), anti (WAR), and output (WAW) dependences */
    for (int i = 1; i < n; i++) {
        t = a[i - 1];           /* Flow: read a[i-1] */
        a[i] = t + b[i];        /* Flow: use t, Anti: write a[i] after read in next iter? */
        c[i] = c[i - 1] * 2;    /* Flow: read c[i-1], Output: write c[i] */
    }
}

/* Test 3: Pointer aliasing with potential self-overlap */
void test_pointer_aliasing(int *p, int *q, int n) {
    /* No restrict - compiler must assume aliasing */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n];
    }
}

void test_restrict_pointers(int *restrict p, int *restrict q, int n) {
    /* With restrict - compiler may optimize more aggressively */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + 1;
    }
}

/* Test 4: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
inline void test_nested_loops(int arr[M][N]) {
    /* Inner loop has carried dependence with distance 2 */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 2
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + i + j;
        }
    }
}

/* Test 5: Volatile memory accesses to force dependences */
void test_volatile_access(volatile int *v, int n) {
    /* Strong memory dependence enforced by volatile */
    for (int i = 1; i < n; i++) {
        v[i] = v[i - 1] + i;
    }
}

/* Test 6: Complex addressing with mixed data types */
void test_complex_addressing(double *d, int *idx, int n) {
    for (int i = 1; i < n; i++) {
        int j = idx[i] % n;
        d[i] = d[j] * 1.5 + d[i - 1];
    }
}

/* Test 7: Loop with multiple carried dependences */
void test_multiple_carried(int *a, int *b, int *c, int n) {
    for (int i = 3; i < n; i++) {
        a[i] = a[i - 1] + b[i - 2];  /* Distance 1 and 2 */
        c[i] = c[i - 3] * a[i];      /* Distance 3 */
    }
}

int main() {
    /* Initialize with some data */
    int *a = (int*)malloc(SIZE * sizeof(int));
    int *b = (int*)malloc(SIZE * sizeof(int));
    int *c = (int*)malloc(SIZE * sizeof(int));
    volatile int *v = (volatile int*)malloc(SIZE * sizeof(int));
    double *d = (double*)malloc(SIZE * sizeof(double));
    int *idx = (int*)malloc(SIZE * sizeof(int));
    int arr[M][N];
    
    srand(time(NULL));
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        v[i] = rand() % 100;
        d[i] = (double)(rand() % 100) / 10.0;
        idx[i] = rand() % SIZE;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = rand() % 100;
        }
    }
    
    volatile int checksum = 0;
    
    /* Execute all tests to trigger various DDG edge creations */
    test_flow_dependence(a, b, SIZE);
    checksum += a[SIZE/2] + b[SIZE/2];
    
    test_mixed_dependences(a, b, c, SIZE);
    checksum += a[SIZE/3] + c[SIZE/3];
    
    test_pointer_aliasing(a, b, SIZE);
    checksum += a[SIZE/4];
    
    test_restrict_pointers(b, c, SIZE);
    checksum += b[SIZE/5];
    
    test_nested_loops(arr);
    checksum += arr[M/2][N/2];
    
    test_volatile_access(v, SIZE);
    checksum += v[SIZE/6];
    
    test_complex_addressing(d, idx, SIZE);
    checksum += (int)d[SIZE/7];
    
    test_multiple_carried(a, b, c, SIZE);
    checksum += a[SIZE/8] + c[SIZE/8];
    
    /* Final computation using all results */
    int final_result = 0;
    for (int i = 0; i < SIZE; i += 8) {
        final_result += a[i] + b[i] + c[i] + (int)d[i] + v[i];
    }
    
    printf("Checksum: %d, Final: %d\n", checksum, final_result);
    
    free(a);
    free(b);
    free(c);
    free((void*)v);
    free(d);
    free(idx);
    
    return 0;
}
