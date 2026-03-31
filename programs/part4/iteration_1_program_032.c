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

/* Mixed comparison function with volatile control to keep all IR */
__attribute__((noinline, noipa))
void compute_mixed(const int* a, const int* b, int* mask, int n, volatile int selector) {
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
    
    /* Initialize with alternating patterns to ensure mix of true/false comparisons */
    for (int i = 0; i < N; i++) {
        src1[i] = (int)(prng() % 1000);
        src2[i] = (int)(prng() % 1000);
    }
    
    /* Force all comparison types to be processed */
    compute_gt(src1, src2, gt_mask, N);
    compute_ge(src1, src2, ge_mask, N);
    compute_lt(src1, src2, lt_mask, N);
    compute_le(src1, src2, le_mask, N);
    
    /* Use volatile selector to ensure all comparison IR remains */
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
        unsigned short usrc1[N], usrc2[N];
        unsigned short umask[N];
        
        for (int i = 0; i < N; i++) {
            usrc1[i] = (unsigned short)(prng() % 65536);
            usrc2[i] = (unsigned short)(prng() % 65536);
        }
        
        /* Test unsigned comparisons */
        for (int i = 0; i < N; i++) {
            umask[i] = usrc1[i] > usrc2[i];   /* GT_EXPR with unsigned */
        }
        
        /* Add to checksum */
        for (int i = 0; i < N; i++) {
            checksum += umask[i];
        }
    }
    
    printf("Final checksum: %llu\n", checksum);
    
    return 0;
}
