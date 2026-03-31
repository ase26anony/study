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
int test_ge_reduction(const int *restrict a, const int *restrict b,
                      const int *restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline))
int test_lt_reduction(const int *restrict a, const int *restrict b,
                      const int *restrict c, int n) {
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

// Additional tests with different integer types to ensure coverage
__attribute__((noinline))
short test_gt_reduction_short(const short *restrict a, const short *restrict b,
                              const short *restrict c, int n) {
    short sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline))
char test_le_reduction_char(const char *restrict a, const char *restrict b,
                            const char *restrict c, int n) {
    char sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

// Helper to initialize arrays with pseudo-random but vectorizable patterns
void init_arrays(int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        // Use patterns that create mixed true/false conditions
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
        c[i] = (i * 11) % 50;
    }
}

void init_arrays_short(short *a, short *b, short *c, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (short)((i * 5) % 100);
        b[i] = (short)((i * 13) % 100);
        c[i] = (short)((i * 17) % 50);
    }
}

void init_arrays_char(char *a, char *b, char *c, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (char)((i * 2) % 100);
        b[i] = (char)((i * 19) % 100);
        c[i] = (char)((i * 23) % 50);
    }
}

int main() {
    // Allocate aligned memory for better vectorization
    int *a = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int *b = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int *c = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    
    short *a_short = (short*)aligned_alloc(ALIGN, N * sizeof(short));
    short *b_short = (short*)aligned_alloc(ALIGN, N * sizeof(short));
    short *c_short = (short*)aligned_alloc(ALIGN, N * sizeof(short));
    
    char *a_char = (char*)aligned_alloc(ALIGN, N * sizeof(char));
    char *b_char = (char*)aligned_alloc(ALIGN, N * sizeof(char));
    char *c_char = (char*)aligned_alloc(ALIGN, N * sizeof(char));
    
    if (!a || !b || !c || !a_short || !b_short || !c_short || 
        !a_char || !b_char || !c_char) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize arrays
    init_arrays(a, b, c, N);
    init_arrays_short(a_short, b_short, c_short, N);
    init_arrays_char(a_char, b_char, c_char, N);
    
    // Use __builtin_assume_aligned to give hints to the vectorizer
    a = (int*)__builtin_assume_aligned(a, ALIGN);
    b = (int*)__builtin_assume_aligned(b, ALIGN);
    c = (int*)__builtin_assume_aligned(c, ALIGN);
    
    // Call all test functions to ensure all switch cases are exercised
    int total = 0;
    
    total += test_gt_reduction(a, b, c, N);
    total += test_ge_reduction(a, b, c, N);
    total += test_lt_reduction(a, b, c, N);
    total += test_le_reduction(a, b, c, N);
    
    total += test_gt_reduction_short(a_short, b_short, c_short, N);
    total += test_le_reduction_char(a_char, b_char, c_char, N);
    
    // Print result to prevent dead code elimination
    printf("Total sum: %d\n", total);
    
    // Cleanup
    free(a); free(b); free(c);
    free(a_short); free(b_short); free(c_short);
    free(a_char); free(b_char); free(c_char);
    
    return 0;
}
