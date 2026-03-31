#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define N 1024
#define ALIGNMENT 64

__attribute__((noinline))
int test_gt_reduction(const int * restrict a, const int * restrict b, 
                      const int * restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline))
int test_ge_reduction(const int * restrict a, const int * restrict b,
                      const int * restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline))
int test_lt_reduction(const int * restrict a, const int * restrict b,
                      const int * restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline))
int test_le_reduction(const int * restrict a, const int * restrict b,
                      const int * restrict c, int n) {
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
short test_gt_reduction_short(const short * restrict a, const short * restrict b,
                              const short * restrict c, int n) {
    short sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline))
char test_le_reduction_char(const char * restrict a, const char * restrict b,
                            const char * restrict c, int n) {
    char sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

int main() {
    // Allocate aligned memory to help vectorizer
    int *a = (int*)aligned_alloc(ALIGNMENT, N * sizeof(int));
    int *b = (int*)aligned_alloc(ALIGNMENT, N * sizeof(int));
    int *c = (int*)aligned_alloc(ALIGNMENT, N * sizeof(int));
    
    short *a_short = (short*)aligned_alloc(ALIGNMENT, N * sizeof(short));
    short *b_short = (short*)aligned_alloc(ALIGNMENT, N * sizeof(short));
    short *c_short = (short*)aligned_alloc(ALIGNMENT, N * sizeof(short));
    
    char *a_char = (char*)aligned_alloc(ALIGNMENT, N * sizeof(char));
    char *b_char = (char*)aligned_alloc(ALIGNMENT, N * sizeof(char));
    char *c_char = (char*)aligned_alloc(ALIGNMENT, N * sizeof(char));
    
    // Initialize with pseudo-random but deterministic values
    // Using simple patterns that create varied comparison results
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
        c[i] = (i * 11) % 50;
        
        a_short[i] = (short)((i * 5) % 100);
        b_short[i] = (short)((i * 9) % 100);
        c_short[i] = (short)((i * 13) % 50);
        
        a_char[i] = (char)((i * 2) % 50);
        b_char[i] = (char)((i * 5) % 50);
        c_char[i] = (char)((i * 7) % 25);
    }
    
    // Use __builtin_assume_aligned to provide alignment hints to the compiler
    a = (int*)__builtin_assume_aligned(a, ALIGNMENT);
    b = (int*)__builtin_assume_aligned(b, ALIGNMENT);
    c = (int*)__builtin_assume_aligned(c, ALIGNMENT);
    
    int total = 0;
    
    // Test all four comparison operators with int type
    total += test_gt_reduction(a, b, c, N);
    total += test_ge_reduction(a, b, c, N);
    total += test_lt_reduction(a, b, c, N);
    total += test_le_reduction(a, b, c, N);
    
    // Test with different integer types
    total += test_gt_reduction_short(a_short, b_short, c_short, N);
    total += test_le_reduction_char(a_char, b_char, c_char, N);
    
    // Use the result to prevent dead code elimination
    printf("Total sum: %d\n", total);
    
    // Clean up
    free(a);
    free(b);
    free(c);
    free(a_short);
    free(b_short);
    free(c_short);
    free(a_char);
    free(b_char);
    free(c_char);
    
    return 0;
}
