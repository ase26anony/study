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
    int *a = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int *b = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int *c = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    
    short *a_short = (short*)aligned_alloc(ALIGN, N * sizeof(short));
    short *b_short = (short*)aligned_alloc(ALIGN, N * sizeof(short));
    short *c_short = (short*)aligned_alloc(ALIGN, N * sizeof(short));
    
    char *a_char = (char*)aligned_alloc(ALIGN, N * sizeof(char));
    char *b_char = (char*)aligned_alloc(ALIGN, N * sizeof(char));
    char *c_char = (char*)aligned_alloc(ALIGN, N * sizeof(char));
    
    // Initialize with pseudo-random but deterministic values
    // Using simple patterns that create mixed true/false conditions
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 2) % 100;
        c[i] = (i % 10) + 1;
        
        a_short[i] = (short)((i * 5) % 100);
        b_short[i] = (short)((i * 3) % 100);
        c_short[i] = (short)((i % 7) + 1);
        
        a_char[i] = (char)((i * 2) % 50);
        b_char[i] = (char)((i * 3) % 50);
        c_char[i] = (char)((i % 5) + 1);
    }
    
    // Use __builtin_assume_aligned to give hints to the compiler
    const int * restrict pa = __builtin_assume_aligned(a, ALIGN);
    const int * restrict pb = __builtin_assume_aligned(b, ALIGN);
    const int * restrict pc = __builtin_assume_aligned(c, ALIGN);
    
    const short * restrict pa_short = __builtin_assume_aligned(a_short, ALIGN);
    const short * restrict pb_short = __builtin_assume_aligned(b_short, ALIGN);
    const short * restrict pc_short = __builtin_assume_aligned(c_short, ALIGN);
    
    const char * restrict pa_char = __builtin_assume_aligned(a_char, ALIGN);
    const char * restrict pb_char = __builtin_assume_aligned(b_char, ALIGN);
    const char * restrict pc_char = __builtin_assume_aligned(c_char, ALIGN);
    
    int total = 0;
    
    // Call all test functions to ensure all switch cases are exercised
    total += test_gt_reduction(pa, pb, pc, N);
    total += test_ge_reduction(pa, pb, pc, N);
    total += test_lt_reduction(pa, pb, pc, N);
    total += test_le_reduction(pa, pb, pc, N);
    
    total += test_gt_reduction_short(pa_short, pb_short, pc_short, N);
    total += test_le_reduction_char(pa_char, pb_char, pc_char, N);
    
    // Prevent dead code elimination
    printf("Total sum: %d\n", total);
    
    // Cleanup
    free(a); free(b); free(c);
    free(a_short); free(b_short); free(c_short);
    free(a_char); free(b_char); free(c_char);
    
    return 0;
}
