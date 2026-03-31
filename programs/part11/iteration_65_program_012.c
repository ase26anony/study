#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define N 100
#define M 50

/* Test 1: Loop-carried flow dependence with distance > 0 */
__attribute__((always_inline))
inline void test_flow_dependence_distance(int *a, int *b, int n, int *sum) {
    volatile int *va = (volatile int*)a;
    volatile int *vb = (volatile int*)b;
    
    /* Flow dependence with distance 2 */
    for (int i = 0; i < n - 2; i++) {
        va[i + 2] = va[i] * vb[i];
    }
    
    /* Anti-dependence (WAR) */
    int temp;
    for (int i = 1; i < n; i++) {
        temp = va[i - 1];  /* Read */
        va[i] = temp + i;  /* Write to same location later */
    }
    
    /* Output dependence (WAW) */
    for (int i = 1; i < n; i++) {
        va[i] = i * 2;
        va[i] = i * 3;  /* Second write to same location */
    }
    
    /* Accumulate checksum */
    for (int i = 0; i < n; i++) {
        *sum += va[i];
    }
}

/* Test 2: Mixed data types for different data_type fields */
__attribute__((always_inline))
inline void test_mixed_data_types(float *fa, double *da, int n, int *sum) {
    volatile float *vfa = (volatile float*)fa;
    volatile double *vda = (volatile double*)da;
    
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
void test_pointer_aliasing(int * restrict p, int *q, int n, int *sum) {
    /* p is restrict, but we create self-dependence */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n];  /* Potential flow dependence */
    }
    
    /* Non-restrict pointers with possible aliasing */
    int *r = p + n/2;
    for (int i = 0; i < n/2; i++) {
        p[i] = r[i] * 2;  /* p and r may alias */
    }
    
    /* Accumulate checksum */
    for (int i = 0; i < n; i++) {
        *sum += p[i];
    }
}

/* Test 4: Nested loops with inner loop carried dependence */
__attribute__((always_inline))
inline void test_nested_loops(int arr[M][N], int *sum) {
    volatile int (*varr)[N] = (volatile int (*)[N])arr;
    
    /* Inner loop with distance 2 dependence */
    for (int i = 0; i < M; i++) {
        #pragma GCC unroll 4
        for (int j = 2; j < N; j++) {
            varr[i][j] = varr[i][j - 2] + i + j;
        }
    }
    
    /* Outer loop carried dependence */
    for (int i = 1; i < M; i++) {
        for (int j = 0; j < N; j++) {
            varr[i][j] += varr[i - 1][j];
        }
    }
    
    /* Accumulate checksum */
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            *sum += varr[i][j];
        }
    }
}

/* Test 5: Complex index calculations and volatile */
void test_complex_indices(volatile int *v, int n, int *sum) {
    /* Multiple interleaved dependences */
    int t1, t2;
    for (int i = 2; i < n - 2; i++) {
        t1 = v[i - 2];           /* Read */
        t2 = v[i - 1];           /* Read */
        v[i] = t1 + t2;          /* Write - flow from i-2 and i-1 */
        v[i + 1] = v[i] * 2;     /* Flow from i to i+1 */
        v[i - 1] = i;            /* Anti-dependence with t2 read */
    }
    
    /* Distance 4 flow dependence */
    for (int i = 0; i < n - 4; i++) {
        v[i + 4] = v[i] + v[i + 1] + v[i + 2] + v[i + 3];
    }
    
    /* Accumulate checksum */
    for (int i = 0; i < n; i++) {
        *sum += v[i];
    }
}

/* Test 6: Multiple dependence types in one complex loop */
void test_complex_dependence_pattern(int *a, int *b, int *c, int n, int *sum) {
    volatile int *va = (volatile int*)a;
    volatile int *vb = (volatile int*)b;
    volatile int *vc = (volatile int*)c;
    
    /* Complex pattern with all dependence types */
    for (int i = 1; i < n - 1; i++) {
        int x = va[i - 1];           /* Read a[i-1] */
        va[i] = x + vb[i];           /* Write a[i] - flow from i-1 */
        vc[i] = vc[i - 1] * 2;       /* Write c[i] - flow from i-1 (through c) */
        vb[i] = va[i] + 1;           /* Write b[i] - anti-dependence (read va[i] above) */
        va[i - 1] = i;               /* Write a[i-1] - output dependence with line 1 */
    }
    
    /* Accumulate checksum from all arrays */
    for (int i = 0; i < n; i++) {
        *sum += va[i] + vb[i] + vc[i];
    }
}

int main() {
    /* Initialize with different patterns */
    int *array1 = (int*)malloc(SIZE * sizeof(int));
    int *array2 = (int*)malloc(SIZE * sizeof(int));
    int *array3 = (int*)malloc(SIZE * sizeof(int));
    float *farrays = (float*)malloc(SIZE * sizeof(float));
    double *darrays = (double*)malloc(SIZE * sizeof(double));
    int (*matrix)[N] = (int(*)[N])malloc(M * N * sizeof(int));
    
    /* Initialize with pseudo-random but deterministic values */
    srand(42);
    for (int i = 0; i < SIZE; i++) {
        array1[i] = rand() % 100;
        array2[i] = rand() % 100;
        array3[i] = rand() % 100;
        farrays[i] = (float)(rand() % 100) / 10.0f;
        darrays[i] = (double)(rand() % 100) / 10.0;
    }
    
    for (int i = 0; i < M; i++) {
        for (int j = 0; j < N; j++) {
            matrix[i][j] = rand() % 100;
        }
    }
    
    volatile int checksum = 0;
    
    /* Execute all test patterns */
    test_flow_dependence_distance(array1, array2, SIZE, (int*)&checksum);
    test_mixed_data_types(farrays, darrays, SIZE, (int*)&checksum);
    test_pointer_aliasing(array1, array3, SIZE, (int*)&checksum);
    test_nested_loops(matrix, (int*)&checksum);
    test_complex_indices(array2, SIZE, (int*)&checksum);
    test_complex_dependence_pattern(array1, array2, array3, SIZE, (int*)&checksum);
    
    /* Final computation to ensure all results are used */
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(array1);
    free(array2);
    free(array3);
    free(farrays);
    free(darrays);
    free(matrix);
    
    return 0;
}
