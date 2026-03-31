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
    for (int i = 0; i < N; i++) {
        mask[i] = (a[i] <= b[i]) ? 1 : 0;
        result += mask[i];
    }
    return result;
}

/* Test with unsigned types */
unsigned test_gt_unsigned(unsigned *a, unsigned *b) {
    unsigned sum = 0;
    for (int i = 0; i < N; i++) {
        sum += (a[i] > b[i]) ? a[i] : 0;
    }
    return sum;
}

/* Test with all four operators in one loop */
int test_all_comparisons(int *a, int *b, int *c, int *d) {
    int results[4] = {0};
    
    for (int i = 0; i < N; i++) {
        // Each comparison in separate conditional
        if (a[i] > b[i]) results[0] += a[i];
        if (a[i] >= b[i]) results[1] += b[i];
        if (a[i] < c[i]) results[2] += c[i];
        if (a[i] <= d[i]) results[3] += d[i];
    }
    
    return results[0] + results[1] + results[2] + results[3];
}

/* Test with ternary operator (creates different IR patterns) */
void test_ternary_comparisons(short *src1, short *src2, short *dst) {
    for (int i = 0; i < N; i++) {
        // Mix of comparison operators in ternary expressions
        dst[i] = (src1[i] > src2[i]) ? src1[i] : 
                 (src1[i] >= 0) ? src2[i] :
                 (src1[i] < -100) ? -1 :
                 (src1[i] <= src2[i]) ? 0 : src1[i];
    }
}

/* Initialize arrays with varied data */
void init_arrays(int *a, int *b, short *s1, short *s2, char *c1, char *c2, 
                 unsigned *u1, unsigned *u2) {
    for (int i = 0; i < N; i++) {
        a[i] = i - N/2;  // Mix of positive and negative
        b[i] = i % 100;
        s1[i] = (short)(i * 3);
        s2[i] = (short)(i * 2 + 1);
        c1[i] = (char)(i % 128);
        c2[i] = (char)(64 - i % 128);
        u1[i] = i * 2;
        u2[i] = i * 3;
    }
}

int main() {
    /* Declare arrays of different types */
    int arr1[N], arr2[N], arr3[N], arr4[N];
    short sarr1[N], sarr2[N], sarr3[N];
    char carr1[M], carr2[M], mask[N];
    unsigned uarr1[N], uarr2[N];
    
    /* Initialize with varied data */
    init_arrays(arr1, arr2, sarr1, sarr2, carr1, carr2, uarr1, uarr2);
    
    /* Initialize additional arrays */
    for (int i = 0; i < N; i++) {
        arr3[i] = i * 5 % 200;
        arr4[i] = 100 - i % 100;
        sarr3[i] = 0;
    }
    
    /* Execute all test functions */
    int total = 0;
    
    total += test_gt_int(arr1, arr2);           // > with int
    total += test_ge_short(sarr1, sarr2);       // >= with short
    total += test_lt_char(carr1, carr2);        // < with char
    total += test_le_mixed(arr1, arr2, mask);   // <= with mixed types
    
    total += test_gt_unsigned(uarr1, uarr2);    // > with unsigned
    total += test_all_comparisons(arr1, arr2, arr3, arr4); // All in one loop
    
    test_ternary_comparisons(sarr1, sarr2, sarr3); // Ternary expressions
    
    /* Use results to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        total += sarr3[i];
        total += mask[i];
    }
    
    printf("Total result: %d\n", total);
    
    /* Additional test with different loop lengths */
    int small_arr1[64], small_arr2[64];
    for (int i = 0; i < 64; i++) {
        small_arr1[i] = i * 7;
        small_arr2[i] = i * 3;
    }
    
    int small_sum = 0;
    for (int i = 0; i < 64; i++) {
        if (small_arr1[i] > small_arr2[i]) small_sum += small_arr1[i];
        if (small_arr1[i] >= small_arr2[i]) small_sum += 1;
        if (small_arr1[i] < small_arr2[i] + 10) small_sum += 2;
        if (small_arr1[i] <= small_arr2[i] * 2) small_sum += 3;
    }
    
    printf("Small loop result: %d\n", small_sum + total);
    
    return 0;
}
