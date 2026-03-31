#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define SEED 42

/* Non-inlineable functions to prevent premature optimization */
__attribute__((noinline, noipa))
void compute_gt(const int* a, const int* b, int* out) {
    for (int i = 0; i < N; i++) {
        out[i] = a[i] > b[i];  /* GT_EXPR */
    }
}

__attribute__((noinline, noipa))
void compute_ge(const int* a, const int* b, int* out) {
    for (int i = 0; i < N; i++) {
        out[i] = a[i] >= b[i]; /* GE_EXPR */
    }
}

__attribute__((noinline, noipa))
void compute_lt(const int* a, const int* b, int* out) {
    /* Use different variable names to prevent canonicalization to GT */
    const int* left = a;
    const int* right = b;
    for (int i = 0; i < N; i++) {
        out[i] = left[i] < right[i]; /* LT_EXPR - ensure operands aren't swapped */
    }
}

__attribute__((noinline, noipa))
void compute_le(const int* a, const int* b, int* out) {
    /* Ensure LE_EXPR isn't transformed to GE_EXPR by keeping original order */
    for (int i = 0; i < N; i++) {
        out[i] = a[i] <= b[i]; /* LE_EXPR */
    }
}

/* Mixed comparison function with volatile control to keep all patterns */
__attribute__((noinline, noipa))
void compute_mixed(const int* a, const int* b, int* out, volatile int selector) {
    for (int i = 0; i < N; i++) {
        switch (selector & 3) {
            case 0: out[i] = a[i] > b[i]; break;   /* GT_EXPR */
            case 1: out[i] = a[i] >= b[i]; break;  /* GE_EXPR */
            case 2: out[i] = a[i] < b[i]; break;   /* LT_EXPR */
            case 3: out[i] = a[i] <= b[i]; break;  /* LE_EXPR */
        }
    }
}

/* Initialize arrays with patterned data for varied comparison results */
void init_arrays(int* a, int* b) {
    unsigned int state = SEED;
    for (int i = 0; i < N; i++) {
        /* Simple PRNG for varied values */
        state = state * 1103515245 + 12345;
        a[i] = (state >> 16) & 0x7FFF;
        
        state = state * 1103515245 + 12345;
        b[i] = (state >> 16) & 0x7FFF;
        
        /* Create some patterns to ensure mixed comparison results */
        if (i % 4 == 0) a[i] = b[i] + 1;      /* a > b */
        if (i % 4 == 1) a[i] = b[i] - 1;      /* a < b */
        if (i % 4 == 2) a[i] = b[i];          /* a == b */
        /* i % 4 == 3: keep random values */
    }
}

/* Compute checksum to prevent dead code elimination */
int compute_checksum(int* gt, int* ge, int* lt, int* le, int* mixed) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum = (sum * 31 + gt[i]) & 0xFFFF;
        sum = (sum * 31 + ge[i]) & 0xFFFF;
        sum = (sum * 31 + lt[i]) & 0xFFFF;
        sum = (sum * 31 + le[i]) & 0xFFFF;
        sum = (sum * 31 + mixed[i]) & 0xFFFF;
    }
    return sum;
}

int main() {
    /* Align arrays for better vectorization */
    __attribute__((aligned(32))) int src1[N];
    __attribute__((aligned(32))) int src2[N];
    __attribute__((aligned(32))) int gt_mask[N];
    __attribute__((aligned(32))) int ge_mask[N];
    __attribute__((aligned(32))) int lt_mask[N];
    __attribute__((aligned(32))) int le_mask[N];
    __attribute__((aligned(32))) int mixed_mask[N];
    
    /* Volatile selector to prevent constant propagation */
    volatile int selector = 2;
    
    /* Initialize with patterned data */
    init_arrays(src1, src2);
    
    /* Execute all comparison types */
    compute_gt(src1, src2, gt_mask);
    compute_ge(src1, src2, ge_mask);
    compute_lt(src1, src2, lt_mask);
    compute_le(src1, src2, le_mask);
    compute_mixed(src1, src2, mixed_mask, selector);
    
    /* Compute and print checksum to ensure execution */
    int checksum = compute_checksum(gt_mask, ge_mask, lt_mask, le_mask, mixed_mask);
    printf("Checksum: %d\n", checksum);
    
    /* Additional test with different data types to increase coverage */
    {
        __attribute__((aligned(32))) short src1_short[N];
        __attribute__((aligned(32))) short src2_short[N];
        __attribute__((aligned(32))) short mask_short[N];
        
        for (int i = 0; i < N; i++) {
            src1_short[i] = src1[i] & 0x7FFF;
            src2_short[i] = src2[i] & 0x7FFF;
        }
        
        /* Use different comparison on short type */
        for (int i = 0; i < N; i++) {
            mask_short[i] = src1_short[i] > src2_short[i];  /* GT_EXPR on shorts */
        }
        
        /* Use result to prevent elimination */
        short short_sum = 0;
        for (int i = 0; i < N; i++) {
            short_sum += mask_short[i];
        }
        printf("Short sum: %d\n", short_sum);
    }
    
    return 0;
}
