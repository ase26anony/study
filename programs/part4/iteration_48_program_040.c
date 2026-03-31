#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define N 1024
#define ALIGNMENT 64

__attribute__((noinline))
int test_gt_reduction(const int* restrict a, const int* restrict b, 
                      const int* restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline))
int test_ge_reduction(const short* restrict a, const short* restrict b,
                      const short* restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline))
int test_lt_reduction(const char* restrict a, const char* restrict b,
                      const char* restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline))
int test_le_reduction(const int* restrict a, const int* restrict b,
                      const int* restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

// Mixed types to ensure different vectorization paths
__attribute__((noinline))
int test_mixed_comparisons(const int* restrict a1, const short* restrict a2,
                           const char* restrict a3, const int* restrict b1,
                           const short* restrict b2, const char* restrict b3,
                           const int* restrict c1, const short* restrict c2,
                           const char* restrict c3, int n) {
    int sum = 0;
    
    // Test all four comparison types in one function
    for (int i = 0; i < n; i++) {
        if (a1[i] > b1[i]) sum += c1[i];
        if (a2[i] >= b2[i]) sum += c2[i];
        if (a3[i] < b3[i]) sum += c3[i];
        if (a1[i] <= b1[i]) sum += c1[i];
    }
    return sum;
}

int main() {
    // Allocate aligned memory for better vectorization
    int* a_int = (int*)aligned_alloc(ALIGNMENT, N * sizeof(int));
    int* b_int = (int*)aligned_alloc(ALIGNMENT, N * sizeof(int));
    int* c_int = (int*)aligned_alloc(ALIGNMENT, N * sizeof(int));
    
    short* a_short = (short*)aligned_alloc(ALIGNMENT, N * sizeof(short));
    short* b_short = (short*)aligned_alloc(ALIGNMENT, N * sizeof(short));
    short* c_short = (short*)aligned_alloc(ALIGNMENT, N * sizeof(short));
    
    char* a_char = (char*)aligned_alloc(ALIGNMENT, N * sizeof(char));
    char* b_char = (char*)aligned_alloc(ALIGNMENT, N * sizeof(char));
    char* c_char = (char*)aligned_alloc(ALIGNMENT, N * sizeof(char));
    
    // Initialize with pseudo-random but deterministic values
    // Using simple patterns that create varied comparison results
    for (int i = 0; i < N; i++) {
        // Create varying patterns for different comparison outcomes
        a_int[i] = (i * 3) % 100;
        b_int[i] = (i * 5) % 100;
        c_int[i] = (i * 7) % 50;
        
        a_short[i] = (i * 11) % 200;
        b_short[i] = (i * 13) % 200;
        c_short[i] = (i * 17) % 100;
        
        a_char[i] = (i * 19) % 128;
        b_char[i] = (i * 23) % 128;
        c_char[i] = (i * 29) % 64;
    }
    
    // Use __builtin_assume_aligned to give hints to the vectorizer
    a_int = (int*)__builtin_assume_aligned(a_int, ALIGNMENT);
    b_int = (int*)__builtin_assume_aligned(b_int, ALIGNMENT);
    c_int = (int*)__builtin_assume_aligned(c_int, ALIGNMENT);
    
    a_short = (short*)__builtin_assume_aligned(a_short, ALIGNMENT);
    b_short = (short*)__builtin_assume_aligned(b_short, ALIGNMENT);
    c_short = (short*)__builtin_assume_aligned(c_short, ALIGNMENT);
    
    a_char = (char*)__builtin_assume_aligned(a_char, ALIGNMENT);
    b_char = (char*)__builtin_assume_aligned(b_char, ALIGNMENT);
    c_char = (char*)__builtin_assume_aligned(c_char, ALIGNMENT);
    
    int total_sum = 0;
    
    // Call each test function to exercise all four comparison cases
    total_sum += test_gt_reduction(a_int, b_int, c_int, N);
    total_sum += test_ge_reduction(a_short, b_short, c_short, N);
    total_sum += test_lt_reduction(a_char, b_char, c_char, N);
    total_sum += test_le_reduction(a_int, b_int, c_int, N);
    
    // Also test mixed comparisons
    total_sum += test_mixed_comparisons(a_int, a_short, a_char,
                                       b_int, b_short, b_char,
                                       c_int, c_short, c_char, N);
    
    printf("Total sum: %d\n", total_sum);
    
    // Free allocated memory
    free(a_int);
    free(b_int);
    free(c_int);
    free(a_short);
    free(b_short);
    free(c_short);
    free(a_char);
    free(b_char);
    free(c_char);
    
    return 0;
}
