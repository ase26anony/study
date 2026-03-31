#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define INNER_SIZE 128
#define VOL_SIZE 256

/* Force inlining to create larger basic blocks */
__attribute__((always_inline)) 
static inline int compute_index(int i, int n) {
    return (i * 7) % n;
}

/* Test 1: Loop-carried flow dependence with distance > 0 */
void test_flow_dependence(int *a, int *b, int n, int *checksum) {
    /* Distance 2 flow dependence */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i] + 1;
    }
    
    /* Distance 3 anti-dependence (WAR) */
    int temp;
    for (int i = 1; i < n - 3; i++) {
        temp = a[i];      /* Read */
        a[i + 3] = temp + b[i]; /* Write later - anti-dependence */
    }
    
    /* Accumulate checksum */
    for (int i = 0; i < 10; i++) {
        *checksum += a[i] + b[i];
    }
}

/* Test 2: Multiple interleaved dependence types */
void test_mixed_dependences(int *a, int *b, int *c, int n, int *checksum) {
    int t;
    
    /* Contains flow, anti, and output dependences */
    for (int i = 1; i < n; i++) {
        t = a[i - 1];               /* Flow dependence from previous iteration */
        a[i] = t + b[i];            /* Anti-dependence on t */
        c[i] = c[i - 1] * 2;        /* Flow dependence on c */
        b[i] = a[i] + c[i];         /* Output dependence on b[i] in next iteration? */
    }
    
    /* WAW (output) dependence example */
    for (int i = 0; i < n - 1; i++) {
        a[i] = b[i] + 1;            /* Write a[i] */
        a[i] = a[i] * 2;            /* Write a[i] again - output dependence */
    }
    
    for (int i = 0; i < 10; i++) {
        *checksum += a[i] + b[i] + c[i];
    }
}

/* Test 3: Indirect addressing with potential aliasing */
void test_indirect_addressing(int *p, int *q, int n, int *checksum) {
    /* No restrict - compiler must assume aliasing */
    for (int i = 0; i < n; i++) {
        int idx = compute_index(i, n);
        p[i] = q[i] + p[idx];  /* Potential flow dependence if p overlaps */
    }
    
    /* With restrict - no assumed aliasing */
    int *restrict r1 = p;
    int *restrict r2 = q;
    for (int i = 1; i < n; i++) {
        r1[i] = r2[i] + r1[i-1];  /* Only loop-carried dependence */
    }
    
    for (int i = 0; i < 10; i++) {
        *checksum += p[i] + q[i];
    }
}

/* Test 4: Nested loops with inner loop carried dependence */
void test_nested_loops(int arr[INNER_SIZE][INNER_SIZE], int m, int n, int *checksum) {
    /* Inner loop has distance 2 flow dependence */
    for (int i = 0; i < m; i++) {
        #pragma GCC unroll 2
        for (int j = 2; j < n; j++) {
            arr[i][j] = arr[i][j-2] + arr[i][j-1];
        }
    }
    
    /* Outer loop carried dependence */
    for (int i = 1; i < m; i++) {
        for (int j = 0; j < n; j++) {
            arr[i][j] += arr[i-1][j];
        }
    }
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            *checksum += arr[i][j];
        }
    }
}

/* Test 5: Volatile memory accesses to force dependencies */
void test_volatile_accesses(int *checksum) {
    volatile int v[VOL_SIZE];
    volatile int *vp = v;
    
    /* Simple flow dependence with volatile */
    for (int i = 1; i < VOL_SIZE; i++) {
        v[i] = v[i-1] + i;
    }
    
    /* Anti-dependence with volatile */
    int temp;
    for (int i = 0; i < VOL_SIZE - 1; i++) {
        temp = vp[i];      /* Volatile read */
        vp[i+1] = temp * 2; /* Volatile write - anti-dependence */
    }
    
    /* Mixed with non-volatile computation */
    int local_sum = 0;
    for (int i = 0; i < VOL_SIZE; i++) {
        local_sum += v[i];
    }
    
    *checksum += local_sum;
}

/* Test 6: Different data types for different edge data types */
void test_mixed_data_types(float *fa, double *da, int *ia, int n, int *checksum) {
    /* Float array with flow dependence */
    for (int i = 1; i < n; i++) {
        fa[i] = fa[i-1] * 1.5f;
    }
    
    /* Double array with anti-dependence */
    double temp_d;
    for (int i = 0; i < n - 1; i++) {
        temp_d = da[i];
        da[i+1] = temp_d * 2.0;
    }
    
    /* Integer array with output dependence */
    for (int i = 0; i < n; i++) {
        ia[i] = (int)fa[i];
        ia[i] = ia[i] * 3;  /* Output dependence */
    }
    
    for (int i = 0; i < 10; i++) {
        *checksum += (int)fa[i] + (int)da[i] + ia[i];
    }
}

/* Test 7: Complex index calculations to obscure analysis */
void test_complex_indices(int *arr, int n, int *checksum) {
    /* Non-linear index with modulo - hard to analyze */
    for (int i = 0; i < n; i++) {
        int idx1 = (i * 3 + 7) % n;
        int idx2 = (i * 5 + 11) % n;
        arr[idx1] = arr[idx2] + i;
    }
    
    /* Pointer arithmetic with offset */
    int *p = arr;
    for (int i = 0; i < n - 4; i++) {
        *(p + i + 4) = *(p + i) + *(p + i + 2);
    }
    
    for (int i = 0; i < 10; i++) {
        *checksum += arr[i];
    }
}

int main() {
    /* Initialize with deterministic values */
    srand(42);
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(SIZE * sizeof(int));
    int *b = (int*)malloc(SIZE * sizeof(int));
    int *c = (int*)malloc(SIZE * sizeof(int));
    int *p = (int*)malloc(SIZE * sizeof(int));
    int *q = (int*)malloc(SIZE * sizeof(int));
    float *fa = (float*)malloc(SIZE * sizeof(float));
    double *da = (double*)malloc(SIZE * sizeof(double));
    int *ia = (int*)malloc(SIZE * sizeof(int));
    int (*arr2d)[INNER_SIZE] = (int(*)[INNER_SIZE])malloc(INNER_SIZE * INNER_SIZE * sizeof(int));
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        c[i] = rand() % 100;
        p[i] = rand() % 100;
        q[i] = rand() % 100;
        fa[i] = (float)(rand() % 100) / 10.0f;
        da[i] = (double)(rand() % 100) / 10.0;
        ia[i] = rand() % 100;
    }
    
    for (int i = 0; i < INNER_SIZE; i++) {
        for (int j = 0; j < INNER_SIZE; j++) {
            arr2d[i][j] = rand() % 100;
        }
    }
    
    volatile int checksum = 0;
    
    /* Execute all tests to trigger various DDG edge creations */
    test_flow_dependence(a, b, SIZE, (int*)&checksum);
    test_mixed_dependences(a, b, c, SIZE, (int*)&checksum);
    test_indirect_addressing(p, q, SIZE, (int*)&checksum);
    test_nested_loops(arr2d, INNER_SIZE, INNER_SIZE, (int*)&checksum);
    test_volatile_accesses((int*)&checksum);
    test_mixed_data_types(fa, da, ia, SIZE, (int*)&checksum);
    test_complex_indices(a, SIZE, (int*)&checksum);
    
    /* Final computation to ensure everything is used */
    int final_result = checksum;
    for (int i = 0; i < 100; i++) {
        final_result += a[i] + b[i] + c[i];
    }
    
    printf("Result: %d\n", final_result);
    
    /* Cleanup */
    free(a); free(b); free(c); free(p); free(q);
    free(fa); free(da); free(ia); free(arr2d);
    
    return 0;
}
