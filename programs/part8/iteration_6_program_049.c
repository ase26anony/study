/* test_vectorizable_comparison.c
 * Designed to cover GCC's tree-vect-stmts.cc lines 12216-12233
 * Compile with: gcc -O3 -ftree-vectorize -fopt-info-vec -march=native test_vectorizable_comparison.c -o test
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

int main(void) {
    /* Aligned arrays to help vectorization */
    ALIGNED int a[N];
    ALIGNED int b[N];
    ALIGNED char gt_result[N];  /* Results for > comparisons */
    ALIGNED char le_result[N];  /* Results for <= comparisons */
    ALIGNED char lt_result[N];  /* Results for < comparisons */
    ALIGNED char ge_result[N];  /* Results for >= comparisons */
    
    int i;
    int checksum = 0;
    
    /* Initialize with non-trivial patterns to avoid constant folding */
    for (i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
    }
    
    /* Loop 1: GT_EXPR (>) comparison - covers one path in the uncovered block */
    for (i = 0; i < N; i++) {
        gt_result[i] = (a[i] > b[i]);  /* Scalar > comparison to be vectorized */
    }
    
    /* Loop 2: LE_EXPR (<=) comparison - covers another path */
    for (i = 0; i < N; i++) {
        le_result[i] = (a[i] <= b[i]);  /* Scalar <= comparison */
    }
    
    /* Loop 3: LT_EXPR (<) comparison - covers third path */
    for (i = 0; i < N; i++) {
        lt_result[i] = (a[i] < b[i]);  /* Scalar < comparison */
    }
    
    /* Loop 4: GE_EXPR (>=) comparison - covers fourth path */
    for (i = 0; i < N; i++) {
        ge_result[i] = (a[i] >= b[i]);  /* Scalar >= comparison */
    }
    
    /* Use results to prevent dead code elimination */
    for (i = 0; i < N; i++) {
        checksum += gt_result[i] + le_result[i] + lt_result[i] + ge_result[i];
    }
    
    /* Additional pattern: Mixed comparisons in same loop */
    ALIGNED int mixed_result[N];
    for (i = 0; i < N; i++) {
        /* Mix > and <= in same expression to test operand swapping logic */
        mixed_result[i] = (a[i] > b[i]) && (a[i] <= b[i] + 10);
    }
    
    for (i = 0; i < N; i++) {
        checksum += mixed_result[i];
    }
    
    /* Pattern with different integer types to test various vectorization paths */
    ALIGNED short short_a[N], short_b[N];
    ALIGNED char short_result[N];
    
    for (i = 0; i < N; i++) {
        short_a[i] = (i * 5) % 256;
        short_b[i] = (i * 11) % 256;
    }
    
    /* Loop with < comparison on short type */
    for (i = 0; i < N; i++) {
        short_result[i] = (short_a[i] < short_b[i]);
        checksum += short_result[i];
    }
    
    /* Pattern with unsigned comparisons */
    ALIGNED unsigned int ua[N], ub[N];
    ALIGNED char u_result[N];
    
    for (i = 0; i < N; i++) {
        ua[i] = (i * 13) % 200;
        ub[i] = (i * 17) % 200;
    }
    
    /* Loop with >= comparison on unsigned type */
    for (i = 0; i < N; i++) {
        u_result[i] = (ua[i] >= ub[i]);
        checksum += u_result[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Return value based on checksum to ensure all code paths matter */
    return (checksum > N) ? 0 : 1;
}
