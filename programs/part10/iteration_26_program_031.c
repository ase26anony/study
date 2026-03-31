#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define N 256
#define M 128
#define L 512

/* Test functions for each comparison operator */

/* Greater-than (GT_EXPR) */
int test_gt(int *a, int *b, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

/* Greater-than-or-equal (GE_EXPR) */
int test_ge(short *a, short *b, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            sum += b[i];
        }
    }
    return sum;
}

/* Less-than (LT_EXPR) */
int test_lt(char *a, char *b, int n) {
    int count = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            count++;
        }
    }
    return count;
}

/* Less-than-or-equal (LE_EXPR) */
int test_le(unsigned int *a, unsigned int *b, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
            sum += 1;
        }
    }
    return sum;
}

/* Mixed comparisons in one loop to potentially trigger multiple cases */
int test_mixed_comparisons(int *a, int *b, int *c, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        // Using ternary operators with different comparisons
        int val1 = (a[i] > b[i]) ? a[i] : b[i];      // GT_EXPR
        int val2 = (b[i] >= c[i]) ? b[i] : c[i];     // GE_EXPR  
        int val3 = (c[i] < a[i]) ? c[i] : a[i];      // LT_EXPR
        int val4 = (a[i] <= b[i]) ? a[i] : b[i];     // LE_EXPR
        
        result += val1 + val2 + val3 + val4;
    }
    return result;
}

/* Conditional store based on comparisons */
void test_conditional_store(int *src, int *dst, int threshold, int n) {
    for (int i = 0; i < n; i++) {
        // Multiple comparison types in store conditions
        dst[i] = (src[i] > threshold) ? src[i] : 0;      // GT_EXPR
        dst[i] += (src[i] >= threshold/2) ? 1 : 0;       // GE_EXPR
        dst[i] += (src[i] < -threshold) ? 100 : 0;       // LT_EXPR
        dst[i] += (src[i] <= threshold*2) ? 10 : 0;      // LE_EXPR
    }
}

/* Test with unsigned short to trigger different vectorization */
unsigned test_unsigned_comparisons(unsigned short *a, unsigned short *b, int n) {
    unsigned sum = 0;
    for (int i = 0; i < n; i++) {
        // Mix of signed and unsigned-like comparisons
        if (a[i] > b[i]) sum += a[i];        // GT_EXPR
        if (a[i] >= b[i]) sum += b[i];       // GE_EXPR
        if (a[i] < b[i]) sum += 1;           // LT_EXPR  
        if (a[i] <= b[i]) sum += 2;          // LE_EXPR
    }
    return sum;
}

int main() {
    // Initialize arrays with different patterns
    int arr1[N], arr2[N], arr3[N];
    short sarr1[M], sarr2[M];
    char carr1[L], carr2[L];
    unsigned int uarr1[N], uarr2[N];
    unsigned short usarr1[M], usarr2[M];
    int dst[N];
    
    // Fill arrays with varying data to ensure comparisons are meaningful
    for (int i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = N - i;
        arr3[i] = i * 2;
        uarr1[i] = i * 3;
        uarr2[i] = i * 4;
    }
    
    for (int i = 0; i < M; i++) {
        sarr1[i] = i - M/2;
        sarr2[i] = i * 2;
        usarr1[i] = i * 3;
        usarr2[i] = i * 5;
    }
    
    for (int i = 0; i < L; i++) {
        carr1[i] = i % 128;
        carr2[i] = (i * 2) % 128;
    }
    
    // Call all test functions to ensure execution
    int total = 0;
    
    total += test_gt(arr1, arr2, N);          // GT_EXPR
    total += test_ge(sarr1, sarr2, M);        // GE_EXPR
    total += test_lt(carr1, carr2, L);        // LT_EXPR
    total += test_le(uarr1, uarr2, N);        // LE_EXPR
    
    total += test_mixed_comparisons(arr1, arr2, arr3, N);
    
    test_conditional_store(arr1, dst, 100, N);
    for (int i = 0; i < N; i++) {
        total += dst[i];
    }
    
    total += test_unsigned_comparisons(usarr1, usarr2, M);
    
    printf("Total result: %d\n", total);
    
    // Additional verification to prevent dead code elimination
    if (total > 0) {
        printf("Vectorization test completed.\n");
    }
    
    return 0;
}
