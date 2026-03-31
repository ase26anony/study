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
        if (a[i] < (int)b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline))
int test_le_reduction_mixed(const short* restrict a, const int* restrict b,
                            const int* restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if ((int)a[i] <= b[i]) {
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
    
    // Initialize with pseudo-random but reproducible data
    srand(42);
    for (int i = 0; i < N; i++) {
        a_int[i] = rand() % 1000;
        b_int[i] = rand() % 1000;
        c_int[i] = rand() % 100;
        
        a_short[i] = (short)(rand() % 1000);
        b_short[i] = (short)(rand() % 1000);
        c_short[i] = (short)(rand() % 100);
        
        a_char[i] = (char)(rand() % 128);
        b_char[i] = (char)(rand() % 128);
        c_char[i] = (char)(rand() % 32);
    }
    
    // Use __builtin_assume_aligned to give hints to the vectorizer
    a_int = (int*)__builtin_assume_aligned(a_int, ALIGNMENT);
    b_int = (int*)__builtin_assume_aligned(b_int, ALIGNMENT);
    c_int = (int*)__builtin_assume_aligned(c_int, ALIGNMENT);
    
    int total_sum = 0;
    
    // Test all four comparison operators with int type
    total_sum += test_gt_reduction(a_int, b_int, c_int, N);
    total_sum += test_ge_reduction(a_int, b_int, c_int, N);
    total_sum += test_lt_reduction(a_int, b_int, c_int, N);
    total_sum += test_le_reduction(a_int, b_int, c_int, N);
    
    // Test with short type
    total_sum += test_gt_reduction_short(a_short, b_short, c_short, N);
    
    // Test with char type
    total_sum += test_ge_reduction_char(a_char, b_char, c_char, N);
    
    // Test mixed types
    total_sum += test_lt_reduction_mixed(a_int, b_short, c_int, N);
    total_sum += test_le_reduction_mixed(a_short, b_int, c_int, N);
    
    printf("Total sum: %d\n", total_sum);
    
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
