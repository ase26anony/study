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
char test_ge_reduction_char(const char * restrict a, const char * restrict b,
                            const char * restrict c, int n) {
    char sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline))
int test_lt_reduction_mixed(const int * restrict a, const short * restrict b,
                            const int * restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] < (int)b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline))
int test_le_reduction_mixed(const short * restrict a, const int * restrict b,
                            const int * restrict c, int n) {
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
    int *a_int = (int*)aligned_alloc(ALIGNMENT, N * sizeof(int));
    int *b_int = (int*)aligned_alloc(ALIGNMENT, N * sizeof(int));
    int *c_int = (int*)aligned_alloc(ALIGNMENT, N * sizeof(int));
    
    short *a_short = (short*)aligned_alloc(ALIGNMENT, N * sizeof(short));
    short *b_short = (short*)aligned_alloc(ALIGNMENT, N * sizeof(short));
    short *c_short = (short*)aligned_alloc(ALIGNMENT, N * sizeof(short));
    
    char *a_char = (char*)aligned_alloc(ALIGNMENT, N * sizeof(char));
    char *b_char = (char*)aligned_alloc(ALIGNMENT, N * sizeof(char));
    char *c_char = (char*)aligned_alloc(ALIGNMENT, N * sizeof(char));
    
    // Initialize with pseudo-random but reproducible values
    // Using simple patterns that create varying comparison results
    for (int i = 0; i < N; i++) {
        a_int[i] = (i * 3) % 100;
        b_int[i] = (i * 7) % 100;
        c_int[i] = (i * 11) % 50;
        
        a_short[i] = (short)((i * 5) % 100);
        b_short[i] = (short)((i * 13) % 100);
        c_short[i] = (short)((i * 17) % 50);
        
        a_char[i] = (char)((i * 2) % 100);
        b_char[i] = (char)((i * 19) % 100);
        c_char[i] = (char)((i * 23) % 50);
    }
    
    // Use __builtin_assume_aligned to give hints to the vectorizer
    int * restrict aa_int = __builtin_assume_aligned(a_int, ALIGNMENT);
    int * restrict ab_int = __builtin_assume_aligned(b_int, ALIGNMENT);
    int * restrict ac_int = __builtin_assume_aligned(c_int, ALIGNMENT);
    
    short * restrict aa_short = __builtin_assume_aligned(a_short, ALIGNMENT);
    short * restrict ab_short = __builtin_assume_aligned(b_short, ALIGNMENT);
    short * restrict ac_short = __builtin_assume_aligned(c_short, ALIGNMENT);
    
    char * restrict aa_char = __builtin_assume_aligned(a_char, ALIGNMENT);
    char * restrict ab_char = __builtin_assume_aligned(b_char, ALIGNMENT);
    char * restrict ac_char = __builtin_assume_aligned(c_char, ALIGNMENT);
    
    // Call all test functions to ensure all comparison operators are exercised
    int total = 0;
    
    // Test all four comparison operators with int type
    total += test_gt_reduction(aa_int, ab_int, ac_int, N);
    total += test_ge_reduction(aa_int, ab_int, ac_int, N);
    total += test_lt_reduction(aa_int, ab_int, ac_int, N);
    total += test_le_reduction(aa_int, ab_int, ac_int, N);
    
    // Test with short type
    total += test_gt_reduction_short(aa_short, ab_short, ac_short, N);
    
    // Test with char type
    total += test_ge_reduction_char(aa_char, ab_char, ac_char, N);
    
    // Test mixed types
    total += test_lt_reduction_mixed(aa_int, ab_short, ac_int, N);
    total += test_le_reduction_mixed(aa_short, ab_int, ac_int, N);
    
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
