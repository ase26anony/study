/* test_vectorizer_comparisons.c
 * 
 * This program contains vectorizable loops with different comparison
 * operators to trigger the specific transformation logic in
 * tree-vect-stmts.cc lines 12216-12233.
 * 
 * Compile with: gcc -O3 -ftree-vectorize -fno-vect-cost-model -march=core-avx2 test.c -o test
 * Or for debugging: gcc -O3 -ftree-vectorize -fdump-tree-vect-details -S test.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test function for GT_EXPR (>) */
void test_gt(int *restrict a, int *restrict b, int *restrict c, int n) {
    for (int i = 0; i < n; i++) {
        /* This comparison will be transformed to BIT_NOT_EXPR and BIT_AND_EXPR */
        if (a[i] > b[i]) {
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

/* Test function for GE_EXPR (>=) */
void test_ge(float *restrict a, float *restrict b, float *restrict c, int n) {
    for (int i = 0; i < n; i++) {
        /* This comparison will be transformed to BIT_NOT_EXPR and BIT_IOR_EXPR */
        if (a[i] >= b[i]) {
            c[i] = a[i] * b[i];
        } else {
            c[i] = a[i] / (b[i] + 1.0f);
        }
    }
}

/* Test function for LT_EXPR (<) */
void test_lt(int *restrict a, int *restrict b, int *restrict c, int n) {
    for (int i = 0; i < n; i++) {
        /* This comparison will be transformed to BIT_NOT_EXPR and BIT_AND_EXPR
           with operand swap */
        if (a[i] < b[i]) {
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i] * 3;
        }
    }
}

/* Test function for LE_EXPR (<=) */
void test_le(float *restrict a, float *restrict b, float *restrict c, int n) {
    for (int i = 0; i < n; i++) {
        /* This comparison will be transformed to BIT_NOT_EXPR and BIT_IOR_EXPR
           with operand swap */
        if (a[i] <= b[i]) {
            c[i] = a[i] + 1.0f;
        } else {
            c[i] = b[i] - 1.0f;
        }
    }
}

/* Alternative test using ternary operator for GT_EXPR */
void test_gt_ternary(int *restrict a, int *restrict b, int *restrict c, int n) {
    for (int i = 0; i < n; i++) {
        /* Ternary operator also creates comparison that needs vectorization */
        c[i] = (a[i] > b[i]) ? (a[i] | b[i]) : (a[i] & b[i]);
    }
}

/* Test with mixed comparisons in same loop */
void test_mixed_comparisons(int *restrict a, int *restrict b, int *restrict c, 
                           int *restrict d, int n) {
    for (int i = 0; i < n; i++) {
        /* Multiple comparisons to ensure all cases are hit */
        if (a[i] > b[i]) {
            c[i] = 1;
        } else if (a[i] >= b[i]) {
            c[i] = 2;
        } else if (a[i] < b[i]) {
            c[i] = 3;
        } else if (a[i] <= b[i]) {
            c[i] = 4;
        } else {
            c[i] = 0;
        }
        
        /* Another independent comparison */
        d[i] = (a[i] <= b[i]) ? a[i] : b[i];
    }
}

/* Initialize arrays with pattern that creates varied comparison results */
void init_arrays(int *a, int *b, float *fa, float *fb, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = n - i - 1;  /* Creates mix of true/false comparisons */
        fa[i] = (float)i * 1.5f;
        fb[i] = (float)(n - i) * 0.7f;
    }
}

/* Compute checksum to ensure loops execute and produce results */
int compute_checksum(int *c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += c[i];
    }
    return sum;
}

float compute_fchecksum(float *c, int n) {
    float sum = 0.0f;
    for (int i = 0; i < n; i++) {
        sum += c[i];
    }
    return sum;
}

int main(void) {
    /* Use aligned arrays for better vectorization */
    ALIGNED int a[N], b[N], c1[N], c2[N], c3[N], c4[N], c5[N], d[N];
    ALIGNED float fa[N], fb[N], fc1[N], fc2[N];
    
    /* Initialize with patterns that ensure comparisons are not all true/false */
    init_arrays(a, b, fa, fb, N);
    
    printf("Testing vectorizer comparison transformations...\n");
    
    /* Test each comparison operator separately */
    test_gt(a, b, c1, N);
    test_ge(fa, fb, fc1, N);
    test_lt(a, b, c2, N);
    test_le(fa, fb, fc2, N);
    test_gt_ternary(a, b, c3, N);
    test_mixed_comparisons(a, b, c4, d, N);
    
    /* Compute checksums to ensure code executed */
    int sum1 = compute_checksum(c1, N);
    float fsum1 = compute_fchecksum(fc1, N);
    int sum2 = compute_checksum(c2, N);
    float fsum2 = compute_fchecksum(fc2, N);
    int sum3 = compute_checksum(c3, N);
    int sum4 = compute_checksum(c4, N);
    int sum5 = compute_checksum(d, N);
    
    printf("Checksums:\n");
    printf("  GT (>): %d\n", sum1);
    printf("  GE (>=): %.2f\n", fsum1);
    printf("  LT (<): %d\n", sum2);
    printf("  LE (<=): %.2f\n", fsum2);
    printf("  GT ternary: %d\n", sum3);
    printf("  Mixed comparisons: %d\n", sum4);
    printf("  Mixed d array: %d\n", sum5);
    
    /* Verify results are non-zero (ensures loops executed) */
    if (sum1 != 0 && sum2 != 0 && sum3 != 0 && sum4 != 0 && sum5 != 0 &&
        fsum1 != 0.0f && fsum2 != 0.0f) {
        printf("All tests completed successfully.\n");
        return 0;
    } else {
        printf("Error: Some checksums are zero!\n");
        return 1;
    }
}
