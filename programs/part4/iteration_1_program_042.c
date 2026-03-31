#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define ALIGNED __attribute__((aligned(32)))

/* Prevent function inlining to ensure vectorization analysis happens */
__attribute__((noinline, noipa))
void compute_gt(const int* a, const int* b, int* out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = a[i] > b[i];  /* GT_EXPR */
    }
}

__attribute__((noinline, noipa))
void compute_ge(const int* a, const int* b, int* out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = a[i] >= b[i];  /* GE_EXPR */
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
        out[i] = a[i] <= b[i];  /* LE_EXPR */
    }
}

/* Mixed comparison function to force all patterns in one compilation unit */
__attribute__((noinline, noipa))
void compute_mixed(const int* a, const int* b, int* out, int n, volatile int selector) {
    for (int i = 0; i < n; i++) {
        /* Use volatile selector to prevent constant propagation */
        switch (selector & 3) {
            case 0: out[i] = a[i] > b[i]; break;   /* GT_EXPR */
            case 1: out[i] = a[i] >= b[i]; break;  /* GE_EXPR */
            case 2: out[i] = a[i] < b[i]; break;   /* LT_EXPR */
            case 3: out[i] = a[i] <= b[i]; break;  /* LE_EXPR */
        }
    }
}

/* Initialize arrays with pattern to ensure mixed comparison results */
void init_arrays(int* a, int* b, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (i * 3) % 100;      /* Values 0-99 in pattern */
        b[i] = (i * 7) % 100;      /* Different pattern for varied comparisons */
    }
}

int main(void) {
    /* Aligned arrays for better vectorization */
    ALIGNED int src1[N];
    ALIGNED int src2[N];
    ALIGNED int gt_mask[N];
    ALIGNED int ge_mask[N];
    ALIGNED int lt_mask[N];
    ALIGNED int le_mask[N];
    ALIGNED int mixed_mask[N];
    
    /* Initialize with deterministic pattern */
    init_arrays(src1, src2, N);
    
    /* Force all comparison types to be compiled */
    compute_gt(src1, src2, gt_mask, N);
    compute_ge(src1, src2, ge_mask, N);
    compute_lt(src1, src2, lt_mask, N);
    compute_le(src1, src2, le_mask, N);
    
    /* Use volatile to prevent compile-time optimization of selector */
    volatile int selector = 0;
    compute_mixed(src1, src2, mixed_mask, N, selector);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += gt_mask[i] + ge_mask[i] + lt_mask[i] + le_mask[i] + mixed_mask[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    /* Additional test with different data types to increase coverage */
    {
        ALIGNED short src1_short[N];
        ALIGNED short src2_short[N];
        ALIGNED short mask_short[N];
        
        for (int i = 0; i < N; i++) {
            src1_short[i] = (short)(i % 100);
            src2_short[i] = (short)((i * 5) % 100);
        }
        
        /* Use different comparison on short type */
        for (int i = 0; i < N; i++) {
            mask_short[i] = src1_short[i] > src2_short[i];  /* GT_EXPR on shorts */
        }
        
        /* Add to checksum */
        for (int i = 0; i < N; i++) {
            checksum += mask_short[i];
        }
    }
    
    printf("Final checksum: %llu\n", checksum);
    
    return 0;
}
