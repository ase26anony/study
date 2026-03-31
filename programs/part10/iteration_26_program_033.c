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

/* Combined test with all four operators in one loop */
int test_all_comparisons(int *arr1, int *arr2, int *out) {
    int total = 0;
    for (int i = 0; i < N; i++) {
        // Use all four comparison operators
        if (arr1[i] > arr2[i]) {
            out[i] = 1;
        } else if (arr1[i] >= arr2[i] + 1) {
            out[i] = 2;
        } else if (arr1[i] < arr2[i] - 1) {
            out[i] = 3;
        } else if (arr1[i] <= arr2[i]) {
            out[i] = 4;
        } else {
            out[i] = 0;
        }
        total += out[i];
    }
    return total;
}

/* Unsigned comparisons to cover different semantics */
unsigned test_unsigned_gt(unsigned *a, unsigned *b) {
    unsigned sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

unsigned test_unsigned_le(unsigned short *a, unsigned short *b) {
    unsigned count = 0;
    for (int i = 0; i < M; i++) {
        if (a[i] <= b[i]) {
            count++;
        }
    }
    return count;
}

/* Main driver function */
int main() {
    // Initialize arrays with different patterns
    int arr1_int[N], arr2_int[N];
    short arr1_short[N], arr2_short[N];
    char arr1_char[M], arr2_char[M];
    unsigned arr1_uint[N], arr2_uint[N];
    unsigned short arr1_ushort[M], arr2_ushort[M];
    int output[N];
    
    // Fill arrays with varying data
    for (int i = 0; i < N; i++) {
        arr1_int[i] = i * 3 - 128;
        arr2_int[i] = i * 2 + 64;
        arr1_short[i] = (short)(i * 5 - 256);
        arr2_short[i] = (short)(i * 4 + 128);
        arr1_uint[i] = (unsigned)(i * 7);
        arr2_uint[i] = (unsigned)(i * 6 + 100);
    }
    
    for (int i = 0; i < M; i++) {
        arr1_char[i] = (char)(i - 64);
        arr2_char[i] = (char)(i * 2 - 32);
        arr1_ushort[i] = (unsigned short)(i * 3);
        arr2_ushort[i] = (unsigned short)(i * 5 + 50);
    }
    
    // Call all test functions
    int result = 0;
    
    result += test_gt_int(arr1_int, arr2_int);
    result += test_ge_short(arr1_short, arr2_short);
    result += test_lt_char(arr1_char, arr2_char);
    result += test_le_mixed(arr1_int, arr2_short);
    result += test_all_comparisons(arr1_int, arr2_int, output);
    result += (int)test_unsigned_gt(arr1_uint, arr2_uint);
    result += (int)test_unsigned_le(arr1_ushort, arr2_ushort);
    
    // Use the result to prevent dead code elimination
    printf("Final result: %d\n", result);
    
    // Also print a few output values to ensure array writes aren't optimized away
    for (int i = 0; i < 10; i++) {
        printf("output[%d] = %d\n", i, output[i]);
    }
    
    return 0;
}
