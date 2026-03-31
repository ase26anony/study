#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define M 64
#define N 128

/* Pattern 1: Loop-carried flow dependence with distance > 0 */
__attribute__((always_inline))
static inline void pattern1_flow_distance(int *a, int *b, int n) {
    /* Flow dependence with distance 2 */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i];
    }
    
    /* Flow dependence with distance 3, different data type */
    volatile float *vf = (volatile float *)a;
    for (int i = 0; i < n - 3; i++) {
        vf[i + 3] = vf[i] + 1.5f;
    }
}

/* Pattern 2: Multiple dependence types in single loop */
__attribute__((always_inline))
static inline void pattern2_mixed_deps(int *a, int *b, int *c, int n) {
    int t;
    /* Contains flow (RAW), anti (WAR), and output (WAW) dependences */
    for (int i = 1; i < n; i++) {
        t = a[i - 1];           /* Flow: read a[i-1] */
        a[i] = t + b[i];        /* Flow: use t, Anti: write a[i] after read in next iter? */
        c[i] = c[i - 1] * 2;    /* Flow: read c[i-1], Output: write c[i] */
    }
}

/* Pattern 3: Pointer aliasing with non-trivial indexing */
static void pattern3_aliasing(int *p, int *q, int n) {
    /* No restrict - compiler must assume aliasing */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n];
    }
}

static void pattern3_noalias(int *restrict p, int *restrict q, int n) {
    /* With restrict - compiler knows no aliasing */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + 1;
    }
}

/* Pattern 4: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
static inline void pattern4_nested(int arr[M][N]) {
    /* Inner loop has distance 2 flow dependence */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 2
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + i + j;
        }
    }
}

/* Pattern 5: Volatile memory accesses forcing dependences */
static void pattern5_volatile_deps(void) {
    volatile int varr[100];
    volatile int vsum = 0;
    
    /* Chain of volatile flow dependences */
    for (int i = 1; i < 100; i++) {
        varr[i] = varr[i - 1] + i;
    }
    
    /* Anti-dependence with volatile */
    for (int i = 0; i < 99; i++) {
        vsum = varr[i];    /* Read */
        varr[i] = vsum + 1; /* Write - WAR */
    }
}

/* Pattern 6: Complex loop with multiple interleaved dependences */
static void pattern6_complex(int *a, int *b, int *c, int n) {
    int t1, t2;
    
    /* Multiple interleaved dependences with varying distances */
    for (int i = 2; i < n - 2; i++) {
        t1 = a[i - 2];              /* Flow distance 2 */
        t2 = b[i + 1];              /* Anti dependence potential */
        a[i] = t1 + t2;             /* Flow from t1, anti from t2? */
        b[i - 1] = c[i] * 3;        /* Output on b, flow from c */
        c[i + 1] = a[i] + b[i - 1]; /* Flow from a[i] and b[i-1] */
    }
}

/* Pattern 7: Double type for different data_type field */
static void pattern7_double_deps(double *da, double *db, int n) {
    /* Double precision flow dependence */
    for (int i = 1; i < n; i++) {
        da[i] = da[i - 1] + db[i];
    }
    
    /* Double precision with distance 4 */
    for (int i = 4; i < n; i++) {
        db[i] = db[i - 4] * 0.5;
    }
}

int main(void) {
    /* Initialize with deterministic but non-trivial data */
    srand(42);
    
    /* Allocate and initialize arrays */
    int *a = (int *)malloc(SIZE * sizeof(int));
    int *b = (int *)malloc(SIZE * sizeof(int));
    int *c = (int *)malloc(SIZE * sizeof(int));
    double *da = (double *)malloc(SIZE * sizeof(double));
    double *db = (double *)malloc(SIZE * sizeof(double));
    int arr[M][N];
    
    if (!a || !b || !c || !da || !db) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        da[i] = (double)(rand() % 100) / 10.0;
        db[i] = (double)(rand() % 100) / 10.0;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = rand() % 100;
        }
    }
    
    volatile int checksum = 0;
    
    /* Execute patterns to create various DDG edges */
    pattern1_flow_distance(a, b, SIZE);
    for (int i = 0; i < 10; i++) checksum += a[i];
    
    pattern2_mixed_deps(a, b, c, SIZE);
    for (int i = 0; i < 10; i++) checksum += b[i] + c[i];
    
    pattern3_aliasing(a, b, SIZE);
    for (int i = 0; i < 10; i++) checksum += a[i];
    
    pattern3_noalias(b, c, SIZE);
    for (int i = 0; i < 10; i++) checksum += b[i];
    
    pattern4_nested(arr);
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            checksum += arr[i][j];
        }
    }
    
    pattern5_volatile_deps();
    checksum += 1; /* Dummy use */
    
    pattern6_complex(a, b, c, SIZE);
    for (int i = 0; i < 10; i++) checksum += a[i] + b[i] + c[i];
    
    pattern7_double_deps(da, db, SIZE);
    for (int i = 0; i < 10; i++) checksum += (int)(da[i] + db[i]);
    
    /* Final computation to ensure all results are used */
    int final_result = checksum;
    for (int i = 0; i < SIZE; i++) {
        final_result += a[i] + b[i] + c[i];
    }
    
    printf("Result: %d\n", final_result);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(da);
    free(db);
    
    return 0;
}
