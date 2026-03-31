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
    int count = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            count++;
        }
    }
    return count;
}

/* Less-than (LT_EXPR) */
int test_lt(char *a, char *b, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            sum += b[i];
        }
    }
    return sum;
}

/* Less-than-or-equal (LE_EXPR) */
int test_le(unsigned int *a, unsigned int *b, int n) {
    unsigned int mask_sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
            mask_sum |= 1 << (i % 32);
        }
    }
    return (int)mask_sum;
}

/* Mixed comparisons in one loop to potentially trigger multiple cases */
int test_mixed_comparisons(int *arr1, int *arr2, int *thresholds, int n) {
    int results[4] = {0};
    
    for (int i = 0; i < n; i++) {
        // GT_EXPR
        if (arr1[i] > thresholds[0]) {
            results[0] += arr1[i];
        }
        
        // GE_EXPR
        if (arr2[i] >= thresholds[1]) {
            results[1] += arr2[i];
        }
        
        // LT_EXPR
        if (arr1[i] < thresholds[2]) {
            results[2] += thresholds[2];
        }
        
        // LE_EXPR
        if (arr2[i] <= thresholds[3]) {
            results[3] += thresholds[3];
        }
    }
    
    return results[0] + results[1] + results[2] + results[3];
}

/* Conditional assignment using ternary operator */
void test_ternary_gt(int *src, int *dst, int threshold, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src[i] > threshold) ? src[i] : threshold;
    }
}

void test_ternary_le(unsigned short *src, unsigned short *dst, 
                     unsigned short threshold, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src[i] <= threshold) ? src[i] : threshold;
    }
}

/* Different data types and loop lengths */
int test_various_types() {
    int total = 0;
    
    // char arrays with LT_EXPR
    char c1[M], c2[M];
    for (int i = 0; i < M; i++) {
        c1[i] = (char)(i - 64);
        c2[i] = (char)(i * 2 - 64);
    }
    total += test_lt(c1, c2, M);
    
    // short arrays with GE_EXPR
    short s1[N], s2[N];
    for (int i = 0; i < N; i++) {
        s1[i] = (short)(i * 3);
        s2[i] = (short)(i * 2);
    }
    total += test_ge(s1, s2, N);
    
    // int arrays with GT_EXPR
    int i1[L], i2[L];
    for (int i = 0; i < L; i++) {
        i1[i] = i * 5;
        i2[i] = i * 3 + 10;
    }
    total += test_gt(i1, i2, L/2);  // Different loop length
    
    // unsigned arrays with LE_EXPR
    unsigned int u1[N], u2[N];
    for (int i = 0; i < N; i++) {
        u1[i] = (unsigned int)(i * 7);
        u2[i] = (unsigned int)(i * 4 + 5);
    }
    total += test_le(u1, u2, N);
    
    return total;
}

int main() {
    int result = 0;
    
    // Initialize test data
    int arr1[N], arr2[N], thresholds[4];
    short sarr1[M], sarr2[M];
    char carr1[L], carr2[L];
    unsigned int uarr1[N], uarr2[N];
    unsigned short usarr1[M], usarr2[M];
    int dst[N];
    
    // Fill arrays with varied data
    for (int i = 0; i < N; i++) {
        arr1[i] = i * 3 - 128;
        arr2[i] = i * 2 + 64;
        uarr1[i] = (unsigned int)(i * 5);
        uarr2[i] = (unsigned int)(i * 3 + 100);
        dst[i] = 0;
    }
    
    for (int i = 0; i < M; i++) {
        sarr1[i] = (short)(i * 4 - 256);
        sarr2[i] = (short)(i * 3 + 128);
        usarr1[i] = (unsigned short)(i * 6);
        usarr2[i] = (unsigned short)(i * 2 + 50);
    }
    
    for (int i = 0; i < L; i++) {
        carr1[i] = (char)((i % 128) - 64);
        carr2[i] = (char)((i % 128) * 2 - 64);
    }
    
    thresholds[0] = 100;
    thresholds[1] = -50;
    thresholds[2] = 200;
    thresholds[3] = 300;
    
    // Execute all test functions
    result += test_gt(arr1, arr2, N);
    result += test_ge(sarr1, sarr2, M);
    result += test_lt(carr1, carr2, L);
    result += test_le(uarr1, uarr2, N);
    
    // Test mixed comparisons
    result += test_mixed_comparisons(arr1, arr2, thresholds, N/2);
    
    // Test ternary operations
    test_ternary_gt(arr1, dst, 50, N);
    for (int i = 0; i < N; i++) {
        result += dst[i];
    }
    
    test_ternary_le(usarr1, (unsigned short*)dst, 1000, M);
    for (int i = 0; i < M; i++) {
        result += ((unsigned short*)dst)[i];
    }
    
    // Test various types
    result += test_various_types();
    
    // Print result to prevent dead code elimination
    printf("Final result: %d\n", result);
    
    return 0;
}
