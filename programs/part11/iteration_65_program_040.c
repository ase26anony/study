#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 100
#define M 50

/* Pattern 1: Loop-carried flow dependence with distance > 0 */
__attribute__((always_inline))
static inline void pattern1_flow_dependence(int *a, int *b, int n) {
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

/* Pattern 2: Multiple dependence types in single loop */
__attribute__((always_inline))
static inline void pattern2_mixed_dependences(int *a, int *b, int *c, int n) {
    int t;
    /* Contains flow (RAW), anti (WAR), and output (WAW) dependences */
    for (int i = 1; i < n; i++) {
        t = a[i - 1];           /* Flow: read a[i-1] */
        a[i] = t + b[i];        /* Flow: use t, Anti: write a[i] after read in next iter? */
        c[i] = c[i - 1] * 2;    /* Flow: read c[i-1], Output: write c[i] */
    }
}

/* Pattern 3: Pointer aliasing with restrict and non-restrict */
static void pattern3_aliasing(int *restrict p, int *restrict q, int *r, int n) {
    /* p and q are restrict, but r is not - creates different alias scenarios */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + r[(i * 7) % n];  /* Potential flow if r aliases p */
    }
    
    /* Self-overlap with non-restrict pointer */
    for (int i = 1; i < n; i++) {
        r[i] = r[(i * 3) % n] + i;  /* Complex addressing may create dependence */
    }
}

/* Pattern 4: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
static inline void pattern4_nested_loops(int arr[M][N]) {
    /* Inner loop has distance 2 flow dependence */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 2
        for (int j = 2; j < N; j++) {
            arr[i][j] = arr[i][j - 2] + arr[i][j - 1];
        }
    }
}

/* Pattern 5: Volatile memory accesses to force dependences */
static void pattern5_volatile_deps(volatile int *v, int n) {
    /* Simple volatile flow dependence */
    for (int i = 1; i < n; i++) {
        v[i] = v[i - 1] + i;
    }
    
    /* Volatile with distance 3 */
    for (int i = 3; i < n; i++) {
        v[i] = v[i - 3] * 2;
    }
}

/* Pattern 6: Complex addressing with mixed types */
static void pattern6_complex_addressing(double *d, int *idx, int n) {
    /* Different data types in dependence graph */
    for (int i = 1; i < n; i++) {
        int j = idx[i] % n;
        d[i] = d[j] + (double)i;  /* Flow dependence through d[] */
    }
}

/* Pattern 7: Software pipelining candidate with multiple accumulators */
__attribute__((always_inline))
static inline int pattern7_multiple_accumulators(int *a, int *b, int n) {
    int sum1 = 0, sum2 = 0, sum3 = 0;
    
    /* Multiple accumulators create register pressure and dependences */
    for (int i = 0; i < n; i++) {
        sum1 += a[i];
        sum2 += b[i];
        sum3 += a[i] * b[i];  /* Depends on both a[i] and b[i] */
        
        /* Create anti-dependence through reuse */
        a[i] = sum1;
        b[i] = sum2;
    }
    return sum1 + sum2 + sum3;
}

/* Pattern 8: Loop with if-conversion opportunities */
static void pattern8_conditional_deps(int *a, int *b, int *c, int n) {
    for (int i = 1; i < n; i++) {
        if (a[i - 1] > 0) {
            b[i] = b[i - 1] + a[i];  /* Flow dependence */
        } else {
            c[i] = c[i - 1] - a[i];  /* Alternative flow dependence */
        }
        a[i] = b[i] + c[i];  /* Depends on both branches */
    }
}

int main() {
    /* Initialize with different sizes to create various DDG structures */
    int *a = (int*)malloc(SIZE * sizeof(int));
    int *b = (int*)malloc(SIZE * sizeof(int));
    int *c = (int*)malloc(SIZE * sizeof(int));
    int *idx = (int*)malloc(SIZE * sizeof(int));
    volatile int *v = (volatile int*)malloc(SIZE * sizeof(int));
    double *d = (double*)malloc(SIZE * sizeof(double));
    int arr[M][N];
    
    /* Initialize arrays */
    srand(time(NULL));
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        idx[i] = rand() % SIZE;
        v[i] = rand() % 100;
        d[i] = (double)(rand() % 100) / 3.0;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = rand() % 100;
        }
    }
    
    volatile int checksum = 0;
    
    /* Execute patterns to create various DDG edges */
    pattern1_flow_dependence(a, b, SIZE - 10);
    checksum += a[SIZE - 20] + b[SIZE - 20];
    
    pattern2_mixed_dependences(a, b, c, SIZE - 20);
    checksum += a[SIZE - 30] + c[SIZE - 30];
    
    pattern3_aliasing(a, b, c, SIZE - 30);
    checksum += a[SIZE - 40] + b[SIZE - 40];
    
    pattern4_nested_loops(arr);
    checksum += arr[M-1][N-1] + arr[0][0];
    
    pattern5_volatile_deps(v, SIZE - 40);
    checksum += v[SIZE - 50];
    
    pattern6_complex_addressing(d, idx, SIZE - 50);
    checksum += (int)d[SIZE - 60];
    
    checksum += pattern7_multiple_accumulators(a, b, SIZE - 60);
    
    pattern8_conditional_deps(a, b, c, SIZE - 70);
    checksum += a[SIZE - 80] + b[SIZE - 80] + c[SIZE - 80];
    
    /* Additional loop with varying bounds to increase coverage */
    for (int k = 0; k < 10; k++) {
        int limit = SIZE - 100 - k * 10;
        for (int i = 2; i < limit; i++) {
            /* Create output dependence (WAW) */
            a[i] = a[i - 2] + k;
            /* Create anti-dependence (WAR) through b */
            b[i - 1] = a[i] + b[i];
        }
        checksum += a[limit - 1] + b[limit - 1];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(idx);
    free((void*)v);
    free(d);
    
    return 0;
}
