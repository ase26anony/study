#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define M 64
#define N 128

/* Pattern 1: Loop-carried flow dependence with distance > 0 */
__attribute__((always_inline))
inline void pattern1_flow_distance(int *a, int *b, int n) {
    /* Flow dependence with distance 2 */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i];
    }
}

/* Pattern 2: Multiple dependence types in single loop */
__attribute__((always_inline))
inline void pattern2_mixed_deps(int *a, int *b, int *c, int n) {
    int t;
    /* Contains flow (RAW), anti (WAR), and output (WAW) dependences */
    for (int i = 1; i < n; i++) {
        t = a[i - 1];           /* Flow: read a[i-1] */
        a[i] = t + b[i];        /* Flow: use t, Anti: write a[i] after read in next iter? */
        c[i] = c[i - 1] * 2;    /* Flow: read c[i-1], Output: write c[i] */
    }
}

/* Pattern 3: Indirect addressing with potential aliasing */
void pattern3_indirect(int *p, int *q, int n) {
    /* No restrict - compiler must assume p and q may alias */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n];  /* Potential flow dependence if p overlaps */
    }
}

/* Pattern 3b: With restrict for comparison */
void pattern3b_restrict(int *__restrict p, int *__restrict q, int n) {
    /* With restrict - compiler may assume no aliasing */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 13) % n];
    }
}

/* Pattern 4: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
inline void pattern4_nested(int arr[M][N]) {
    /* Inner loop has distance 2 flow dependence */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 2
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + 1;
        }
    }
}

/* Pattern 5: Volatile memory accesses to force dependencies */
void pattern5_volatile(volatile int *v, int n) {
    /* Strong memory dependence enforced by volatile */
    for (int i = 1; i < n; i++) {
        v[i] = v[i - 1] + i;
    }
}

/* Pattern 6: Different data types for different edge data types */
__attribute__((always_inline))
inline void pattern6_mixed_types(float *fa, double *da, int *ia, int n) {
    /* Mix of data types in dependencies */
    for (int i = 1; i < n; i++) {
        fa[i] = fa[i - 1] * 1.5f;      /* float flow dependence */
        da[i] = da[i - 1] / 2.0;       /* double flow dependence */
        ia[i] = ia[i - 1] + i;         /* int flow dependence */
    }
}

/* Pattern 7: Complex index expressions with multiple distances */
void pattern7_complex_indices(int *a, int *b, int n) {
    for (int i = 3; i < n; i++) {
        /* Multiple interleaved dependencies with different distances */
        a[i] = b[i] + a[i - 1];        /* distance 1 */
        b[i] = a[i - 2] * 2;           /* distance 2 */
        a[i] += b[i - 3];              /* distance 3 */
    }
}

/* Pattern 8: While loop with carried dependence */
void pattern8_while_loop(int *x, int *y, int n) {
    int i = 1;
    while (i < n) {
        x[i] = x[i - 1] + y[i];
        y[i] = x[i] * 2;               /* Anti-dependence through x[i] */
        i++;
    }
}

int main() {
    /* Initialize with deterministic but non-trivial data */
    srand(42);
    
    /* Allocate and initialize arrays */
    int *a1 = (int*)malloc(SIZE * sizeof(int));
    int *a2 = (int*)malloc(SIZE * sizeof(int));
    int *b = (int*)malloc(SIZE * sizeof(int));
    int *c = (int*)malloc(SIZE * sizeof(int));
    int *p = (int*)malloc(SIZE * sizeof(int));
    int *q = (int*)malloc(SIZE * sizeof(int));
    volatile int *v = (volatile int*)malloc(SIZE * sizeof(int));
    float *fa = (float*)malloc(SIZE * sizeof(float));
    double *da = (double*)malloc(SIZE * sizeof(double));
    int *ia = (int*)malloc(SIZE * sizeof(int));
    int arr[M][N];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        a1[i] = rand() % 100;
        a2[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        p[i] = rand() % 100;
        q[i] = rand() % 100;
        v[i] = rand() % 100;
        fa[i] = (float)(rand() % 100) / 10.0f;
        da[i] = (double)(rand() % 100) / 10.0;
        ia[i] = rand() % 100;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = rand() % 100;
        }
    }
    
    volatile int checksum = 0;
    
    /* Execute all patterns to trigger various DDG edge creations */
    pattern1_flow_distance(a1, b, SIZE);
    pattern2_mixed_deps(a2, b, c, SIZE);
    pattern3_indirect(p, q, SIZE);
    pattern3b_restrict(p, q, SIZE);
    pattern4_nested(arr);
    pattern5_volatile(v, SIZE);
    pattern6_mixed_types(fa, da, ia, SIZE);
    pattern7_complex_indices(a1, b, SIZE);
    pattern8_while_loop(a2, c, SIZE);
    
    /* Compute checksum to ensure all computations are live */
    for (int i = 0; i < SIZE; i++) {
        checksum += a1[i] + a2[i] + b[i] + c[i] + p[i] + q[i] + v[i] + 
                   (int)fa[i] + (int)da[i] + ia[i];
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            checksum += arr[i][j];
        }
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a1); free(a2); free(b); free(c);
    free(p); free(q); free((void*)v);
    free(fa); free(da); free(ia);
    
    return 0;
}
