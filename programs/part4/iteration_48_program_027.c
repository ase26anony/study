#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define N 1024
#define ALIGN 64

__attribute__((noinline))
int test_gt_reduction(const int *restrict a, const int *restrict b, 
                      const int *restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline))
int test_ge_reduction(const short *restrict a, const short *restrict b,
                      const short *restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline))
int test_lt_reduction(const char *restrict a, const char *restrict b,
                      const char *restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline))
int test_le_reduction(const int *restrict a, const int *restrict b,
                      const int *restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline))
int test_mixed_reductions(const int *restrict a, const int *restrict b,
                          const int *restrict c, int n) {
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    
    // Test all four comparisons in one function
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            sum1 += c[i];
        }
        if (a[i] >= b[i]) {
            sum2 += c[i];
        }
        if (a[i] < b[i]) {
            sum3 += c[i];
        }
        if (a[i] <= b[i]) {
            sum4 += c[i];
        }
    }
    
    return sum1 + sum2 + sum3 + sum4;
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
    // Using simple patterns that create varied comparison results
    for (int i = 0; i < N; i++) {
        a_int[i] = (i * 37) % 100;
        b_int[i] = (i * 53) % 100;
        c_int[i] = (i * 71) % 10;
        
        a_short[i] = (short)((i * 29) % 100);
        b_short[i] = (short)((i * 43) % 100);
        c_short[i] = (short)((i * 61) % 10);
        
        a_char[i] = (char)((i * 19) % 50);
        b_char[i] = (char)((i * 31) % 50);
        c_char[i] = (char)((i * 47) % 10);
    }
    
    // Use __builtin_assume_aligned to give compiler alignment hints
    a_int = (int*)__builtin_assume_aligned(a_int, ALIGN);
    b_int = (int*)__builtin_assume_aligned(b_int, ALIGN);
    c_int = (int*)__builtin_assume_aligned(c_int, ALIGN);
    
    a_short = (short*)__builtin_assume_aligned(a_short, ALIGN);
    b_short = (short*)__builtin_assume_aligned(b_short, ALIGN);
    c_short = (short*)__builtin_assume_aligned(c_short, ALIGN);
    
    a_char = (char*)__builtin_assume_aligned(a_char, ALIGN);
    b_char = (char*)__builtin_assume_aligned(b_char, ALIGN);
    c_char = (char*)__builtin_assume_aligned(c_char, ALIGN);
    
    // Call all test functions
    int result = 0;
    
    result += test_gt_reduction(a_int, b_int, c_int, N);
    result += test_ge_reduction(a_short, b_short, c_short, N);
    result += test_lt_reduction(a_char, b_char, c_char, N);
    result += test_le_reduction(a_int, b_int, c_int, N);
    result += test_mixed_reductions(a_int, b_int, c_int, N);
    
    printf("Total result: %d\n", result);
    
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
