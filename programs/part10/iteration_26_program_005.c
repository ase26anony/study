#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

/* Test functions for each comparison operator */

int test_gt(int *a, int *b) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

int test_ge(short *a, short *b) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            sum += b[i];
        }
    }
    return sum;
}

int test_lt(unsigned char *a, unsigned char *b) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

int test_le(int *a, int threshold) {
    int count = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] <= threshold) {
            count++;
        }
    }
    return count;
}

/* Mixed comparison in single loop */
int test_mixed_comparisons(int *a, int *b, int *c, int *d) {
    int result = 0;
    for (int i = 0; i < M; i++) {
        // Using all four comparison operators in one loop
        if (a[i] > b[i]) result += 1;
        if (a[i] >= c[i]) result += 2;
        if (b[i] < d[i]) result += 4;
        if (c[i] <= d[i]) result += 8;
    }
    return result;
}

/* Conditional assignment using ternary operator */
void test_ternary_gt(short *src1, short *src2, short *dst) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] > src2[i]) ? src1[i] : src2[i];
    }
}

void test_ternary_le(unsigned int *src1, unsigned int *src2, unsigned int *dst) {
    for (int i = 0; i < M; i++) {
        dst[i] = (src1[i] <= src2[i]) ? src1[i] : src2[i];
    }
}

/* Generate mask based on comparison */
void test_mask_generation(int *a, int *b, char *mask) {
    for (int i = 0; i < N; i++) {
        mask[i] = (a[i] < b[i]) ? 1 : 0;
    }
}

/* Test with negative values */
int test_negative_values(int *a, int *b) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] <= b[i]) {
            sum -= a[i];
        } else {
            sum += b[i];
        }
    }
    return sum;
}

/* Main driver function */
int main() {
    // Initialize arrays with different patterns
    int a_int[N], b_int[N], c_int[M], d_int[M];
    short a_short[N], b_short[N];
    unsigned char a_uchar[N], b_uchar[N];
    short src1_short[N], src2_short[N], dst_short[N];
    unsigned int src1_uint[M], src2_uint[M], dst_uint[M];
    char mask[N];
    
    // Fill arrays with varying data
    for (int i = 0; i < N; i++) {
        a_int[i] = i - N/2;          // Mix of negative and positive
        b_int[i] = i % 100;
        a_short[i] = i * 2;
        b_short[i] = i * 3;
        a_uchar[i] = i % 256;
        b_uchar[i] = (i * 7) % 256;
        src1_short[i] = i;
        src2_short[i] = N - i;
    }
    
    for (int i = 0; i < M; i++) {
        c_int[i] = i * 3;
        d_int[i] = i * 5;
        src1_uint[i] = i * 100;
        src2_uint[i] = i * 75;
    }
    
    int total = 0;
    
    // Call all test functions
    total += test_gt(a_int, b_int);
    total += test_ge(a_short, b_short);
    total += test_lt(a_uchar, b_uchar);
    total += test_le(a_int, 50);
    total += test_mixed_comparisons(a_int, b_int, c_int, d_int);
    
    test_ternary_gt(src1_short, src2_short, dst_short);
    test_ternary_le(src1_uint, src2_uint, dst_uint);
    test_mask_generation(a_int, b_int, mask);
    total += test_negative_values(a_int, b_int);
    
    // Use results to prevent dead code elimination
    for (int i = 0; i < N; i++) {
        total += dst_short[i] + mask[i];
    }
    for (int i = 0; i < M; i++) {
        total += dst_uint[i];
    }
    
    printf("Total result: %d\n", total);
    return 0;
}
