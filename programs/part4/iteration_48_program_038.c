#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define N 1024
#define ALIGN 64

// Prevent inlining to keep loop structure intact for vectorizer analysis
#define NOINLINE __attribute__((noinline))

// Helper to allocate aligned memory
static inline void* aligned_alloc_ptr(size_t size, size_t alignment) {
    void* ptr;
    if (posix_memalign(&ptr, alignment, size) != 0) {
        return NULL;
    }
    return ptr;
}

// Initialize arrays with semi-random data
static void init_arrays(int* restrict a, int* restrict b, int* restrict c) {
    for (int i = 0; i < N; i++) {
        // Use i and some arithmetic to create varying patterns
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
        c[i] = (i % 10) + 1;  // Values 1-10 for accumulation
    }
}

// Test cases for each comparison operator

NOINLINE int test_gt_reduction(const int* restrict a, const int* restrict b, 
                               const int* restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {  // GT_EXPR case
            sum += c[i];
        }
    }
    return sum;
}

NOINLINE int test_ge_reduction(const int* restrict a, const int* restrict b, 
                               const int* restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {  // GE_EXPR case
            sum += c[i];
        }
    }
    return sum;
}

NOINLINE int test_lt_reduction(const int* restrict a, const int* restrict b, 
                               const int* restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {  // LT_EXPR case
            sum += c[i];
        }
    }
    return sum;
}

NOINLINE int test_le_reduction(const int* restrict a, const int* restrict b, 
                               const int* restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {  // LE_EXPR case
            sum += c[i];
        }
    }
    return sum;
}

// Additional test with different integer types to ensure coverage
NOINLINE short test_gt_reduction_short(const short* restrict a, const short* restrict b, 
                                       const short* restrict c, int n) {
    short sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {  // GT_EXPR case with short
            sum += c[i];
        }
    }
    return sum;
}

NOINLINE char test_ge_reduction_char(const char* restrict a, const char* restrict b, 
                                     const char* restrict c, int n) {
    char sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {  // GE_EXPR case with char
            sum += c[i];
        }
    }
    return sum;
}

int main(void) {
    // Allocate aligned arrays to help vectorizer
    int* a = (int*)aligned_alloc_ptr(N * sizeof(int), ALIGN);
    int* b = (int*)aligned_alloc_ptr(N * sizeof(int), ALIGN);
    int* c = (int*)aligned_alloc_ptr(N * sizeof(int), ALIGN);
    
    if (!a || !b || !c) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize arrays
    init_arrays(a, b, c);
    
    // Tell compiler about alignment (helps vectorizer)
    __builtin_assume_aligned(a, ALIGN);
    __builtin_assume_aligned(b, ALIGN);
    __builtin_assume_aligned(c, ALIGN);
    
    // Run all test cases
    int total = 0;
    
    total += test_gt_reduction(a, b, c, N);
    total += test_ge_reduction(a, b, c, N);
    total += test_lt_reduction(a, b, c, N);
    total += test_le_reduction(a, b, c, N);
    
    // Test with different integer types
    short* a_short = (short*)aligned_alloc_ptr(N * sizeof(short), ALIGN);
    short* b_short = (short*)aligned_alloc_ptr(N * sizeof(short), ALIGN);
    short* c_short = (short*)aligned_alloc_ptr(N * sizeof(short), ALIGN);
    
    char* a_char = (char*)aligned_alloc_ptr(N * sizeof(char), ALIGN);
    char* b_char = (char*)aligned_alloc_ptr(N * sizeof(char), ALIGN);
    char* c_char = (char*)aligned_alloc_ptr(N * sizeof(char), ALIGN);
    
    if (a_short && b_short && c_short && a_char && b_char && c_char) {
        // Initialize with similar patterns
        for (int i = 0; i < N; i++) {
            a_short[i] = (i * 3) % 100;
            b_short[i] = (i * 7) % 100;
            c_short[i] = (i % 10) + 1;
            
            a_char[i] = (i * 3) % 100;
            b_char[i] = (i * 7) % 100;
            c_char[i] = (i % 10) + 1;
        }
        
        total += test_gt_reduction_short(a_short, b_short, c_short, N);
        total += test_ge_reduction_char(a_char, b_char, c_char, N);
    }
    
    // Use the result to prevent dead code elimination
    printf("Total sum: %d\n", total);
    
    // Cleanup
    free(a);
    free(b);
    free(c);
    free(a_short);
    free(b_short);
    free(c_short);
    free(a_char);
    free(b_char);
    free(c_char);
    
    return 0;
}
