#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128
#define L 512

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

unsigned int test_ge_unsigned(unsigned short *x, unsigned short *y, int n) {
    unsigned int count = 0;
    for (int i = 0; i < n; i++) {
        if (x[i] >= y[i]) {
            count += 1;
        }
    }
    return count;
}

short test_lt_short(short *arr, short threshold, int n) {
    short result = 0;
    for (int i = 0; i < n; i++) {
        if (arr[i] < threshold) {
            result += arr[i];
        }
    }
    return result;
}

char test_le_char(char *data1, char *data2, int n) {
    char diff = 0;
    for (int i = 0; i < n; i++) {
        if (data1[i] <= data2[i]) {
            diff += (data1[i] - data2[i]);
        }
    }
    return diff;
}

/* Mixed comparisons in a single loop */
int test_mixed_comparisons(int *a, int *b, int *c, int *d, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        // Use all four comparison operators in different branches
        if (a[i] > b[i]) {
            result += 1;
        }
        if (a[i] >= c[i]) {
            result += 2;
        }
        if (b[i] < d[i]) {
            result += 4;
        }
        if (c[i] <= d[i]) {
            result += 8;
        }
    }
    return result;
}

/* Conditional assignment using ternary operator */
void test_ternary_gt(int *src, int *dst, int threshold, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src[i] > threshold) ? src[i] : threshold;
    }
}

void test_ternary_le(unsigned char *src, unsigned char *dst, unsigned char limit, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src[i] <= limit) ? src[i] : 0;
    }
}

int main() {
    /* Initialize arrays with different patterns */
    int a_int[N], b_int[N];
    unsigned short x_ushort[M], y_ushort[M];
    short arr_short[L];
    char data1_char[N], data2_char[N];
    int mixed_a[N], mixed_b[N], mixed_c[N], mixed_d[N];
    int src_int[M], dst_int[M];
    unsigned char src_uchar[N], dst_uchar[N];
    
    /* Initialize with varied data to ensure comparisons hit different cases */
    for (int i = 0; i < N; i++) {
        a_int[i] = i - N/2;          /* Mix of negative and positive */
        b_int[i] = i % 100 - 50;     /* Different range */
        data1_char[i] = (i % 128) - 64;
        data2_char[i] = (i % 96) - 32;
        mixed_a[i] = i;
        mixed_b[i] = N - i;
        mixed_c[i] = i * 2;
        mixed_d[i] = i / 2;
        src_uchar[i] = i % 256;
        dst_uchar[i] = 0;
    }
    
    for (int i = 0; i < M; i++) {
        x_ushort[i] = i * 3;
        y_ushort[i] = i * 2 + 1;
        src_int[i] = i * 5;
        dst_int[i] = 0;
    }
    
    for (int i = 0; i < L; i++) {
        arr_short[i] = (i % 200) - 100;
    }
    
    /* Call all test functions to ensure execution */
    int total = 0;
    
    total += test_gt_int(a_int, b_int, N);
    total += test_ge_unsigned(x_ushort, y_ushort, M);
    total += test_lt_short(arr_short, 0, L);  /* Threshold = 0 */
    total += test_le_char(data1_char, data2_char, N);
    total += test_mixed_comparisons(mixed_a, mixed_b, mixed_c, mixed_d, N);
    
    test_ternary_gt(src_int, dst_int, 100, M);
    test_ternary_le(src_uchar, dst_uchar, 128, N);
    
    /* Use results to prevent dead code elimination */
    for (int i = 0; i < M; i++) {
        total += dst_int[i];
    }
    
    for (int i = 0; i < N; i++) {
        total += dst_uchar[i];
    }
    
    printf("Result: %d\n", total);
    return total != 0 ? 0 : 1;
}
