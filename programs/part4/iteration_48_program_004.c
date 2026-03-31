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
    
    // Mix of all four comparisons in separate loops
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

void init_arrays(int* a, int* b, int* c, short* as, short* bs, short* cs,
                 char* ac, char* bc, char* cc, int n) {
    for (int i = 0; i < n; i++) {
        // Use i-based patterns with some randomness-like behavior
        // to prevent compile-time optimization
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
        c[i] = (i * 11) % 50;
        
        as[i] = (short)((i * 5) % 100);
        bs[i] = (short)((i * 13) % 100);
        cs[i] = (short)((i * 17) % 50);
        
        ac[i] = (char)((i * 19) % 100);
        bc[i] = (char)((i * 23) % 100);
        cc[i] = (char)((i * 29) % 50);
    }
}

int main() {
    // Allocate aligned memory to help vectorizer
    int* a = (int*)aligned_alloc(ALIGNMENT, N * sizeof(int));
    int* b = (int*)aligned_alloc(ALIGNMENT, N * sizeof(int));
    int* c = (int*)aligned_alloc(ALIGNMENT, N * sizeof(int));
    
    short* as = (short*)aligned_alloc(ALIGNMENT, N * sizeof(short));
    short* bs = (short*)aligned_alloc(ALIGNMENT, N * sizeof(short));
    short* cs = (short*)aligned_alloc(ALIGNMENT, N * sizeof(short));
    
    char* ac = (char*)aligned_alloc(ALIGNMENT, N * sizeof(char));
    char* bc = (char*)aligned_alloc(ALIGNMENT, N * sizeof(char));
    char* cc = (char*)aligned_alloc(ALIGNMENT, N * sizeof(char));
    
    if (!a || !b || !c || !as || !bs || !cs || !ac || !bc || !cc) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize with data that varies per iteration
    init_arrays(a, b, c, as, bs, cs, ac, bc, cc, N);
    
    // Use __builtin_assume_aligned to give hints to the compiler
    a = (int*)__builtin_assume_aligned(a, ALIGNMENT);
    b = (int*)__builtin_assume_aligned(b, ALIGNMENT);
    c = (int*)__builtin_assume_aligned(c, ALIGNMENT);
    
    // Call all test functions to ensure they're not optimized away
    int total = 0;
    total += test_gt_reduction(a, b, c, N);
    total += test_ge_reduction(as, bs, cs, N);
    total += test_lt_reduction(ac, bc, cc, N);
    total += test_le_reduction(a, b, c, N);
    total += test_mixed_comparisons(a, b, c, N);
    
    printf("Total sum: %d\n", total);
    
    // Free allocated memory
    free(a); free(b); free(c);
    free(as); free(bs); free(cs);
    free(ac); free(bc); free(cc);
    
    return 0;
}
