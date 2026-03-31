/* test_vector_comparisons.c
 * Designed to trigger vectorizer logic for GT_EXPR, GE_EXPR, LT_EXPR, LE_EXPR
 * Each comparison type is tested in separate vectorizable loops
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test function for > comparison */
void test_gt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        /* This if-statement with > comparison should trigger GT_EXPR case */
        if (a[i] > b[i]) {
            c[i] = a[i] + b[i];
        } else {
            c[i] = a[i] - b[i];
        }
    }
}

/* Test function for >= comparison */
void test_ge(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        /* This if-statement with >= comparison should trigger GE_EXPR case */
        if (a[i] >= b[i]) {
            c[i] = a[i] * 2;
        } else {
            c[i] = b[i] * 2;
        }
    }
}

/* Test function for < comparison */
void test_lt(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        /* This if-statement with < comparison should trigger LT_EXPR case */
        if (a[i] < b[i]) {
            c[i] = a[i] + 100;
        } else {
            c[i] = b[i] - 100;
        }
    }
}

/* Test function for <= comparison */
void test_le(int *restrict a, int *restrict b, int *restrict c) {
    for (int i = 0; i < N; i++) {
        /* This if-statement with <= comparison should trigger LE_EXPR case */
        if (a[i] <= b[i]) {
            c[i] = a[i] * b[i];
        } else {
            c[i] = a[i] / 2;
        }
    }
}

/* Additional test with floating point comparisons */
void test_float_gt(float *restrict a, float *restrict b, float *restrict c) {
    for (int i = 0; i < N; i++) {
        /* Floating point > comparison */
        if (a[i] > b[i]) {
            c[i] = a[i] * 2.0f;
        } else {
            c[i] = b[i] * 2.0f;
        }
    }
}

/* Test with mixed comparisons in same loop */
void test_mixed_comparisons(int *restrict a, int *restrict b, 
                           int *restrict c, int *restrict d) {
    for (int i = 0; i < N; i++) {
        /* Multiple comparisons that should all be vectorized */
        c[i] = (a[i] > b[i]) ? a[i] : b[i];      // GT_EXPR
        d[i] = (a[i] <= b[i]) ? a[i] + b[i] : a[i] - b[i];  // LE_EXPR
    }
}

/* Initialize arrays with pattern that creates varied comparison results */
void init_arrays(int *a, int *b) {
    for (int i = 0; i < N; i++) {
        a[i] = i;              // 0, 1, 2, 3, ...
        b[i] = N - i - 1;      // 1023, 1022, 1021, ...
    }
}

void init_float_arrays(float *a, float *b) {
    for (int i = 0; i < N; i++) {
        a[i] = (float)i * 1.5f;
        b[i] = (float)(N - i) * 0.5f;
    }
}

/* Compute checksum to ensure loops execute and prevent dead code elimination */
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
    /* Use aligned arrays for better vectorization */
    ALIGNED int a[N], b[N], c1[N], c2[N], c3[N], c4[N], d1[N], d2[N];
    ALIGNED float fa[N], fb[N], fc[N];
    
    /* Initialize test data */
    init_arrays(a, b);
    init_float_arrays(fa, fb);
    
    printf("Testing vectorization of comparison operators...\n");
    
    /* Test each comparison operator in separate loops */
    test_gt(a, b, c1);
    test_ge(a, b, c2);
    test_lt(a, b, c3);
    test_le(a, b, c4);
    
    /* Test floating point comparison */
    test_float_gt(fa, fb, fc);
    
    /* Test mixed comparisons */
    test_mixed_comparisons(a, b, d1, d2);
    
    /* Compute and print checksums to ensure code executes */
    printf("Checksum GT: %d\n", compute_checksum(c1));
    printf("Checksum GE: %d\n", compute_checksum(c2));
    printf("Checksum LT: %d\n", compute_checksum(c3));
    printf("Checksum LE: %d\n", compute_checksum(c4));
    printf("Checksum Float GT: %f\n", compute_float_checksum(fc));
    printf("Checksum Mixed 1: %d\n", compute_checksum(d1));
    printf("Checksum Mixed 2: %d\n", compute_checksum(d2));
    
    /* Verify a few values to ensure correctness */
    printf("\nVerification (first 5 elements):\n");
    for (int i = 0; i < 5; i++) {
        printf("a[%d]=%d, b[%d]=%d, c1[%d]=%d, c4[%d]=%d\n",
               i, a[i], i, b[i], i, c1[i], i, c4[i]);
    }
    
    return 0;
}
