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
        out[i] = a[i] < b[i];  // LT_EXPR - ensure operands aren't swapped
    }
}

__attribute__((noinline, noipa))
void compute_le(const int* a, const int* b, int* out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = a[i] <= b[i];  // LE_EXPR - ensure operands aren't swapped
    }
}

/* Mixed comparison function with volatile control to keep all IR alive */
__attribute__((noinline, noipa))
void compute_mixed(const int* a, const int* b, int* out, int n, volatile int selector) {
    for (int i = 0; i < n; i++) {
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
        // Create patterns where comparisons will be sometimes true, sometimes false
        a[i] = (i * 3) % 256;          // Values 0-255
        b[i] = (i * 5 + 128) % 256;    // Different pattern
    }
}

int main(void) {
    int src1[N], src2[N];
    int gt_mask[N], ge_mask[N], lt_mask[N], le_mask[N];
    int mixed_mask[N];
    
    /* Initialize with deterministic pattern */
    init_arrays(src1, src2, N);
    
    /* Force all comparison types to be compiled */
    compute_gt(src1, src2, gt_mask, N);
    compute_ge(src1, src2, ge_mask, N);
    compute_lt(src1, src2, lt_mask, N);
    compute_le(src1, src2, le_mask, N);
    
    /* Use volatile to prevent constant propagation in mixed function */
    volatile int selector = 0;
    compute_mixed(src1, src2, mixed_mask, N, selector);
    
    /* Compute checksums to prevent dead code elimination */
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
            usrc2[i] = (i * 11 + 32768) % 65535;
        }
        
        /* Test unsigned comparisons - may trigger different patterns */
        for (int i = 0; i < N; i++) {
            umask[i] = usrc1[i] > usrc2[i];  // Unsigned GT_EXPR
        }
        
        unsigned short usum = 0;
        for (int i = 0; i < N; i++) {
            usum += umask[i];
        }
        printf("Unsigned checksum: %u\n", usum);
    }
    
    /* Test with immediate values (scalar comparisons) */
    {
        int imm_mask[N];
        for (int i = 0; i < N; i++) {
            imm_mask[i] = src1[i] > 100;  // Compare with immediate
        }
        
        int imm_sum = 0;
        for (int i = 0; i < N; i++) {
            imm_sum += imm_mask[i];
        }
        printf("Immediate comparison checksum: %d\n", imm_sum);
    }
    
    return 0;
}
