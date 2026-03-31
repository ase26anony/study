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

/* Mixed comparison function using volatile selector */
__attribute__((noinline, noipa))
void compute_mixed(const int* a, const int* b, int* mask, int n, volatile int selector) {
    for (int i = 0; i < n; i++) {
        /* Force all comparison types to appear in IR */
        switch (selector & 3) {
            case 0: mask[i] = a[i] > b[i]; break;   /* GT_EXPR */
            case 1: mask[i] = a[i] >= b[i]; break;  /* GE_EXPR */
            case 2: mask[i] = a[i] < b[i]; break;   /* LT_EXPR */
            case 3: mask[i] = a[i] <= b[i]; break;  /* LE_EXPR */
        }
    }
}

/* Initialize arrays with pattern to ensure mixed true/false results */
void init_arrays(int* a, int* b, int n) {
    for (int i = 0; i < n; i++) {
        /* Create patterns where comparisons yield both true and false */
        a[i] = (i * 3) % 256;          /* Values 0-255 with pattern */
        b[i] = (i * 5 + 128) % 256;    /* Different pattern offset */
    }
}

/* Compute checksum to prevent dead code elimination */
int compute_checksum(int* gt_mask, int* ge_mask, int* lt_mask, int* le_mask, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += gt_mask[i] + ge_mask[i] + lt_mask[i] + le_mask[i];
    }
    return sum;
}

int main() {
    /* Declare and initialize arrays */
    int src1[N], src2[N];
    int gt_mask[N], ge_mask[N], lt_mask[N], le_mask[N];
    int mixed_mask[N];
    
    init_arrays(src1, src2, N);
    
    /* Execute all comparison types */
    compute_gt(src1, src2, gt_mask, N);
    compute_ge(src1, src2, ge_mask, N);
    compute_lt(src1, src2, lt_mask, N);
    compute_le(src1, src2, le_mask, N);
    
    /* Force all comparison patterns with volatile selector */
    volatile int selector = 0;
    compute_mixed(src1, src2, mixed_mask, N, selector);
    
    /* Compute and print checksum to ensure execution */
    int checksum = compute_checksum(gt_mask, ge_mask, lt_mask, le_mask, N);
    printf("Checksum: %d\n", checksum);
    
    /* Additional verification to prevent optimization */
    int mixed_sum = 0;
    for (int i = 0; i < N; i++) {
        mixed_sum += mixed_mask[i];
    }
    printf("Mixed sum: %d\n", mixed_sum);
    
    return 0;
}
