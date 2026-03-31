#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 100
#define M 50

/* Force inlining to create larger basic blocks */
__attribute__((always_inline)) 
static inline int compute_hash(int x) {
    return (x * 1103515245 + 12345) & 0x7fffffff;
}

/* Test 1: Loop-carried flow dependence with distance > 0 */
void test_flow_dependence(int *a, int *b, int n, int *checksum) {
    volatile int *volatile_a = a; /* Force memory dependence */
    
    /* Flow dependence with distance 2 */
    for (int i = 0; i < n - 2; i++) {
        volatile_a[i + 2] = volatile_a[i] * b[i];
        *checksum += volatile_a[i + 2];
    }
    
    /* Flow dependence with distance 1, mixed data types */
    float *fa = (float *)a;
    for (int i = 1; i < n; i++) {
        fa[i] = fa[i - 1] + 0.5f;
        *checksum += (int)fa[i];
    }
}

/* Test 2: Multiple dependence types in single loop */
void test_mixed_dependences(int *a, int *b, int *c, int n, int *checksum) {
    int t = 0;
    
    /* Contains flow (RAW), anti (WAR), and output (WAW) dependences */
    for (int i = 1; i < n; i++) {
        t = a[i - 1];               /* Read a[i-1] - creates flow dep from previous iteration */
        a[i] = t + b[i];            /* Write a[i] - creates anti dep for next iteration's read */
        c[i] = c[i - 1] * 2;        /* Flow and output dependence on c */
        *checksum += a[i] + c[i];
    }
}

/* Test 3: Pointer aliasing with potential self-overlap */
void test_pointer_aliasing(int *restrict p, int *restrict q, int *r, int n, int *checksum) {
    /* r is not restrict - compiler must assume aliasing */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + r[(i * 7) % n];  /* Potential flow dependence if r aliases p */
        *checksum += p[i];
    }
    
    /* No restrict - full aliasing assumed */
    void test_no_restrict(int *p, int *q, int n, int *checksum) {
        for (int i = 1; i < n; i++) {
            p[i] = p[i - 1] + q[i];    /* Clear flow dependence */
            *checksum += p[i];
        }
    }
    test_no_restrict(p, q, n, checksum);
}

/* Test 4: Nested loops with inner loop carried dependence */
void test_nested_loops(int arr[M][N], int *checksum) {
    /* Inner loop has distance 2 flow dependence */
    for (int i = 0; i < M; i++) {
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + i + j;
            *checksum += arr[i][j];
        }
    }
    
    /* Outer loop carried dependence */
    for (int i = 1; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = arr[i - 1][j] * 2;
            *checksum += arr[i][j];
        }
    }
}

/* Test 5: Volatile memory accesses */
void test_volatile_access(int *checksum) {
    volatile int varr[SIZE];
    
    /* Simple flow dependence through volatile */
    for (int i = 1; i < SIZE; i++) {
        varr[i] = varr[i - 1] + i;
        *checksum += varr[i];
    }
    
    /* Anti-dependence with volatile */
    int temp = 0;
    for (int i = 0; i < SIZE - 1; i++) {
        temp = varr[i];          /* Read */
        varr[i] = varr[i + 1];   /* Write - creates anti-dependence */
        *checksum += temp;
    }
}

/* Test 6: Complex index calculations */
void test_complex_indices(int *a, int *b, int n, int *checksum) {
    /* Non-linear access pattern */
    for (int i = 0; i < n; i++) {
        int idx = (i * 3 + 7) % n;
        int idx2 = (i * 5 + 11) % n;
        a[idx] = b[idx2] + a[(idx + 1) % n];
        *checksum += a[idx];
    }
}

/* Test 7: Software pipelining candidate */
#pragma GCC unroll 4
void test_modulo_sched_candidate(double *a, double *b, double *c, int n, int *checksum) {
    /* Triple nested dependency chain */
    for (int i = 2; i < n - 2; i++) {
        a[i] = b[i - 2] * c[i - 1];
        b[i] = a[i] + a[i - 1];
        c[i] = b[i] * b[i - 1];
        *checksum += (int)(a[i] + b[i] + c[i]);
    }
}

int main() {
    /* Initialize with deterministic but non-trivial data */
    srand(42);
    
    int *a = malloc(SIZE * sizeof(int));
    int *b = malloc(SIZE * sizeof(int));
    int *c = malloc(SIZE * sizeof(int));
    int *d = malloc(SIZE * sizeof(int));
    int arr[M][N];
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        d[i] = rand() % 100;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = rand() % 100;
        }
    }
    
    volatile int checksum = 0; /* Volatile to prevent dead code elimination */
    
    /* Execute all tests to trigger various DDG edge creations */
    test_flow_dependence(a, b, SIZE, (int *)&checksum);
    test_mixed_dependences(a, b, c, SIZE, (int *)&checksum);
    test_pointer_aliasing(a, b, d, SIZE, (int *)&checksum);
    test_nested_loops(arr, (int *)&checksum);
    test_volatile_access((int *)&checksum);
    test_complex_indices(a, b, SIZE, (int *)&checksum);
    
    double *da = malloc(SIZE * sizeof(double));
    double *db = malloc(SIZE * sizeof(double));
    double *dc = malloc(SIZE * sizeof(double));
    
    for (int i = 0; i < SIZE; i++) {
        da[i] = rand() % 100 * 0.5;
        db[i] = rand() % 100 * 0.5;
        dc[i] = rand() % 100 * 0.5;
    }
    
    test_modulo_sched_candidate(da, db, dc, SIZE, (int *)&checksum);
    
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(d);
    free(da);
    free(db);
    free(dc);
    
    return 0;
}
