#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SIZE 1024
#define INLINE __attribute__((always_inline))

/* Test 1: Loop-carried flow dependence with distance > 0 */
INLINE void test_flow_dependence(int *a, int *b, int n, int *checksum) {
    /* Distance 2 flow dependence */
    for (int i = 0; i < n - 2; i++) {
        a[i + 2] = a[i] * b[i] + 1;
    }
    
    /* Distance 3 anti-dependence (WAR) */
    int temp;
    for (int i = 1; i < n - 2; i++) {
        temp = a[i - 1];      /* Read a[i-1] */
        a[i - 1] = b[i] + 3;  /* Write a[i-1] - anti-dependence */
        b[i] = temp;
    }
    
    /* Update checksum */
    for (int i = 0; i < 10; i++) {
        *checksum += a[i] + b[i];
    }
}

/* Test 2: Multiple interleaved dependence types */
INLINE void test_mixed_dependences(double *arr1, double *arr2, int n, int *checksum) {
    double t1, t2;
    
    /* Contains flow, anti, and output dependences */
    for (int i = 1; i < n; i++) {
        t1 = arr1[i - 1];               /* Flow dependence on arr1[i-1] from previous iteration */
        arr1[i] = t1 + arr2[i];         /* Flow dependence on t1 */
        arr2[i - 1] = arr2[i] * 0.5;    /* Anti-dependence on arr2[i] */
        arr1[i - 1] = arr2[i - 1] + 1;  /* Output dependence on arr1[i-1] */
    }
    
    for (int i = 0; i < 10; i++) {
        *checksum += (int)(arr1[i] + arr2[i]);
    }
}

/* Test 3: Indirect addressing with potential aliasing */
void test_indirect_addressing(int *p, int *q, int n, int *checksum) {
    /* No restrict - compiler must assume aliasing */
    for (int i = 0; i < n; i++) {
        int idx = (i * 7) % n;
        p[i] = q[i] + p[idx] + i;  /* Potential flow dependence if p overlaps */
    }
    
    /* With restrict - no assumed aliasing */
    int *restrict r1 = p;
    int *restrict r2 = q;
    for (int i = 1; i < n; i++) {
        r1[i] = r2[i] + r1[i - 1];  /* Only loop-carried flow dependence */
    }
    
    for (int i = 0; i < 10; i++) {
        *checksum += p[i] + q[i];
    }
}

/* Test 4: Nested loops with inner loop carried dependence */
INLINE void test_nested_loops(int arr[][SIZE], int m, int n, int *checksum) {
    /* Inner loop has distance 2 flow dependence */
    for (int i = 0; i < m; i++) {
        #pragma GCC unroll 2
        for (int j = 2; j < n; j++) {
            arr[i][j] = arr[i][j - 2] + arr[i][j - 1] + 1;
        }
    }
    
    /* Outer loop carried dependence */
    for (int i = 1; i < m; i++) {
        for (int j = 0; j < n; j++) {
            arr[i][j] += arr[i - 1][j] * 2;
        }
    }
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            *checksum += arr[i][j];
        }
    }
}

/* Test 5: Volatile memory accesses to force dependences */
INLINE void test_volatile_access(int *checksum) {
    volatile int varr[100];
    volatile int v1, v2;
    
    /* Simple volatile flow dependence chain */
    v1 = 0;
    for (int i = 0; i < 50; i++) {
        v2 = v1 + i;
        varr[i] = v2;
        v1 = varr[i] * 2;
    }
    
    /* Volatile array with distance */
    for (int i = 2; i < 50; i++) {
        varr[i] = varr[i - 2] + varr[i - 1];
    }
    
    *checksum += varr[10] + varr[20];
}

/* Test 6: Different data types for different edge data_type fields */
INLINE void test_mixed_data_types(int *checksum) {
    float farr[SIZE];
    double darr[SIZE];
    int iarr[SIZE];
    
    /* Initialize */
    for (int i = 0; i < SIZE; i++) {
        farr[i] = i * 0.5f;
        darr[i] = i * 0.25;
        iarr[i] = i;
    }
    
    /* Float loop with dependence */
    for (int i = 1; i < SIZE; i++) {
        farr[i] = farr[i - 1] * 1.1f + farr[i];
    }
    
    /* Double loop with anti-dependence */
    double temp;
    for (int i = 1; i < SIZE - 1; i++) {
        temp = darr[i];
        darr[i] = darr[i + 1] * 0.9;
        darr[i + 1] = temp;
    }
    
    /* Integer loop with output dependence */
    for (int i = 0; i < SIZE - 1; i++) {
        iarr[i] = iarr[i] + iarr[i + 1];
        iarr[i + 1] = iarr[i] * 2;
    }
    
    *checksum += (int)farr[10] + (int)darr[20] + iarr[30];
}

/* Test 7: Complex index calculations to obscure analysis */
INLINE void test_complex_indices(int *a, int *b, int n, int *checksum) {
    /* Non-linear index with modulo - harder to analyze */
    for (int i = 0; i < n; i++) {
        int idx1 = (i * 3 + 7) % n;
        int idx2 = (i * 5 + 11) % n;
        a[idx1] = b[idx2] + a[i] * 2;
    }
    
    /* Triangular loop */
    for (int i = 0; i < n; i++) {
        for (int j = i; j < n; j++) {
            a[j] += b[i] * (j - i);
        }
    }
    
    for (int i = 0; i < 10; i++) {
        *checksum += a[i] + b[i];
    }
}

int main() {
    /* Initialize with deterministic values */
    srand(42);
    
    /* Allocate and initialize arrays */
    int *a = (int*)malloc(SIZE * sizeof(int));
    int *b = (int*)malloc(SIZE * sizeof(int));
    double *darr1 = (double*)malloc(SIZE * sizeof(double));
    double *darr2 = (double*)malloc(SIZE * sizeof(double));
    int (*nested_arr)[SIZE] = (int(*)[SIZE])malloc(10 * SIZE * sizeof(int));
    
    for (int i = 0; i < SIZE; i++) {
        a[i] = rand() % 100;
        b[i] = rand() % 100;
        darr1[i] = rand() % 100 * 0.1;
        darr2[i] = rand() % 100 * 0.1;
    }
    
    for (int i = 0; i < 10; i++) {
        for (int j = 0; j < SIZE; j++) {
            nested_arr[i][j] = rand() % 50;
        }
    }
    
    /* Volatile checksum to ensure all computations are live */
    volatile int checksum = 0;
    
    /* Execute all tests to trigger various DDG edge creations */
    test_flow_dependence(a, b, SIZE, (int*)&checksum);
    test_mixed_dependences(darr1, darr2, SIZE, (int*)&checksum);
    test_indirect_addressing(a, b, SIZE, (int*)&checksum);
    test_nested_loops(nested_arr, 10, SIZE, (int*)&checksum);
    test_volatile_access((int*)&checksum);
    test_mixed_data_types((int*)&checksum);
    test_complex_indices(a, b, SIZE, (int*)&checksum);
    
    /* Final computation to use all results */
    int final_result = 0;
    for (int i = 0; i < 100; i++) {
        final_result += a[i] + b[i] + (int)darr1[i] + (int)darr2[i];
    }
    
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 100; j++) {
            final_result += nested_arr[i][j];
        }
    }
    
    final_result += checksum;
    
    printf("Result: %d\n", final_result);
    
    /* Cleanup */
    free(a);
    free(b);
    free(darr1);
    free(darr2);
    free(nested_arr);
    
    return 0;
}
