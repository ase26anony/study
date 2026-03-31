#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define N 1024
#define ALIGN 64

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
int test_lt_reduction(const short* restrict a, const short* restrict b,
                      const short* restrict c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            sum += c[i];
        }
    }
    return sum;
}

__attribute__((noinline))
int test_le_reduction(const char* restrict a, const char* restrict b,
                      const char* restrict c, int n) {
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
    
    // Multiple conditional reductions with different operators
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

void init_arrays(int* a, int* b, int* c, short* sa, short* sb, short* sc,
                 char* ca, char* cb, char* cc, int n) {
    for (int i = 0; i < n; i++) {
        // Use i and modulo to create varying patterns
        a[i] = (i * 3) % 100;
        b[i] = (i * 5) % 100;
        c[i] = (i * 7) % 50;
        
        sa[i] = (short)((i * 11) % 100);
        sb[i] = (short)((i * 13) % 100);
        sc[i] = (short)((i * 17) % 50);
        
        ca[i] = (char)((i * 19) % 100);
        cb[i] = (char)((i * 23) % 100);
        cc[i] = (char)((i * 29) % 50);
    }
}

int main() {
    // Allocate aligned memory to help vectorizer
    int* a = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* b = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    int* c = (int*)aligned_alloc(ALIGN, N * sizeof(int));
    
    short* sa = (short*)aligned_alloc(ALIGN, N * sizeof(short));
    short* sb = (short*)aligned_alloc(ALIGN, N * sizeof(short));
    short* sc = (short*)aligned_alloc(ALIGN, N * sizeof(short));
    
    char* ca = (char*)aligned_alloc(ALIGN, N * sizeof(char));
    char* cb = (char*)aligned_alloc(ALIGN, N * sizeof(char));
    char* cc = (char*)aligned_alloc(ALIGN, N * sizeof(char));
    
    if (!a || !b || !c || !sa || !sb || !sc || !ca || !cb || !cc) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    // Initialize with pattern that creates varied comparison results
    init_arrays(a, b, c, sa, sb, sc, ca, cb, cc, N);
    
    // Use __builtin_assume_aligned to give hints to the compiler
    a = (int*)__builtin_assume_aligned(a, ALIGN);
    b = (int*)__builtin_assume_aligned(b, ALIGN);
    c = (int*)__builtin_assume_aligned(c, ALIGN);
    
    sa = (short*)__builtin_assume_aligned(sa, ALIGN);
    sb = (short*)__builtin_assume_aligned(sb, ALIGN);
    sc = (short*)__builtin_assume_aligned(sc, ALIGN);
    
    ca = (char*)__builtin_assume_aligned(ca, ALIGN);
    cb = (char*)__builtin_assume_aligned(cb, ALIGN);
    cc = (char*)__builtin_assume_aligned(cc, ALIGN);
    
    int total = 0;
    
    // Test each comparison operator separately
    total += test_gt_reduction(a, b, c, N);
    total += test_ge_reduction(a, b, c, N);
    total += test_lt_reduction(sa, sb, sc, N);
    total += test_le_reduction(ca, cb, cc, N);
    total += test_mixed_comparisons(a, b, c, N);
    
    printf("Total sum: %d\n", total);
    
    // Cleanup
    free(a); free(b); free(c);
    free(sa); free(sb); free(sc);
    free(ca); free(cb); free(cc);
    
    return 0;
}
