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
    int count = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            count++;
        }
    }
    return count;
}

int test_lt_char(char *a, char *b) {
    int sum = 0;
    for (int i = 0; i < M; i++) {
        if (a[i] < b[i]) {
            sum += b[i];
        }
    }
    return sum;
}

int test_le_mixed(int *a, int *b, char *mask) {
    int result = 0;
    for (int i = 0; i < M; i++) {
        mask[i] = (a[i] <= b[i]) ? 1 : 0;
        result += mask[i];
    }
    return result;
}

/* Function using all four operators in one loop */
int test_all_comparisons(int *a, int *b, int *c, int *d) {
    int results[4] = {0};
    
    for (int i = 0; i < N; i++) {
        // Each comparison uses a different operator
        results[0] += (a[i] > b[i]) ? 1 : 0;   // GT_EXPR
        results[1] += (b[i] >= c[i]) ? 1 : 0;  // GE_EXPR  
        results[2] += (c[i] < d[i]) ? 1 : 0;   // LT_EXPR
        results[3] += (d[i] <= a[i]) ? 1 : 0;  // LE_EXPR
    }
    
    return results[0] + results[1] + results[2] + results[3];
}

/* Test with unsigned types */
unsigned test_unsigned_gt(unsigned short *a, unsigned short *b) {
    unsigned sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

unsigned test_unsigned_le(unsigned char *a, unsigned char *b) {
    unsigned count = 0;
    for (int i = 0; i < M; i++) {
        if (a[i] <= b[i]) {
            count++;
        }
    }
    return count;
}

/* Conditional store based on comparison */
void test_conditional_store(int *src, int *dst, int threshold) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src[i] > threshold) ? src[i] : 0;      // GT_EXPR
    }
}

int main() {
    /* Initialize arrays with different patterns */
    int a_int[N], b_int[N], c_int[N], d_int[N];
    short a_short[N], b_short[N];
    char a_char[M], b_char[M], mask[M];
    unsigned short us_a[N], us_b[N];
    unsigned char uc_a[M], uc_b[M];
    int dst[N];
    
    /* Fill arrays with varying data */
    for (int i = 0; i < N; i++) {
        a_int[i] = i;
        b_int[i] = N - i;
        c_int[i] = i * 2;
        d_int[i] = i / 2;
        a_short[i] = (short)(i * 3);
        b_short[i] = (short)(i * 2);
        us_a[i] = (unsigned short)(i * 5);
        us_b[i] = (unsigned short)(i * 3);
    }
    
    for (int i = 0; i < M; i++) {
        a_char[i] = (char)(i - M/2);
        b_char[i] = (char)(M/2 - i);
        uc_a[i] = (unsigned char)(i * 2);
        uc_b[i] = (unsigned char)(i * 3);
    }
    
    /* Call all test functions to ensure execution */
    int total = 0;
    
    total += test_gt_int(a_int, b_int);
    total += test_ge_short(a_short, b_short);
    total += test_lt_char(a_char, b_char);
    total += test_le_mixed(a_int, b_int, mask);
    total += test_all_comparisons(a_int, b_int, c_int, d_int);
    total += test_unsigned_gt(us_a, us_b);
    total += test_unsigned_le(uc_a, uc_b);
    
    test_conditional_store(a_int, dst, N/2);
    
    /* Use dst to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        total += dst[i];
    }
    
    printf("Total result: %d\n", total);
    return total > 0 ? 0 : 1;
}
