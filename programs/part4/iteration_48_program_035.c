#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define N 1024
#define ALIGN 32

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
    // Allocate aligned memory
    int* a_int = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* b_int = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* c_int = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    
    short* a_short = (short*)aligned_alloc(ALIGN, N * sizeof(short));
    short* b_short = (short*)aligned_alloc(ALIGN, N * sizeof(short));
    short* c_short = (short*)aligned_alloc(ALIGN, N * sizeof(short));
    
    char* a_char = (char*)aligned_alloc(ALIGN, N * sizeof(char));
    char* b_char = (char*)aligned_alloc(ALIGN, N * sizeof(char));
    char* c_char = (char*)aligned_alloc(ALIGN, N * sizeof(char));
    
    // Initialize with pseudo-random data to prevent compile-time optimization
    for (int i = 0; i < N; i++) {
        // Use i-based patterns that create varying comparisons
        a_int[i] = (i * 3) % 100;
        b_int[i] = (i * 7) % 100;
        c_int[i] = (i * 11) % 50;
        
        a_short[i] = (short)((i * 5) % 100);
        b_short[i] = (short)((i * 9) % 100);
        c_short[i] = (short)((i * 13) % 50);
        
        a_char[i] = (char)((i * 2) % 100);
        b_char[i] = (char)((i * 4) % 100);
        c_char[i] = (char)((i * 6) % 50);
    }
    
    // Use __builtin_assume_aligned to give alignment hints to the compiler
    a_int = (int*)__builtin_assume_aligned(a_int, ALIGN);
    b_int = (int*)__builtin_assume_aligned(b_int, ALIGN);
    c_int = (int*)__builtin_assume_aligned(c_int, ALIGN);
    
    a_short = (short*)__builtin_assume_aligned(a_short, ALIGN);
    b_short = (short*)__builtin_assume_aligned(b_short, ALIGN);
    c_short = (short*)__builtin_assume_aligned(c_short, ALIGN);
    
    a_char = (char*)__builtin_assume_aligned(a_char, ALIGN);
    b_char = (char*)__builtin_assume_aligned(b_char, ALIGN);
    c_char = (char*)__builtin_assume_aligned(c_char, ALIGN);
    
    // Call all test functions to ensure all switch cases are exercised
    int total = 0;
    
    total += test_gt_reduction(a_int, b_int, c_int, N);
    total += test_ge_reduction(a_short, b_short, c_short, N);
    total += test_lt_reduction(a_char, b_char, c_char, N);
    total += test_le_reduction(a_int, b_int, c_int, N);
    
    // Print result to prevent dead code elimination
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
