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
        if (a[i] > b[i]) {  // GT_EXPR case
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
        if (a[i] >= b[i]) {  // GE_EXPR case
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
        if (a[i] < b[i]) {  // LT_EXPR case
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
        if (a[i] <= b[i]) {  // LE_EXPR case
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline))
int test_mixed_reductions(const int* restrict a, const int* restrict b,
                          const int* restrict c, int n) {
    int sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    
    // Mix all four comparisons in one function
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) sum1 += c[i];   // GT_EXPR
        if (a[i] >= b[i]) sum2 += c[i];  // GE_EXPR
        if (a[i] < b[i]) sum3 += c[i];   // LT_EXPR
        if (a[i] <= b[i]) sum4 += c[i];  // LE_EXPR
    }
    
    return sum1 + sum2 + sum3 + sum4;
}

void init_arrays(int* a, int* b, int* c, short* sa, short* sb, short* sc,
                 char* ca, char* cb, char* cc, int n) {
    // Initialize with semi-random but reproducible patterns
    for (int i = 0; i < n; i++) {
        a[i] = (i * 17) % 100;
        b[i] = (i * 13) % 100;
        c[i] = (i * 19) % 50;
        
        sa[i] = (short)((i * 23) % 100);
        sb[i] = (short)((i * 29) % 100);
        sc[i] = (short)((i * 31) % 50);
        
        ca[i] = (char)((i * 37) % 100);
        cb[i] = (char)((i * 41) % 100);
        cc[i] = (char)((i * 43) % 50);
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
    
    // Initialize arrays with data
    init_arrays(a, b, c, sa, sb, sc, ca, cb, cc, N);
    
    // Use __builtin_assume_aligned to provide alignment hints
    a = (int*)__builtin_assume_aligned(a, ALIGNMENT);
    b = (int*)__builtin_assume_aligned(b, ALIGNMENT);
    c = (int*)__builtin_assume_aligned(c, ALIGNMENT);
    sa = (short*)__builtin_assume_aligned(sa, ALIGNMENT);
    sb = (short*)__builtin_assume_aligned(sb, ALIGNMENT);
    sc = (short*)__builtin_assume_aligned(sc, ALIGNMENT);
    ca = (char*)__builtin_assume_aligned(ca, ALIGNMENT);
    cb = (char*)__builtin_assume_aligned(cb, ALIGNMENT);
    cc = (char*)__builtin_assume_aligned(cc, ALIGNMENT);
    
    // Call all test functions to ensure they're not optimized away
    int total = 0;
    
    total += test_gt_reduction(a, b, c, N);      // > comparison
    total += test_ge_reduction(sa, sb, sc, N);   // >= comparison
    total += test_lt_reduction(ca, cb, cc, N);   // < comparison
    total += test_le_reduction(a, b, c, N);      // <= comparison
    total += test_mixed_reductions(a, b, c, N);  // All comparisons mixed
    
    // Use the result to prevent dead code elimination
    printf("Total sum: %d\n", total);
    
    // Cleanup
    free(a); free(b); free(c);
    free(sa); free(sb); free(sc);
    free(ca); free(cb); free(cc);
    
    return 0;
}
