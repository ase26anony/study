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

/* Mixed comparison function with volatile control flow */
__attribute__((noinline, noipa))
void compute_mixed(const int* a, const int* b, int* mask, int n, volatile int selector) {
    for (int i = 0; i < n; i++) {
        /* Force all comparison types to appear in IR */
        if (selector == 0) {
            mask[i] = a[i] > b[i];   /* GT_EXPR */
        } else if (selector == 1) {
            mask[i] = a[i] >= b[i];  /* GE_EXPR */
        } else if (selector == 2) {
            mask[i] = a[i] < b[i];   /* LT_EXPR */
        } else {
            mask[i] = a[i] <= b[i];  /* LE_EXPR */
        }
    }
}

/* Initialize arrays with pattern to ensure mixed comparison results */
void init_arrays(int* a, int* b, int n) {
    for (int i = 0; i < n; i++) {
        /* Create patterns where comparisons yield both true and false */
        a[i] = (i * 3) % 256;      /* Values 0-255 with pattern */
        b[i] = (i * 5) % 256;      /* Different pattern for varied comparisons */
    }
}

int main(void) {
    /* Declare arrays with sufficient size for vectorization */
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
    
    /* Use volatile to prevent constant propagation */
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
        unsigned short usrc1[N], usrc2[N];
        unsigned short umask[N];
        
        for (int i = 0; i < N; i++) {
            usrc1[i] = (i * 7) % 65535;
            usrc2[i] = (i * 11) % 65535;
        }
        
        /* Test with unsigned comparisons */
        for (int i = 0; i < N; i++) {
            umask[i] = usrc1[i] > usrc2[i];  /* Unsigned GT_EXPR */
        }
        
        /* Add to checksum */
        for (int i = 0; i < N; i++) {
            checksum += umask[i];
        }
    }
    
    printf("Final checksum: %llu\n", checksum);
    
    return 0;
}
