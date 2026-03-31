#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define N 1024
#define ALIGN 64

__attribute__((noinline))
int test_gt_reduction(const int *restrict a, const int *restrict b, const int *restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline))
int test_ge_reduction(const int *restrict a, const int *restrict b, const int *restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline))
int test_lt_reduction(const int *restrict a, const int *restrict b, const int *restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline))
int test_le_reduction(const int *restrict a, const int *restrict b, const int *restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

// Additional tests with different integer types to increase coverage
__attribute__((noinline))
short test_gt_reduction_short(const short *restrict a, const short *restrict b, 
                              const short *restrict c, int n) {
    short sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline))
char test_ge_reduction_char(const char *restrict a, const char *restrict b, 
                            const char *restrict c, int n) {
    char sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline))
int test_lt_reduction_mixed(const short *restrict a, const int *restrict b, 
                            const int *restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline))
int test_le_reduction_mixed(const int *restrict a, const short *restrict b, 
                            const int *restrict c, int n) {
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
    int *a_int = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int *b_int = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int *c_int = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    
    short *a_short = (short*)aligned_alloc(ALIGN, N * sizeof(short));
    short *b_short = (short*)aligned_alloc(ALIGN, N * sizeof(short));
    short *c_short = (short*)aligned_alloc(ALIGN, N * sizeof(short));
    
    char *a_char = (char*)aligned_alloc(ALIGN, N * sizeof(char));
    char *b_char = (char*)aligned_alloc(ALIGN, N * sizeof(char));
    char *c_char = (char*)aligned_alloc(ALIGN, N * sizeof(char));
    
    // Initialize with pseudo-random but deterministic values
    for (int i = 0; i < N; i++) {
        // Use simple pattern that creates mix of true/false conditions
        a_int[i] = (i * 3) % 100;
        b_int[i] = (i * 5) % 100;
        c_int[i] = (i * 7) % 10;
        
        a_short[i] = (short)((i * 11) % 100);
        b_short[i] = (short)((i * 13) % 100);
        c_short[i] = (short)((i * 17) % 10);
        
        a_char[i] = (char)((i * 19) % 100);
        b_char[i] = (char)((i * 23) % 100);
        c_char[i] = (char)((i * 29) % 10);
    }
    
    // Use __builtin_assume_aligned to provide alignment hints to the compiler
    a_int = (int*)__builtin_assume_aligned(a_int, ALIGN);
    b_int = (int*)__builtin_assume_aligned(b_int, ALIGN);
    c_int = (int*)__builtin_assume_aligned(c_int, ALIGN);
    
    a_short = (short*)__builtin_assume_aligned(a_short, ALIGN);
    b_short = (short*)__builtin_assume_aligned(b_short, ALIGN);
    c_short = (short*)__builtin_assume_aligned(c_short, ALIGN);
    
    a_char = (char*)__builtin_assume_aligned(a_char, ALIGN);
    b_char = (char*)__builtin_assume_aligned(b_char, ALIGN);
    c_char = (char*)__builtin_assume_aligned(c_char, ALIGN);
    
    int total = 0;
    
    // Test all four comparison operators with int type
    total += test_gt_reduction(a_int, b_int, c_int, N);
    total += test_ge_reduction(a_int, b_int, c_int, N);
    total += test_lt_reduction(a_int, b_int, c_int, N);
    total += test_le_reduction(a_int, b_int, c_int, N);
    
    // Test with short type
    total += test_gt_reduction_short(a_short, b_short, c_short, N);
    
    // Test with char type
    total += test_ge_reduction_char(a_char, b_char, c_char, N);
    
    // Test mixed types
    total += test_lt_reduction_mixed(a_short, b_int, c_int, N);
    total += test_le_reduction_mixed(a_int, b_short, c_int, N);
    
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
