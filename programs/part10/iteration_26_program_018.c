#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 256
#define M 128
#define L 512

/* Test functions for each comparison operator */

// Greater than (GT_EXPR)
int test_gt(int *a, int *b, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

// Greater than or equal (GE_EXPR)
int test_ge(short *a, short *b, int n) {
    int count = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            count++;
        }
    }
    return count;
}

// Less than (LT_EXPR)
int test_lt(char *a, char *b, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            sum += b[i];
        }
    }
    return sum;
}

// Less than or equal (LE_EXPR)
int test_le(unsigned int *a, unsigned int *b, int n) {
    unsigned int mask_sum = 0;
    for (int i = 0; i < n; i++) {
        mask_sum += (a[i] <= b[i]) ? a[i] : 0;
    }
    return (int)mask_sum;
}

// Mixed comparisons in one loop to potentially trigger multiple cases
int test_mixed_comparisons(int *a, int *b, int *c, int n) {
    int results[4] = {0};
    
    for (int i = 0; i < n; i++) {
        // GT_EXPR
        if (a[i] > b[i]) {
            results[0] += a[i];
        }
        
        // GE_EXPR
        if (a[i] >= c[i]) {
            results[1] += b[i];
        }
        
        // LT_EXPR
        if (b[i] < c[i]) {
            results[2] += c[i];
        }
        
        // LE_EXPR
        if (c[i] <= a[i]) {
            results[3] += b[i];
        }
    }
    
    return results[0] + results[1] + results[2] + results[3];
}

// Conditional assignment using ternary operator
void test_ternary_operators(short *src1, short *src2, short *dst, int n) {
    for (int i = 0; i < n; i++) {
        // Mix of different comparison operators in ternary
        dst[i] = (src1[i] > src2[i]) ? src1[i] : 
                 (src1[i] >= 0) ? src2[i] :
                 (src1[i] < -100) ? (short)-100 :
                 (src1[i] <= src2[i]) ? src1[i] : src2[i];
    }
}

// Test with different data types and loop lengths
int test_various_types() {
    int total = 0;
    
    // char arrays - should vectorize to V16QI or V32QI
    char char_arr1[L], char_arr2[L];
    for (int i = 0; i < L; i++) {
        char_arr1[i] = (char)(i % 128);
        char_arr2[i] = (char)((i + 64) % 128);
    }
    total += test_lt(char_arr1, char_arr2, L);
    
    // short arrays - should vectorize to V8HI or V16HI
    short short_arr1[M], short_arr2[M];
    for (int i = 0; i < M; i++) {
        short_arr1[i] = (short)(i * 2);
        short_arr2[i] = (short)(i * 2 + 1);
    }
    total += test_ge(short_arr1, short_arr2, M);
    
    // int arrays - should vectorize to V4SI or V8SI
    int int_arr1[N], int_arr2[N];
    for (int i = 0; i < N; i++) {
        int_arr1[i] = i * 3;
        int_arr2[i] = i * 2;
    }
    total += test_gt(int_arr1, int_arr2, N);
    
    // unsigned int arrays
    unsigned int uint_arr1[N], uint_arr2[N];
    for (int i = 0; i < N; i++) {
        uint_arr1[i] = (unsigned int)(i + 100);
        uint_arr2[i] = (unsigned int)(i * 2);
    }
    total += test_le(uint_arr1, uint_arr2, N);
    
    return total;
}

int main() {
    srand(time(NULL));
    int final_result = 0;
    
    // Initialize test data
    int arr1[N], arr2[N], arr3[N];
    short sarr1[M], sarr2[M], sdst[M];
    
    for (int i = 0; i < N; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        arr3[i] = rand() % 1000;
    }
    
    for (int i = 0; i < M; i++) {
        sarr1[i] = (short)(rand() % 1000);
        sarr2[i] = (short)(rand() % 1000);
    }
    
    // Execute all test functions
    final_result += test_gt(arr1, arr2, N);
    final_result += test_ge(sarr1, sarr2, M);
    
    char carr1[L], carr2[L];
    for (int i = 0; i < L; i++) {
        carr1[i] = (char)(rand() % 256 - 128);
        carr2[i] = (char)(rand() % 256 - 128);
    }
    final_result += test_lt(carr1, carr2, L);
    
    unsigned int uarr1[N], uarr2[N];
    for (int i = 0; i < N; i++) {
        uarr1[i] = (unsigned int)(rand() % 1000);
        uarr2[i] = (unsigned int)(rand() % 1000);
    }
    final_result += test_le(uarr1, uarr2, N);
    
    // Test mixed comparisons
    final_result += test_mixed_comparisons(arr1, arr2, arr3, N);
    
    // Test ternary operators
    test_ternary_operators(sarr1, sarr2, sdst, M);
    for (int i = 0; i < M; i++) {
        final_result += sdst[i];
    }
    
    // Test various types
    final_result += test_various_types();
    
    // Print result to prevent dead code elimination
    printf("Final result: %d\n", final_result);
    
    return 0;
}
