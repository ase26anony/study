/* Program to trigger vectorization pattern matching for integer comparisons */
#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define SEED 42

/* Non-inlineable functions to prevent premature optimization */
__attribute__((noinline, noipa))
void compute_gt(const int *a, const int *b, int *out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = a[i] > b[i];  /* GT_EXPR */
    }
}

__attribute__((noinline, noipa))
void compute_ge(const int *a, const int *b, int *out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = a[i] >= b[i]; /* GE_EXPR */
    }
}

__attribute__((noinline, noipa))
void compute_lt(const int *a, const int *b, int *out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = a[i] < b[i];  /* LT_EXPR - ensure operands aren't swapped */
    }
}

__attribute__((noinline, noipa))
void compute_le(const int *a, const int *b, int *out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = a[i] <= b[i]; /* LE_EXPR - ensure operands aren't swapped */
    }
}

/* Mixed comparison function using volatile selector */
__attribute__((noinline, noipa))
void compute_mixed(const int *a, const int *b, int *out, int n, volatile int selector) {
    for (int i = 0; i < n; i++) {
        switch (selector) {
            case 0: out[i] = a[i] > b[i]; break;   /* GT_EXPR */
            case 1: out[i] = a[i] >= b[i]; break;  /* GE_EXPR */
            case 2: out[i] = a[i] < b[i]; break;   /* LT_EXPR */
            case 3: out[i] = a[i] <= b[i]; break;  /* LE_EXPR */
            default: out[i] = 0; break;
        }
    }
}

/* Initialize arrays with pattern to ensure mixed comparison results */
void init_arrays(int *a, int *b, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (i * 3) % 100;      /* Pattern: 0, 3, 6, 9, ... */
        b[i] = (i * 2) % 100;      /* Pattern: 0, 2, 4, 6, ... */
    }
}

int main(void) {
    /* Declare arrays with alignment hint for vectorization */
    int src1[N] __attribute__((aligned(32)));
    int src2[N] __attribute__((aligned(32)));
    int gt_mask[N] __attribute__((aligned(32)));
    int ge_mask[N] __attribute__((aligned(32)));
    int lt_mask[N] __attribute__((aligned(32)));
    int le_mask[N] __attribute__((aligned(32)));
    int mixed_mask[N] __attribute__((aligned(32)));
    
    /* Initialize source arrays */
    init_arrays(src1, src2, N);
    
    /* Call all comparison functions */
    compute_gt(src1, src2, gt_mask, N);
    compute_ge(src1, src2, ge_mask, N);
    compute_lt(src1, src2, lt_mask, N);
    compute_le(src1, src2, le_mask, N);
    
    /* Use volatile selector to force all comparison types in IR */
    volatile int selector = 0;
    for (int s = 0; s < 4; s++) {
        selector = s;
        compute_mixed(src1, src2, mixed_mask, N, selector);
    }
    
    /* Compute checksum to prevent dead code elimination */
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
            src1_short[i] = (short)(i % 256);
            src2_short[i] = (short)((i + 128) % 256);
        }
        
        /* Test with short type - different vectorization path */
        for (int i = 0; i < N; i++) {
            mask_short[i] = src1_short[i] > src2_short[i];  /* GT_EXPR */
        }
        
        /* Add to checksum */
        for (int i = 0; i < N; i++) {
            checksum += mask_short[i];
        }
    }
    
    printf("Final checksum: %llu\n", checksum);
    
    return 0;
}
