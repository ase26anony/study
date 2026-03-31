#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 1000
#define M 100

/* Test 1: Flow dependence with distance > 0 */
__attribute__((always_inline))
inline void test_flow_dependence(int *a, int *b, int n, int *checksum) {
    /* Distance 2 flow dependence */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i];
        *checksum += a[i + 2];
    }
    
    /* Distance 1 flow dependence with different data type */
    volatile float *vf = (volatile float *)a;
    for (int i = 1; i < n; i++) {
        vf[i] = vf[i - 1] + 1.5f;
        *checksum += (int)vf[i];
    }
}

/* Test 2: Multiple dependence types in single loop */
__attribute__((always_inline))
inline void test_mixed_dependences(int *a, int *b, int *c, int n, int *checksum) {
    int t;
    /* Contains flow (RAW), anti (WAR), and output (WAW) dependences */
    for (int i = 1; i < n; i++) {
        t = a[i - 1];           /* Flow: read a[i-1] */
        a[i] = t + b[i];        /* Flow: use t, Anti: write a[i] after read in next iter? */
        c[i] = c[i - 1] * 2;    /* Flow: read c[i-1], Output: write c[i] */
        *checksum += a[i] + c[i];
    }
}

/* Test 3: Pointer aliasing with potential self-overlap */
void test_pointer_aliasing(int *p, int *q, int n, int *checksum) {
    /* No restrict - compiler must assume p and q may alias */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n];  /* Potential flow dependence */
        *checksum += p[i];
    }
}

/* Test 3b: With restrict for comparison */
void test_restrict_pointers(int *__restrict__ rp, int *__restrict__ rq, int n, int *checksum) {
    /* With restrict - compiler may optimize more aggressively */
    for (int i = 0; i < n; i++) {
        rp[i] = rq[i] + rp[(i * 3) % n];  /* Still self-dependence */
        *checksum += rp[i];
    }
}

/* Test 4: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
inline void test_nested_loops(int arr[M][N], int *checksum) {
    /* Inner loop has distance 2 flow dependence */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 2
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + 1;
            *checksum += arr[i][j];
        }
    }
}

/* Test 5: Volatile memory for forced dependences */
void test_volatile_dependence(volatile int *v, int n, int *checksum) {
    /* Strong forced memory dependence */
    for (int i = 1; i < n; i++) {
        v[i] = v[i - 1] + i;
        *checksum += v[i];
    }
}

/* Test 6: Complex loop with multiple arrays and indices */
void test_complex_dependences(int *a, int *b, int *c, int *d, int n, int *checksum) {
    for (int i = 2; i < n - 2; i++) {
        /* Multiple interleaved dependences */
        a[i] = b[i - 1] + c[i - 2];      /* Flow from b and c */
        b[i + 1] = a[i] * 3;             /* Flow from a, Anti with next iter's read of b[i-1] */
        c[i] = d[i] + a[i - 1];          /* Flow from a, Output with c[i-2] read */
        d[i + 1] = b[i] - c[i];          /* Flow from b and c */
        
        *checksum += a[i] + b[i + 1] + c[i] + d[i + 1];
    }
}

/* Test 7: Different data types for different edge data_type fields */
void test_mixed_data_types(float *fa, double *da, int *ia, int n, int *checksum) {
    for (int i = 1; i < n; i++) {
        fa[i] = fa[i - 1] * 1.1f;        /* float flow dependence */
        da[i] = da[i - 1] / 2.0;         /* double flow dependence */
        ia[i] = (int)fa[i] + (int)da[i]; /* int flow from float/double */
        *checksum += ia[i];
    }
}

int main() {
    /* Initialize with some data */
    int *a = (int*)malloc(SIZE * sizeof(int));
    int *b = (int*)malloc(SIZE * sizeof(int));
    int *c = (int*)malloc(SIZE * sizeof(int));
    int *d = (int*)malloc(SIZE * sizeof(int));
    float *fa = (float*)malloc(SIZE * sizeof(float));
    double *da = (double*)malloc(SIZE * sizeof(double));
    int arr[M][N];
    volatile int *v = (volatile int*)malloc(SIZE * sizeof(int));
    
    srand(time(NULL));
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        d[i] = rand() % 100;
        fa[i] = (float)(rand() % 100) / 10.0f;
        da[i] = (double)(rand() % 100) / 10.0;
        if (i < SIZE) v[i] = rand() % 100;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = rand() % 100;
        }
    }
    
    volatile int checksum = 0;
    
    /* Execute all tests to trigger various DDG edge creations */
    test_flow_dependence(a, b, SIZE, (int*)&checksum);
    test_mixed_dependences(a, b, c, SIZE, (int*)&checksum);
    test_pointer_aliasing(a, b, SIZE, (int*)&checksum);
    test_restrict_pointers(c, d, SIZE, (int*)&checksum);
    test_nested_loops(arr, (int*)&checksum);
    test_volatile_dependence(v, SIZE, (int*)&checksum);
    test_complex_dependences(a, b, c, d, SIZE, (int*)&checksum);
    test_mixed_data_types(fa, da, a, SIZE, (int*)&checksum);
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    free(fa);
    free(da);
    free((void*)v);
    
    return 0;
}
