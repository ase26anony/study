#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define N 1024
#define ALIGN 64

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
int test_ge_reduction(const int* restrict a, const int* restrict b,
                      const int* restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline))
int test_lt_reduction(const int* restrict a, const int* restrict b,
                      const int* restrict c, int n) {
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

// Additional tests with different integer types to ensure coverage
__attribute__((noinline))
short test_gt_reduction_short(const short* restrict a, const short* restrict b,
                              const short* restrict c, int n) {
    short sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline))
char test_ge_reduction_char(const char* restrict a, const char* restrict b,
                            const char* restrict c, int n) {
    char sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline))
int test_lt_reduction_mixed(const int* restrict a, const short* restrict b,
                            const int* restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

int main() {
    // Allocate aligned memory to help vectorizer
    int* a_int = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* b_int = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* c_int = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    
    short* a_short = (short*)aligned_alloc(ALIGN, N * sizeof(short));
    short* b_short = (short*)aligned_alloc(ALIGN, N * sizeof(short));
    short* c_short = (short*)aligned_alloc(ALIGN, N * sizeof(short));
    
    char* a_char = (char*)aligned_alloc(ALIGN, N * sizeof(char));
    char* b_char = (char*)aligned_alloc(ALIGN, N * sizeof(char));
    char* c_char = (char*)aligned_alloc(ALIGN, N * sizeof(char));
    
    // Initialize with pseudo-random but reproducible data
    // Using simple patterns that create varied comparison results
    for (int i = 0; i < N; i++) {
        // For integer arrays
        a_int[i] = (i * 3) % 100;
        b_int[i] = (i * 7) % 100;
        c_int[i] = (i * 11) % 50;
        
        // For short arrays
        a_short[i] = (i * 5) % 100;
        b_short[i] = (i * 13) % 100;
        c_short[i] = (i * 17) % 50;
        
        // For char arrays
        a_char[i] = (i * 2) % 100;
        b_char[i] = (i * 19) % 100;
        c_char[i] = (i * 23) % 50;
    }
    
    // Use __builtin_assume_aligned to give hints to the compiler
    a_int = (int*)__builtin_assume_aligned(a_int, ALIGN);
    b_int = (int*)__builtin_assume_aligned(b_int, ALIGN);
    c_int = (int*)__builtin_assume_aligned(c_int, ALIGN);
    
    int total = 0;
    
    // Test all four comparison operators with int type
    total += test_gt_reduction(a_int, b_int, c_int, N);
    total += test_ge_reduction(a_int, b_int, c_int, N);
    total += test_lt_reduction(a_int, b_int, c_int, N);
    total += test_le_reduction(a_int, b_int, c_int, N);
    
    // Test with different integer types
    total += test_gt_reduction_short(a_short, b_short, c_short, N);
    total += test_ge_reduction_char(a_char, b_char, c_char, N);
    total += test_lt_reduction_mixed(a_int, b_short, c_int, N);
    
    // Print result to prevent optimization
    printf("Total sum: %d\n", total);
    
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
