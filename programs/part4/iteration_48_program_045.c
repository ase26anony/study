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

__attribute__((noinline))
int test_mixed_comparisons(const int* restrict a, const int* restrict b,
                           const int* restrict c, int n) {
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    
    // Mix of all four comparison types in separate loops
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) sum1 += c[i];
    }
    
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) sum2 += c[i];
    }
    
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) sum3 += c[i];
    }
    
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) sum4 += c[i];
    }
    
    return sum1 + sum2 + sum3 + sum4;
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
    
    // Initialize with pseudo-random but deterministic values
    // Using simple patterns that create varied comparison results
    for (int i = 0; i < N; i++) {
        // For integer arrays
        a_int[i] = (i * 3) % 100;
        b_int[i] = (i * 7) % 100;
        c_int[i] = (i * 11) % 50;
        
        // For short arrays
        a_short[i] = (i * 5) % 100;
        b_short[i] = (i * 9) % 100;
        c_short[i] = (i * 13) % 50;
        
        // For char arrays
        a_char[i] = (i * 2) % 100;
        b_char[i] = (i * 5) % 100;
        c_char[i] = (i * 7) % 50;
    }
    
    // Add some variation to ensure comparisons aren't trivial
    a_int[N/2] = 200;
    b_int[N/2] = 100;
    
    a_short[N/3] = 150;
    b_short[N/3] = 50;
    
    a_char[N/4] = 120;
    b_char[N/4] = 20;
    
    int total = 0;
    
    // Test all four comparison operators
    total += test_gt_reduction(a_int, b_int, c_int, N);
    total += test_ge_reduction(a_short, b_short, c_short, N);
    total += test_lt_reduction(a_char, b_char, c_char, N);
    total += test_le_reduction(a_int, b_int, c_int, N);
    total += test_mixed_comparisons(a_int, b_int, c_int, N);
    
    printf("Total result: %d\n", total);
    
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
