#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 256
#define M 128
#define L 512

/* Test functions for each comparison operator */

/* Greater-than (GT_EXPR) */
int test_gt(int *a, int *b, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

/* Greater-than-or-equal (GE_EXPR) */
int test_ge(short *a, short *b, int n) {
    int count = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            count++;
        }
    }
    return count;
}

/* Less-than (LT_EXPR) */
int test_lt(unsigned char *a, unsigned char *b, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            sum += b[i];
        }
    }
    return sum;
}

/* Less-than-or-equal (LE_EXPR) */
int test_le(int *a, int threshold, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] <= threshold) {
            sum += a[i];
        }
    }
    return sum;
}

/* Mixed comparisons in one loop */
int test_mixed_comparisons(int *a, int *b, int n) {
    int results[4] = {0, 0, 0, 0};
    
    for (int i = 0; i < n; i++) {
        // All four comparison operators in separate branches
        if (a[i] > b[i]) {
            results[0] += a[i];  // GT_EXPR
        }
        
        if (a[i] >= b[i]) {
            results[1] += b[i];  // GE_EXPR
        }
        
        if (a[i] < b[i]) {
            results[2] += a[i];  // LT_EXPR
        }
        
        if (a[i] <= b[i]) {
            results[3] += b[i];  // LE_EXPR
        }
    }
    
    return results[0] + results[1] + results[2] + results[3];
}

/* Conditional assignment using ternary operator */
void test_ternary_gt(int *a, int *b, int *out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = (a[i] > b[i]) ? a[i] : b[i];  // GT_EXPR in ternary
    }
}

/* Conditional assignment with LE_EXPR */
void test_ternary_le(short *a, short threshold, short *out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = (a[i] <= threshold) ? a[i] : threshold;  // LE_EXPR in ternary
    }
}

/* Test with negative values */
int test_negative_values(int *a, int *b, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {      // LT_EXPR
            sum += a[i];
        }
        if (a[i] >= b[i]) {     // GE_EXPR
            sum -= b[i];
        }
    }
    return sum;
}

/* Different data types and loop lengths */
int test_various_types() {
    int total = 0;
    
    /* char arrays with GT_EXPR */
    unsigned char c1[128], c2[128];
    for (int i = 0; i < 128; i++) {
        c1[i] = i % 256;
        c2[i] = (i * 3) % 256;
    }
    
    for (int i = 0; i < 128; i++) {
        if (c1[i] > c2[i]) {  // GT_EXPR on chars
            total += c1[i];
        }
    }
    
    /* short arrays with LE_EXPR */
    short s1[256], s2[256];
    for (int i = 0; i < 256; i++) {
        s1[i] = i - 128;
        s2[i] = i % 100;
    }
    
    for (int i = 0; i < 256; i++) {
        if (s1[i] <= s2[i]) {  // LE_EXPR on shorts
            total += s2[i];
        }
    }
    
    return total;
}

int main() {
    srand(time(NULL));
    
    /* Initialize test arrays */
    int arr1[N], arr2[N];
    short sarr1[M], sarr2[M];
    unsigned char carr1[L], carr2[L];
    int out[N];
    
    for (int i = 0; i < N; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
    }
    
    for (int i = 0; i < M; i++) {
        sarr1[i] = rand() % 1000;
        sarr2[i] = rand() % 1000;
    }
    
    for (int i = 0; i < L; i++) {
        carr1[i] = rand() % 256;
        carr2[i] = rand() % 256;
    }
    
    /* Execute all test functions */
    int result = 0;
    
    result += test_gt(arr1, arr2, N);           // GT_EXPR
    result += test_ge(sarr1, sarr2, M);         // GE_EXPR
    result += test_lt(carr1, carr2, L);         // LT_EXPR
    result += test_le(arr1, 500, N);            // LE_EXPR
    
    test_ternary_gt(arr1, arr2, out, N);        // GT_EXPR in ternary
    for (int i = 0; i < N; i++) {
        result += out[i];
    }
    
    short sout[M];
    test_ternary_le(sarr1, 250, sout, M);       // LE_EXPR in ternary
    for (int i = 0; i < M; i++) {
        result += sout[i];
    }
    
    result += test_mixed_comparisons(arr1, arr2, N);  // All four operators
    result += test_negative_values(arr1, arr2, N);    // Mixed with negatives
    result += test_various_types();                   // Various types/lengths
    
    printf("Final result: %d\n", result);
    
    return 0;
}
