#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define ALIGN __attribute__((aligned(32)))

// Test functions for different comparison operators
// Each function tests a specific comparison operator in a vectorizable loop

// GT_EXPR (>)
void test_gt(int *restrict a, int *restrict b, int *restrict mask, int n) {
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] > b[i];
    }
}

// GE_EXPR (>=) with conditional blend
void test_ge(float *restrict a, float *restrict b, float *restrict x, 
             float *restrict y, float *restrict dst, int n) {
    for (int i = 0; i < n; ++i) {
        dst[i] = (a[i] >= b[i]) ? x[i] : y[i];
    }
}

// LT_EXPR (<) with conditional accumulation
int test_lt(unsigned int *restrict a, unsigned int *restrict b, int n) {
    int sum = 0;
    for (int i = 0; i < n; ++i) {
        if (a[i] < b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

// LE_EXPR (<=) with mask usage
void test_le(double *restrict a, double *restrict b, int *restrict mask, 
             double *restrict result, int n) {
    // First create the mask
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] <= b[i];
    }
    
    // Then use the mask in a way that might trigger bitwise optimization
    for (int i = 0; i < n; ++i) {
        // This pattern might encourage mask transformation
        result[i] = mask[i] ? a[i] : b[i];
    }
}

// Additional test with mixed types to stress different paths
void test_mixed_comparisons(int *restrict a, float *restrict b, 
                           int *restrict mask_gt, int *restrict mask_lt, int n) {
    // GT_EXPR with mixed types
    for (int i = 0; i < n; ++i) {
        mask_gt[i] = a[i] > (int)b[i];
    }
    
    // LT_EXPR with mixed types
    for (int i = 0; i < n; ++i) {
        mask_lt[i] = a[i] < (int)b[i];
    }
}

// Initialize arrays with pseudo-random data
void init_arrays(int *a, int *b, float *fa, float *fb, float *fx, float *fy,
                 unsigned int *ua, unsigned int *ub, double *da, double *db, int n) {
    srand(42);
    for (int i = 0; i < n; ++i) {
        a[i] = rand() % 1000;
        b[i] = rand() % 1000;
        fa[i] = (float)(rand() % 1000) / 10.0f;
        fb[i] = (float)(rand() % 1000) / 10.0f;
        fx[i] = (float)(rand() % 1000) / 5.0f;
        fy[i] = (float)(rand() % 1000) / 5.0f;
        ua[i] = rand() % 1000;
        ub[i] = rand() % 1000;
        da[i] = (double)(rand() % 1000) / 10.0;
        db[i] = (double)(rand() % 1000) / 10.0;
    }
}

int main() {
    // Aligned arrays for different data types
    int a[N] ALIGN;
    int b[N] ALIGN;
    int mask_gt[N] ALIGN;
    int mask_ge_int[N] ALIGN;
    
    float fa[N] ALIGN;
    float fb[N] ALIGN;
    float fx[N] ALIGN;
    float fy[N] ALIGN;
    float dst_ge[N] ALIGN;
    
    unsigned int ua[N] ALIGN;
    unsigned int ub[N] ALIGN;
    
    double da[N] ALIGN;
    double db[N] ALIGN;
    int mask_le[N] ALIGN;
    double result_le[N] ALIGN;
    
    // Mixed type arrays
    int mixed_mask_gt[N] ALIGN;
    int mixed_mask_lt[N] ALIGN;
    
    // Initialize all arrays
    init_arrays(a, b, fa, fb, fx, fy, ua, ub, da, db, N);
    
    int total_sum = 0;
    
    // Test 1: GT_EXPR (>)
    test_gt(a, b, mask_gt, N);
    for (int i = 0; i < N; ++i) total_sum += mask_gt[i];
    
    // Test 2: GE_EXPR (>=) with conditional blend
    test_ge(fa, fb, fx, fy, dst_ge, N);
    for (int i = 0; i < N; ++i) total_sum += (int)dst_ge[i];
    
    // Test 3: LT_EXPR (<) with conditional accumulation
    total_sum += test_lt(ua, ub, N);
    
    // Test 4: LE_EXPR (<=) with mask usage
    test_le(da, db, mask_le, result_le, N);
    for (int i = 0; i < N; ++i) total_sum += (int)result_le[i] + mask_le[i];
    
    // Test 5: Mixed type comparisons
    test_mixed_comparisons(a, fa, mixed_mask_gt, mixed_mask_lt, N);
    for (int i = 0; i < N; ++i) {
        total_sum += mixed_mask_gt[i] + mixed_mask_lt[i];
    }
    
    // Additional test: Multiple comparison operators in sequence
    // This might trigger different optimization paths
    int temp_mask[N] ALIGN;
    for (int i = 0; i < N; ++i) {
        // GT
        temp_mask[i] = a[i] > b[i];
        total_sum += temp_mask[i];
        
        // GE
        temp_mask[i] = a[i] >= b[i];
        total_sum += temp_mask[i];
        
        // LT
        temp_mask[i] = a[i] < b[i];
        total_sum += temp_mask[i];
        
        // LE
        temp_mask[i] = a[i] <= b[i];
        total_sum += temp_mask[i];
    }
    
    printf("Total sum: %d\n", total_sum);
    printf("(This prevents dead code elimination)\n");
    
    return 0;
}
