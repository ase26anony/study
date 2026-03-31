#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define N 1024
#define ALIGN 64

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
        // Use i as seed for deterministic but non-constant pattern
        int seed = i * 1103515245 + 12345;
        
        a_int[i] = (seed >> 16) & 0x7FFF;
        b_int[i] = (seed >> 8) & 0x7FFF;
        c_int[i] = (seed >> 4) & 0xFF;
        
        a_short[i] = (short)((seed >> 12) & 0x7FF);
        b_short[i] = (short)((seed >> 8) & 0x7FF);
        c_short[i] = (short)((seed >> 4) & 0xFF);
        
        a_char[i] = (char)((seed >> 8) & 0x7F);
        b_char[i] = (char)((seed >> 4) & 0x7F);
        c_char[i] = (char)((seed >> 2) & 0x3F);
    }
    
    // Add alignment hints using builtin assume aligned
    __builtin_assume_aligned(a_int, ALIGN);
    __builtin_assume_aligned(b_int, ALIGN);
    __builtin_assume_aligned(c_int, ALIGN);
    __builtin_assume_aligned(a_short, ALIGN);
    __builtin_assume_aligned(b_short, ALIGN);
    __builtin_assume_aligned(c_short, ALIGN);
    __builtin_assume_aligned(a_char, ALIGN);
    __builtin_assume_aligned(b_char, ALIGN);
    __builtin_assume_aligned(c_char, ALIGN);
    
    // Call all test functions to ensure they're not optimized away
    int total = 0;
    
    total += test_gt_reduction(a_int, b_int, c_int, N);
    total += test_ge_reduction(a_short, b_short, c_short, N);
    total += test_lt_reduction(a_char, b_char, c_char, N);
    total += test_le_reduction(a_int, b_int, c_int, N);
    
    // Use the result to prevent dead code elimination
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
