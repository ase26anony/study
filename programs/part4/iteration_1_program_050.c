#include <stdio.h>
#include <stdlib.h>

#define N 1024
#define SEED 42

/* Prevent inlining to keep IR intact for pattern matching */
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
    /* Force original operand order */
    const int* x = a;
    const int* y = b;
    for (int i = 0; i < N; i++) {
        out[i] = x[i] <= y[i]; /* LE_EXPR */
    }
}

/* Mixed comparison function to ensure all patterns are present */
__attribute__((noinline, noipa))
void compute_mixed(const int* a, const int* b, int* out, volatile int selector) {
    for (int i = 0; i < N; i++) {
        /* Volatile selector prevents constant propagation */
        switch (selector & 3) {
            case 0: out[i] = a[i] > b[i]; break;   /* GT_EXPR */
            case 1: out[i] = a[i] >= b[i]; break;  /* GE_EXPR */
            case 2: out[i] = a[i] < b[i]; break;   /* LT_EXPR */
            case 3: out[i] = a[i] <= b[i]; break;  /* LE_EXPR */
        }
    }
}

/* Initialize arrays with pattern to ensure mixed comparison results */
void init_arrays(int* a, int* b) {
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;      /* 0, 3, 6, 9, ... */
        b[i] = (i * 2) % 100;      /* 0, 2, 4, 6, ... */
    }
}

int main() {
    /* Allocate aligned arrays for better vectorization */
    int* src1 __attribute__((aligned(64))) = (int*)malloc(N * sizeof(int));
    int* src2 __attribute__((aligned(64))) = (int*)malloc(N * sizeof(int));
    int* gt_mask __attribute__((aligned(64))) = (int*)malloc(N * sizeof(int));
    int* ge_mask __attribute__((aligned(64))) = (int*)malloc(N * sizeof(int));
    int* lt_mask __attribute__((aligned(64))) = (int*)malloc(N * sizeof(int));
    int* le_mask __attribute__((aligned(64))) = (int*)malloc(N * sizeof(int));
    int* mixed_mask __attribute__((aligned(64))) = (int*)malloc(N * sizeof(int));
    
    if (!src1 || !src2 || !gt_mask || !ge_mask || !lt_mask || !le_mask || !mixed_mask) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    init_arrays(src1, src2);
    
    /* Execute all comparison types */
    compute_gt(src1, src2, gt_mask);
    compute_ge(src1, src2, ge_mask);
    compute_lt(src1, src2, lt_mask);
    compute_le(src1, src2, le_mask);
    
    /* Force generation of all comparison patterns with volatile selector */
    volatile int selector = 0;
    compute_mixed(src1, src2, mixed_mask, selector);
    
    /* Compute checksum to prevent dead code elimination */
    unsigned long long checksum = 0;
    for (int i = 0; i < N; i++) {
        checksum += gt_mask[i] + ge_mask[i] + lt_mask[i] + le_mask[i] + mixed_mask[i];
    }
    
    printf("Checksum: %llu\n", checksum);
    
    /* Additional test with different data types to increase coverage */
    {
        short* src1_short __attribute__((aligned(64))) = (short*)malloc(N * sizeof(short));
        short* src2_short __attribute__((aligned(64))) = (short*)malloc(N * sizeof(short));
        short* out_short __attribute__((aligned(64))) = (short*)malloc(N * sizeof(short));
        
        if (src1_short && src2_short && out_short) {
            for (int i = 0; i < N; i++) {
                src1_short[i] = (short)(i % 256);
                src2_short[i] = (short)((i + 128) % 256);
            }
            
            /* Different integer type may trigger different code paths */
            for (int i = 0; i < N; i++) {
                out_short[i] = src1_short[i] > src2_short[i];  /* GT_EXPR with short */
            }
            
            /* Use result to prevent elimination */
            for (int i = 0; i < N; i++) {
                checksum += out_short[i];
            }
        }
        
        free(src1_short);
        free(src2_short);
        free(out_short);
    }
    
    printf("Final checksum: %llu\n", checksum);
    
    free(src1);
    free(src2);
    free(gt_mask);
    free(ge_mask);
    free(lt_mask);
    free(le_mask);
    free(mixed_mask);
    
    return 0;
}
