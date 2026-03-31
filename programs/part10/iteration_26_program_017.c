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
int test_le(unsigned int *a, unsigned int *b, int n) {
    unsigned int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

/* Mixed comparisons in one loop to trigger multiple cases */
int test_mixed_comparisons(int *a, int *b, int *c, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        // Using all four comparison operators
        if (a[i] > b[i]) {
            result += 1;
        }
        if (a[i] >= c[i]) {
            result += 2;
        }
        if (b[i] < c[i]) {
            result += 4;
        }
        if (b[i] <= a[i]) {
            result += 8;
        }
    }
    return result;
}

/* Conditional assignment using ternary operator */
void test_ternary_gt(int *a, int *b, int *out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
}

void test_ternary_le(short *a, short *b, short *out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = (a[i] <= b[i]) ? a[i] : b[i];
    }
}

/* Different data types and loop lengths */
int test_various_types() {
    int total = 0;
    
    // char arrays with < comparison
    char chars1[M];
    char chars2[M];
    for (int i = 0; i < M; i++) {
        chars1[i] = (char)(i - 64);
        chars2[i] = (char)(i);
    }
    total += test_lt(chars1, chars2, M);
    
    // short arrays with >= comparison
    short shorts1[N];
    short shorts2[N];
    for (int i = 0; i < N; i++) {
        shorts1[i] = (short)(i * 2);
        shorts2[i] = (short)(i * 3);
    }
    total += test_ge(shorts1, shorts2, N);
    
    // int arrays with > comparison
    int ints1[L];
    int ints2[L];
    for (int i = 0; i < L; i++) {
        ints1[i] = i * 5;
        ints2[i] = i * 3;
    }
    total += test_gt(ints1, ints2, L/2);
    
    // unsigned arrays with <= comparison
    unsigned int uints1[N];
    unsigned int uints2[N];
    for (int i = 0; i < N; i++) {
        uints1[i] = i * 7;
        uints2[i] = i * 4;
    }
    total += test_le(uints1, uints2, N);
    
    return total;
}

int main() {
    // Initialize test data
    int data1[N], data2[N], data3[N];
    short sdata1[M], sdata2[M];
    char cdata1[L], cdata2[L];
    unsigned int udata1[N], udata2[N];
    
    // Fill arrays with varying patterns
    for (int i = 0; i < N; i++) {
        data1[i] = i * 2;
        data2[i] = i * 3;
        data3[i] = i;
        udata1[i] = i * 4;
        udata2[i] = i * 5;
    }
    
    for (int i = 0; i < M; i++) {
        sdata1[i] = (short)(i - 50);
        sdata2[i] = (short)(i + 50);
    }
    
    for (int i = 0; i < L; i++) {
        cdata1[i] = (char)(i % 128);
        cdata2[i] = (char)((i + 64) % 128);
    }
    
    int result = 0;
    
    // Execute all test functions
    result += test_gt(data1, data2, N);
    result += test_ge(sdata1, sdata2, M);
    result += test_lt(cdata1, cdata2, L);
    result += test_le(udata1, udata2, N);
    
    // Test mixed comparisons
    result += test_mixed_comparisons(data1, data2, data3, N);
    
    // Test ternary operations
    int out1[N];
    short out2[M];
    test_ternary_gt(data1, data2, out1, N);
    test_ternary_le(sdata1, sdata2, out2, M);
    
    // Verify ternary results aren't optimized away
    for (int i = 0; i < 10; i++) {
        result += out1[i];
        result += out2[i];
    }
    
    // Test various types
    result += test_various_types();
    
    printf("Final result: %d\n", result);
    
    return 0;
}
