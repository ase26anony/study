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

int test_le_mixed(unsigned short *a, int *b) {
    int result = 0;
    for (int i = 0; i < N; i++) {
        result += (a[i] <= (unsigned short)b[i]) ? a[i] : 0;
    }
    return result;
}

/* Combined test with all operators in one loop */
int test_all_operators(int *arr1, int *arr2, int *out) {
    int total = 0;
    for (int i = 0; i < N; i++) {
        int val = 0;
        if (arr1[i] > arr2[i]) {
            val |= 1;  // GT_EXPR
        }
        if (arr1[i] >= arr2[i]) {
            val |= 2;  // GE_EXPR
        }
        if (arr1[i] < arr2[i]) {
            val |= 4;  // LT_EXPR
        }
        if (arr1[i] <= arr2[i]) {
            val |= 8;  // LE_EXPR
        }
        out[i] = val;
        total += val;
    }
    return total;
}

/* Additional tests with different data types and patterns */

int test_gt_unsigned(unsigned int *a, unsigned int *b) {
    unsigned int sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

int test_le_with_ternary(char *a, char threshold, char *out) {
    int count = 0;
    for (int i = 0; i < M; i++) {
        out[i] = (a[i] <= threshold) ? 1 : 0;
        count += out[i];
    }
    return count;
}

int test_lt_with_negative(short *a, short *b) {
    int diff = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] < b[i]) {
            diff += b[i] - a[i];
        }
    }
    return diff;
}

int test_ge_different_strides(int *a, int *b) {
    int sum = 0;
    // Different stride to potentially trigger different vectorization patterns
    for (int i = 0; i < N; i += 2) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

/* Initialize arrays with varied data */
void init_arrays(int *a_int, int *b_int, short *a_short, short *b_short,
                 char *a_char, char *b_char, unsigned short *a_ushort,
                 unsigned int *a_uint, unsigned int *b_uint) {
    for (int i = 0; i < N; i++) {
        a_int[i] = i - N/2;  // Mix of negative and positive
        b_int[i] = i % 100;
        a_short[i] = (i * 3) % 256;
        b_short[i] = (i * 5) % 256;
        a_ushort[i] = i * 7;
        
        if (i < M) {
            a_char[i] = (i % 128) - 64;
            b_char[i] = (i * 2) % 128;
        }
        
        a_uint[i] = i * 11;
        b_uint[i] = i * 13;
    }
}

int main() {
    /* Declare arrays of different types */
    int arr1_int[N], arr2_int[N], out_int[N];
    short arr1_short[N], arr2_short[N];
    char arr1_char[M], arr2_char[M], out_char[M];
    unsigned short arr1_ushort[N];
    unsigned int arr1_uint[N], arr2_uint[N];
    
    /* Initialize all arrays */
    init_arrays(arr1_int, arr2_int, arr1_short, arr2_short,
                arr1_char, arr2_char, arr1_ushort,
                arr1_uint, arr2_uint);
    
    /* Execute all test functions */
    int total = 0;
    
    total += test_gt_int(arr1_int, arr2_int);          // GT_EXPR with int
    total += test_ge_short(arr1_short, arr2_short);    // GE_EXPR with short
    total += test_lt_char(arr1_char, arr2_char);       // LT_EXPR with char
    total += test_le_mixed(arr1_ushort, arr2_int);     // LE_EXPR mixed types
    
    total += test_all_operators(arr1_int, arr2_int, out_int);  // All operators
    
    total += test_gt_unsigned(arr1_uint, arr2_uint);   // GT_EXPR unsigned
    
    char threshold = 50;
    total += test_le_with_ternary(arr1_char, threshold, out_char); // LE_EXPR with ternary
    
    total += test_lt_with_negative(arr1_short, arr2_short); // LT_EXPR with negative
    
    total += test_ge_different_strides(arr1_int, arr2_int); // GE_EXPR with stride
    
    /* Use the result to prevent dead code elimination */
    printf("Total result: %d\n", total);
    
    /* Also print a few values from output arrays to ensure they're used */
    printf("Sample outputs: %d %d %d\n", out_int[0], out_int[N/2], out_int[N-1]);
    printf("Char outputs: %d %d\n", out_char[0], out_char[M/2]);
    
    return total != 0 ? 0 : 1;
}
