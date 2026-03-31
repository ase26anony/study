#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define N 256
#define M 128
#define L 512

/* Test functions for each comparison operator */

// Greater-than (>)
int test_gt(int *a, int *b, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

// Greater-than-or-equal (>=)
unsigned short test_ge(short *x, short *y, int n) {
    unsigned short count = 0;
    for (int i = 0; i < n; i++) {
        if (x[i] >= y[i]) {
            count++;
        }
    }
    return count;
}

// Less-than (<)
char test_lt(char *src, char threshold, int n) {
    char result = 0;
    for (int i = 0; i < n; i++) {
        if (src[i] < threshold) {
            result |= src[i];
        }
    }
    return result;
}

// Less-than-or-equal (<=)
int test_le(int *data, int *limits, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += (data[i] <= limits[i]) ? data[i] : 0;
    }
    return sum;
}

// Mixed comparisons in one loop to potentially trigger multiple cases
int test_mixed_comparisons(int *a, int *b, int *c, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        // Using all four comparison operators in different branches
        if (a[i] > b[i]) {
            result += 1;
        }
        if (a[i] >= b[i]) {
            result += 2;
        }
        if (a[i] < c[i]) {
            result += 4;
        }
        if (a[i] <= c[i]) {
            result += 8;
        }
    }
    return result;
}

// Unsigned comparisons (different semantics)
unsigned int test_unsigned_gt(unsigned short *u1, unsigned short *u2, int n) {
    unsigned int sum = 0;
    for (int i = 0; i < n; i++) {
        if (u1[i] > u2[i]) {
            sum += u1[i];
        }
    }
    return sum;
}

// Nested conditional with comparison
int test_nested_conditional(int *arr, int *thresh1, int *thresh2, int n) {
    int count = 0;
    for (int i = 0; i < n; i++) {
        // Complex conditional using <= and >=
        if (arr[i] >= thresh1[i] && arr[i] <= thresh2[i]) {
            count++;
        }
    }
    return count;
}

// Generate mask array based on comparison
void test_mask_generation(char *src, char *dst, char threshold, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src[i] > threshold) ? 1 : 0;
    }
}

// Conditional store with comparison
void test_conditional_store(int *src, int *dst, int threshold, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src[i] <= threshold) ? src[i] : -1;
    }
}

int main() {
    // Initialize arrays with different patterns
    int a[N], b[N], c[N];
    short x[M], y[M];
    char chars[L];
    unsigned short us1[M], us2[M];
    int data1[L], limits[L];
    char src_chars[L], dst_chars[L];
    int thresh1[N], thresh2[N];
    
    // Fill arrays with varying data
    for (int i = 0; i < N; i++) {
        a[i] = i - N/2;          // Mixed positive/negative
        b[i] = i % 100;          // 0-99
        c[i] = 50 - i % 100;     // -49 to 50
        thresh1[i] = i % 50;
        thresh2[i] = 100 - i % 50;
    }
    
    for (int i = 0; i < M; i++) {
        x[i] = i * 2;
        y[i] = i * 3 - 100;
        us1[i] = i * 5;
        us2[i] = i * 3;
    }
    
    for (int i = 0; i < L; i++) {
        chars[i] = (char)(i - L/2);
        src_chars[i] = (char)(i % 128);
        data1[i] = i * 3;
        limits[i] = i * 2 + 50;
    }
    
    // Call all test functions to ensure execution
    int total = 0;
    
    total += test_gt(a, b, N);
    total += test_ge(x, y, M);
    total += test_lt(chars, 0, L);
    total += test_le(data1, limits, L);
    total += test_mixed_comparisons(a, b, c, N);
    total += test_unsigned_gt(us1, us2, M);
    total += test_nested_conditional(a, thresh1, thresh2, N);
    
    test_mask_generation(src_chars, dst_chars, 64, L);
    test_conditional_store(a, b, 25, N);
    
    // Use results to prevent dead code elimination
    for (int i = 0; i < L; i++) {
        total += dst_chars[i];
    }
    
    for (int i = 0; i < N; i++) {
        total += b[i];
    }
    
    printf("Total result: %d\n", total);
    return total > 0 ? 0 : 1;
}
