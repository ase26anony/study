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
        out[i] = a[i] <= b[i];  /* LE_EXPR - ensure operands aren't swapped */
    }
}

/* Mixed comparison function using volatile selector */
__attribute__((noinline, noipa))
void compute_mixed(const int* a, const int* b, int* out, int n, volatile int selector) {
    for (int i = 0; i < n; i++) {
        /* Use selector to force all comparison types into IR */
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
        /* Create patterns that will yield both true and false comparisons */
        a[i] = (i * 3) % 256;      /* Values 0-255 with pattern */
        b[i] = (i * 5) % 256;      /* Different pattern for varied comparisons */
    }
}

int main(void) {
    /* Declare and initialize source arrays */
    int src1[N], src2[N];
    init_arrays(src1, src2, N);
    
    /* Destination arrays for comparison results */
    int gt_mask[N], ge_mask[N], lt_mask[N], le_mask[N];
    int mixed_mask[N];
    
    /* Execute all comparison types */
    compute_gt(src1, src2, gt_mask, N);
    compute_ge(src1, src2, ge_mask, N);
    compute_lt(src1, src2, lt_mask, N);
    compute_le(src1, src2, le_mask, N);
    
    /* Force all comparison types with volatile selector */
    volatile int selector = 0;
    compute_mixed(src1, src2, mixed_mask, N, selector);
    
    /* Compute checksums to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += gt_mask[i] + ge_mask[i] + lt_mask[i] + le_mask[i] + mixed_mask[i];
    }
    
    /* Additional variation: different data types */
    {
        short src1_short[N], src2_short[N];
        short mask_short[N];
        
        for (int i = 0; i < N; i++) {
            src1_short[i] = (short)((i * 7) % 32768);
            src2_short[i] = (short)((i * 11) % 32768);
        }
        
        /* Use different comparison operators on short type */
        for (int i = 0; i < N; i++) {
            mask_short[i] = src1_short[i] > src2_short[i];   /* GT_EXPR on short */
        }
        
        for (int i = 0; i < N; i++) {
            checksum += mask_short[i];
        }
    }
    
    /* Print checksum to ensure execution */
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
