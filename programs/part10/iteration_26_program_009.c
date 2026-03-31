#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define N 256
#define M 128

// Test functions for each comparison operator

int test_gt_int(int *a, int *b, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {  // GT_EXPR
            sum += a[i];
        }
    }
    return sum;
}

unsigned test_ge_short(short *x, short *y, short threshold, int n) {
    unsigned count = 0;
    for (int i = 0; i < n; i++) {
        if (x[i] >= y[i]) {  // GE_EXPR
            count++;
        }
    }
    return count;
}

int test_lt_char(char *src1, char *src2, char *out, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        // LT_EXPR with result used in computation
        out[i] = (src1[i] < src2[i]) ? src1[i] : src2[i];
        sum += out[i];
    }
    return sum;
}

unsigned test_le_mixed(short *a, int *b, int n) {
    unsigned result = 0;
    for (int i = 0; i < n; i++) {
        // LE_EXPR with type conversion
        if ((int)a[i] <= b[i]) {  // LE_EXPR
            result += a[i];
        }
    }
    return result;
}

// Function using all four operators in one loop
int test_all_comparisons(int *arr1, int *arr2, int *mask, int n) {
    int total = 0;
    for (int i = 0; i < n; i++) {
        // All four comparisons in one loop body
        if (arr1[i] > arr2[i]) {    // GT_EXPR
            mask[i] = 1;
        } else if (arr1[i] >= arr2[i]) {  // GE_EXPR
            mask[i] = 2;
        } else if (arr1[i] < arr2[i]) {   // LT_EXPR
            mask[i] = 3;
        } else if (arr1[i] <= arr2[i]) {  // LE_EXPR
            mask[i] = 4;
        } else {
            mask[i] = 0;
        }
        total += mask[i];
    }
    return total;
}

// Unsigned comparisons to trigger different code paths
unsigned test_unsigned_gt(uint16_t *a, uint16_t *b, int n) {
    unsigned sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {  // GT_EXPR with unsigned
            sum += a[i];
        }
    }
    return sum;
}

unsigned test_unsigned_le(uint8_t *x, uint8_t *y, int n) {
    unsigned count = 0;
    for (int i = 0; i < n; i++) {
        if (x[i] <= y[i]) {  // LE_EXPR with unsigned
            count++;
        }
    }
    return count;
}

// Main driver function
int main() {
    // Initialize arrays with different patterns
    int a_int[N], b_int[N];
    short a_short[M], b_short[M];
    char a_char[N], b_char[N];
    uint16_t a_u16[N], b_u16[N];
    uint8_t a_u8[N], b_u8[N];
    int arr1[N], arr2[N], mask[N];
    char out_char[N];
    
    // Fill arrays with varying data
    for (int i = 0; i < N; i++) {
        a_int[i] = i - N/2;           // Mixed positive/negative
        b_int[i] = (i * 3) % 100;
        a_char[i] = (i % 128) - 64;   // Signed char range
        b_char[i] = (i * 5) % 128;
        a_u16[i] = i * 2;
        b_u16[i] = i * 3;
        a_u8[i] = i % 256;
        b_u8[i] = (i * 7) % 256;
        arr1[i] = i;
        arr2[i] = N - i - 1;
    }
    
    for (int i = 0; i < M; i++) {
        a_short[i] = i * 10;
        b_short[i] = i * 7 + 5;
    }
    
    // Call all test functions
    int result = 0;
    
    result += test_gt_int(a_int, b_int, N);
    result += test_ge_short(a_short, b_short, 50, M);
    result += test_lt_char(a_char, b_char, out_char, N);
    result += test_le_mixed(a_short, a_int, M);
    result += test_all_comparisons(arr1, arr2, mask, N);
    result += test_unsigned_gt(a_u16, b_u16, N);
    result += test_unsigned_le(a_u8, b_u8, N);
    
    // Use the result to prevent dead code elimination
    printf("Final result: %d\n", result);
    
    // Also print some array elements to ensure all loops execute
    printf("Sample outputs: %d %d %d\n", mask[10], out_char[20], (int)a_u16[30]);
    
    return 0;
}
