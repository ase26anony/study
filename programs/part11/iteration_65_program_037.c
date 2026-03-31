#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define SIZE 1024
#define ITER 100

/* Force inlining to create larger basic blocks */
__attribute__((always_inline)) 
static inline int compute_hash(int a, int b) {
    return (a * 31 + b) ^ 0x5A5A5A5A;
}

/* Test 1: Loop-carried flow dependence with distance > 0 */
void test_flow_dependence_distance(int *a, int *b, int n, volatile int *checksum) {
    /* Distance 2 flow dependence: a[i+2] depends on a[i] */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i] + 1;
    }
    
    /* Anti-dependence: b[i] written after being read */
    for (int i = 1; i < n; i++) {
        int temp = b[i - 1];
        b[i] = temp + a[i];
    }
    
    /* Update checksum to prevent dead code elimination */
    for (int i = 0; i < n; i++) {
        *checksum += a[i] + b[i];
    }
}

/* Test 2: Multiple dependence types in single loop */
void test_mixed_dependences(int *a, int *b, int *c, int n, volatile int *checksum) {
    int t = 0;
    
    /* Contains flow, anti, and output dependences */
    for (int i = 1; i < n; i++) {
        t = a[i - 1];               /* Flow: read a[i-1] */
        a[i] = t + b[i];            /* Flow: use t, Anti: b[i] read before potential write */
        b[i] = c[i] * 2;            /* Output: b[i] written again */
        c[i] = c[i - 1] + 1;        /* Flow: c[i] depends on c[i-1] */
    }
    
    for (int i = 0; i < n; i++) {
        *checksum += a[i] + b[i] + c[i];
    }
}

/* Test 3: Pointer aliasing with potential self-overlap */
void test_pointer_aliasing(int * __restrict p, int *q, int n, volatile int *checksum) {
    /* Without restrict, compiler must assume p and q may alias */
    for (int i = 0; i < n; i++) {
        p[i] = q[i] + p[(i * 7) % n];  /* Potential flow dependence if p overlaps */
    }
    
    /* Another function without restrict to force conservative analysis */
    int *r = p;
    for (int i = 1; i < n; i++) {
        r[i] = r[i - 1] + i;
    }
    
    for (int i = 0; i < n; i++) {
        *checksum += p[i] + q[i];
    }
}

/* Test 4: Nested loops with inner loop carried dependence */
void test_nested_loops(int arr[][SIZE], int m, int n, volatile int *checksum) {
    /* Inner loop has distance 2 flow dependence */
    for (int i = 0; i < m; i++) {
        for (int j = 2; j < n; j++) {
            arr[i][j] = arr[i][j - 2] + arr[i][j - 1];
        }
    }
    
    /* Outer loop carried dependence */
    for (int i = 1; i < m; i++) {
        for (int j = 0; j < n; j++) {
            arr[i][j] += arr[i - 1][j];
        }
    }
    
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            *checksum += arr[i][j];
        }
    }
}

/* Test 5: Volatile memory accesses to force dependences */
void test_volatile_access(volatile int *v, int n, volatile int *checksum) {
    /* Strong memory dependence enforced by volatile */
    for (int i = 1; i < n; i++) {
        v[i] = v[i - 1] + i;
    }
    
    /* WAR with volatile */
    for (int i = 0; i < n - 1; i++) {
        int temp = v[i];
        v[i] = v[i + 1];
        v[i + 1] = temp;
    }
    
    for (int i = 0; i < n; i++) {
        *checksum += v[i];
    }
}

/* Test 6: Different data types for varied data_type field */
void test_mixed_data_types(float *fa, double *da, int *ia, int n, volatile int *checksum) {
    /* Float array with flow dependence */
    for (int i = 1; i < n; i++) {
        fa[i] = fa[i - 1] * 1.1f + ia[i];
    }
    
    /* Double array with anti-dependence */
    for (int i = 0; i < n - 1; i++) {
        double temp = da[i];
        da[i] = da[i + 1];
        da[i + 1] = temp * 0.9;
    }
    
    /* Integer with output dependence */
    for (int i = 0; i < n; i++) {
        ia[i] = ia[i] * 2 + 1;
    }
    
    for (int i = 0; i < n; i++) {
        *checksum += (int)fa[i] + (int)da[i] + ia[i];
    }
}

/* Test 7: Complex index calculations to obscure analysis */
void test_complex_indices(int *arr, int n, volatile int *checksum) {
    /* Non-linear access pattern */
    for (int i = 0; i < n; i++) {
        int idx1 = (i * 3 + 7) % n;
        int idx2 = (i * 5 + 11) % n;
        arr[idx1] = arr[idx2] + i;
    }
    
    /* Strided access with potential overlap */
    for (int i = 0; i < n / 2; i++) {
        arr[i * 2] = arr[i * 2 + 1] * arr[i];
    }
    
    for (int i = 0; i < n; i++) {
        *checksum += arr[i];
    }
}

int main() {
    /* Allocate and initialize arrays with varying patterns */
    int *a = (int*)malloc(SIZE * sizeof(int));
    int *b = (int*)malloc(SIZE * sizeof(int));
    int *c = (int*)malloc(SIZE * sizeof(int));
    int *p = (int*)malloc(SIZE * sizeof(int));
    int *q = (int*)malloc(SIZE * sizeof(int));
    volatile int *v = (volatile int*)malloc(SIZE * sizeof(int));
    float *fa = (float*)malloc(SIZE * sizeof(float));
    double *da = (double*)malloc(SIZE * sizeof(double));
    int *ia = (int*)malloc(SIZE * sizeof(int));
    int (*arr2d)[SIZE] = (int(*)[SIZE])malloc(10 * SIZE * sizeof(int));
    
    /* Initialize with pseudo-random but deterministic values */
    for (int i = 0; i < SIZE; i++) {
        a[i] = i * 3 + 1;
        b[i] = i * 5 + 2;
        c[i] = i * 7 + 3;
        p[i] = i * 11 + 5;
        q[i] = i * 13 + 7;
        v[i] = i * 17 + 11;
        fa[i] = i * 1.5f;
        da[i] = i * 2.5;
        ia[i] = i * 19 + 13;
    }
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < SIZE; j++) {
            arr2d[i][j] = i * SIZE + j;
        }
    }
    
    volatile int checksum = 0;
    
    /* Execute tests multiple times to increase execution time
       and give scheduler more opportunities */
    for (int iter = 0; iter < ITER; iter++) {
        /* Vary parameters slightly each iteration */
        int size = SIZE - (iter % 16);
        
        test_flow_dependence_distance(a, b, size, &checksum);
        test_mixed_dependences(a, b, c, size, &checksum);
        test_pointer_aliasing(p, q, size, &checksum);
        test_nested_loops(arr2d, 10, size, &checksum);
        test_volatile_access(v, size, &checksum);
        test_mixed_data_types(fa, da, ia, size, &checksum);
        test_complex_indices(a, size, &checksum);
        
        /* Mix up data to create varying patterns */
        for (int i = 0; i < size; i++) {
            a[i] = compute_hash(a[i], iter);
            b[i] = compute_hash(b[i], iter + 1);
        }
    }
    
    printf("Final checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(c); free(p); free(q); 
    free((void*)v); free(fa); free(da); free(ia); free(arr2d);
    
    return 0;
}
