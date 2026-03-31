#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define N 1024
#define ALIGN 64

// Prevent inlining to keep loop structure intact for vectorizer analysis
#define NOINLINE __attribute__((noinline))

// Helper to allocate aligned memory
static inline void* aligned_alloc_wrap(size_t alignment, size_t size) {
    void* ptr = aligned_alloc(alignment, size);
    if (!ptr) {
        fprintf(stderr, "Failed to allocate aligned memory\n");
        exit(1);
    }
    return ptr;
}

// Initialize arrays with semi-random data to prevent compile-time optimization
static void init_arrays(int* restrict a, int* restrict b, int* restrict c) {
    for (int i = 0; i < N; i++) {
        // Use i-based pattern that's not trivially predictable but ensures
        // some conditions will be true and some false
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
        c[i] = (i * 11) % 100;
    }
}

// Greater-than conditional reduction
NOINLINE int test_gt_reduction(const int* restrict a, const int* restrict b, 
                               const int* restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

// Greater-or-equal conditional reduction
NOINLINE int test_ge_reduction(const int* restrict a, const int* restrict b,
                               const int* restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

// Less-than conditional reduction
NOINLINE int test_lt_reduction(const int* restrict a, const int* restrict b,
                               const int* restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

// Less-or-equal conditional reduction
NOINLINE int test_le_reduction(const int* restrict a, const int* restrict b,
                               const int* restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
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
        if (a[i] > b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

NOINLINE char test_lt_reduction_char(const char* restrict a, const char* restrict b,
                                     const char* restrict c, int n) {
    char sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

int main() {
    // Allocate aligned arrays
    int* a = (int*)aligned_alloc_wrap(ALIGN, N * sizeof(int));
    int* b = (int*)aligned_alloc_wrap(ALIGN, N * sizeof(int));
    int* c = (int*)aligned_alloc_wrap(ALIGN, N * sizeof(int));
    
    // Initialize with non-constant data
    init_arrays(a, b, c);
    
    // Use __builtin_assume_aligned to give hints to the vectorizer
    a = (int*)__builtin_assume_aligned(a, ALIGN);
    b = (int*)__builtin_assume_aligned(b, ALIGN);
    c = (int*)__builtin_assume_aligned(c, ALIGN);
    
    int total = 0;
    
    // Test all four comparison operators
    total += test_gt_reduction(a, b, c, N);
    total += test_ge_reduction(a, b, c, N);
    total += test_lt_reduction(a, b, c, N);
    total += test_le_reduction(a, b, c, N);
    
    // Test with different integer types
    short* a_short = (short*)aligned_alloc_wrap(ALIGN, N * sizeof(short));
    short* b_short = (short*)aligned_alloc_wrap(ALIGN, N * sizeof(short));
    short* c_short = (short*)aligned_alloc_wrap(ALIGN, N * sizeof(short));
    
    for (int i = 0; i < N; i++) {
        a_short[i] = (short)((i * 5) % 256);
        b_short[i] = (short)((i * 13) % 256);
        c_short[i] = (short)((i * 17) % 256);
    }
    
    a_short = (short*)__builtin_assume_aligned(a_short, ALIGN);
    b_short = (short*)__builtin_assume_aligned(b_short, ALIGN);
    c_short = (short*)__builtin_assume_aligned(c_short, ALIGN);
    
    total += test_gt_reduction_short(a_short, b_short, c_short, N);
    
    // Test with char type
    char* a_char = (char*)aligned_alloc_wrap(ALIGN, N * sizeof(char));
    char* b_char = (char*)aligned_alloc_wrap(ALIGN, N * sizeof(char));
    char* c_char = (char*)aligned_alloc_wrap(ALIGN, N * sizeof(char));
    
    for (int i = 0; i < N; i++) {
        a_char[i] = (char)((i * 7) % 128);
        b_char[i] = (char)((i * 19) % 128);
        c_char[i] = (char)((i * 23) % 128);
    }
    
    a_char = (char*)__builtin_assume_aligned(a_char, ALIGN);
    b_char = (char*)__builtin_assume_aligned(b_char, ALIGN);
    c_char = (char*)__builtin_assume_aligned(c_char, ALIGN);
    
    total += test_lt_reduction_char(a_char, b_char, c_char, N);
    
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
