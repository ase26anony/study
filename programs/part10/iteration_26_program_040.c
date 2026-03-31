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

int test_le_mixed(int *a, short *b) {
    int result = 0;
    for (int i = 0; i < N; i++) {
        result += (a[i] <= (int)b[i]) ? a[i] : 0;
    }
    return result;
}

/* Test with unsigned types */
unsigned test_gt_unsigned(unsigned *a, unsigned *b) {
    unsigned sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

/* Test with all four operators in one loop */
int test_all_comparisons(int *a, int *b, int *c, int *d) {
    int result = 0;
    for (int i = 0; i < N; i++) {
        // Use all four comparison operators
        if (a[i] > b[i]) result += 1;
        if (a[i] >= c[i]) result += 2;
        if (a[i] < d[i]) result += 3;
        if (a[i] <= b[i]) result += 4;
    }
    return result;
}

/* Test with conditional assignment (ternary operator) */
void test_ternary_assign(int *src1, int *src2, int *dst) {
    for (int i = 0; i < N; i++) {
        dst[i] = (src1[i] > src2[i]) ? src1[i] : src2[i];
    }
}

/* Test with different loop lengths and data types */
int test_various_lengths(char *a, char *b, short *c, short *d) {
    int total = 0;
    
    // Short loop
    for (int i = 0; i < 64; i++) {
        if (a[i] > b[i]) total++;
    }
    
    // Medium loop
    for (int i = 0; i < 128; i++) {
        if (c[i] >= d[i]) total += 2;
    }
    
    // Long loop
    for (int i = 0; i < 256; i++) {
        if (c[i] < d[i]) total += 3;
    }
    
    return total;
}

int main() {
    // Initialize arrays with varying data
    int a_int[N], b_int[N], c_int[N], d_int[N];
    short a_short[N], b_short[N];
    char a_char[M], b_char[M];
    unsigned a_unsigned[N], b_unsigned[N];
    int dst[N];
    
    // Fill arrays with pseudo-random but deterministic data
    for (int i = 0; i < N; i++) {
        a_int[i] = (i * 37) % 100;
        b_int[i] = (i * 53) % 100;
        c_int[i] = (i * 71) % 100;
        d_int[i] = (i * 97) % 100;
        a_short[i] = (short)((i * 29) % 256);
        b_short[i] = (short)((i * 43) % 256);
        a_unsigned[i] = (unsigned)((i * 61) % 200);
        b_unsigned[i] = (unsigned)((i * 79) % 200);
    }
    
    for (int i = 0; i < M; i++) {
        a_char[i] = (char)((i * 11) % 128);
        b_char[i] = (char)((i * 17) % 128);
    }
    
    // Call all test functions
    int total = 0;
    
    total += test_gt_int(a_int, b_int);
    total += test_ge_short(a_short, b_short);
    total += test_lt_char(a_char, b_char);
    total += test_le_mixed(a_int, b_short);
    total += (int)test_gt_unsigned(a_unsigned, b_unsigned);
    total += test_all_comparisons(a_int, b_int, c_int, d_int);
    
    test_ternary_assign(a_int, b_int, dst);
    for (int i = 0; i < N; i++) {
        total += dst[i];
    }
    
    total += test_various_lengths(a_char, b_char, a_short, b_short);
    
    // Print result to prevent dead code elimination
    printf("Total result: %d\n", total);
    
    return 0;
}
