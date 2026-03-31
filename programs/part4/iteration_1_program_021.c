/* Program to trigger GCC's vectorization pattern matching for comparison operations */
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
        out[i] = a[i] >= b[i]; /* GE_EXPR */
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
        out[i] = a[i] <= b[i]; /* LE_EXPR - ensure operands aren't swapped */
    }
}

/* Mixed comparison function with volatile control to keep all patterns in IR */
__attribute__((noinline, noipa))
void compute_mixed(const int* a, const int* b, int* out, int n, volatile int selector) {
    for (int i = 0; i < n; i++) {
        /* Use volatile selector to prevent constant propagation */
        switch (selector & 3) {
            case 0: out[i] = a[i] > b[i]; break;   /* GT_EXPR */
            case 1: out[i] = a[i] >= b[i]; break;  /* GE_EXPR */
            case 2: out[i] = a[i] < b[i]; break;   /* LT_EXPR */
            case 3: out[i] = a[i] <= b[i]; break;  /* LE_EXPR */
        }
    }
}

/* Initialize arrays with pattern to ensure mixed comparison results */
void init_arrays(int* a, int* b, int n) {
    for (int i = 0; i < n; i++) {
        /* Create patterns that will produce both true and false comparisons */
        a[i] = (i * 3) % 256;      /* Values 0-255 */
        b[i] = (i * 5) % 256;      /* Different pattern for varied comparisons */
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
    /* Declare arrays with alignment hint for better vectorization */
    int src1[N] __attribute__((aligned(32)));
    int src2[N] __attribute__((aligned(32)));
    int gt_mask[N] __attribute__((aligned(32)));
    int ge_mask[N] __attribute__((aligned(32)));
    int lt_mask[N] __attribute__((aligned(32)));
    int le_mask[N] __attribute__((aligned(32)));
    int mixed_mask[N] __attribute__((aligned(32)));
    
    /* Initialize with deterministic pattern */
    init_arrays(src1, src2, N);
    
    /* Volatile selector to force all comparison types in IR */
    volatile int selector = 0;
    
    /* Perform all comparison types */
    compute_gt(src1, src2, gt_mask, N);
    compute_ge(src1, src2, ge_mask, N);
    compute_lt(src1, src2, lt_mask, N);
    compute_le(src1, src2, le_mask, N);
    
    /* Also call mixed function to ensure all patterns are present */
    compute_mixed(src1, src2, mixed_mask, N, selector);
    
    /* Compute and print checksum to prevent optimization */
    int checksum = compute_checksum(gt_mask, ge_mask, lt_mask, le_mask, N);
    
    /* Add mixed_mask to checksum */
    for (int i = 0; i < N; i++) {
        checksum += mixed_mask[i];
    }
    
    printf("Checksum: %d\n", checksum);
    
    /* Additional test with different data types to increase coverage */
    {
        unsigned short usrc1[N], usrc2[N];
        unsigned short umask[N];
        
        for (int i = 0; i < N; i++) {
            usrc1[i] = (i * 7) % 65535;
            usrc2[i] = (i * 11) % 65535;
        }
        
        /* Test with unsigned short comparisons */
        for (int i = 0; i < N; i++) {
            umask[i] = usrc1[i] > usrc2[i];  /* GT_EXPR with unsigned */
        }
        
        /* Add to checksum */
        unsigned short usum = 0;
        for (int i = 0; i < N; i++) {
            usum += umask[i];
        }
        printf("Unsigned checksum: %u\n", (unsigned)usum);
    }
    
    return 0;
}
