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

__attribute__((noinline))
int test_mixed_reductions(const int* restrict a, const short* restrict b, 
                          const char* restrict c, const int* restrict d, int n) {
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    
    // Test all four comparison operators in separate loops
    for (int i = 0; i < n; i++) {
        if (a[i] > d[i]) {
            sum1 += a[i];
        }
    }
    
    for (int i = 0; i < n; i++) {
        if (b[i] >= (short)(i % 256)) {
            sum2 += b[i];
        }
    }
    
    for (int i = 0; i < n; i++) {
        if (c[i] < (char)(i % 128)) {
            sum3 += c[i];
        }
    }
    
    for (int i = 0; i < n; i++) {
        if (a[i] <= d[n - i - 1]) {
            sum4 += d[i];
        }
    }
    
    return sum1 + sum2 + sum3 + sum4;
}

int main() {
    // Allocate aligned memory for arrays
    int* a_int = (int*)aligned_alloc(ALIGNMENT, N * sizeof(int));
    int* b_int = (int*)aligned_alloc(ALIGNMENT, N * sizeof(int));
    int* c_int = (int*)aligned_alloc(ALIGNMENT, N * sizeof(int));
    int* d_int = (int*)aligned_alloc(ALIGNMENT, N * sizeof(int));
    
    short* a_short = (short*)aligned_alloc(ALIGNMENT, N * sizeof(short));
    short* b_short = (short*)aligned_alloc(ALIGNMENT, N * sizeof(short));
    short* c_short = (short*)aligned_alloc(ALIGNMENT, N * sizeof(short));
    
    char* a_char = (char*)aligned_alloc(ALIGNMENT, N * sizeof(char));
    char* b_char = (char*)aligned_alloc(ALIGNMENT, N * sizeof(char));
    char* c_char = (char*)aligned_alloc(ALIGNMENT, N * sizeof(char));
    
    // Initialize with pseudo-random but deterministic data
    for (int i = 0; i < N; i++) {
        // Use i-based patterns that create varied comparison results
        a_int[i] = (i * 17) % 1000;
        b_int[i] = (i * 23) % 1000;
        c_int[i] = (i * 29) % 100;
        d_int[i] = (i * 31) % 1000;
        
        a_short[i] = (short)((i * 13) % 32767);
        b_short[i] = (short)((i * 19) % 32767);
        c_short[i] = (short)((i * 7) % 100);
        
        a_char[i] = (char)((i * 11) % 127);
        b_char[i] = (char)((i * 3) % 127);
        c_char[i] = (char)((i * 5) % 100);
    }
    
    // Add some variation to ensure comparisons aren't always true/false
    for (int i = 0; i < N; i += 3) {
        a_int[i] = b_int[i] + 1;      // Makes a[i] > b[i] true
        a_short[i] = b_short[i] - 1;  // Makes a[i] >= b[i] false
        a_char[i] = b_char[i] - 1;    // Makes a[i] < b[i] true
    }
    
    // Use __builtin_assume_aligned to give hints to the compiler
    a_int = (int*)__builtin_assume_aligned(a_int, ALIGNMENT);
    b_int = (int*)__builtin_assume_aligned(b_int, ALIGNMENT);
    c_int = (int*)__builtin_assume_aligned(c_int, ALIGNMENT);
    d_int = (int*)__builtin_assume_aligned(d_int, ALIGNMENT);
    
    int total = 0;
    
    // Call all test functions to ensure all comparison operators are exercised
    total += test_gt_reduction(a_int, b_int, c_int, N);
    total += test_ge_reduction(a_short, b_short, c_short, N);
    total += test_lt_reduction(a_char, b_char, c_char, N);
    total += test_le_reduction(a_int, d_int, c_int, N);
    total += test_mixed_reductions(a_int, b_short, a_char, d_int, N);
    
    printf("Total result: %d\n", total);
    
    // Free allocated memory
    free(a_int);
    free(b_int);
    free(c_int);
    free(d_int);
    free(a_short);
    free(b_short);
    free(c_short);
    free(a_char);
    free(b_char);
    free(c_char);
    
    return 0;
}
