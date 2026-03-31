#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define N 1024
#define ALIGN __attribute__((aligned(32)))

// Test functions for each comparison operator
// Each function is defined separately to ensure they're analyzed independently

// GT_EXPR: greater than
void test_gt(int *restrict a, int *restrict b, int *restrict mask, int n) {
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] > b[i];  // GT_EXPR
    }
}

// GE_EXPR: greater than or equal
void test_ge(float *restrict a, float *restrict b, float *restrict x, 
             float *restrict y, float *restrict dst, int n) {
    for (int i = 0; i < n; ++i) {
        // Conditional blend using GE_EXPR
        dst[i] = (a[i] >= b[i]) ? x[i] : y[i];  // GE_EXPR
    }
}

// LT_EXPR: less than
void test_lt(unsigned int *restrict a, unsigned int *restrict b, 
             unsigned int *restrict sum_ptr, int n) {
    unsigned int sum = *sum_ptr;
    for (int i = 0; i < n; ++i) {
        if (a[i] < b[i]) {  // LT_EXPR
            sum += a[i];
        }
    }
    *sum_ptr = sum;
}

// LE_EXPR: less than or equal
void test_le(double *restrict a, double *restrict b, int *restrict mask, 
             int *restrict processed, int n) {
    for (int i = 0; i < n; ++i) {
        mask[i] = a[i] <= b[i];  // LE_EXPR
    }
    
    // Use the mask in bitwise operations to encourage the transformation
    for (int i = 0; i < n; ++i) {
        processed[i] = mask[i] & (i & 0xFF);
    }
}

// Mixed types test to cover different comparison semantics
void test_mixed_types() {
    ALIGN int int_a[N], int_b[N], int_mask[N];
    ALIGN float float_a[N], float_b[N], float_x[N], float_y[N], float_dst[N];
    ALIGN unsigned int uint_a[N], uint_b[N];
    ALIGN double double_a[N], double_b[N];
    ALIGN int processed[N];
    
    unsigned int uint_sum = 0;
    
    // Initialize arrays with varying patterns
    for (int i = 0; i < N; ++i) {
        int_a[i] = i;
        int_b[i] = N/2 - i;
        
        float_a[i] = i * 0.5f;
        float_b[i] = (N - i) * 0.3f;
        float_x[i] = i * 1.1f;
        float_y[i] = i * 0.9f;
        
        uint_a[i] = i * 2;
        uint_b[i] = i * 3;
        
        double_a[i] = i * 0.25;
        double_b[i] = (i % 64) * 0.5;
    }
    
    // Test each comparison operator in separate loops
    test_gt(int_a, int_b, int_mask, N);
    test_ge(float_a, float_b, float_x, float_y, float_dst, N);
    test_lt(uint_a, uint_b, &uint_sum, N);
    test_le(double_a, double_b, int_mask, processed, N);
    
    // Prevent dead code elimination by computing and printing a checksum
    int checksum = 0;
    for (int i = 0; i < N; ++i) {
        checksum += int_mask[i];
        checksum += (int)float_dst[i];
        checksum += processed[i];
    }
    checksum += uint_sum;
    
    printf("Checksum: %d\n", checksum);
}

// Additional test with different array sizes to cover edge cases
void test_various_sizes() {
    const int sizes[] = {64, 128, 256, 512, 1024};
    
    for (int s = 0; s < 5; ++s) {
        int size = sizes[s];
        
        ALIGN int a[size], b[size], mask[size];
        
        for (int i = 0; i < size; ++i) {
            a[i] = i;
            b[i] = size - i - 1;
        }
        
        // Test GT_EXPR with different sizes
        for (int i = 0; i < size; ++i) {
            mask[i] = a[i] > b[i];  // GT_EXPR
        }
        
        // Test LE_EXPR with different sizes
        int temp_sum = 0;
        for (int i = 0; i < size; ++i) {
            if (a[i] <= b[i]) {  // LE_EXPR
                temp_sum += mask[i];
            }
        }
        
        // Use result to prevent elimination
        printf("Size %d: %d\n", size, temp_sum);
    }
}

// Main function that calls all tests
int main() {
    printf("Testing vector comparison transformations...\n");
    
    // Seed for reproducible patterns
    srand(42);
    
    // Run main test
    test_mixed_types();
    
    // Run additional size tests
    test_various_sizes();
    
    return 0;
}
