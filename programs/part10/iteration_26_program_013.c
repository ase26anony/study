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
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

/* Less-than-or-equal (LE_EXPR) */
int test_le(unsigned short *a, unsigned short *b, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
            sum += b[i];
        }
    }
    return sum;
}

/* Mixed comparisons in one loop to trigger multiple cases */
int test_mixed_comparisons(int *arr1, int *arr2, int *thresholds, int n) {
    int results[4] = {0};
    
    for (int i = 0; i < n; i++) {
        // GT_EXPR case
        if (arr1[i] > thresholds[0]) {
            results[0] += arr1[i];
        }
        
        // GE_EXPR case  
        if (arr1[i] >= thresholds[1]) {
            results[1] += arr2[i];
        }
        
        // LT_EXPR case
        if (arr2[i] < thresholds[2]) {
            results[2] += arr1[i];
        }
        
        // LE_EXPR case
        if (arr2[i] <= thresholds[3]) {
            results[3] += arr2[i];
        }
    }
    
    return results[0] + results[1] + results[2] + results[3];
}

/* Conditional assignment using ternary operator */
void test_ternary_gt(int *a, int *b, int *out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
}

void test_ternary_le(unsigned char *a, unsigned char *b, unsigned char *out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = (a[i] <= b[i]) ? a[i] : b[i];
    }
}

/* Function with different loop lengths and data types */
int test_various_sizes() {
    int total = 0;
    
    // Small loop with char
    char c1[64], c2[64];
    for (int i = 0; i < 64; i++) {
        c1[i] = i - 32;
        c2[i] = i * 2 - 32;
    }
    
    for (int i = 0; i < 64; i++) {
        if (c1[i] < c2[i]) {  // LT_EXPR
            total += c1[i];
        }
    }
    
    // Medium loop with short
    short s1[128], s2[128];
    for (int i = 0; i < 128; i++) {
        s1[i] = i * 3;
        s2[i] = i * 2;
    }
    
    for (int i = 0; i < 128; i++) {
        if (s1[i] >= s2[i]) {  // GE_EXPR
            total += s2[i];
        }
    }
    
    // Large loop with int
    int i1[256], i2[256];
    for (int i = 0; i < 256; i++) {
        i1[i] = i * 5;
        i2[i] = i * 3;
    }
    
    for (int i = 0; i < 256; i++) {
        if (i1[i] > i2[i]) {  // GT_EXPR
            total += i1[i];
        }
    }
    
    return total;
}

int main() {
    // Initialize arrays with varying data
    int arr1[N], arr2[N];
    short sarr1[M], sarr2[M];
    char carr1[L], carr2[L];
    unsigned short usarr1[N], usarr2[N];
    unsigned char ucarr1[M], ucarr2[M];
    int thresholds[4] = {100, 200, 300, 400};
    int out[N];
    
    // Fill arrays with pseudo-random but deterministic data
    for (int i = 0; i < N; i++) {
        arr1[i] = (i * 17) % 1000;
        arr2[i] = (i * 23) % 1000;
        usarr1[i] = (i * 7) % 65535;
        usarr2[i] = (i * 11) % 65535;
    }
    
    for (int i = 0; i < M; i++) {
        sarr1[i] = (i * 13) % 32767;
        sarr2[i] = (i * 19) % 32767;
        ucarr1[i] = (i * 29) % 255;
        ucarr2[i] = (i * 31) % 255;
    }
    
    for (int i = 0; i < L; i++) {
        carr1[i] = (i * 37) % 127;
        carr2[i] = (i * 41) % 127;
    }
    
    // Call all test functions to ensure execution
    int total = 0;
    
    total += test_gt(arr1, arr2, N);           // GT_EXPR
    total += test_ge(sarr1, sarr2, M);         // GE_EXPR  
    total += test_lt(carr1, carr2, L);         // LT_EXPR
    total += test_le(usarr1, usarr2, N);       // LE_EXPR
    
    total += test_mixed_comparisons(arr1, arr2, thresholds, N);
    
    test_ternary_gt(arr1, arr2, out, N);
    for (int i = 0; i < N; i++) {
        total += out[i];
    }
    
    test_ternary_le(ucarr1, ucarr2, (unsigned char*)out, M);
    for (int i = 0; i < M; i++) {
        total += ((unsigned char*)out)[i];
    }
    
    total += test_various_sizes();
    
    // Print result to prevent dead code elimination
    printf("Total result: %d\n", total);
    
    return 0;
}
