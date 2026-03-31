/* test_vectorizable_comparison.c
 * Designed to cover GCC's tree-vect-stmts.cc lines 12216-12233
 * Compile with: gcc -O3 -ftree-vectorize -fopt-info-vec -march=native test_vectorizable_comparison.c -o test_vec
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

int main(void) {
    /* Aligned arrays to help vectorization */
    ALIGNED int a[N], b[N];
    ALIGNED char gt_result[N], ge_result[N], lt_result[N], le_result[N];
    
    int i;
    int checksum = 0;
    
    /* Initialize with non-trivial patterns to ensure comparisons vary */
    for (i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
    }
    
    /* Loop 1: GT_EXPR (>) transformation */
    for (i = 0; i < N; i++) {
        gt_result[i] = a[i] > b[i];  /* Should trigger GT_EXPR path */
    }
    
    /* Loop 2: GE_EXPR (>=) transformation */
    for (i = 0; i < N; i++) {
        ge_result[i] = a[i] >= b[i];  /* Should trigger GE_EXPR path */
    }
    
    /* Loop 3: LT_EXPR (<) transformation */
    for (i = 0; i < N; i++) {
        lt_result[i] = a[i] < b[i];  /* Should trigger LT_EXPR path */
    }
    
    /* Loop 4: LE_EXPR (<=) transformation */
    for (i = 0; i < N; i++) {
        le_result[i] = a[i] <= b[i];  /* Should trigger LE_EXPR path */
    }
    
    /* Use results to prevent dead code elimination */
    for (i = 0; i < N; i++) {
        checksum += gt_result[i] + ge_result[i] + lt_result[i] + le_result[i];
    }
    
    /* Additional test with mixed comparisons in same loop */
    ALIGNED int mixed_results[N];
    for (i = 0; i < N; i++) {
        /* Mix of different comparison operators */
        if (a[i] > b[i]) {
            mixed_results[i] = 1;
        } else if (a[i] <= b[i]) {
            mixed_results[i] = -1;
        } else {
            mixed_results[i] = 0;
        }
        checksum += mixed_results[i];
    }
    
    /* Test with scalar comparison */
    int threshold = 50;
    ALIGNED int scalar_comp[N];
    for (i = 0; i < N; i++) {
        scalar_comp[i] = a[i] > threshold;  /* Scalar comparison */
        checksum += scalar_comp[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Verify some results to ensure correctness */
    int verify_count = 0;
    for (i = 0; i < 10; i++) {
        int idx = i * 100;
        if (idx < N) {
            int expected_gt = a[idx] > b[idx];
            int expected_le = a[idx] <= b[idx];
            if (gt_result[idx] == expected_gt && le_result[idx] == expected_le) {
                verify_count++;
            }
        }
    }
    
    if (verify_count == 10) {
        printf("Verification passed\n");
        return 0;
    } else {
        printf("Verification failed\n");
        return 1;
    }
}
