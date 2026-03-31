#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define N 1024
#define ALIGNMENT 64

__attribute__((noinline))
int test_gt_reduction(const int * restrict a, const int * restrict b, const int * restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline))
int test_ge_reduction(const short * restrict a, const short * restrict b, const short * restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline))
int test_lt_reduction(const char * restrict a, const char * restrict b, const char * restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline))
int test_le_reduction(const int * restrict a, const int * restrict b, const int * restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline))
int test_mixed_comparisons(const int * restrict a, const int * restrict b, 
                           const int * restrict c, const int * restrict d, int n) {
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    
    // Mix of different comparison operators in separate loops
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            sum1 += c[i];
        }
    }
    
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            sum2 += d[i];
        }
    }
    
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            sum3 += c[i];
        }
    }
    
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
            sum4 += d[i];
        }
    }
    
    return sum1 + sum2 + sum3 + sum4;
}

int main() {
    // Allocate aligned memory to help vectorizer
    int *a_int = (int*)aligned_alloc(ALIGNMENT, N * sizeof(int));
    int *b_int = (int*)aligned_alloc(ALIGNMENT, N * sizeof(int));
    int *c_int = (int*)aligned_alloc(ALIGNMENT, N * sizeof(int));
    int *d_int = (int*)aligned_alloc(ALIGNMENT, N * sizeof(int));
    
    short *a_short = (short*)aligned_alloc(ALIGNMENT, N * sizeof(short));
    short *b_short = (short*)aligned_alloc(ALIGNMENT, N * sizeof(short));
    short *c_short = (short*)aligned_alloc(ALIGNMENT, N * sizeof(short));
    
    char *a_char = (char*)aligned_alloc(ALIGNMENT, N * sizeof(char));
    char *b_char = (char*)aligned_alloc(ALIGNMENT, N * sizeof(char));
    char *c_char = (char*)aligned_alloc(ALIGNMENT, N * sizeof(char));
    
    // Initialize with pseudo-random but deterministic values
    // Using simple patterns that prevent compile-time optimization
    for (int i = 0; i < N; i++) {
        a_int[i] = (i * 37) % 100;
        b_int[i] = (i * 53) % 100;
        c_int[i] = (i * 71) % 10;
        d_int[i] = (i * 29) % 10;
        
        a_short[i] = (i * 41) % 128;
        b_short[i] = (i * 67) % 128;
        c_short[i] = (i * 83) % 10;
        
        a_char[i] = (i * 19) % 64;
        b_char[i] = (i * 31) % 64;
        c_char[i] = (i * 47) % 10;
    }
    
    // Use __builtin_assume_aligned to give hints to the compiler
    const int * restrict pa = __builtin_assume_aligned(a_int, ALIGNMENT);
    const int * restrict pb = __builtin_assume_aligned(b_int, ALIGNMENT);
    const int * restrict pc = __builtin_assume_aligned(c_int, ALIGNMENT);
    const int * restrict pd = __builtin_assume_aligned(d_int, ALIGNMENT);
    
    const short * restrict psa = __builtin_assume_aligned(a_short, ALIGNMENT);
    const short * restrict psb = __builtin_assume_aligned(b_short, ALIGNMENT);
    const short * restrict psc = __builtin_assume_aligned(c_short, ALIGNMENT);
    
    const char * restrict pca = __builtin_assume_aligned(a_char, ALIGNMENT);
    const char * restrict pcb = __builtin_assume_aligned(b_char, ALIGNMENT);
    const char * restrict pcc = __builtin_assume_aligned(c_char, ALIGNMENT);
    
    int total = 0;
    
    // Test all four comparison operators
    total += test_gt_reduction(pa, pb, pc, N);
    total += test_ge_reduction(psa, psb, psc, N);
    total += test_lt_reduction(pca, pcb, pcc, N);
    total += test_le_reduction(pa, pb, pc, N);
    
    // Test mixed comparisons
    total += test_mixed_comparisons(pa, pb, pc, pd, N);
    
    printf("Total result: %d\n", total);
    
    // Free allocated memory
    free(a_int); free(b_int); free(c_int); free(d_int);
    free(a_short); free(b_short); free(c_short);
    free(a_char); free(b_char); free(c_char);
    
    return 0;
}
