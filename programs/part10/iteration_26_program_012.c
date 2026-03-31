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

unsigned test_ge_short(short *x, short *y, int n) {
    unsigned count = 0;
    for (int i = 0; i < n; i++) {
        if (x[i] >= y[i]) {
            count++;
        }
    }
    return count;
}

void test_lt_char(char *src, char threshold, char *out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = (src[i] < threshold) ? src[i] : 0;
    }
}

int test_le_mixed(int *arr1, int *arr2, char *mask, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        mask[i] = (arr1[i] <= arr2[i]) ? 1 : 0;
        sum += mask[i];
    }
    return sum;
}

/* Combined test using all four operators in one loop */
int test_all_comparisons(int *a, int *b, int *c, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        // Use all four comparison operators
        if (a[i] > b[i]) result += 1;
        if (a[i] >= b[i]) result += 2;
        if (a[i] < c[i]) result += 4;
        if (a[i] <= c[i]) result += 8;
    }
    return result;
}

/* Test with unsigned types */
unsigned test_unsigned_gt(unsigned *u1, unsigned *u2, int n) {
    unsigned sum = 0;
    for (int i = 0; i < n; i++) {
        if (u1[i] > u2[i]) {
            sum += u1[i];
        }
    }
    return sum;
}

/* Test with different loop lengths and data types */
short test_variable_length(short *s1, short *s2, int n) {
    short diff = 0;
    for (int i = 0; i < n; i++) {
        if (s1[i] <= s2[i]) {
            diff += s1[i] - s2[i];
        }
    }
    return diff;
}

int main() {
    /* Initialize arrays with different patterns */
    int a[N], b[N], c[N];
    short xs[M], ys[M];
    char data[N], out[N], mask[N];
    unsigned uarr[N], ubrr[N];
    short sarr[512], sbrr[512];
    
    /* Fill arrays with varying data to ensure comparisons are meaningful */
    for (int i = 0; i < N; i++) {
        a[i] = i - N/2;          /* Mixed positive/negative */
        b[i] = i % 100;          /* Range 0-99 */
        c[i] = (i * 3) % 200;    /* Different pattern */
        data[i] = (char)(i - 128); /* Signed char values */
        uarr[i] = (unsigned)i * 2;
        ubrr[i] = (unsigned)i * 3;
    }
    
    for (int i = 0; i < M; i++) {
        xs[i] = (short)(i * 2);
        ys[i] = (short)(i * 3 - 100);
    }
    
    for (int i = 0; i < 512; i++) {
        sarr[i] = (short)((i * 7) % 1000);
        sbrr[i] = (short)((i * 11) % 1000);
    }
    
    /* Call all test functions */
    int sum1 = test_gt_int(a, b, N);
    unsigned count1 = test_ge_short(xs, ys, M);
    
    test_lt_char(data, 0, out, N);
    
    int sum2 = test_le_mixed(a, c, mask, N);
    
    int combined = test_all_comparisons(a, b, c, N);
    
    unsigned usum = test_unsigned_gt(uarr, ubrr, N);
    
    short sdiff = test_variable_length(sarr, sbrr, 512);
    
    /* Aggregate results to prevent dead code elimination */
    int total = sum1 + count1 + sum2 + combined + usum + sdiff;
    
    /* Also use the output array from test_lt_char */
    for (int i = 0; i < N; i++) {
        total += out[i];
    }
    
    printf("Result: %d\n", total);
    
    return 0;
}
