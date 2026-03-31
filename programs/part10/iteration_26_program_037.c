#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

/* Test functions for each comparison operator */

int test_gt_char(char *a, char *b, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

unsigned test_ge_short(short *x, short threshold, int n) {
    unsigned count = 0;
    for (int i = 0; i < n; i++) {
        if (x[i] >= threshold) {
            count++;
        }
    }
    return count;
}

int test_lt_int(int *src1, int *src2, int *out, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        out[i] = (src1[i] < src2[i]) ? src1[i] : 0;
        sum += out[i];
    }
    return sum;
}

unsigned test_le_mixed(unsigned short *a, unsigned short *b, int n) {
    unsigned result = 0;
    for (int i = 0; i < n; i++) {
        result = (a[i] <= b[i]) ? (result + a[i]) : (result - b[i]);
    }
    return result;
}

/* Combined test with all four operators in one loop */
int test_all_comparisons(int *arr1, int *arr2, int n) {
    int gt_sum = 0, ge_sum = 0, lt_sum = 0, le_sum = 0;
    
    for (int i = 0; i < n; i++) {
        // Each comparison in separate conditional
        if (arr1[i] > arr2[i]) gt_sum += arr1[i];
        if (arr1[i] >= arr2[i]) ge_sum += arr2[i];
        if (arr1[i] < arr2[i]) lt_sum += arr1[i];
        if (arr1[i] <= arr2[i]) le_sum += arr2[i];
    }
    
    return gt_sum + ge_sum + lt_sum + le_sum;
}

/* Additional tests with different data types and patterns */

int test_gt_unsigned(unsigned int *a, unsigned int *b, int n) {
    unsigned int mask = 0;
    for (int i = 0; i < n; i++) {
        mask |= (a[i] > b[i]) ? (1U << (i % 32)) : 0;
    }
    return (int)mask;
}

int test_le_signed_char(signed char *data, signed char limit, int n) {
    int total = 0;
    for (int i = 0; i < n; i++) {
        total += (data[i] <= limit) ? data[i] : -limit;
    }
    return total;
}

void initialize_arrays() {
    /* Arrays will be initialized in main */
}

int main() {
    /* Initialize arrays with different patterns */
    char char_a[N], char_b[N];
    short short_arr[M];
    int int_arr1[N], int_arr2[N], int_out[N];
    unsigned short ushort_arr1[M], ushort_arr2[M];
    unsigned int uint_arr1[N], uint_arr2[N];
    signed char schar_arr[N];
    
    /* Fill arrays with varying data to ensure comparisons are meaningful */
    for (int i = 0; i < N; i++) {
        char_a[i] = (i % 3) * 10 - 15;
        char_b[i] = (i % 5) * 7 - 10;
        int_arr1[i] = i * 2 - N;
        int_arr2[i] = i * 3 - N * 2;
        uint_arr1[i] = i * 100;
        uint_arr2[i] = i * 75 + 50;
        schar_arr[i] = (i % 7) * 5 - 20;
        if (i < M) {
            short_arr[i] = i * 10 - M * 5;
            ushort_arr1[i] = i * 3;
            ushort_arr2[i] = i * 2 + 100;
        }
    }
    
    /* Execute all test functions */
    int result = 0;
    
    result += test_gt_char(char_a, char_b, N);
    result += test_ge_short(short_arr, 0, M);
    result += test_lt_int(int_arr1, int_arr2, int_out, N);
    result += test_le_mixed(ushort_arr1, ushort_arr2, M);
    result += test_all_comparisons(int_arr1, int_arr2, N);
    result += test_gt_unsigned(uint_arr1, uint_arr2, N);
    result += test_le_signed_char(schar_arr, -5, N);
    
    printf("Final result: %d\n", result);
    
    /* Use the output array to prevent dead code elimination */
    int out_sum = 0;
    for (int i = 0; i < N; i++) {
        out_sum += int_out[i];
    }
    printf("Output array sum: %d\n", out_sum);
    
    return 0;
}
