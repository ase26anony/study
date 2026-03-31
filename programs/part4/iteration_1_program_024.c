#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define SEED 42

/* Prevent inlining to keep IR patterns intact */
__attribute__((noinline, noipa))
void compute_gt(const int* a, const int* b, int* out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = a[i] > b[i];  // GT_EXPR
    }
}

__attribute__((noinline, noipa))
void compute_ge(const int* a, const int* b, int* out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = a[i] >= b[i];  // GE_EXPR
    }
}

__attribute__((noinline, noipa))
void compute_lt(const int* a, const int* b, int* out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = a[i] < b[i];  // LT_EXPR (ensure operands aren't swapped)
    }
}

__attribute__((noinline, noipa))
void compute_le(const int* a, const int* b, int* out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = a[i] <= b[i];  // LE_EXPR
    }
}

/* Mixed comparison function to force all patterns in IR */
__attribute__((noinline, noipa))
void compute_mixed(const int* a, const int* b, int* out, int n, volatile int selector) {
    for (int i = 0; i < n; i++) {
        /* Use volatile selector to prevent constant propagation */
        switch (selector & 3) {
            case 0: out[i] = a[i] > b[i]; break;   // GT_EXPR
            case 1: out[i] = a[i] >= b[i]; break;  // GE_EXPR
            case 2: out[i] = a[i] < b[i]; break;   // LT_EXPR
            case 3: out[i] = a[i] <= b[i]; break;  // LE_EXPR
        }
    }
}

/* Initialize arrays with pattern to ensure mixed comparison results */
void init_arrays(int* a, int* b, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (i * 3) % 100;      // Pattern: 0, 3, 6, 9, ...
        b[i] = (i * 2) % 100;      // Pattern: 0, 2, 4, 6, ...
    }
}

int main(void) {
    int src1[N], src2[N];
    int gt_mask[N], ge_mask[N], lt_mask[N], le_mask[N];
    int mixed_mask[N];
    
    /* Initialize with deterministic pattern */
    init_arrays(src1, src2, N);
    
    /* Force all comparison types to be processed */
    compute_gt(src1, src2, gt_mask, N);
    compute_ge(src1, src2, ge_mask, N);
    compute_lt(src1, src2, lt_mask, N);
    compute_le(src1, src2, le_mask, N);
    
    /* Use volatile to prevent constant propagation of selector */
    volatile int selector = 1;
    compute_mixed(src1, src2, mixed_mask, N, selector);
    
    /* Compute checksum to prevent dead code elimination */
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
            src1_short[i] = (short)((i * 5) % 256);
            src2_short[i] = (short)((i * 3) % 256);
        }
        
        /* Test with short type - different vectorization path */
        for (int i = 0; i < N; i++) {
            mask_short[i] = src1_short[i] > src2_short[i];  // GT_EXPR with shorts
        }
        
        /* Use result to prevent elimination */
        short short_sum = 0;
        for (int i = 0; i < N; i++) {
            short_sum += mask_short[i];
        }
        printf("Short checksum: %d\n", (int)short_sum);
    }
    
    /* Test with unsigned types */
    {
        unsigned int src1_uint[N], src2_uint[N];
        unsigned int mask_uint[N];
        
        for (int i = 0; i < N; i++) {
            src1_uint[i] = (unsigned int)(i * 7);
            src2_uint[i] = (unsigned int)(i * 4);
        }
        
        /* Mix of comparison operators with unsigned */
        for (int i = 0; i < N; i++) {
            mask_uint[i] = (src1_uint[i] >= src2_uint[i]) ? 1 : 0;  // GE_EXPR
        }
        
        unsigned int uint_sum = 0;
        for (int i = 0; i < N; i++) {
            uint_sum += mask_uint[i];
        }
        printf("Unsigned checksum: %u\n", uint_sum);
    }
    
    return 0;
}
