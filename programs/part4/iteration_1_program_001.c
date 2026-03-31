#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define SEED 42

/* Non-inlineable functions to prevent premature optimization */
__attribute__((noinline, noipa))
void compute_gt(const int* a, const int* b, int* out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = a[i] > b[i];  /* GT_EXPR */
    }
}

__attribute__((noinline, noipa))
void compute_ge(const int* a, const int* b, int* out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = a[i] >= b[i]; /* GE_EXPR */
    }
}

__attribute__((noinline, noipa))
void compute_lt(const int* a, const int* b, int* out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = a[i] < b[i];  /* LT_EXPR - ensure operands aren't swapped */
    }
}

__attribute__((noinline, noipa))
void compute_le(const int* a, const int* b, int* out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = a[i] <= b[i]; /* LE_EXPR - ensure operands aren't swapped */
    }
}

/* Mixed comparison function with volatile control to keep all IR */
__attribute__((noinline, noipa))
void compute_mixed(const int* a, const int* b, int* out, int n, volatile int selector) {
    for (int i = 0; i < n; i++) {
        /* Force all comparison types to appear in IR */
        if (selector == 0) {
            out[i] = a[i] > b[i];   /* GT_EXPR */
        } else if (selector == 1) {
            out[i] = a[i] >= b[i];  /* GE_EXPR */
        } else if (selector == 2) {
            out[i] = a[i] < b[i];   /* LT_EXPR */
        } else {
            out[i] = a[i] <= b[i];  /* LE_EXPR */
        }
    }
}

/* Initialize arrays with pattern to ensure mixed comparison results */
void init_arrays(int* a, int* b, int n) {
    for (int i = 0; i < n; i++) {
        /* Create patterns that yield both true and false comparisons */
        a[i] = (i * 3) % 256;      /* Values 0-255 with pattern */
        b[i] = (i * 5) % 256;      /* Different pattern for varied comparisons */
    }
}

int main(void) {
    /* Declare arrays with enough elements for vectorization */
    int src1[N], src2[N];
    int gt_mask[N], ge_mask[N], lt_mask[N], le_mask[N];
    int mixed_mask[N];
    
    /* Initialize with deterministic pattern */
    init_arrays(src1, src2, N);
    
    /* Volatile selector to prevent constant propagation */
    volatile int selector = 3;
    
    /* Execute all comparison types */
    compute_gt(src1, src2, gt_mask, N);
    compute_ge(src1, src2, ge_mask, N);
    compute_lt(src1, src2, lt_mask, N);
    compute_le(src1, src2, le_mask, N);
    compute_mixed(src1, src2, mixed_mask, N, selector);
    
    /* Compute checksums to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += gt_mask[i] + ge_mask[i] + lt_mask[i] + le_mask[i] + mixed_mask[i];
    }
    
    /* Use results to prevent optimization */
    printf("Checksum: %llu\n", checksum);
    
    /* Additional test with different data types to increase coverage */
    {
        short src1_short[N], src2_short[N];
        short mask_short[N];
        
        for (int i = 0; i < N; i++) {
            src1_short[i] = (short)((i * 7) % 32768);
            src2_short[i] = (short)((i * 11) % 32768);
        }
        
        /* Test with short type - may trigger different vectorization paths */
        for (int i = 0; i < N; i++) {
            mask_short[i] = src1_short[i] > src2_short[i];  /* GT_EXPR with shorts */
        }
        
        /* Add to checksum */
        for (int i = 0; i < N; i++) {
            checksum += mask_short[i];
        }
    }
    
    /* Test with unsigned types */
    {
        unsigned int src1_uint[N], src2_uint[N];
        unsigned int mask_uint[N];
        
        for (int i = 0; i < N; i++) {
            src1_uint[i] = (unsigned int)((i * 13) % 1024);
            src2_uint[i] = (unsigned int)((i * 17) % 1024);
        }
        
        /* Test all comparison operators with unsigned */
        for (int i = 0; i < N; i++) {
            mask_uint[i] = src1_uint[i] > src2_uint[i];    /* GT_EXPR */
            mask_uint[i] += src1_uint[i] >= src2_uint[i];  /* GE_EXPR */
            mask_uint[i] += src1_uint[i] < src2_uint[i];   /* LT_EXPR */
            mask_uint[i] += src1_uint[i] <= src2_uint[i];  /* LE_EXPR */
        }
        
        for (int i = 0; i < N; i++) {
            checksum += mask_uint[i];
        }
    }
    
    printf("Final checksum: %llu\n", checksum);
    
    return 0;
}
