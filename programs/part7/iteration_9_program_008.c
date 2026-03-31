#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 1024

/* Function prototypes for each comparison type */
void vector_gt(int *src, int *dst, int threshold);
void vector_ge(int *src, int *dst, int limit);
void vector_lt(int *src, int *dst, int lower_bound);
void vector_le(int *src, int *dst, int upper_bound);

int main(void) {
    int i;
    int src[N];
    int dst_gt[N], dst_ge[N], dst_lt[N], dst_le[N];
    int sum = 0;
    
    /* Initialize with non-uniform, non-constant data */
    srand(time(NULL));
    for (i = 0; i < N; i++) {
        src[i] = (rand() % 200) - 100;  /* Values between -100 and 99 */
    }
    
    /* Test GT_EXPR transformation */
    vector_gt(src, dst_gt, 25);  /* Elements > 25 get 100, others get 0 */
    
    /* Test GE_EXPR transformation */
    vector_ge(src, dst_ge, -50); /* Elements >= -50 get src value, others get -1 */
    
    /* Test LT_EXPR transformation */
    vector_lt(src, dst_lt, 75);  /* Elements < 75 get 0, others get 200 */
    
    /* Test LE_EXPR transformation */
    vector_le(src, dst_le, 0);   /* Elements <= 0 get -src value, others get src */
    
    /* Compute checksum to prevent dead code elimination */
    for (i = 0; i < N; i++) {
        sum += dst_gt[i] + dst_ge[i] + dst_lt[i] + dst_le[i];
    }
    
    printf("Final checksum: %d\n", sum);
    return 0;
}

/* GT_EXPR: > comparison */
void vector_gt(int *src, int *dst, int threshold) {
    int i;
    for (i = 0; i < N; i++) {
        if (src[i] > threshold) {
            dst[i] = 100;  /* Assign if condition true */
        } else {
            dst[i] = 0;    /* Different value if false */
        }
    }
}

/* GE_EXPR: >= comparison */
void vector_ge(int *src, int *dst, int limit) {
    int i;
    for (i = 0; i < N; i++) {
        if (src[i] >= limit) {
            dst[i] = src[i];  /* Keep original value */
        } else {
            dst[i] = -1;      /* Mark as invalid */
        }
    }
}

/* LT_EXPR: < comparison */
void vector_lt(int *src, int *dst, int lower_bound) {
    int i;
    for (i = 0; i < N; i++) {
        if (src[i] < lower_bound) {
            dst[i] = 0;      /* Zero out small values */
        } else {
            dst[i] = 200;    /* Large value for others */
        }
    }
}

/* LE_EXPR: <= comparison */
void vector_le(int *src, int *dst, int upper_bound) {
    int i;
    for (i = 0; i < N; i++) {
        if (src[i] <= upper_bound) {
            dst[i] = -src[i];  /* Negate if within bound */
        } else {
            dst[i] = src[i];   /* Keep positive if above */
        }
    }
}
