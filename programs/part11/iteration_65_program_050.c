#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 100
#define M 50

/* Test 1: Loop-carried flow dependence with distance > 0 */
__attribute__((always_inline))
inline void test1_flow_dependence_distance(int *a, int *b, int n, int *sum) {
    volatile int *va = (volatile int *)a;
    volatile int *vb = (volatile int *)b;
    
    /* Flow dependence with distance 2 */
    for (int i = 0; i < n - 2; i++) {
        va[i + 2] = va[i] * vb[i];
    }
    
    /* Anti-dependence (WAR) */
    for (int i = 1; i < n; i++) {
        int temp = va[i - 1];
        va[i] = temp + vb[i];
    }
    
    /* Output dependence (WAW) */
    for (int i = 1; i < n; i++) {
        va[i] = va[i] * 2;
        va[i] = va[i] + 1;  /* WAW on va[i] */
    }
    
    /* Accumulate checksum */
    for (int i = 0; i < n; i++) {
        *sum += va[i];
    }
}

/* Test 2: Mixed data types for different edge data types */
__attribute__((always_inline))
inline void test2_mixed_data_types(float *fa, double *da, int n, int *sum) {
    volatile float *vfa = (volatile float *)fa;
    volatile double *vda = (volatile double *)da;
    
    /* Flow dependence with floats */
    for (int i = 1; i < n; i++) {
        vfa[i] = vfa[i - 1] * 1.5f;
    }
    
    /* Flow dependence with doubles, distance 3 */
    for (int i = 0; i < n - 3; i++) {
        vda[i + 3] = vda[i] * 2.0;
    }
    
    /* Accumulate checksum */
    for (int i = 0; i < n; i++) {
        *sum += (int)vfa[i] + (int)vda[i];
    }
}

/* Test 3: Pointer aliasing with restrict and non-restrict */
void test3_pointer_aliasing(int *restrict p, int *q, int n, int *sum) {
    volatile int *vp = (volatile int *)p;
    volatile int *vq = (volatile int *)q;
    
    /* Potential flow dependence due to possible aliasing */
    for (int i = 0; i < n; i++) {
        vp[i] = vq[i] + vp[(i * 7) % n];
    }
    
    /* Explicit anti-dependence */
    for (int i = 1; i < n; i++) {
        int temp = vp[i - 1];
        vp[i] = temp + vq[i];
        vp[i - 1] = vq[i];  /* WAR on vp[i-1] */
    }
    
    /* Accumulate checksum */
    for (int i = 0; i < n; i++) {
        *sum += vp[i];
    }
}

/* Test 4: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
inline void test4_nested_loops(int arr[M][N], int *sum) {
    volatile int (*varr)[N] = (volatile int (*)[N])arr;
    
    /* Inner loop with carried dependence, distance 2 */
    #pragma GCC unroll 2
    for (int i = 0; i < M; i++) {
        for (int j = 2; j < N; j++) {
            varr[i][j] = varr[i][j - 2] + i + j;
        }
    }
    
    /* Outer loop carried dependence */
    for (int i = 1; i < M; i++) {
        for (int j = 0; j < N; j++) {
            varr[i][j] = varr[i - 1][j] * 2;
        }
    }
    
    /* Accumulate checksum */
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            *sum += varr[i][j];
        }
    }
}

/* Test 5: Complex index calculations and volatile arrays */
void test5_complex_indices(int *a, int *b, int n, int *sum) {
    volatile int *va = (volatile int *)a;
    volatile int *vb = (volatile int *)b;
    
    /* Multiple interleaved dependences */
    for (int i = 2; i < n - 2; i++) {
        /* Flow dependence distance 2 */
        int t1 = va[i - 2];
        
        /* Anti-dependence */
        va[i - 1] = vb[i] * 3;
        
        /* Output dependence */
        va[i] = t1 + vb[i - 1];
        va[i] = va[i] * 2;  /* WAW */
        
        /* Another flow dependence distance 1 */
        vb[i + 1] = va[i] + 1;
    }
    
    /* Accumulate checksum */
    for (int i = 0; i < n; i++) {
        *sum += va[i] + vb[i];
    }
}

/* Test 6: Software pipelining candidate with multiple accumulators */
__attribute__((always_inline))
inline void test6_software_pipeline(int *a, int *b, int *c, int n, int *sum) {
    volatile int *va = (volatile int *)a;
    volatile int *vb = (volatile int *)b;
    volatile int *vc = (volatile int *)c;
    
    /* Multiple accumulators to create parallel dependences */
    int acc1 = 0, acc2 = 0, acc3 = 0;
    
    #pragma GCC unroll 4
    for (int i = 1; i < n; i++) {
        /* Independent chains with different dependence distances */
        acc1 = acc1 + va[i - 1];      /* Flow distance 1 */
        acc2 = acc2 + vb[i];          /* No carried dependence */
        acc3 = acc3 + vc[(i + 1) % n]; /* Flow distance varies */
        
        va[i] = acc1;
        vb[i] = acc2;
        vc[i] = acc3;
    }
    
    /* Accumulate checksum */
    for (int i = 0; i < n; i++) {
        *sum += va[i] + vb[i] + vc[i];
    }
}

int main() {
    /* Initialize with deterministic values */
    srand(42);
    
    /* Allocate and initialize arrays */
    int *a = (int *)malloc(SIZE * sizeof(int));
    int *b = (int *)malloc(SIZE * sizeof(int));
    int *c = (int *)malloc(SIZE * sizeof(int));
    float *fa = (float *)malloc(SIZE * sizeof(float));
    double *da = (double *)malloc(SIZE * sizeof(double));
    int arr[M][N];
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        fa[i] = (float)(rand() % 100) / 10.0f;
        da[i] = (double)(rand() % 100) / 10.0;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            arr[i][j] = rand() % 100;
        }
    }
    
    volatile int checksum = 0;
    
    /* Execute all tests to trigger various DDG edge creations */
    test1_flow_dependence_distance(a, b, SIZE, (int *)&checksum);
    test2_mixed_data_types(fa, da, SIZE, (int *)&checksum);
    test3_pointer_aliasing(a, b, SIZE, (int *)&checksum);
    test4_nested_loops(arr, (int *)&checksum);
    test5_complex_indices(a, b, SIZE, (int *)&checksum);
    test6_software_pipeline(a, b, c, SIZE, (int *)&checksum);
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a);
    free(b);
    free(c);
    free(fa);
    free(da);
    
    return 0;
}
