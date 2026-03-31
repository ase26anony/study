#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define N 1024
#define ALIGN 64

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

int main() {
    // Allocate aligned memory
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
        // Use i-based pattern that creates varied comparisons
        a_int[i] = (i * 37) % 100;
        b_int[i] = (i * 53) % 100;
        c_int[i] = (i * 71) % 10;
        
        a_short[i] = (short)((i * 41) % 100);
        b_short[i] = (short)((i * 59) % 100);
        c_short[i] = (short)((i * 73) % 10);
        
        a_char[i] = (char)((i * 43) % 100);
        b_char[i] = (char)((i * 61) % 100);
        c_char[i] = (char)((i * 79) % 10);
    }
    
    // Tell compiler about alignment
    __builtin_assume_aligned(a_int, ALIGN);
    __builtin_assume_aligned(b_int, ALIGN);
    __builtin_assume_aligned(c_int, ALIGN);
    __builtin_assume_aligned(a_short, ALIGN);
    __builtin_assume_aligned(b_short, ALIGN);
    __builtin_assume_aligned(c_short, ALIGN);
    __builtin_assume_aligned(a_char, ALIGN);
    __builtin_assume_aligned(b_char, ALIGN);
    __builtin_assume_aligned(c_char, ALIGN);
    
    int total = 0;
    
    // Test all four comparison operators with int type
    total += test_gt_reduction(a_int, b_int, c_int, N);
    total += test_ge_reduction(a_int, b_int, c_int, N);
    total += test_lt_reduction(a_int, b_int, c_int, N);
    total += test_le_reduction(a_int, b_int, c_int, N);
    
    // Test with different integer types
    total += test_gt_reduction_short(a_short, b_short, c_short, N);
    total += test_ge_reduction_char(a_char, b_char, c_char, N);
    
    // Use the result to prevent optimization
    printf("Total sum: %d\n", total);
    
    // Cleanup
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
