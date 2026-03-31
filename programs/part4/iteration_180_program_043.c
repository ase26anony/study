/* test_vector_cond_bitops.c
 * Designed to trigger vectorizer transformation of comparison operations
 * to bit operations (BIT_NOT_EXPR, BIT_AND_EXPR, BIT_IOR_EXPR) for
 * GT_EXPR, GE_EXPR, LT_EXPR, and LE_EXPR cases.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N 1024
#define ALIGN 32

/* Aligned allocations for better vectorization */
static float *a_f, *b_f, *c_f, *d_f;
static int *a_i, *b_i, *c_i, *d_i;

__attribute__((noinline))
void init_data() {
    /* Allocate aligned memory */
    a_f = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    b_f = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    c_f = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    d_f = (float*)aligned_alloc(ALIGN, N * sizeof(float));
    
    a_i = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    b_i = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    c_i = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    d_i = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    
    /* Initialize with patterns that create mixed true/false comparisons */
    for (int i = 0; i < N; i++) {
        a_f[i] = (float)(i - N/2);      /* Range: [-512, 511] */
        b_f[i] = (float)(i % 100);      /* Range: [0, 99] */
        c_f[i] = 0.0f;
        d_f[i] = (float)i;
        
        a_i[i] = i * 2 - N;             /* Range: [-1024, 1022] */
        b_i[i] = i;                     /* Range: [0, 1023] */
        c_i[i] = 0;
        d_i[i] = i * 3;
    }
}

/* GT_EXPR (>) - should trigger BIT_NOT_EXPR + BIT_AND_EXPR */
__attribute__((noinline))
void test_gt_expr() {
    /* Pattern 1: Conditional assignment with > */
    for (int i = 0; i < N; i++) {
        if (a_f[i] > b_f[i]) {
            c_f[i] = a_f[i] * b_f[i];  /* Use both operands */
        } else {
            c_f[i] = a_f[i] + b_f[i];
        }
    }
    
    /* Pattern 2: Masked operation with integer > */
    for (int i = 0; i < N; i++) {
        c_i[i] = (a_i[i] > b_i[i]) ? (a_i[i] - b_i[i]) : (b_i[i] - a_i[i]);
    }
    
    /* Pattern 3: Reduction with > comparison */
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += (a_i[i] > b_i[i]) ? a_i[i] : 0;
    }
    c_i[0] = sum;  /* Store to prevent elimination */
}

/* GE_EXPR (>=) - should trigger BIT_NOT_EXPR + BIT_IOR_EXPR */
__attribute__((noinline))
void test_ge_expr() {
    /* Pattern 1: Conditional assignment with >= */
    for (int i = 0; i < N; i++) {
        if (a_f[i] >= b_f[i]) {
            c_f[i] = a_f[i] / (b_f[i] + 1.0f);  /* Avoid division by zero */
        } else {
            c_f[i] = b_f[i] / (a_f[i] + 1.0f);
        }
    }
    
    /* Pattern 2: Blend operation with >= */
    for (int i = 0; i < N; i++) {
        d_i[i] = (a_i[i] >= b_i[i]) ? a_i[i] : b_i[i];
    }
    
    /* Pattern 3: Conditional increment with >= */
    int count = 0;
    for (int i = 0; i < N; i++) {
        count += (a_i[i] >= N/2) ? 1 : 0;
    }
    c_i[1] = count;
}

/* LT_EXPR (<) - should trigger BIT_NOT_EXPR + BIT_AND_EXPR with swap */
__attribute__((noinline))
void test_lt_expr() {
    /* Pattern 1: Conditional assignment with < */
    for (int i = 0; i < N; i++) {
        if (a_f[i] < b_f[i]) {
            c_f[i] = a_f[i] - b_f[i];
        } else {
            c_f[i] = b_f[i] - a_f[i];
        }
    }
    
    /* Pattern 2: Masked store with < */
    for (int i = 0; i < N; i++) {
        if (a_i[i] < b_i[i]) {
            d_i[i] = a_i[i] * b_i[i];
        }
    }
    
    /* Pattern 3: Reduction with < comparison */
    float product = 1.0f;
    for (int i = 0; i < N; i++) {
        if (a_f[i] < 0.0f) {
            product *= (a_f[i] + 100.0f);  /* Avoid zero product */
        }
    }
    c_f[0] = product;
}

/* LE_EXPR (<=) - should trigger BIT_NOT_EXPR + BIT_IOR_EXPR with swap */
__attribute__((noinline))
void test_le_expr() {
    /* Pattern 1: Conditional assignment with <= */
    for (int i = 0; i < N; i++) {
        c_f[i] = (a_f[i] <= b_f[i]) ? (a_f[i] * 2.0f) : (b_f[i] * 3.0f);
    }
    
    /* Pattern 2: Complex conditional with <= */
    for (int i = 0; i < N; i++) {
        if (a_i[i] <= b_i[i]) {
            d_i[i] = a_i[i] + b_i[i] + i;
        } else {
            d_i[i] = a_i[i] - b_i[i] - i;
        }
    }
    
    /* Pattern 3: Nested conditionals with <= */
    for (int i = 0; i < N; i++) {
        if (a_i[i] <= 0) {
            c_i[i] = -a_i[i];
        } else if (a_i[i] <= N/2) {
            c_i[i] = a_i[i] * 2;
        } else {
            c_i[i] = a_i[i];
        }
    }
}

/* Verification function to ensure computations aren't optimized away */
__attribute__((noinline))
int verify_results() {
    int checksum = 0;
    
    /* Compute checksums from all result arrays */
    for (int i = 0; i < N; i++) {
        checksum += (int)c_f[i];
        checksum += c_i[i];
        checksum += d_i[i];
    }
    
    return checksum;
}

int main() {
    init_data();
    
    /* Execute all test functions */
    test_gt_expr();
    test_ge_expr();
    test_lt_expr();
    test_le_expr();
    
    /* Verify results to prevent dead code elimination */
    int result = verify_results();
    
    /* Clean up */
    free(a_f); free(b_f); free(c_f); free(d_f);
    free(a_i); free(b_i); free(c_i); free(d_i);
    
    printf("Result checksum: %d\n", result);
    printf("All conditional patterns executed.\n");
    
    return 0;
}
