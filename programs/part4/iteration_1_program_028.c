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
        mask[i] = a[i] >= b[i]; /* GE_EXPR */
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
        mask[i] = a[i] <= b[i]; /* LE_EXPR - ensure operands aren't swapped */
    }
}

/* Mixed comparison function with volatile control to keep all patterns in IR */
__attribute__((noinline, noipa))
void compute_mixed(const int *a, const int *b, int *mask, int n, volatile int selector) {
    for (int i = 0; i < n; i++) {
        switch (selector) {
            case 0: mask[i] = a[i] > b[i]; break;   /* GT_EXPR */
            case 1: mask[i] = a[i] >= b[i]; break;  /* GE_EXPR */
            case 2: mask[i] = a[i] < b[i]; break;   /* LT_EXPR */
            case 3: mask[i] = a[i] <= b[i]; break;  /* LE_EXPR */
            default: mask[i] = 0; break;
        }
    }
}

/* Simple PRNG for deterministic pattern */
static unsigned int prng_state = SEED;
static inline unsigned int simple_rand(void) {
    prng_state = prng_state * 1103515245 + 12345;
    return (unsigned int)(prng_state >> 16) & 32767;
}

int main(void) {
    /* Declare arrays with size multiple of typical vector widths */
    int src1[N], src2[N];
    int gt_mask[N], ge_mask[N], lt_mask[N], le_mask[N];
    int mixed_mask[N];
    
    /* Initialize with patterned data to ensure mix of true/false comparisons */
    for (int i = 0; i < N; i++) {
        /* src1: ascending values with some variation */
        src1[i] = i * 2 + (simple_rand() % 5);
        /* src2: descending values with variation */
        src2[i] = (N - i) * 2 + (simple_rand() % 5);
    }
    
    /* Force all comparison types to be processed */
    compute_gt(src1, src2, gt_mask, N);
    compute_ge(src1, src2, ge_mask, N);
    compute_lt(src1, src2, lt_mask, N);
    compute_le(src1, src2, le_mask, N);
    
    /* Use volatile selector to prevent constant propagation */
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
        short src1_short[N], src2_short[N];
        short mask_short[N];
        
        for (int i = 0; i < N; i++) {
            src1_short[i] = (short)(i % 256);
            src2_short[i] = (short)((i + 128) % 256);
        }
        
        /* Test with short type - might trigger different vectorization */
        for (int i = 0; i < N; i++) {
            mask_short[i] = src1_short[i] > src2_short[i];  /* GT_EXPR with shorts */
        }
        
        /* Add to checksum */
        for (int i = 0; i < N; i++) {
            checksum += mask_short[i];
        }
    }
    
    printf("Final checksum: %llu\n", checksum);
    
    return 0;
}
