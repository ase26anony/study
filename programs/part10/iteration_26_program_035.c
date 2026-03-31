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

unsigned int test_ge_short(short *a, short *b, int n) {
    unsigned int count = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            count++;
        }
    }
    return count;
}

long test_lt_char(char *a, char threshold, int n) {
    long sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] < threshold) {
            sum += a[i];
        }
    }
    return sum;
}

unsigned long test_le_mixed(int *a, unsigned short *b, int n) {
    unsigned long result = 0;
    for (int i = 0; i < n; i++) {
        // Using <= with mixed signed/unsigned types
        if ((unsigned int)a[i] <= b[i]) {
            result += a[i];
        }
    }
    return result;
}

/* Function using all four operators in one loop */
int test_all_comparisons(int *a, int *b, int *c, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        // Each comparison uses a different operator
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

/* Function with conditional assignment using ternary operator */
void test_ternary_gt(int *src1, int *src2, int *dst, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] > src2[i]) ? src1[i] : src2[i];  // GT_EXPR in ternary
    }
}

/* Function with unsigned comparisons */
unsigned test_unsigned_le(unsigned *a, unsigned *b, int n) {
    unsigned sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {  // LE_EXPR with unsigned
            sum += a[i];
        }
    }
    return sum;
}

/* Main driver function */
int main() {
    // Initialize arrays with different patterns
    int arr1[N], arr2[N], arr3[N], dst[N];
    short sarr1[M], sarr2[M];
    char carr[N];
    unsigned uarr1[N], uarr2[N];
    unsigned short usarr[N];
    
    // Fill arrays with varying data to ensure comparisons are meaningful
    for (int i = 0; i < N; i++) {
        arr1[i] = i - N/2;           // Mixed positive/negative
        arr2[i] = i * 2;
        arr3[i] = N - i;
        carr[i] = (char)(i - 128);
        uarr1[i] = i * 3;
        uarr2[i] = i * 4;
        usarr[i] = (unsigned short)(i * 5);
        dst[i] = 0;
    }
    
    for (int i = 0; i < M; i++) {
        sarr1[i] = (short)(i * 3 - M);
        sarr2[i] = (short)(i * 2);
    }
    
    // Call all test functions
    int sum1 = test_gt_int(arr1, arr2, N);
    unsigned int count1 = test_ge_short(sarr1, sarr2, M);
    long sum2 = test_lt_char(carr, 0, N);
    unsigned long sum3 = test_le_mixed(arr1, usarr, N);
    int all_result = test_all_comparisons(arr1, arr2, arr3, N);
    test_ternary_gt(arr1, arr3, dst, N);
    unsigned usum = test_unsigned_le(uarr1, uarr2, N);
    
    // Use results to prevent dead code elimination
    int final_result = sum1 + count1 + sum2 + sum3 + all_result + dst[N-1] + usum;
    
    printf("Result: %d\n", final_result);
    
    // Additional loops with different sizes to trigger different vectorization decisions
    // Small loop that might still vectorize
    int small_arr[16];
    int small_sum = 0;
    for (int i = 0; i < 16; i++) {
        if (i > 8) {  // GT_EXPR
            small_sum += i;
        }
    }
    
    // Loop with stride (still vectorizable)
    int strided_sum = 0;
    for (int i = 0; i < N; i += 2) {
        if (arr1[i] <= arr2[i]) {  // LE_EXPR
            strided_sum += arr1[i];
        }
    }
    
    printf("Small sum: %d, Strided sum: %d\n", small_sum, strided_sum);
    
    return final_result > 0 ? 0 : 1;
}
