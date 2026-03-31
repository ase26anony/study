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
        mask[i] = a[i] <= b[i];  /* LE_EXPR */
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

/* Initialize arrays with pattern that ensures mixed comparison results */
void init_arrays(int* a, int* b, int n) {
    for (int i = 0; i < n; i++) {
        /* Create patterns where comparisons will be sometimes true, sometimes false */
        a[i] = (i * 3) % 256;          /* Values 0-255 with period */
        b[i] = (i * 5 + 128) % 256;    /* Different pattern for varied comparisons */
    }
}

int main(void) {
    /* Declare and initialize source arrays */
    int src1[N], src2[N];
    init_arrays(src1, src2, N);
    
    /* Destination arrays for each comparison type */
    int gt_mask[N], ge_mask[N], lt_mask[N], le_mask[N], mixed_mask[N];
    
    /* Force all comparison types to be processed */
    compute_gt(src1, src2, gt_mask, N);
    compute_ge(src1, src2, ge_mask, N);
    compute_lt(src1, src2, lt_mask, N);
    compute_le(src1, src2, le_mask, N);
    
    /* Use volatile to prevent constant propagation of the selector */
    volatile int op_selector = 2;  /* Will use LT_EXPR, but compiler must handle all cases */
    compute_mixed(src1, src2, mixed_mask, N, op_selector);
    
    /* Compute checksums to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += gt_mask[i] + ge_mask[i] + lt_mask[i] + le_mask[i] + mixed_mask[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    /* Additional test with different data types to increase coverage */
    {
        unsigned short us_src1[N], us_src2[N];
        unsigned short us_mask[N];
        
        for (int i = 0; i < N; i++) {
            us_src1[i] = (i * 7) % 65535;
            us_src2[i] = (i * 11 + 32768) % 65535;
        }
        
        /* Test with unsigned short to trigger different vectorization paths */
        for (int i = 0; i < N; i++) {
            us_mask[i] = us_src1[i] > us_src2[i];  /* GT_EXPR with unsigned */
        }
        
        /* Add to checksum */
        for (int i = 0; i < N; i++) {
            checksum += us_mask[i];
        }
    }
    
    printf("Final checksum: %llu\n", checksum);
    
    return 0;
}
