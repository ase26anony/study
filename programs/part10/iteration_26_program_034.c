#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

/* Test functions for each comparison operator */

int test_gt_int(int *a, int *b) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

int test_ge_short(short *a, short *b) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += b[i];
        }
    }
    return sum;
}

int test_lt_char(char *a, char *b) {
    int sum = 0;
    for (int i = 0; i < M; i++) {
        if (a[i] < b[i]) {
            sum += 1;
        }
    }
    return sum;
}

int test_le_mixed(int *a, int *b, char *mask) {
    int sum = 0;
    for (int i = 0; i < M; i++) {
        mask[i] = (a[i] <= b[i]) ? 1 : 0;
        sum += mask[i];
    }
    return sum;
}

/* Test with unsigned types */
unsigned test_gt_unsigned(unsigned *a, unsigned *b) {
    unsigned sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            sum += a[i] - b[i];
        }
    }
    return sum;
}

/* Test with all four operators in one loop */
int test_all_comparisons(int *a, int *b, int *c, int *d) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        // Use all four comparison operators
        if (a[i] > b[i]) sum += 1;      // GT_EXPR
        if (c[i] >= d[i]) sum += 2;     // GE_EXPR
        if (b[i] < c[i]) sum += 3;      // LT_EXPR (will swap operands)
        if (d[i] <= a[i]) sum += 4;     // LE_EXPR (will swap operands)
    }
    return sum;
}

/* Test with conditional assignment (ternary operator) */
void test_ternary_gt(int *a, int *b, int *out) {
    for (int i = 0; i < N; i++) {
        out[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
}

void test_ternary_le(short *a, short *b, short *out) {
    for (int i = 0; i < N; i++) {
        out[i] = (a[i] <= b[i]) ? a[i] : b[i];
    }
}

int main() {
    // Initialize arrays with different patterns
    int a_int[N], b_int[N], c_int[N], d_int[N];
    short a_short[N], b_short[N];
    char a_char[M], b_char[M];
    unsigned a_unsigned[N], b_unsigned[N];
    char mask[M];
    int out_int[N];
    short out_short[N];
    
    // Fill arrays with varying data to ensure comparisons are meaningful
    for (int i = 0; i < N; i++) {
        a_int[i] = i;
        b_int[i] = N - i;
        c_int[i] = i * 2;
        d_int[i] = i / 2;
        a_short[i] = (short)(i * 3);
        b_short[i] = (short)(i * 5);
        a_unsigned[i] = (unsigned)(i * 7);
        b_unsigned[i] = (unsigned)(i * 11);
    }
    
    for (int i = 0; i < M; i++) {
        a_char[i] = (char)(i - M/2);  // Mix positive and negative
        b_char[i] = (char)(M/2 - i);
    }
    
    // Call all test functions
    int result = 0;
    
    result += test_gt_int(a_int, b_int);
    result += test_ge_short(a_short, b_short);
    result += test_lt_char(a_char, b_char);
    result += test_le_mixed(a_int, b_int, mask);
    result += test_gt_unsigned(a_unsigned, b_unsigned);
    result += test_all_comparisons(a_int, b_int, c_int, d_int);
    
    test_ternary_gt(a_int, b_int, out_int);
    test_ternary_le(a_short, b_short, out_short);
    
    // Use results to prevent dead code elimination
    for (int i = 0; i < N; i++) {
        result += out_int[i];
        result += out_short[i];
    }
    
    for (int i = 0; i < M; i++) {
        result += mask[i];
    }
    
    printf("Final result: %d\n", result);
    return 0;
}
