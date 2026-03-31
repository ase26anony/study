#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define N 1024
#define ALIGNMENT 64

__attribute__((noinline))
int test_gt_reduction(const int* restrict a, const int* restrict b, const int* restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline))
int test_ge_reduction(const short* restrict a, const short* restrict b, const short* restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline))
int test_lt_reduction(const char* restrict a, const char* restrict b, const char* restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline))
int test_le_reduction(const int* restrict a, const int* restrict b, const int* restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

int main() {
    // Allocate aligned memory for arrays
    int* a_int = (int*)aligned_alloc(ALIGNMENT, N * sizeof(int));
    int* b_int = (int*)aligned_alloc(ALIGNMENT, N * sizeof(int));
    int* c_int = (int*)aligned_alloc(ALIGNMENT, N * sizeof(int));
    
    short* a_short = (short*)aligned_alloc(ALIGNMENT, N * sizeof(short));
    short* b_short = (short*)aligned_alloc(ALIGNMENT, N * sizeof(short));
    short* c_short = (short*)aligned_alloc(ALIGNMENT, N * sizeof(short));
    
    char* a_char = (char*)aligned_alloc(ALIGNMENT, N * sizeof(char));
    char* b_char = (char*)aligned_alloc(ALIGNMENT, N * sizeof(char));
    char* c_char = (char*)aligned_alloc(ALIGNMENT, N * sizeof(char));
    
    // Initialize with pattern that creates mixed true/false conditions
    for (int i = 0; i < N; i++) {
        // Use i and i% patterns to create varying comparisons
        a_int[i] = i;
        b_int[i] = (i % 3) * 100;
        c_int[i] = i * 2;
        
        a_short[i] = (short)(i * 3);
        b_short[i] = (short)((i % 5) * 50);
        c_short[i] = (short)(i + 10);
        
        a_char[i] = (char)(i % 128);
        b_char[i] = (char)((i % 7) * 20);
        c_char[i] = (char)(i % 64);
    }
    
    // Add some randomness to prevent complete optimization
    a_int[N/2] = rand();
    b_int[N/3] = rand();
    
    // Use __builtin_assume_aligned to give hints to the compiler
    const int* restrict a_int_aligned = (const int*)__builtin_assume_aligned(a_int, ALIGNMENT);
    const int* restrict b_int_aligned = (const int*)__builtin_assume_aligned(b_int, ALIGNMENT);
    const int* restrict c_int_aligned = (const int*)__builtin_assume_aligned(c_int, ALIGNMENT);
    
    const short* restrict a_short_aligned = (const short*)__builtin_assume_aligned(a_short, ALIGNMENT);
    const short* restrict b_short_aligned = (const short*)__builtin_assume_aligned(b_short, ALIGNMENT);
    const short* restrict c_short_aligned = (const short*)__builtin_assume_aligned(c_short, ALIGNMENT);
    
    const char* restrict a_char_aligned = (const char*)__builtin_assume_aligned(a_char, ALIGNMENT);
    const char* restrict b_char_aligned = (const char*)__builtin_assume_aligned(b_char, ALIGNMENT);
    const char* restrict c_char_aligned = (const char*)__builtin_assume_aligned(c_char, ALIGNMENT);
    
    // Call all test functions to ensure they're not optimized away
    int total = 0;
    
    total += test_gt_reduction(a_int_aligned, b_int_aligned, c_int_aligned, N);
    total += test_ge_reduction(a_short_aligned, b_short_aligned, c_short_aligned, N);
    total += test_lt_reduction(a_char_aligned, b_char_aligned, c_char_aligned, N);
    total += test_le_reduction(a_int_aligned, b_int_aligned, c_int_aligned, N);
    
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
