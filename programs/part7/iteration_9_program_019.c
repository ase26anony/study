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
    
    /* Seed random number generator for non-constant data */
    srand(time(NULL));
    
    /* Initialize source array with mixed values (-100 to 100) */
    for (i = 0; i < N; i++) {
        src[i] = (rand() % 201) - 100;  /* Range: -100 to 100 */
    }
    
    /* Initialize output arrays */
    for (i = 0; i < N; i++) {
        dst_gt[i] = 0;
        dst_ge[i] = 0;
        dst_lt[i] = 0;
        dst_le[i] = 0;
    }
    
    /* Test all four comparison operators in separate loops */
    vector_gt(src, dst_gt, 50);      /* GT_EXPR: > 50 */
    vector_ge(src, dst_ge, -25);     /* GE_EXPR: >= -25 */
    vector_lt(src, dst_lt, 25);      /* LT_EXPR: < 25 */
    vector_le(src, dst_le, -50);     /* LE_EXPR: <= -50 */
    
    /* Compute checksum to prevent dead code elimination */
    for (i = 0; i < N; i++) {
        sum += dst_gt[i] + dst_ge[i] + dst_lt[i] + dst_le[i];
    }
    
    printf("Checksum: %d\n", sum);
    return 0;
}

/* GT_EXPR transformation target: if (src[i] > threshold) dst[i] = src[i] */
void vector_gt(int *src, int *dst, int threshold) {
    int i;
    for (i = 0; i < N; i++) {
        if (src[i] > threshold) {          /* GT_EXPR */
            dst[i] = src[i] * 2;           /* Conditional assignment */
        } else {
            dst[i] = 0;
        }
    }
}

/* GE_EXPR transformation target: if (src[i] >= limit) dst[i] = src[i] */
void vector_ge(int *src, int *dst, int limit) {
    int i;
    for (i = 0; i < N; i++) {
        if (src[i] >= limit) {             /* GE_EXPR */
            dst[i] = src[i] + 100;         /* Conditional assignment */
        } else {
            dst[i] = -1;
        }
    }
}

/* LT_EXPR transformation target: if (src[i] < lower_bound) dst[i] = src[i] */
void vector_lt(int *src, int *dst, int lower_bound) {
    int i;
    for (i = 0; i < N; i++) {
        if (src[i] < lower_bound) {        /* LT_EXPR */
            dst[i] = src[i] - 50;          /* Conditional assignment */
        } else {
            dst[i] = 1;
        }
    }
}

/* LE_EXPR transformation target: if (src[i] <= upper_bound) dst[i] = src[i] */
void vector_le(int *src, int *dst, int upper_bound) {
    int i;
    for (i = 0; i < N; i++) {
        if (src[i] <= upper_bound) {       /* LE_EXPR */
            dst[i] = src[i] * 3;           /* Conditional assignment */
        } else {
            dst[i] = 0;
        }
    }
}
