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
int test_le(unsigned int *a, unsigned int *b, int n) {
    unsigned int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

/* Mixed comparison types in one loop */
int test_mixed_comparisons(int *a, int *b, int *c, int n) {
    int count = 0;
    for (int i = 0; i < n; i++) {
        // All four comparison types in one loop
        if (a[i] > b[i])  count++;   // GT_EXPR
        if (a[i] >= c[i]) count += 2; // GE_EXPR  
        if (b[i] < c[i])  count += 3; // LT_EXPR
        if (b[i] <= a[i]) count += 4; // LE_EXPR
    }
    return count;
}

/* Conditional assignment using ternary operator */
void test_ternary_gt(int *src1, int *src2, int *dst, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] > src2[i]) ? src1[i] : src2[i];
    }
}

void test_ternary_le(unsigned short *src1, unsigned short *src2, 
                     unsigned short *dst, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] <= src2[i]) ? src1[i] : src2[i];
    }
}

/* Generate mask arrays based on comparisons */
void test_mask_generation(int *a, int threshold, char *mask, int n) {
    for (int i = 0; i < n; i++) {
        mask[i] = (a[i] > threshold) ? 1 : 0;  // GT_EXPR
    }
}

void test_mask_generation_lt(short *a, short threshold, char *mask, int n) {
    for (int i = 0; i < n; i++) {
        mask[i] = (a[i] < threshold) ? 1 : 0;  // LT_EXPR
    }
}

int main() {
    /* Initialize arrays with different data patterns */
    int arr1[N], arr2[N], arr3[N];
    short sarr1[M], sarr2[M];
    char carr1[L], carr2[L];
    unsigned int uarr1[N], uarr2[N];
    unsigned short usarr1[M], usarr2[M];
    int dst[N];
    char mask1[N], mask2[M];
    
    /* Fill arrays with varying data to ensure comparisons are meaningful */
    for (int i = 0; i < N; i++) {
        arr1[i] = i;
        arr2[i] = N - i - 1;  // Reverse order
        arr3[i] = i * 2;
        uarr1[i] = i * 3;
        uarr2[i] = i * 2;
    }
    
    for (int i = 0; i < M; i++) {
        sarr1[i] = i - M/2;  // Mix positive and negative
        sarr2[i] = i * 2;
        usarr1[i] = i * 4;
        usarr2[i] = i * 3;
    }
    
    for (int i = 0; i < L; i++) {
        carr1[i] = i % 128;
        carr2[i] = 64 - (i % 128);  // Mix positive and negative
    }
    
    /* Call all test functions to ensure execution */
    int total = 0;
    
    total += test_gt(arr1, arr2, N);           // GT_EXPR
    total += test_ge(sarr1, sarr2, M);         // GE_EXPR
    total += test_lt(carr1, carr2, L);         // LT_EXPR
    total += test_le(uarr1, uarr2, N);         // LE_EXPR
    total += test_mixed_comparisons(arr1, arr2, arr3, N);
    
    test_ternary_gt(arr1, arr2, dst, N);
    test_ternary_le(usarr1, usarr2, (unsigned short*)dst, M/2);
    
    test_mask_generation(arr1, N/2, mask1, N);
    test_mask_generation_lt(sarr1, 0, mask2, M);
    
    /* Use results to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        total += dst[i];
    }
    for (int i = 0; i < N; i++) {
        total += mask1[i];
    }
    for (int i = 0; i < M; i++) {
        total += mask2[i];
    }
    
    printf("Total result: %d\n", total);
    
    return 0;
}
