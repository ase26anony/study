#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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
unsigned short test_ge(short *a, short *b, int n) {
    unsigned short count = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            count++;
        }
    }
    return count;
}

/* Less-than (LT_EXPR) */
int test_lt(char *a, char *b, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            sum += b[i];
        }
    }
    return sum;
}

/* Less-than-or-equal (LE_EXPR) */
unsigned test_le(unsigned *a, unsigned *b, int n) {
    unsigned mask_sum = 0;
    for (int i = 0; i < n; i++) {
        mask_sum += (a[i] <= b[i]) ? 1 : 0;
    }
    return mask_sum;
}

/* Mixed comparisons in one loop to hit multiple cases */
int test_mixed_comparisons(int *arr1, int *arr2, int *thresholds, int n) {
    int results[4] = {0};
    
    for (int i = 0; i < n; i++) {
        // GT_EXPR
        if (arr1[i] > thresholds[0]) {
            results[0] += arr1[i];
        }
        
        // GE_EXPR
        if (arr2[i] >= thresholds[1]) {
            results[1] += arr2[i];
        }
        
        // LT_EXPR
        if (arr1[i] < thresholds[2]) {
            results[2] += arr1[i];
        }
        
        // LE_EXPR
        if (arr2[i] <= thresholds[3]) {
            results[3] += arr2[i];
        }
    }
    
    return results[0] + results[1] + results[2] + results[3];
}

/* Conditional assignment using ternary operator */
void test_ternary_operators(int *src1, int *src2, int *dst, int n) {
    for (int i = 0; i < n; i++) {
        // Using all four comparison operators in ternary expressions
        dst[i] = (src1[i] > src2[i]) ? src1[i] : 
                 (src1[i] >= src2[i] + 10) ? src2[i] :
                 (src1[i] < src2[i] - 5) ? src1[i] * 2 :
                 (src1[i] <= src2[i]) ? src1[i] + src2[i] : 0;
    }
}

/* Test with different data types and loop lengths */
long test_various_types() {
    long total = 0;
    
    // char arrays - V16QI vectorization
    char c1[M], c2[M];
    for (int i = 0; i < M; i++) {
        c1[i] = (char)(i - 64);
        c2[i] = (char)(i % 128);
    }
    total += test_lt(c1, c2, M);
    
    // short arrays - V8HI vectorization  
    short s1[N], s2[N];
    for (int i = 0; i < N; i++) {
        s1[i] = (short)(i * 2);
        s2[i] = (short)(i * 3 - 100);
    }
    total += test_ge(s1, s2, N);
    
    // int arrays - V4SI vectorization
    int i1[L], i2[L];
    for (int i = 0; i < L; i++) {
        i1[i] = i * 3;
        i2[i] = i * 2 + 50;
    }
    total += test_gt(i1, i2, L);
    
    // unsigned arrays
    unsigned u1[N], u2[N];
    for (int i = 0; i < N; i++) {
        u1[i] = (unsigned)(i + 1000);
        u2[i] = (unsigned)(i * 2);
    }
    total += test_le(u1, u2, N);
    
    return total;
}

int main() {
    // Initialize arrays with varying data
    int arr1[N], arr2[N], thresholds[4];
    int dst[N];
    
    for (int i = 0; i < N; i++) {
        arr1[i] = i * 3 - 150;
        arr2[i] = i * 2 + 100;
    }
    
    thresholds[0] = 100;  // for GT
    thresholds[1] = 50;   // for GE
    thresholds[2] = 200;  // for LT
    thresholds[3] = 300;  // for LE
    
    // Run all tests
    int result = 0;
    
    result += test_gt(arr1, arr2, N);
    result += test_ge((short*)arr1, (short*)arr2, N/2);
    
    char c1[N], c2[N];
    for (int i = 0; i < N; i++) {
        c1[i] = (char)(arr1[i] % 128);
        c2[i] = (char)(arr2[i] % 128);
    }
    result += test_lt(c1, c2, N);
    
    unsigned u1[N], u2[N];
    for (int i = 0; i < N; i++) {
        u1[i] = (unsigned)(arr1[i] + 1000);
        u2[i] = (unsigned)(arr2[i] + 500);
    }
    result += test_le(u1, u2, N);
    
    result += test_mixed_comparisons(arr1, arr2, thresholds, N);
    
    test_ternary_operators(arr1, arr2, dst, N);
    for (int i = 0; i < N; i++) {
        result += dst[i];
    }
    
    result += test_various_types();
    
    printf("Final result: %d\n", result);
    return 0;
}
