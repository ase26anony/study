#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define ALIGN 32

// Aligned allocation for better vectorization
static void* aligned_alloc(size_t size) {
    void* ptr;
    if (posix_memalign(&ptr, ALIGN, size) != 0) {
        return NULL;
    }
    return ptr;
}

// GT_EXPR (>)
void test_gt_expr(int* restrict a, int* restrict b, int* restrict c, int* restrict d) {
    for (int i = 0; i < N; ++i) {
        // Conditional assignment using > comparison
        // This should trigger GT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR transformation
        c[i] = (a[i] > b[i]) ? (a[i] * 2) : (b[i] + 1);
        
        // Additional operation to prevent optimization
        d[i] = (a[i] > 0) ? c[i] : -c[i];
    }
}

// GE_EXPR (>=)
void test_ge_expr(float* restrict a, float* restrict b, float* restrict c, float* restrict d) {
    for (int i = 0; i < N; ++i) {
        // Conditional assignment using >= comparison
        // This should trigger GE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR transformation
        c[i] = (a[i] >= b[i]) ? (a[i] * 1.5f) : (b[i] * 0.5f);
        
        // Masked store pattern
        if (a[i] >= 0.0f) {
            d[i] = c[i] * 2.0f;
        } else {
            d[i] = c[i] * 0.5f;
        }
    }
}

// LT_EXPR (<)
void test_lt_expr(int* restrict a, int* restrict b, int* restrict c, int* restrict d) {
    for (int i = 0; i < N; ++i) {
        // Conditional assignment using < comparison
        // This should trigger LT_EXPR -> BIT_NOT_EXPR + BIT_AND_EXPR + swap transformation
        c[i] = (a[i] < b[i]) ? (a[i] - b[i]) : (b[i] - a[i]);
        
        // Reduction-like pattern
        d[i] = (a[i] < N/2) ? c[i] * 2 : c[i];
    }
}

// LE_EXPR (<=)
void test_le_expr(float* restrict a, float* restrict b, float* restrict c, float* restrict d) {
    for (int i = 0; i < N; ++i) {
        // Conditional assignment using <= comparison
        // This should trigger LE_EXPR -> BIT_NOT_EXPR + BIT_IOR_EXPR + swap transformation
        c[i] = (a[i] <= b[i]) ? (a[i] + b[i]) : (a[i] - b[i]);
        
        // Complex conditional with multiple uses
        if (a[i] <= 0.0f && b[i] <= 0.0f) {
            d[i] = c[i] * 3.0f;
        } else if (a[i] <= b[i]) {
            d[i] = c[i] * 2.0f;
        } else {
            d[i] = c[i];
        }
    }
}

// Mixed comparisons in one loop to potentially trigger multiple paths
void test_mixed_comparisons(int* restrict a, int* restrict b, int* restrict c, 
                           int* restrict d, int* restrict e) {
    for (int i = 0; i < N; ++i) {
        // Use all four comparison types in one loop
        int gt_mask = (a[i] > b[i]) ? 1 : 0;
        int ge_mask = (a[i] >= b[i]) ? 1 : 0;
        int lt_mask = (a[i] < b[i]) ? 1 : 0;
        int le_mask = (a[i] <= b[i]) ? 1 : 0;
        
        // Combine masks in ways that might trigger bit operations
        c[i] = gt_mask ? (a[i] * 2) : b[i];
        d[i] = ge_mask ? (a[i] + b[i]) : (a[i] - b[i]);
        e[i] = (lt_mask && le_mask) ? a[i] : b[i];  // BIT_AND of masks
    }
}

// Reduction with comparison - often vectorized with mask operations
int test_reduction_with_comparison(int* restrict a, int* restrict b) {
    int sum = 0;
    for (int i = 0; i < N; ++i) {
        // Conditional increment using > comparison
        sum += (a[i] > b[i]) ? a[i] : 0;
        
        // Additional conditional using < comparison
        sum -= (a[i] < b[i]) ? b[i] : 0;
    }
    return sum;
}

// Initialize arrays with pattern that creates mix of true/false comparisons
void init_arrays(int* a, int* b, float* fa, float* fb) {
    for (int i = 0; i < N; ++i) {
        a[i] = i - N/2;  // Range: -512 to 511
        b[i] = i % 100;  // Range: 0 to 99
        
        fa[i] = (float)(i - N/2) * 0.5f;
        fb[i] = (float)(i % 100) * 0.3f;
    }
}

// Verify results by comparing with sequential computation
int verify_results(int* a, int* b, int* c, float* fa, float* fb, float* fc) {
    int errors = 0;
    
    // Verify GT test
    for (int i = 0; i < N; ++i) {
        int expected = (a[i] > b[i]) ? (a[i] * 2) : (b[i] + 1);
        if (c[i] != expected) {
            errors++;
            if (errors < 5) {
                printf("GT mismatch at %d: got %d, expected %d\n", i, c[i], expected);
            }
        }
    }
    
    // Verify GE test
    for (int i = 0; i < N; ++i) {
        float expected = (fa[i] >= fb[i]) ? (fa[i] * 1.5f) : (fb[i] * 0.5f);
        if (fc[i] != expected) {
            errors++;
            if (errors < 5) {
                printf("GE mismatch at %d: got %f, expected %f\n", i, fc[i], expected);
            }
        }
    }
    
    return errors;
}

int main() {
    // Allocate aligned memory for better vectorization
    int* a = (int*)aligned_alloc(N * sizeof(int));
    int* b = (int*)aligned_alloc(N * sizeof(int));
    int* c = (int*)aligned_alloc(N * sizeof(int));
    int* d = (int*)aligned_alloc(N * sizeof(int));
    int* e = (int*)aligned_alloc(N * sizeof(int));
    int* f = (int*)aligned_alloc(N * sizeof(int));
    int* g = (int*)aligned_alloc(N * sizeof(int));
    
    float* fa = (float*)aligned_alloc(N * sizeof(float));
    float* fb = (float*)aligned_alloc(N * sizeof(float));
    float* fc = (float*)aligned_alloc(N * sizeof(float));
    float* fd = (float*)aligned_alloc(N * sizeof(float));
    
    if (!a || !b || !c || !d || !e || !f || !g || !fa || !fb || !fc || !fd) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize with patterns that create varied comparison results
    init_arrays(a, b, fa, fb);
    
    // Run all test functions
    test_gt_expr(a, b, c, d);
    test_ge_expr(fa, fb, fc, fd);
    test_lt_expr(a, b, e, f);
    test_le_expr(fa, fb, fc, fd);  // Reusing fc, fd
    test_mixed_comparisons(a, b, c, d, g);
    
    // Run reduction test
    int reduction_result = test_reduction_with_comparison(a, b);
    
    // Verify results
    int errors = verify_results(a, b, c, fa, fb, fc);
    
    // Compute checksum to ensure computations aren't optimized away
    int checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += c[i] + d[i] + e[i] + f[i] + g[i];
        checksum += (int)fc[i] + (int)fd[i];
    }
    checksum += reduction_result;
    
    printf("Test completed with %d errors\n", errors);
    printf("Final checksum: %d\n", checksum);
    
    // Free allocated memory
    free(a); free(b); free(c); free(d); free(e); free(f); free(g);
    free(fa); free(fb); free(fc); free(fd);
    
    return errors > 0 ? 1 : 0;
}
