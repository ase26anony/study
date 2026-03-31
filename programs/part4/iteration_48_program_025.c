#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define N 1024
#define ALIGNMENT 64

__attribute__((noinline, optimize("O3")))
int test_gt_reduction(const int* restrict a, const int* restrict b, const int* restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline, optimize("O3")))
int test_ge_reduction(const int* restrict a, const int* restrict b, const int* restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline, optimize("O3")))
int test_lt_reduction(const short* restrict a, const short* restrict b, const short* restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline, optimize("O3")))
int test_le_reduction(const char* restrict a, const char* restrict b, const char* restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline, optimize("O3")))
int test_mixed_reductions(const int* restrict a, const int* restrict b, const int* restrict c, int n) {
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    
    // Test all four comparisons in one function to ensure coverage
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            sum1 += c[i];
        }
        if (a[i] >= b[i]) {
            sum2 += c[i];
        }
        if (a[i] < b[i]) {
            sum3 += c[i];
        }
        if (a[i] <= b[i]) {
            sum4 += c[i];
        }
    }
    
    return sum1 + sum2 + sum3 + sum4;
}

__attribute__((noinline))
void init_arrays(int* a, int* b, int* c, short* sa, short* sb, short* sc, 
                 char* ca, char* cb, char* cc, int n) {
    for (int i = 0; i < n; i++) {
        // Use varying patterns to prevent compile-time optimization
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
        c[i] = (i * 11) % 50;
        
        sa[i] = (short)((i * 5) % 100);
        sb[i] = (short)((i * 13) % 100);
        sc[i] = (short)((i * 17) % 50);
        
        ca[i] = (char)((i * 19) % 100);
        cb[i] = (char)((i * 23) % 100);
        cc[i] = (char)((i * 29) % 50);
    }
}

int main() {
    // Allocate aligned memory to help vectorizer
    int* a = (int*)aligned_alloc(ALIGNMENT, N * sizeof(int));
    int* b = (int*)aligned_alloc(ALIGNMENT, N * sizeof(int));
    int* c = (int*)aligned_alloc(ALIGNMENT, N * sizeof(int));
    
    short* sa = (short*)aligned_alloc(ALIGNMENT, N * sizeof(short));
    short* sb = (short*)aligned_alloc(ALIGNMENT, N * sizeof(short));
    short* sc = (short*)aligned_alloc(ALIGNMENT, N * sizeof(short));
    
    char* ca = (char*)aligned_alloc(ALIGNMENT, N * sizeof(char));
    char* cb = (char*)aligned_alloc(ALIGNMENT, N * sizeof(char));
    char* cc = (char*)aligned_alloc(ALIGNMENT, N * sizeof(char));
    
    if (!a || !b || !c || !sa || !sb || !sc || !ca || !cb || !cc) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize arrays with non-constant data
    init_arrays(a, b, c, sa, sb, sc, ca, cb, cc, N);
    
    // Use __builtin_assume_aligned to give hints to the vectorizer
    a = (int*)__builtin_assume_aligned(a, ALIGNMENT);
    b = (int*)__builtin_assume_aligned(b, ALIGNMENT);
    c = (int*)__builtin_assume_aligned(c, ALIGNMENT);
    
    sa = (short*)__builtin_assume_aligned(sa, ALIGNMENT);
    sb = (short*)__builtin_assume_aligned(sb, ALIGNMENT);
    sc = (short*)__builtin_assume_aligned(sc, ALIGNMENT);
    
    ca = (char*)__builtin_assume_aligned(ca, ALIGNMENT);
    cb = (char*)__builtin_assume_aligned(cb, ALIGNMENT);
    cc = (char*)__builtin_assume_aligned(cc, ALIGNMENT);
    
    // Call all test functions to ensure coverage of all four cases
    int total = 0;
    
    total += test_gt_reduction(a, b, c, N);
    total += test_ge_reduction(a, b, c, N);
    total += test_lt_reduction(sa, sb, sc, N);
    total += test_le_reduction(ca, cb, cc, N);
    total += test_mixed_reductions(a, b, c, N);
    
    printf("Total result: %d\n", total);
    printf("(This value varies based on array contents)\n");
    
    // Free allocated memory
    free(a); free(b); free(c);
    free(sa); free(sb); free(sc);
    free(ca); free(cb); free(cc);
    
    return 0;
}
