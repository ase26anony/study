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
    int* a_int = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* b_int = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* c_int = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    
    short* a_short = (short*)aligned_alloc(ALIGN, N * sizeof(short));
    short* b_short = (short*)aligned_alloc(ALIGN, N * sizeof(short));
    short* c_short = (short*)aligned_alloc(ALIGN, N * sizeof(short));
    
    char* a_char = (char*)aligned_alloc(ALIGN, N * sizeof(char));
    char* b_char = (char*)aligned_alloc(ALIGN, N * sizeof(char));
    char* c_char = (char*)aligned_alloc(ALIGN, N * sizeof(char));
    
    // Initialize with pseudo-random but deterministic values
    // Using simple patterns that create mixed true/false conditions
    for (int i = 0; i < N; i++) {
        // For integer arrays
        a_int[i] = (i * 3) % 100;
        b_int[i] = (i * 2) % 100;
        c_int[i] = (i % 10) + 1;
        
        // For short arrays
        a_short[i] = (i * 5) % 100;
        b_short[i] = (i * 7) % 100;
        c_short[i] = (i % 7) + 1;
        
        // For char arrays
        a_char[i] = (i * 11) % 127;
        b_char[i] = (i * 13) % 127;
        c_char[i] = (i % 5) + 1;
    }
    
    // Add some variation to ensure not all comparisons are true/false
    a_int[N/2] = 200;
    b_int[N/2] = 50;
    a_short[N/3] = 150;
    b_short[N/3] = 10;
    a_char[N/4] = 100;
    b_char[N/4] = 20;
    
    // Use __builtin_assume_aligned to give hints to the compiler
    const int* restrict aa_int = __builtin_assume_aligned(a_int, ALIGN);
    const int* restrict ab_int = __builtin_assume_aligned(b_int, ALIGN);
    const int* restrict ac_int = __builtin_assume_aligned(c_int, ALIGN);
    
    const short* restrict aa_short = __builtin_assume_aligned(a_short, ALIGN);
    const short* restrict ab_short = __builtin_assume_aligned(b_short, ALIGN);
    const short* restrict ac_short = __builtin_assume_aligned(c_short, ALIGN);
    
    const char* restrict aa_char = __builtin_assume_aligned(a_char, ALIGN);
    const char* restrict ab_char = __builtin_assume_aligned(b_char, ALIGN);
    const char* restrict ac_char = __builtin_assume_aligned(c_char, ALIGN);
    
    // Call all test functions
    int result = 0;
    
    result += test_gt_reduction(aa_int, ab_int, ac_int, N);
    result += test_ge_reduction(aa_short, ab_short, ac_short, N);
    result += test_lt_reduction(aa_char, ab_char, ac_char, N);
    result += test_le_reduction(aa_int, ab_int, ac_int, N);
    
    printf("Total result: %d\n", result);
    
    // Free allocated memory
    free(a_int); free(b_int); free(c_int);
    free(a_short); free(b_short); free(c_short);
    free(a_char); free(b_char); free(c_char);
    
    return 0;
}
