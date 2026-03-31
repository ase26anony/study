#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1024

/* Function prototypes for each comparison type */
void vector_gt(int *src, int *dst, int threshold);
void vector_ge(int *src, int *dst, int limit);
void vector_lt(int *src, int *dst, int lower_bound);
void vector_le(int *src, int *dst, int upper_bound);

int main() {
    /* Allocate and initialize arrays */
    int src[N], dst_gt[N], dst_ge[N], dst_lt[N], dst_le[N];
    int i, sum = 0;
    
    /* Seed random number generator for predictable but non-constant data */
    srand(42);
    
    /* Initialize source array with values in range [-100, 100] */
    for (i = 0; i < N; i++) {
        src[i] = (rand() % 201) - 100;  /* Values from -100 to 100 */
    }
    
    /* Initialize output arrays */
    for (i = 0; i < N; i++) {
        dst_gt[i] = 0;
        dst_ge[i] = 0;
        dst_lt[i] = 0;
        dst_le[i] = 0;
    }
    
    /* Test all four comparison operators */
    vector_gt(src, dst_gt, 50);      /* GT_EXPR: > 50 */
    vector_ge(src, dst_ge, -25);     /* GE_EXPR: >= -25 */
    vector_lt(src, dst_lt, 75);      /* LT_EXPR: < 75 */
    vector_le(src, dst_le, -50);     /* LE_EXPR: <= -50 */
    
    /* Compute checksum to prevent dead code elimination */
    for (i = 0; i < N; i++) {
        sum += dst_gt[i] + dst_ge[i] + dst_lt[i] + dst_le[i];
    }
    
    printf("Checksum: %d\n", sum);
    return 0;
}

/* GT_EXPR transformation target */
void vector_gt(int *src, int *dst, int threshold) {
    int i;
    for (i = 0; i < N; i++) {
        /* This should trigger the GT_EXPR -> BIT_NOT_EXPR, BIT_AND_EXPR transformation */
        if (src[i] > threshold) {
            dst[i] = src[i] * 2;  /* Non-trivial operation to ensure vectorization attempt */
        } else {
            dst[i] = 0;
        }
    }
}

/* GE_EXPR transformation target */
void vector_ge(int *src, int *dst, int limit) {
    int i;
    for (i = 0; i < N; i++) {
        /* This should trigger the GE_EXPR -> BIT_NOT_EXPR, BIT_IOR_EXPR transformation */
        if (src[i] >= limit) {
            dst[i] = src[i] + 100;  /* Different operation to avoid pattern merging */
        } else {
            dst[i] = -1;
        }
    }
}

/* LT_EXPR transformation target */
void vector_lt(int *src, int *dst, int lower_bound) {
    int i;
    for (i = 0; i < N; i++) {
        /* This should trigger the LT_EXPR -> BIT_NOT_EXPR, BIT_AND_EXPR transformation
           with std::swap(cond_expr0, cond_expr1) */
        if (src[i] < lower_bound) {
            dst[i] = src[i] / 2;  /* Different operation */
        } else {
            dst[i] = 1;
        }
    }
}

/* LE_EXPR transformation target */
void vector_le(int *src, int *dst, int upper_bound) {
    int i;
    for (i = 0; i < N; i++) {
        /* This should trigger the LE_EXPR -> BIT_NOT_EXPR, BIT_IOR_EXPR transformation
           with std::swap(cond_expr0, cond_expr1) */
        if (src[i] <= upper_bound) {
            dst[i] = src[i] * 3;  /* Different operation */
        } else {
            dst[i] = -2;
        }
    }
}
