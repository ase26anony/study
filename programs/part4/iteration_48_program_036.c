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
    
    // Initialize with pseudo-random but deterministic values
    srand(42);
    for (int i = 0; i < N; i++) {
        // Use i and rand() to create varying patterns
        a_int[i] = (i * 3) % 100;
        b_int[i] = (i * 7) % 100;
        c_int[i] = (rand() % 10) + 1;
        
        a_short[i] = (i * 5) % 100;
        b_short[i] = (i * 11) % 100;
        c_short[i] = (rand() % 10) + 1;
        
        a_char[i] = (i * 13) % 100;
        b_char[i] = (i * 17) % 100;
        c_char[i] = (rand() % 10) + 1;
    }
    
    // Use __builtin_assume_aligned to provide alignment hints
    const int* restrict a_int_a = (const int*)__builtin_assume_aligned(a_int, ALIGNMENT);
    const int* restrict b_int_a = (const int*)__builtin_assume_aligned(b_int, ALIGNMENT);
    const int* restrict c_int_a = (const int*)__builtin_assume_aligned(c_int, ALIGNMENT);
    
    const short* restrict a_short_a = (const short*)__builtin_assume_aligned(a_short, ALIGNMENT);
    const short* restrict b_short_a = (const short*)__builtin_assume_aligned(b_short, ALIGNMENT);
    const short* restrict c_short_a = (const short*)__builtin_assume_aligned(c_short, ALIGNMENT);
    
    const char* restrict a_char_a = (const char*)__builtin_assume_aligned(a_char, ALIGNMENT);
    const char* restrict b_char_a = (const char*)__builtin_assume_aligned(b_char, ALIGNMENT);
    const char* restrict c_char_a = (const char*)__builtin_assume_aligned(c_char, ALIGNMENT);
    
    // Call all test functions to ensure all switch cases are exercised
    int total = 0;
    total += test_gt_reduction(a_int_a, b_int_a, c_int_a, N);
    total += test_ge_reduction(a_short_a, b_short_a, c_short_a, N);
    total += test_lt_reduction(a_char_a, b_char_a, c_char_a, N);
    total += test_le_reduction(a_int_a, b_int_a, c_int_a, N);
    
    printf("Total sum: %d\n", total);
    
    // Free allocated memory
    free(a_int); free(b_int); free(c_int);
    free(a_short); free(b_short); free(c_short);
    free(a_char); free(b_char); free(c_char);
    
    return 0;
}
