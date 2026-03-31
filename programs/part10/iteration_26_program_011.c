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

unsigned int test_ge_unsigned(unsigned short *x, unsigned short *y, int n) {
    unsigned int count = 0;
    for (int i = 0; i < n; i++) {
        if (x[i] >= y[i]) {
            count++;
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

char test_le_char(char *src1, char *src2, char *dst, int n) {
    char sum = 0;
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] <= src2[i]) ? src1[i] : src2[i];
        sum += dst[i];
    }
    return sum;
}

/* Mixed comparisons in a single loop to hit multiple cases */
int test_mixed_comparisons(int *a, int *b, int *c, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            result += 1;  // GT_EXPR
        }
        if (a[i] >= c[i]) {
            result += 2;  // GE_EXPR
        }
        if (b[i] < c[i]) {
            result += 4;  // LT_EXPR
        }
        if (b[i] <= a[i]) {
            result += 8;  // LE_EXPR
        }
    }
    return result;
}

/* Additional tests with different data types and patterns */

int test_gt_ge_mixed(int *data, int *thresholds, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        // Using both > and >= in conditional expression
        int val = (data[i] > thresholds[i]) ? data[i] : 
                 (data[i] >= thresholds[i]/2) ? data[i]/2 : 0;
        sum += val;
    }
    return sum;
}

void test_lt_le_unsigned_char(unsigned char *a, unsigned char *b, 
                              unsigned char *out, int n) {
    for (int i = 0; i < n; i++) {
        // Both < and <= in same loop
        if (a[i] < b[i]) {
            out[i] = a[i];
        } else if (a[i] <= b[i] + 1) {
            out[i] = b[i];
        } else {
            out[i] = 0;
        }
    }
}

int main() {
    // Initialize arrays with varying data to ensure comparisons are meaningful
    int a_int[N], b_int[N], c_int[N];
    unsigned short x_ushort[M], y_ushort[M];
    short arr_short[N];
    char src1_char[N], src2_char[N], dst_char[N];
    unsigned char uc_a[N], uc_b[N], uc_out[N];
    
    // Fill arrays with pseudo-random but deterministic data
    for (int i = 0; i < N; i++) {
        a_int[i] = (i * 73) % 100;      // Varying values
        b_int[i] = (i * 59) % 100;      // Different pattern
        c_int[i] = (i * 83) % 100;
        arr_short[i] = (short)((i * 47) % 200 - 100);  // Mix of positive and negative
        src1_char[i] = (char)((i * 29) % 128);
        src2_char[i] = (char)((i * 17) % 128);
        uc_a[i] = (unsigned char)((i * 31) % 256);
        uc_b[i] = (unsigned char)((i * 67) % 256);
    }
    
    for (int i = 0; i < M; i++) {
        x_ushort[i] = (unsigned short)((i * 53) % 1000);
        y_ushort[i] = (unsigned short)((i * 71) % 1000);
    }
    
    // Call all test functions to ensure execution
    int total = 0;
    
    total += test_gt_int(a_int, b_int, N);           // > comparison
    total += test_ge_unsigned(x_ushort, y_ushort, M); // >= comparison
    total += test_lt_short(arr_short, 0, N);         // < comparison
    total += test_le_char(src1_char, src2_char, dst_char, N); // <= comparison
    
    // Mixed comparisons
    total += test_mixed_comparisons(a_int, b_int, c_int, N);
    total += test_gt_ge_mixed(a_int, b_int, N);
    
    test_lt_le_unsigned_char(uc_a, uc_b, uc_out, N);
    
    // Use output to prevent dead code elimination
    for (int i = 0; i < N; i++) {
        total += uc_out[i];
    }
    
    printf("Result: %d\n", total);
    return 0;
}
