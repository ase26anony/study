#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define SEED 42

/* Non-inlineable functions to prevent premature optimization */
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
        out[i] = a[i] < b[i];  // LT_EXPR - ensure this stays as LT
    }
}

__attribute__((noinline, noipa))
void compute_le(const int* a, const int* b, int* out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = a[i] <= b[i];  // LE_EXPR - ensure this stays as LE
    }
}

/* Mixed comparison function with volatile selector */
__attribute__((noinline, noipa))
void compute_mixed(const int* a, const int* b, int* out, int n, volatile int selector) {
    for (int i = 0; i < n; i++) {
        /* Use volatile selector to prevent constant propagation */
        if (selector == 0) {
            out[i] = a[i] > b[i];   // GT_EXPR
        } else if (selector == 1) {
            out[i] = a[i] >= b[i];  // GE_EXPR
        } else if (selector == 2) {
            out[i] = a[i] < b[i];   // LT_EXPR
        } else {
            out[i] = a[i] <= b[i];  // LE_EXPR
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

/* Compute checksum to prevent dead code elimination */
int compute_checksum(int* gt_mask, int* ge_mask, int* lt_mask, int* le_mask, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += gt_mask[i] + ge_mask[i] + lt_mask[i] + le_mask[i];
    }
    return sum;
}

int main() {
    /* Declare arrays with enough elements for vectorization */
    int src1[N], src2[N];
    int gt_mask[N], ge_mask[N], lt_mask[N], le_mask[N];
    int mixed_mask[N];
    
    /* Initialize with deterministic pattern */
    init_arrays(src1, src2, N);
    
    /* Execute all comparison types */
    compute_gt(src1, src2, gt_mask, N);
    compute_ge(src1, src2, ge_mask, N);
    compute_lt(src1, src2, lt_mask, N);
    compute_le(src1, src2, le_mask, N);
    
    /* Execute mixed comparisons with volatile selector */
    volatile int selector = 0;
    compute_mixed(src1, src2, mixed_mask, N, selector);
    
    /* Additional test with different selector values */
    selector = 2;
    compute_mixed(src1, src2, mixed_mask, N, selector);
    
    /* Compute and print checksum to ensure execution */
    int checksum = compute_checksum(gt_mask, ge_mask, lt_mask, le_mask, N);
    printf("Checksum: %d\n", checksum);
    
    /* Additional test with different data types */
    {
        unsigned short usrc1[N], usrc2[N];
        unsigned short umask[N];
        
        for (int i = 0; i < N; i++) {
            usrc1[i] = (i * 5) % 256;
            usrc2[i] = (i * 3) % 256;
        }
        
        /* Test unsigned comparisons which might also trigger the pattern */
        for (int i = 0; i < N; i++) {
            umask[i] = usrc1[i] > usrc2[i];  // GT_EXPR with unsigned
        }
        
        /* Use the result */
        unsigned short usum = 0;
        for (int i = 0; i < N; i++) {
            usum += umask[i];
        }
        printf("Unsigned checksum: %u\n", usum);
    }
    
    return 0;
}
