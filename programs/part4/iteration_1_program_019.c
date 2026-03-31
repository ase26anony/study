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

/* Simple PRNG for deterministic data */
static unsigned int prng_state = SEED;
static unsigned int prng() {
    prng_state = prng_state * 1103515245 + 12345;
    return prng_state;
}

int main() {
    /* Declare and initialize arrays with pattern data */
    int src1[N], src2[N];
    int gt_mask[N], ge_mask[N], lt_mask[N], le_mask[N];
    int mixed_mask[N];
    
    /* Initialize with patterned data to ensure mixed true/false results */
    for (int i = 0; i < N; i++) {
        src1[i] = (int)(prng() % 1000);      /* Random values 0-999 */
        src2[i] = (int)(prng() % 1000);      /* Random values 0-999 */
        
        /* Also create some ascending/descending patterns */
        if (i % 4 == 0) src1[i] = i;         /* Ascending */
        if (i % 5 == 0) src2[i] = N - i;     /* Descending */
    }
    
    /* Call all comparison functions to ensure all operators are processed */
    compute_gt(src1, src2, gt_mask, N);
    compute_ge(src1, src2, ge_mask, N);
    compute_lt(src1, src2, lt_mask, N);
    compute_le(src1, src2, le_mask, N);
    
    /* Use volatile selector to force all comparison types in IR */
    volatile int op_selector = 0;
    compute_mixed(src1, src2, mixed_mask, N, op_selector);
    
    /* Compute checksums to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += gt_mask[i] + ge_mask[i] + lt_mask[i] + le_mask[i] + mixed_mask[i];
    }
    
    /* Additional variation: nested loops with different comparison types */
    int temp[N];
    for (int outer = 0; outer < 4; outer++) {
        for (int i = 0; i < N; i++) {
            /* Different comparison in inner loop based on outer */
            switch (outer) {
                case 0: temp[i] = src1[i] > src2[i]; break;
                case 1: temp[i] = src1[i] >= src2[i]; break;
                case 2: temp[i] = src1[i] < src2[i]; break;
                case 3: temp[i] = src1[i] <= src2[i]; break;
            }
        }
        /* Use temp to prevent optimization */
        for (int i = 0; i < N; i++) {
            checksum += temp[i];
        }
    }
    
    printf("Final checksum: %llu\n", checksum);
    
    /* Additional test with different integer types */
    {
        short src1_short[N], src2_short[N];
        short mask_short[N];
        
        for (int i = 0; i < N; i++) {
            src1_short[i] = (short)(i % 256);
            src2_short[i] = (short)((i * 3) % 256);
        }
        
        /* Test with short type - may trigger different vectorization */
        for (int i = 0; i < N; i++) {
            mask_short[i] = src1_short[i] > src2_short[i];  /* GT_EXPR */
            checksum += mask_short[i];
        }
        
        for (int i = 0; i < N; i++) {
            mask_short[i] = src1_short[i] <= src2_short[i]; /* LE_EXPR */
            checksum += mask_short[i];
        }
    }
    
    printf("Final checksum with shorts: %llu\n", checksum);
    
    return 0;
}
