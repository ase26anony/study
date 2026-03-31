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
        out[i] = a[i] < b[i];  // LT_EXPR (ensure it's not canonicalized to >)
    }
}

__attribute__((noinline, noipa))
void compute_le(const int* a, const int* b, int* out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = a[i] <= b[i];  // LE_EXPR
    }
}

/* Mixed comparison function with volatile control flow */
__attribute__((noinline, noipa))
void compute_mixed(const int* a, const int* b, int* out, int n, volatile int selector) {
    for (int i = 0; i < n; i++) {
        switch (selector) {
            case 0: out[i] = a[i] > b[i]; break;   // GT_EXPR
            case 1: out[i] = a[i] >= b[i]; break;  // GE_EXPR
            case 2: out[i] = a[i] < b[i]; break;   // LT_EXPR
            case 3: out[i] = a[i] <= b[i]; break;  // LE_EXPR
            default: out[i] = 0; break;
        }
    }
}

/* Initialize arrays with patterned data */
void init_arrays(int* a, int* b, int n) {
    for (int i = 0; i < n; i++) {
        // Create patterns that yield mixed comparison results
        a[i] = (i * 3) % 256;          // 0, 3, 6, 9, ...
        b[i] = (i * 5) % 256;          // 0, 5, 10, 15, ...
    }
}

/* Compute checksum to prevent dead code elimination */
int compute_checksum(int* gt, int* ge, int* lt, int* le, int* mixed, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += gt[i] + ge[i] + lt[i] + le[i] + mixed[i];
    }
    return sum;
}

int main() {
    /* Declare arrays with size multiple of typical vector widths */
    int src1[N], src2[N];
    int gt_mask[N], ge_mask[N], lt_mask[N], le_mask[N], mixed_mask[N];
    
    /* Initialize with patterned data */
    init_arrays(src1, src2, N);
    
    /* Call all comparison functions */
    compute_gt(src1, src2, gt_mask, N);
    compute_ge(src1, src2, ge_mask, N);
    compute_lt(src1, src2, lt_mask, N);
    compute_le(src1, src2, le_mask, N);
    
    /* Use volatile selector to force all comparison types in IR */
    volatile int selector = 2;  // Will use LT_EXPR, but compiler must handle all cases
    compute_mixed(src1, src2, mixed_mask, N, selector);
    
    /* Compute and print checksum to ensure execution */
    int checksum = compute_checksum(gt_mask, ge_mask, lt_mask, le_mask, mixed_mask, N);
    printf("Checksum: %d\n", checksum);
    
    /* Additional test with different data types to increase coverage */
    {
        unsigned short usrc1[N], usrc2[N];
        unsigned short uout[N];
        
        for (int i = 0; i < N; i++) {
            usrc1[i] = (i * 7) % 65535;
            usrc2[i] = (i * 11) % 65535;
        }
        
        /* Different comparison with unsigned types */
        for (int i = 0; i < N; i++) {
            uout[i] = usrc1[i] > usrc2[i];  // GT_EXPR with unsigned
        }
        
        /* Use result to prevent elimination */
        unsigned short usum = 0;
        for (int i = 0; i < N; i++) {
            usum += uout[i];
        }
        printf("Unsigned checksum: %u\n", usum);
    }
    
    return 0;
}
