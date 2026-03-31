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
        mask[i] = a[i] >= b[i];  /* GE_EXPR */
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
        mask[i] = a[i] <= b[i];  /* LE_EXPR - ensure operands aren't swapped */
    }
}

/* Function with runtime-selected comparison to keep all IR alive */
__attribute__((noinline, noipa))
void compute_mixed(const int* a, const int* b, int* mask, int n, volatile int op) {
    for (int i = 0; i < n; i++) {
        switch (op) {
            case 0: mask[i] = a[i] > b[i]; break;   /* GT_EXPR */
            case 1: mask[i] = a[i] >= b[i]; break;  /* GE_EXPR */
            case 2: mask[i] = a[i] < b[i]; break;   /* LT_EXPR */
            case 3: mask[i] = a[i] <= b[i]; break;  /* LE_EXPR */
            default: mask[i] = 0; break;
        }
    }
}

/* Simple PRNG for deterministic but varied data */
static unsigned int prng_state = SEED;
static unsigned int simple_rand() {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

int main() {
    /* Declare and initialize arrays with pattern that ensures mixed comparison results */
    int src1[N], src2[N];
    int gt_mask[N], ge_mask[N], lt_mask[N], le_mask[N];
    int mixed_mask[N];
    
    /* Initialize with alternating patterns to get mix of true/false comparisons */
    for (int i = 0; i < N; i++) {
        src1[i] = i;                     /* 0, 1, 2, 3, ... */
        src2[i] = (i % 3) * 10;          /* 0, 10, 20, 0, 10, 20, ... */
    }
    
    /* Also add some random variations */
    for (int i = 0; i < N/4; i++) {
        int idx = simple_rand() % N;
        src1[idx] = simple_rand() % 100;
        src2[idx] = simple_rand() % 100;
    }
    
    /* Call all comparison functions to ensure all patterns are processed */
    compute_gt(src1, src2, gt_mask, N);
    compute_ge(src1, src2, ge_mask, N);
    compute_lt(src1, src2, lt_mask, N);
    compute_le(src1, src2, le_mask, N);
    
    /* Use volatile to prevent constant propagation of the selector */
    volatile int op_selector = 2;  /* Will use LT_EXPR, but compiler must prepare for all */
    compute_mixed(src1, src2, mixed_mask, N, op_selector);
    
    /* Compute checksums to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += gt_mask[i] + ge_mask[i] + lt_mask[i] + le_mask[i] + mixed_mask[i];
    }
    
    /* Additional variation: different data types to trigger different vectorization paths */
    {
        short src1_short[N], src2_short[N];
        short mask_short[N];
        
        for (int i = 0; i < N; i++) {
            src1_short[i] = (short)(i * 3);
            src2_short[i] = (short)(i * 2);
        }
        
        /* This loop should also vectorize with short comparisons */
        for (int i = 0; i < N; i++) {
            mask_short[i] = src1_short[i] > src2_short[i];  /* GT_EXPR with shorts */
        }
        
        for (int i = 0; i < N; i++) {
            checksum += mask_short[i];
        }
    }
    
    /* More complex pattern: comparisons with constants */
    {
        int const_mask[N];
        for (int i = 0; i < N; i++) {
            /* Mix different comparison operators with constants */
            if (i % 4 == 0) const_mask[i] = src1[i] > 50;     /* GT_EXPR with constant */
            else if (i % 4 == 1) const_mask[i] = src1[i] >= 25; /* GE_EXPR with constant */
            else if (i % 4 == 2) const_mask[i] = src1[i] < 75;  /* LT_EXPR with constant */
            else const_mask[i] = src1[i] <= 100;                /* LE_EXPR with constant */
        }
        
        for (int i = 0; i < N; i++) {
            checksum += const_mask[i];
        }
    }
    
    printf("Final checksum: %llu\n", checksum);
    return 0;
}
