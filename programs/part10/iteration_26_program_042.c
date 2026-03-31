#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 256
#define M 128
#define L 512

// Test functions for each comparison operator

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
            count += 1;
        }
    }
    return count;
}

int test_lt_char(char *data, char threshold, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (data[i] < threshold) {
            sum += data[i];
        }
    }
    return sum;
}

int test_le_mixed(short *a, short *b, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        result = (a[i] <= b[i]) ? result + a[i] : result - b[i];
    }
    return result;
}

// Function using all four operators in one loop
int test_all_comparisons(int *arr1, int *arr2, int *out, int n) {
    int total = 0;
    for (int i = 0; i < n; i++) {
        // Use all four comparison operators
        if (arr1[i] > arr2[i]) {
            out[i] = 1;
        } else if (arr1[i] >= arr2[i]) {
            out[i] = 2;
        } else if (arr1[i] < arr2[i]) {
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

// Function with conditional assignment using ternary operator
void test_ternary_gt(int *src1, int *src2, char *mask, int n) {
    for (int i = 0; i < n; i++) {
        mask[i] = (src1[i] > src2[i]) ? 1 : 0;
    }
}

void test_ternary_le(unsigned int *a, unsigned int *b, unsigned char *out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = (a[i] <= b[i]) ? 0xFF : 0x00;
    }
}

// Initialize arrays with varied data
void init_arrays(int *a, int *b, unsigned short *x, unsigned short *y, 
                 char *chars, short *shorts1, short *shorts2, int size) {
    srand(time(NULL));
    
    for (int i = 0; i < size; i++) {
        a[i] = rand() % 1000 - 500;  // Mixed positive and negative
        b[i] = rand() % 1000 - 500;
        x[i] = rand() % 65535;
        y[i] = rand() % 65535;
        chars[i] = rand() % 256 - 128;
        shorts1[i] = rand() % 1000 - 500;
        shorts2[i] = rand() % 1000 - 500;
    }
}

int main() {
    // Declare arrays of different types and sizes
    int arr1[N], arr2[N], arr3[M], arr4[M];
    unsigned short us_arr1[L], us_arr2[L];
    char char_arr[N];
    short short_arr1[M], short_arr2[M];
    int output[N];
    char mask[N];
    unsigned char uchar_mask[L];
    
    // Initialize all arrays
    init_arrays(arr1, arr2, us_arr1, us_arr2, char_arr, short_arr1, short_arr2, N);
    
    // Initialize additional arrays
    for (int i = 0; i < M; i++) {
        arr3[i] = i * 2;
        arr4[i] = i * 3;
    }
    
    for (int i = 0; i < L; i++) {
        uchar_mask[i] = 0;
    }
    
    // Call test functions with different comparison operators
    int result1 = test_gt_int(arr1, arr2, N);           // GT_EXPR
    unsigned int result2 = test_ge_unsigned(us_arr1, us_arr2, L);  // GE_EXPR
    int result3 = test_lt_char(char_arr, 0, N);         // LT_EXPR
    int result4 = test_le_mixed(short_arr1, short_arr2, M); // LE_EXPR
    
    // Test with all operators in one loop
    int result5 = test_all_comparisons(arr3, arr4, output, M);
    
    // Test ternary conditional assignments
    test_ternary_gt(arr1, arr2, mask, N);               // GT_EXPR with ternary
    test_ternary_le((unsigned int*)arr1, (unsigned int*)arr2, uchar_mask, N/2); // LE_EXPR with ternary
    
    // Additional loops with different data types and sizes
    int sum_gt = 0;
    for (int i = 0; i < 128; i++) {
        sum_gt += (arr1[i] > arr2[i]) ? arr1[i] : 0;  // GT_EXPR with ternary
    }
    
    int sum_le = 0;
    for (int i = 0; i < 256; i += 2) {
        if (arr1[i] <= arr2[i]) {                     // LE_EXPR
            sum_le += arr1[i];
        }
    }
    
    // Use results to prevent dead code elimination
    int final_result = result1 + result2 + result3 + result4 + result5 + sum_gt + sum_le;
    
    // Also use mask arrays
    int mask_sum = 0;
    for (int i = 0; i < N; i++) {
        mask_sum += mask[i];
    }
    
    for (int i = 0; i < L/2; i++) {
        mask_sum += uchar_mask[i];
    }
    
    final_result += mask_sum;
    
    printf("Final result: %d\n", final_result);
    
    return 0;
}
