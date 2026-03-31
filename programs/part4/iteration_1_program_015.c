/* Program to trigger GCC's vectorization pattern matching for 
 * integer comparisons (GT_EXPR, GE_EXPR, LT_EXPR, LE_EXPR) 
 * and their transformation to bitwise operations.
 */

#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define SEED 42

/* Non-inlineable functions to prevent premature optimization */
__attribute__((noinline, noipa))
void compute_gt(const int *a, const int *b, int *mask, int n) {
    for (int i = 0; i < n; i++) {
        mask[i] = a[i] > b[i];  /* GT_EXPR */
    }
}

__attribute__((noinline, noipa))
void compute_ge(const int *a, const int *b, int *mask, int n) {
    for (int i = 0; i < n; i++) {
        mask[i] = a[i] >= b[i];  /* GE_EXPR */
    }
}

__attribute__((noinline, noipa))
void compute_lt(const int *a, const int *b, int *mask, int n) {
    for (int i = 0; i < n; i++) {
        mask[i] = a[i] < b[i];  /* LT_EXPR - ensure operands aren't swapped */
    }
}

__attribute__((noinline, noipa))
void compute_le(const int *a, const int *b, int *mask, int n) {
    for (int i = 0; i < n; i++) {
        mask[i] = a[i] <= b[i];  /* LE_EXPR */
    }
}

/* Mixed comparison function with volatile selector to ensure 
 * all comparison types appear in IR */
__attribute__((noinline, noipa))
void compute_mixed(const int *a, const int *b, int *mask, int n, volatile int selector) {
    for (int i = 0; i < n; i++) {
        /* Use volatile selector to prevent constant propagation */
        switch (selector & 3) {
            case 0: mask[i] = a[i] > b[i]; break;   /* GT_EXPR */
            case 1: mask[i] = a[i] >= b[i]; break;  /* GE_EXPR */
            case 2: mask[i] = a[i] < b[i]; break;   /* LT_EXPR */
            case 3: mask[i] = a[i] <= b[i]; break;  /* LE_EXPR */
        }
    }
}

/* Initialize arrays with patterned data to ensure mixed comparison results */
void init_arrays(int *a, int *b, int n) {
    for (int i = 0; i < n; i++) {
        /* Create patterns that will yield both true and false comparisons */
        a[i] = (i * 3) % 256;          /* Values 0-255, cycling */
        b[i] = (i * 5) % 256;          /* Different pattern for varied comparisons */
        
        /* Add some specific patterns to trigger edge cases */
        if (i % 16 == 0) a[i] = b[i];      /* Equal values */
        if (i % 32 == 1) a[i] = b[i] + 1;  /* a > b by 1 */
        if (i % 64 == 2) a[i] = b[i] - 1;  /* a < b by 1 */
    }
}

/* Compute checksum to prevent dead code elimination */
int compute_checksum(const int *arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum = (sum * 31 + arr[i]) & 0xFFFF;
    }
    return sum;
}

int main(void) {
    /* Declare arrays with enough elements for vectorization */
    int src1[N], src2[N];
    int gt_mask[N], ge_mask[N], lt_mask[N], le_mask[N];
    int mixed_mask[N];
    
    /* Initialize with patterned data */
    init_arrays(src1, src2, N);
    
    /* Execute all comparison types */
    compute_gt(src1, src2, gt_mask, N);
    compute_ge(src1, src2, ge_mask, N);
    compute_lt(src1, src2, lt_mask, N);
    compute_le(src1, src2, le_mask, N);
    
    /* Execute mixed comparisons with volatile selector */
    volatile int selector = 0;
    compute_mixed(src1, src2, mixed_mask, N, selector);
    
    /* Compute checksums to ensure all results are used */
    int checksum = 0;
    checksum ^= compute_checksum(gt_mask, N);
    checksum ^= compute_checksum(ge_mask, N);
    checksum ^= compute_checksum(lt_mask, N);
    checksum ^= compute_checksum(le_mask, N);
    checksum ^= compute_checksum(mixed_mask, N);
    
    printf("Final checksum: %d\n", checksum);
    
    /* Additional test with different data types to increase coverage */
    {
        unsigned short usrc1[N], usrc2[N];
        unsigned short umask[N];
        
        for (int i = 0; i < N; i++) {
            usrc1[i] = (i * 7) % 65535;
            usrc2[i] = (i * 11) % 65535;
        }
        
        /* Test unsigned comparisons as well */
        for (int i = 0; i < N; i++) {
            umask[i] = usrc1[i] > usrc2[i];  /* Unsigned GT_EXPR */
        }
        
        int usum = 0;
        for (int i = 0; i < N; i++) usum += umask[i];
        printf("Unsigned comparison sum: %d\n", usum);
    }
    
    return 0;
}
