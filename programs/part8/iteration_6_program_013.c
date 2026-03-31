/* test_vectorizable_comparison.c
 * Designed to exercise GCC's tree vectorizer comparison transformations
 * for GT_EXPR, GE_EXPR, LT_EXPR, and LE_EXPR operators.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(16)))

int main(void) {
    /* Aligned arrays to help vectorization */
    ALIGNED int a[N];
    ALIGNED int b[N];
    ALIGNED char gt_result[N];  /* Store > comparisons */
    ALIGNED char le_result[N];  /* Store <= comparisons */
    ALIGNED char lt_result[N];  /* Store < comparisons */
    ALIGNED char ge_result[N];  /* Store >= comparisons */
    
    int i;
    int checksum = 0;
    
    /* Initialize arrays with non-trivial patterns */
    for (i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;      /* Values 0-99 */
        b[i] = (i * 7) % 100;      /* Different pattern */
    }
    
    /* Loop 1: Greater-than comparisons (GT_EXPR) */
    for (i = 0; i < N; i++) {
        gt_result[i] = a[i] > b[i];  /* Should trigger GT_EXPR transformation */
    }
    
    /* Loop 2: Less-than-or-equal comparisons (LE_EXPR) */
    for (i = 0; i < N; i++) {
        le_result[i] = a[i] <= b[i]; /* Should trigger LE_EXPR transformation */
    }
    
    /* Loop 3: Less-than comparisons (LT_EXPR) */
    for (i = 0; i < N; i++) {
        lt_result[i] = a[i] < b[i];  /* Should trigger LT_EXPR transformation */
    }
    
    /* Loop 4: Greater-than-or-equal comparisons (GE_EXPR) */
    for (i = 0; i < N; i++) {
        ge_result[i] = a[i] >= b[i]; /* Should trigger GE_EXPR transformation */
    }
    
    /* Use results to prevent dead code elimination */
    for (i = 0; i < N; i++) {
        checksum += gt_result[i] + le_result[i] + lt_result[i] + ge_result[i];
    }
    
    /* Additional test with mixed comparisons in same loop */
    ALIGNED int mixed_result[N];
    for (i = 0; i < N; i++) {
        /* Mix of different comparison types in same expression */
        mixed_result[i] = (a[i] > b[i]) && (a[i] <= 50);
    }
    
    for (i = 0; i < N; i++) {
        checksum += mixed_result[i];
    }
    
    /* Print checksum to ensure all computations are used */
    printf("Checksum: %d\n", checksum);
    
    /* Return non-zero if any comparison was true (just for variety) */
    return checksum == 0 ? 1 : 0;
}
