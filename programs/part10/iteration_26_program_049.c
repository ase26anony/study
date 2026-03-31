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

int test_ge_short(short *a, short *b, int n) {
    int count = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            count++;
        }
    }
    return count;
}

void test_lt_char(char *src, char *dst, char threshold, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src[i] < threshold) ? src[i] : 0;
    }
}

unsigned int test_le_mixed(unsigned char *a, unsigned char *b, int n) {
    unsigned int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += (a[i] <= b[i]) ? a[i] : b[i];
    }
    return sum;
}

/* Combined test with all four operators in one loop */
int test_all_comparisons(int *a, int *b, int *c, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        // Using all four comparison operators
        if (a[i] > b[i]) result += 1;
        if (a[i] >= b[i]) result += 2;
        if (a[i] < c[i]) result += 4;
        if (a[i] <= c[i]) result += 8;
    }
    return result;
}

/* Additional tests with different data types and patterns */

int test_gt_unsigned(unsigned int *a, unsigned int *b, int n) {
    unsigned int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    return (int)sum;
}

void test_lt_short_with_mask(short *src, short *dst, short *mask, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src[i] < mask[i]) ? src[i] : mask[i];
    }
}

int test_le_int_conditional(int *a, int *b, int *c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        // Complex conditional using <=
        sum += (a[i] <= b[i]) ? c[i] : -c[i];
    }
    return sum;
}

int main() {
    // Initialize arrays with different patterns
    int a_int[N], b_int[N], c_int[N];
    short a_short[M], b_short[M];
    char a_char[N], dst_char[N];
    unsigned char a_uchar[N], b_uchar[N];
    unsigned int a_uint[N], b_uint[N];
    short src_short[M], dst_short[M], mask_short[M];
    
    // Fill arrays with varying data
    for (int i = 0; i < N; i++) {
        a_int[i] = i * 3;
        b_int[i] = i * 2;
        c_int[i] = i * 4;
        a_char[i] = (char)(i - 128);
        dst_char[i] = 0;
        a_uchar[i] = (unsigned char)(i % 256);
        b_uchar[i] = (unsigned char)((i * 7) % 256);
        a_uint[i] = (unsigned int)(i * 5);
        b_uint[i] = (unsigned int)(i * 3);
    }
    
    for (int i = 0; i < M; i++) {
        a_short[i] = (short)(i * 2 - 64);
        b_short[i] = (short)(i * 3 - 96);
        src_short[i] = (short)(i * 4);
        mask_short[i] = (short)(i * 5);
        dst_short[i] = 0;
    }
    
    int total_result = 0;
    
    // Execute all test functions
    total_result += test_gt_int(a_int, b_int, N);
    total_result += test_ge_short(a_short, b_short, M);
    
    test_lt_char(a_char, dst_char, 0, N);
    for (int i = 0; i < N; i++) {
        total_result += dst_char[i];
    }
    
    total_result += test_le_mixed(a_uchar, b_uchar, N);
    total_result += test_all_comparisons(a_int, b_int, c_int, N);
    total_result += test_gt_unsigned(a_uint, b_uint, N);
    
    test_lt_short_with_mask(src_short, dst_short, mask_short, M);
    for (int i = 0; i < M; i++) {
        total_result += dst_short[i];
    }
    
    total_result += test_le_int_conditional(a_int, b_int, c_int, N);
    
    // Print final result to prevent dead code elimination
    printf("Total result: %d\n", total_result);
    
    // Additional verification prints
    printf("Test completed with array sizes: N=%d, M=%d\n", N, M);
    
    return 0;
}
