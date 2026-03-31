#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define SEED 42

/* Non-inlineable functions to prevent premature optimization */
__attribute__((noinline, noipa))
void compute_gt(const int* a, const int* b, int* mask, int n) {
    for (int i = 0; i < n; i++) {
        mask[i] = a[i] > b[i];  /* GT_EXPR */
    }
}

__attribute__((noinline, noipa))
void compute_ge(const int* a, const int* b, int* mask, int n) {
    for (int i = 0; i < n; i++) {
        mask[i] = a[i] >= b[i]; /* GE_EXPR */
    }
}

__attribute__((noinline, noipa))
void compute_lt(const int* a, const int* b, int* mask, int n) {
    for (int i = 0; i < n; i++) {
        mask[i] = a[i] < b[i];  /* LT_EXPR - ensure operands aren't swapped */
    }
}

__attribute__((noinline, noipa))
void compute_le(const int* a, const int* b, int* mask, int n) {
    for (int i = 0; i < n; i++) {
        mask[i] = a[i] <= b[i]; /* LE_EXPR - ensure operands aren't swapped */
    }
}

/* Mixed comparison function using volatile selector */
__attribute__((noinline, noipa))
void compute_mixed(const int* a, const int* b, int* mask, int n, volatile int selector) {
    for (int i = 0; i < n; i++) {
        /* Force all comparison types to appear in IR */
        switch (selector & 3) {
            case 0: mask[i] = a[i] > b[i]; break;   /* GT_EXPR */
            case 1: mask[i] = a[i] >= b[i]; break;  /* GE_EXPR */
            case 2: mask[i] = a[i] < b[i]; break;   /* LT_EXPR */
            case 3: mask[i] = a[i] <= b[i]; break;  /* LE_EXPR */
        }
    }
}

/* Initialize arrays with pattern to ensure mixed comparison results */
void init_arrays(int* a, int* b, int n) {
    for (int i = 0; i < n; i++) {
        /* Create patterns where comparisons will be sometimes true, sometimes false */
        a[i] = (i * 3) % 100;
        b[i] = (i * 2) % 100;
    }
}

int main(void) {
    /* Declare arrays with alignment hint for better vectorization */
    int src1[N] __attribute__((aligned(32)));
    int src2[N] __attribute__((aligned(32)));
    int gt_mask[N] __attribute__((aligned(32)));
    int ge_mask[N] __attribute__((aligned(32)));
    int lt_mask[N] __attribute__((aligned(32)));
    int le_mask[N] __attribute__((aligned(32)));
    int mixed_mask[N] __attribute__((aligned(32)));
    
    /* Initialize with deterministic pattern */
    init_arrays(src1, src2, N);
    
    /* Force all comparison types to be processed */
    compute_gt(src1, src2, gt_mask, N);
    compute_ge(src1, src2, ge_mask, N);
    compute_lt(src1, src2, lt_mask, N);
    compute_le(src1, src2, le_mask, N);
    
    /* Use volatile to prevent constant propagation in mixed function */
    volatile int selector = 0;
    compute_mixed(src1, src2, mixed_mask, N, selector);
    
    /* Compute checksums to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += gt_mask[i] + ge_mask[i] + lt_mask[i] + le_mask[i] + mixed_mask[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    /* Additional test with different data types to increase coverage */
    {
        short src1_short[N] __attribute__((aligned(32)));
        short src2_short[N] __attribute__((aligned(32)));
        short mask_short[N] __attribute__((aligned(32)));
        
        for (int i = 0; i < N; i++) {
            src1_short[i] = (i * 5) % 256;
            src2_short[i] = (i * 3) % 256;
        }
        
        /* Use different comparison types with short integers */
        for (int i = 0; i < N; i++) {
            mask_short[i] = src1_short[i] > src2_short[i];   /* GT_EXPR with shorts */
        }
        
        /* Add to checksum */
        for (int i = 0; i < N; i++) {
            checksum += mask_short[i];
        }
    }
    
    printf("Final checksum: %llu\n", checksum);
    
    return 0;
}
