#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Test functions for each comparison operator */
void test_gt_expr(int *a, int *b, int *c, int *d) {
    /* GT_EXPR (>): if (a[i] > b[i]) c[i] = d[i] * 2 */
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            c[i] = d[i] * 2;
        } else {
            c[i] = d[i] / 2;
        }
    }
}

void test_ge_expr(int *a, int *b, int *c, int *d) {
    /* GE_EXPR (>=): Conditional assignment with blend pattern */
    for (int i = 0; i < N; i++) {
        c[i] = (a[i] >= b[i]) ? d[i] + a[i] : d[i] - b[i];
    }
}

void test_lt_expr(int *a, int *b, int *c, int *d) {
    /* LT_EXPR (<): Masked store with arithmetic */
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            c[i] = a[i] * b[i] + d[i];
        } else {
            c[i] = a[i] + b[i] - d[i];
        }
    }
}

void test_le_expr(int *a, int *b, int *c, int *d) {
    /* LE_EXPR (<=): Reduction with conditional increment */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            sum += d[i] * 3;
        } else {
            sum += d[i];
        }
    }
    /* Store result to prevent elimination */
    c[0] = sum;
}

/* Additional test with floating point to ensure different type handling */
void test_float_comparisons(float *a, float *b, float *c) {
    /* Mix of comparison operators on floats */
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            c[i] = a[i] * 2.0f;
        } else if (a[i] >= b[i]) {
            c[i] = a[i] + b[i];
        } else if (a[i] < b[i]) {
            c[i] = b[i] - a[i];
        } else if (a[i] <= b[i]) {
            c[i] = a[i] / (b[i] + 1.0f);
        }
    }
}

/* Helper to initialize arrays with pattern that creates mix of true/false comparisons */
void init_arrays(int *a, int *b, int *c, int *d) {
    for (int i = 0; i < N; i++) {
        a[i] = i - N/2;          /* Range: [-N/2, N/2-1] */
        b[i] = (i % 3) * 10;     /* Pattern: 0, 10, 20, 0, 10, 20... */
        c[i] = 0;                /* Output array */
        d[i] = i * 7 + 3;        /* Different pattern for computations */
    }
}

void init_float_arrays(float *a, float *b, float *c) {
    for (int i = 0; i < N; i++) {
        a[i] = (float)(i - N/2) * 0.5f;
        b[i] = (float)(i % 5) * 2.0f;
        c[i] = 0.0f;
    }
}

/* Verification function to ensure computations are correct */
int verify_results(int *ref, int *test, int size) {
    for (int i = 0; i < size; i++) {
        if (ref[i] != test[i]) {
            return 0;
        }
    }
    return 1;
}

int main() {
    /* Aligned allocations for better vectorization */
    int *a = (int*)aligned_alloc(32, N * sizeof(int));
    int *b = (int*)aligned_alloc(32, N * sizeof(int));
    int *c1 = (int*)aligned_alloc(32, N * sizeof(int));
    int *c2 = (int*)aligned_alloc(32, N * sizeof(int));
    int *c3 = (int*)aligned_alloc(32, N * sizeof(int));
    int *c4 = (int*)aligned_alloc(32, N * sizeof(int));
    int *d = (int*)aligned_alloc(32, N * sizeof(int));
    
    float *fa = (float*)aligned_alloc(32, N * sizeof(float));
    float *fb = (float*)aligned_alloc(32, N * sizeof(float));
    float *fc = (float*)aligned_alloc(32, N * sizeof(float));
    
    if (!a || !b || !c1 || !c2 || !c3 || !c4 || !d || !fa || !fb || !fc) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Initialize arrays */
    init_arrays(a, b, c1, d);
    memcpy(c2, c1, N * sizeof(int));
    memcpy(c3, c1, N * sizeof(int));
    memcpy(c4, c1, N * sizeof(int));
    init_float_arrays(fa, fb, fc);
    
    /* Execute all test functions */
    test_gt_expr(a, b, c1, d);   /* Should trigger GT_EXPR case */
    test_ge_expr(a, b, c2, d);   /* Should trigger GE_EXPR case */
    test_lt_expr(a, b, c3, d);   /* Should trigger LT_EXPR case */
    test_le_expr(a, b, c4, d);   /* Should trigger LE_EXPR case */
    test_float_comparisons(fa, fb, fc);  /* Additional coverage */
    
    /* Compute checksum to prevent dead code elimination */
    int checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += c1[i] + c2[i] + c3[i] + c4[i] + (int)fc[i];
    }
    
    /* Print result to ensure code isn't optimized away */
    printf("Test completed. Checksum: %d\n", checksum);
    
    /* Cleanup */
    free(a); free(b); free(c1); free(c2); free(c3); free(c4); free(d);
    free(fa); free(fb); free(fc);
    
    return 0;
}
