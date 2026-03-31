#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define SEED 42

/* Non-inlineable functions to prevent premature optimization */
__attribute__((noinline, noipa))
void compute_gt(const int* a, const int* b, int* mask, int n) {
    for (int i = 0; i < n; i++) {
        mask[i] = a[i] > b[i];  // GT_EXPR
    }
}

__attribute__((noinline, noipa))
void compute_ge(const int* a, const int* b, int* mask, int n) {
    for (int i = 0; i < n; i++) {
        mask[i] = a[i] >= b[i];  // GE_EXPR
    }
}

__attribute__((noinline, noipa))
void compute_lt(const int* a, const int* b, int* mask, int n) {
    for (int i = 0; i < n; i++) {
        mask[i] = a[i] < b[i];  // LT_EXPR (ensure operands aren't swapped)
    }
}

__attribute__((noinline, noipa))
void compute_le(const int* a, const int* b, int* mask, int n) {
    for (int i = 0; i < n; i++) {
        mask[i] = a[i] <= b[i];  // LE_EXPR
    }
}

/* Mixed comparison function to force all patterns in IR */
__attribute__((noinline, noipa))
void compute_mixed(const int* a, const int* b, int* mask, int n, volatile int selector) {
    for (int i = 0; i < n; i++) {
        switch (selector) {
            case 0: mask[i] = a[i] > b[i]; break;   // GT_EXPR
            case 1: mask[i] = a[i] >= b[i]; break;  // GE_EXPR
            case 2: mask[i] = a[i] < b[i]; break;   // LT_EXPR
            case 3: mask[i] = a[i] <= b[i]; break;  // LE_EXPR
            default: mask[i] = 0; break;
        }
    }
}

/* Initialize arrays with pattern to ensure mixed true/false results */
void init_arrays(int* a, int* b, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (i * 3) % 100;      // Pattern: 0, 3, 6, 9, ...
        b[i] = (i * 2) % 100;      // Pattern: 0, 2, 4, 6, ...
    }
}

/* Compute checksum to prevent dead code elimination */
int checksum(const int* arr, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += arr[i];
    }
    return sum;
}

int main() {
    /* Declare arrays with size multiple of typical vector widths */
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
    
    /* Force mixed comparisons with volatile selector */
    volatile int selector = 0;
    compute_mixed(src1, src2, mixed_mask, N, selector);
    
    /* Compute and print checksums to ensure execution */
    int total = 0;
    total += checksum(gt_mask, N);
    total += checksum(ge_mask, N);
    total += checksum(lt_mask, N);
    total += checksum(le_mask, N);
    total += checksum(mixed_mask, N);
    
    printf("Total checksum: %d\n", total);
    
    /* Additional test with different data types to increase coverage */
    {
        unsigned short usrc1[N], usrc2[N];
        unsigned short umask[N];
        
        for (int i = 0; i < N; i++) {
            usrc1[i] = (i * 5) % 256;
            usrc2[i] = (i * 7) % 256;
        }
        
        /* Test with unsigned comparisons */
        for (int i = 0; i < N; i++) {
            umask[i] = usrc1[i] > usrc2[i];  // Unsigned GT_EXPR
        }
        
        int usum = 0;
        for (int i = 0; i < N; i++) usum += umask[i];
        printf("Unsigned checksum: %d\n", usum);
    }
    
    return 0;
}
