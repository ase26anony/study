/* test_vectorize_comparisons.c
 * 
 * This program contains loops with different comparison operators
 * that should trigger GCC's vectorizer to convert comparison tree codes
 * to bitwise operation sequences in tree-vect-stmts.cc
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test function for GT_EXPR (>) */
void test_gt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {  /* This should trigger GT_EXPR case */
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

/* Test function for GE_EXPR (>=) */
void test_ge(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {  /* This should trigger GE_EXPR case */
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i] * 2;
        }
    }
}

/* Test function for LT_EXPR (<) */
void test_lt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {  /* This should trigger LT_EXPR case */
            c[i] = a[i] << 1;
        } else {
            c[i] = b[i] << 1;
        }
    }
}

/* Test function for LE_EXPR (<=) */
void test_le(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {  /* This should trigger LE_EXPR case */
            c[i] = a[i] | b[i];
        } else {
            c[i] = a[i] & b[i];
        }
    }
}

/* Additional test with floating point comparisons */
void test_float_gt(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {  /* Floating point GT_EXPR */
            c[i] = a[i] * b[i];
        } else {
            c[i] = a[i] + b[i];
        }
    }
}

/* Test with mixed operations to ensure different patterns */
void test_mixed_comparisons(int *restrict a, int *restrict b, int *restrict c, int *restrict d) {
    for (int i = 0; i < N; i++) {
        /* Multiple comparisons in one loop */
        int cond1 = (a[i] > b[i]);   /* GT_EXPR */
        int cond2 = (a[i] >= b[i]);  /* GE_EXPR */
        int cond3 = (a[i] < b[i]);   /* LT_EXPR */
        int cond4 = (a[i] <= b[i]);  /* LE_EXPR */
        
        /* Use comparisons in complex expression */
        c[i] = (cond1 && cond2) ? a[i] : b[i];
        d[i] = (cond3 || cond4) ? a[i] + b[i] : a[i] - b[i];
    }
}

/* Initialize arrays with patterned data to create varied comparison results */
void init_arrays(int *a, int *b) {
    for (int i = 0; i < N; i++) {
        a[i] = i;           /* 0, 1, 2, 3, ... */
        b[i] = N - i - 1;   /* 1023, 1022, 1021, ... */
    }
}

void init_float_arrays(float *a, float *b) {
    for (int i = 0; i < N; i++) {
        a[i] = i * 0.5f;
        b[i] = (N - i) * 0.5f;
    }
}

/* Compute checksum to verify correctness and prevent dead code elimination */
int compute_checksum(int *arr) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += arr[i];
    }
    return sum;
}

float compute_float_checksum(float *arr) {
    float sum = 0.0f;
    for (int i = 0; i < N; i++) {
        sum += arr[i];
    }
    return sum;
}

int main() {
    /* Aligned arrays to help vectorization */
    ALIGNED int a[N], b[N], c[N], d[N], e[N], f[N], g[N];
    ALIGNED float fa[N], fb[N], fc[N];
    
    /* Initialize data */
    init_arrays(a, b);
    init_float_arrays(fa, fb);
    
    /* Test each comparison operator separately */
    test_gt(a, b, c);
    test_ge(a, b, d);
    test_lt(a, b, e);
    test_le(a, b, f);
    test_float_gt(fa, fb, fc);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(a, b, g, c);  /* Reuse c array */
    
    /* Compute and print checksums to ensure code executes */
    printf("GT checksum: %d\n", compute_checksum(c));
    printf("GE checksum: %d\n", compute_checksum(d));
    printf("LT checksum: %d\n", compute_checksum(e));
    printf("LE checksum: %d\n", compute_checksum(f));
    printf("Float GT checksum: %f\n", compute_float_checksum(fc));
    printf("Mixed checksum: %d\n", compute_checksum(g));
    
    /* Verify some results */
    int errors = 0;
    for (int i = 0; i < 10; i++) {
        if (c[i] != ((a[i] > b[i]) ? a[i] + b[i] : a[i] - b[i])) errors++;
    }
    
    if (errors == 0) {
        printf("All tests passed!\n");
    } else {
        printf("Found %d errors in verification\n", errors);
    }
    
    return 0;
}
