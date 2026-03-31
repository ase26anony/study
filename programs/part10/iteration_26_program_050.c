#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

/* Test functions for each comparison operator */

int test_gt_int(int *a, int *b, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

unsigned int test_ge_short(short *a, short *b, int n) {
    unsigned int count = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            count++;
        }
    }
    return count;
}

char test_lt_char(char *a, char threshold, int n) {
    char sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] < threshold) {
            sum += a[i];
        }
    }
    return sum;
}

int test_le_mixed(int *a, short *b, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        result += (a[i] <= b[i]) ? a[i] : b[i];
    }
    return result;
}

/* Function using all four operators in one loop */
int test_all_comparisons(int *a, int *b, int *c, int n) {
    int results[4] = {0};
    
    for (int i = 0; i < n; i++) {
        // Each comparison in separate conditional
        if (a[i] > b[i]) results[0] += a[i];
        if (a[i] >= b[i]) results[1] += b[i];
        if (a[i] < c[i]) results[2] += a[i];
        if (a[i] <= c[i]) results[3] += c[i];
    }
    
    return results[0] + results[1] + results[2] + results[3];
}

/* Unsigned comparisons to cover different semantics */
unsigned test_unsigned_gt(unsigned *a, unsigned *b, int n) {
    unsigned sum = 0;
    for (int i = 0; i < n; i++) {
        sum += (a[i] > b[i]) ? a[i] : 0;
    }
    return sum;
}

unsigned test_unsigned_le(unsigned char *a, unsigned char *b, int n) {
    unsigned sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

/* Main driver that uses all test functions */
int main() {
    // Initialize arrays with different patterns
    int arr1[N], arr2[N], arr3[N];
    short sarr1[M], sarr2[M];
    char carr1[N];
    unsigned uarr1[N], uarr2[N];
    unsigned char ucarr1[N], ucarr2[N];
    
    // Fill arrays with varying data
    for (int i = 0; i < N; i++) {
        arr1[i] = i - N/2;          // Mixed positive/negative
        arr2[i] = i * 2;
        arr3[i] = N - i;
        carr1[i] = (i % 128) - 64;  // Range -64..63
        uarr1[i] = i * 3;
        uarr2[i] = i * 2 + 1;
        ucarr1[i] = i % 256;
        ucarr2[i] = (i * 7) % 256;
    }
    
    for (int i = 0; i < M; i++) {
        sarr1[i] = i * 3;
        sarr2[i] = i * 5 - M/2;
    }
    
    // Call all test functions
    int total = 0;
    
    total += test_gt_int(arr1, arr2, N);
    total += test_ge_short(sarr1, sarr2, M);
    total += test_lt_char(carr1, 0, N);  // Threshold = 0
    total += test_le_mixed(arr1, sarr2, (N < M) ? N : M);
    total += test_all_comparisons(arr1, arr2, arr3, N);
    total += test_unsigned_gt(uarr1, uarr2, N);
    total += test_unsigned_le(ucarr1, ucarr2, N);
    
    printf("Total result: %d\n", total);
    
    // Additional loops with different lengths to encourage vectorization
    int small_arr[16], small_arr2[16];
    for (int i = 0; i < 16; i++) {
        small_arr[i] = i;
        small_arr2[i] = 15 - i;
    }
    
    int small_sum = 0;
    // Loop with < comparison (should trigger std::swap)
    for (int i = 0; i < 16; i++) {
        if (small_arr[i] < small_arr2[i]) {
            small_sum += small_arr[i];
        }
    }
    
    // Loop with <= comparison (should trigger std::swap)
    for (int i = 0; i < 16; i++) {
        if (small_arr[i] <= small_arr2[i]) {
            small_sum += small_arr2[i];
        }
    }
    
    printf("Small sum: %d\n", small_sum);
    
    return total + small_sum;
}
