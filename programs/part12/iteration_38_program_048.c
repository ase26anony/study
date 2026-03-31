/* test_vectorized_comparisons.c
 * 
 * This program contains loops with different comparison operators
 * that should trigger GCC's vectorizer to convert comparison tree codes
 * to bitwise operation sequences in tree-vect-stmts.cc.
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test function for GT_EXPR (>) */
void test_gt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

/* Test function for GE_EXPR (>=) */
void test_ge(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i] * 2;
        }
    }
}

/* Test function for LT_EXPR (<) */
void test_lt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            c[i] = a[i] + 1;
        } else {
            c[i] = b[i] + 1;
        }
    }
}

/* Test function for LE_EXPR (<=) */
void test_le(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            c[i] = a[i] - 1;
        } else {
            c[i] = b[i] - 1;
        }
    }
}

/* Additional test with floating point to ensure different type handling */
void test_float_gt(float *restrict fa, float *restrict fb, float *restrict fc) {
    for (int i = 0; i < N; i++) {
        if (fa[i] > fb[i]) {
            fc[i] = fa[i] + fb[i];
        } else {
            fc[i] = fa[i] - fb[i];
        }
    }
}

/* Test with mixed operators in same loop */
void test_mixed_comparisons(int *restrict a, int *restrict b, int *restrict c, 
                           int *restrict d, int *restrict e) {
    for (int i = 0; i < N; i++) {
        /* This should trigger multiple comparison transformations */
        if (a[i] > b[i]) {
            c[i] = 1;
        } else {
            c[i] = 0;
        }
        
        if (a[i] >= b[i]) {
            d[i] = 1;
        } else {
            d[i] = 0;
        }
        
        if (a[i] < b[i]) {
            e[i] = 1;
        } else {
            e[i] = 0;
        }
    }
}

/* Helper function to initialize arrays with pattern */
void init_arrays(int *a, int *b) {
    for (int i = 0; i < N; i++) {
        a[i] = i;           /* 0, 1, 2, 3, ... */
        b[i] = N - i - 1;   /* 1023, 1022, 1021, ... */
    }
}

void init_float_arrays(float *fa, float *fb) {
    for (int i = 0; i < N; i++) {
        fa[i] = (float)i * 1.5f;
        fb[i] = (float)(N - i) * 0.5f;
    }
}

/* Compute checksum to ensure loops execute and produce results */
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
    /* Use aligned arrays to help vectorization */
    ALIGNED int a[N], b[N], c[N], d[N], e[N], f[N], g[N], h[N];
    ALIGNED int mixed_c[N], mixed_d[N], mixed_e[N];
    ALIGNED float fa[N], fb[N], fc[N];
    
    /* Initialize arrays */
    init_arrays(a, b);
    init_float_arrays(fa, fb);
    
    printf("Testing vectorized comparisons...\n");
    
    /* Test each comparison operator separately */
    test_gt(a, b, c);
    test_ge(a, b, d);
    test_lt(a, b, e);
    test_le(a, b, f);
    
    /* Test with swapped arrays to get different comparison results */
    test_gt(b, a, g);  /* This should produce different results */
    test_ge(b, a, h);
    
    /* Test floating point comparisons */
    test_float_gt(fa, fb, fc);
    
    /* Test mixed comparisons in one loop */
    test_mixed_comparisons(a, b, mixed_c, mixed_d, mixed_e);
    
    /* Compute and print checksums to ensure execution */
    printf("Checksums (for verification):\n");
    printf("  GT: %d\n", compute_checksum(c));
    printf("  GE: %d\n", compute_checksum(d));
    printf("  LT: %d\n", compute_checksum(e));
    printf("  LE: %d\n", compute_checksum(f));
    printf("  GT swapped: %d\n", compute_checksum(g));
    printf("  GE swapped: %d\n", compute_checksum(h));
    printf("  Float GT: %.2f\n", compute_float_checksum(fc));
    printf("  Mixed C: %d\n", compute_checksum(mixed_c));
    printf("  Mixed D: %d\n", compute_checksum(mixed_d));
    printf("  Mixed E: %d\n", compute_checksum(mixed_e));
    
    /* Quick validation - just check first few elements */
    printf("\nFirst 5 elements of GT test:\n");
    for (int i = 0; i < 5; i++) {
        printf("  a[%d]=%d, b[%d]=%d, c[%d]=%d\n", 
               i, a[i], i, b[i], i, c[i]);
    }
    
    return 0;
}
