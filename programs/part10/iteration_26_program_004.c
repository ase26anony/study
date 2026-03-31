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

unsigned int test_ge_short(short *x, short *y) {
    unsigned int count = 0;
    for (int i = 0; i < N; i++) {
        if (x[i] >= y[i]) {
            count++;
        }
    }
    return count;
}

char test_lt_char(char *src1, char *src2, char *out) {
    char sum = 0;
    for (int i = 0; i < M; i++) {
        out[i] = (src1[i] < src2[i]) ? src1[i] : src2[i];
        sum += out[i];
    }
    return sum;
}

short test_le_mixed(short *arr, int *thresh, short *mask) {
    short result = 0;
    for (int i = 0; i < N; i++) {
        mask[i] = (arr[i] <= thresh[i]) ? 1 : 0;
        result += mask[i];
    }
    return result;
}

/* Combined test with all four operators in one loop */
int test_all_comparisons(int *a, int *b, int *c, int *d) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) sum += a[i];
        if (c[i] >= d[i]) sum -= c[i];
        if (b[i] < a[i]) sum += i;
        if (d[i] <= c[i]) sum -= i;
    }
    return sum;
}

/* Unsigned variants to test different comparison semantics */
unsigned int test_unsigned_gt(unsigned char *u1, unsigned char *u2) {
    unsigned int diff = 0;
    for (int i = 0; i < M; i++) {
        if (u1[i] > u2[i]) {
            diff += u1[i] - u2[i];
        }
    }
    return diff;
}

int test_unsigned_le(unsigned short *us1, unsigned short *us2) {
    int matches = 0;
    for (int i = 0; i < N; i++) {
        matches += (us1[i] <= us2[i]) ? us1[i] : 0;
    }
    return matches;
}

int main() {
    /* Initialize arrays with different patterns */
    int a[N], b[N], c[N], d[N];
    short xs[N], ys[N];
    char cs1[M], cs2[M], out[M];
    short arr[N], mask[N];
    int thresh[N];
    unsigned char uc1[M], uc2[M];
    unsigned short us1[N], us2[N];
    
    /* Fill arrays with varying data to ensure comparisons are meaningful */
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = N - i;
        c[i] = i * 2;
        d[i] = i / 2;
        xs[i] = (short)(i - 128);
        ys[i] = (short)(i % 100);
        arr[i] = (short)(i * 3);
        thresh[i] = i * 2;
        us1[i] = (unsigned short)(i * 7);
        us2[i] = (unsigned short)(i * 5);
    }
    
    for (int i = 0; i < M; i++) {
        cs1[i] = (char)(i - 64);
        cs2[i] = (char)(i % 80);
        uc1[i] = (unsigned char)(i * 3);
        uc2[i] = (unsigned char)(i * 2);
    }
    
    /* Call all test functions */
    int total = 0;
    
    total += test_gt_int(a, b);
    total += test_ge_short(xs, ys);
    total += test_lt_char(cs1, cs2, out);
    total += test_le_mixed(arr, thresh, mask);
    total += test_all_comparisons(a, b, c, d);
    total += test_unsigned_gt(uc1, uc2);
    total += test_unsigned_le(us1, us2);
    
    printf("Result: %d\n", total);
    
    /* Use results to prevent dead code elimination */
    volatile int dummy = total;
    
    return 0;
}
