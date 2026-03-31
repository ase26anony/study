#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define M 64
#define N 128

/* Test 1: Loop-carried flow dependence with distance > 0 */
__attribute__((always_inline))
static inline void test_flow_dependence(int *a, int *b, int n, volatile int *checksum) {
    /* Flow dependence: distance 2 */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i];
        *checksum += a[i + 2];
    }
    
    /* Flow dependence: distance 1 with float */
    volatile float *vf = (volatile float *)a;
    for (int i = 1; i < n; i++) {
        vf[i] = vf[i - 1] + 0.5f;
        *checksum += (int)vf[i];
    }
}

/* Test 2: Multiple dependence types in single loop */
__attribute__((always_inline))
static inline void test_mixed_dependences(int *a, int *b, int *c, int n, volatile int *checksum) {
    int t;
    /* Contains flow (RAW), anti (WAR), and output (WAW) dependences */
    for (int i = 1; i < n; i++) {
        t = a[i - 1];               /* Read a[i-1] */
        a[i] = t + b[i];            /* Write a[i] - flow dependence on t, anti on a[i] */
        c[i] = c[i - 1] * 2;        /* Flow on c[i-1], output on c[i] */
        *checksum += a[i] + c[i];
    }
}

/* Test 3: Pointer aliasing with potential self-overlap */
static void test_pointer_aliasing(int *p, int *q, int n, volatile int *checksum) {
    /* No restrict - compiler must assume p and q may alias */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n];  /* Potential flow dependence due to p self-overlap */
        *checksum += p[i];
    }
}

/* Test 4: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
static inline void test_nested_loops(int arr[M][N], volatile int *checksum) {
    /* Inner loop has distance 2 flow dependence */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 2
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + i + j;
            *checksum += arr[i][j];
        }
    }
}

/* Test 5: Volatile memory accesses forcing dependences */
static void test_volatile_access(volatile int *v, int n, volatile int *checksum) {
    /* All accesses are volatile - cannot be reordered */
    for (int i = 1; i < n; i++) {
        v[i] = v[i - 1] + i;
        *checksum += v[i];
    }
    
    /* Double type with volatile */
    volatile double vd[100];
    for (int i = 2; i < 50; i++) {
        vd[i] = vd[i - 2] * 1.1;
        *checksum += (int)vd[i];
    }
}

/* Test 6: Complex index calculations with modulo */
static void test_complex_indices(int *data, int n, volatile int *checksum) {
    /* Complex addressing that may create various dependence distances */
    for (int i = 0; i < n; i++) {
        int idx1 = (i * 3) % n;
        int idx2 = (i + 5) % n;
        int idx3 = (i * 2 + 1) % n;
        
        data[idx2] = data[idx1] + data[idx3];
        *checksum += data[idx2];
    }
}

/* Test 7: Software pipelining candidate with multiple accumulators */
__attribute__((always_inline))
static inline void test_software_pipeline(int *a, int *b, int *c, int n, volatile int *checksum) {
    /* Multiple interleaved dependences good for modulo scheduling */
    int acc1 = 0, acc2 = 0;
    for (int i = 0; i < n; i++) {
        acc1 = acc1 + a[i];      /* Flow on acc1 */
        b[i] = acc1 * 2;         /* Flow on acc1 */
        acc2 = acc2 + b[i];      /* Flow on acc2, anti on b[i] */
        c[i] = acc2 - a[i];      /* Flow on acc2, anti on a[i] */
        *checksum += c[i];
    }
}

int main(void) {
    /* Initialize with deterministic values */
    srand(42);
    
    /* Allocate and initialize arrays */
    int *a = (int *)malloc(SIZE * sizeof(int));
    int *b = (int *)malloc(SIZE * sizeof(int));
    int *c = (int *)malloc(SIZE * sizeof(int));
    int *q = (int *)malloc(SIZE * sizeof(int));
    volatile int *v = (volatile int *)malloc(SIZE * sizeof(int));
    int arr[M][N];
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        q[i] = rand() % 100;
        v[i] = rand() % 100;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = rand() % 100;
        }
    }
    
    volatile int checksum = 0;
    
    /* Execute all test patterns to trigger various DDG edge creations */
    test_flow_dependence(a, b, SIZE, &checksum);
    test_mixed_dependences(a, b, c, SIZE, &checksum);
    test_pointer_aliasing(a, q, SIZE, &checksum);
    test_nested_loops(arr, &checksum);
    test_volatile_access(v, SIZE, &checksum);
    test_complex_indices(a, SIZE, &checksum);
    test_software_pipeline(a, b, c, SIZE, &checksum);
    
    /* Use the checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(q);
    free((void *)v);
    
    return 0;
}
