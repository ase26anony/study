#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128
#define L 512

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

unsigned int test_ge_short(short *x, short *y, int n) {
    unsigned int count = 0;
    for (int i = 0; i < n; i++) {
        if (x[i] >= y[i]) {
            count++;
        }
    }
    return count;
}

long test_lt_char(char *data, char threshold, int n) {
    long sum = 0;
    for (int i = 0; i < n; i++) {
        if (data[i] < threshold) {
            sum += data[i];
        }
    }
    return sum;
}

unsigned long test_le_mixed(int *arr1, int *arr2, char *mask, int n) {
    unsigned long result = 0;
    for (int i = 0; i < n; i++) {
        if (arr1[i] <= arr2[i]) {
            mask[i] = 1;
            result += arr1[i];
        } else {
            mask[i] = 0;
        }
    }
    return result;
}

/* Function using all four operators in one loop */
int test_all_comparisons(int *a, int *b, int *c, int *d, int n) {
    int total = 0;
    for (int i = 0; i < n; i++) {
        // Each comparison uses a different operator
        if (a[i] > b[i]) total += 1;    // GT_EXPR
        if (c[i] >= d[i]) total += 2;   // GE_EXPR  
        if (b[i] < c[i]) total += 3;    // LT_EXPR
        if (d[i] <= a[i]) total += 4;   // LE_EXPR
    }
    return total;
}

/* Additional tests with unsigned types */
unsigned int test_gt_unsigned(unsigned short *u1, unsigned short *u2, int n) {
    unsigned int sum = 0;
    for (int i = 0; i < n; i++) {
        if (u1[i] > u2[i]) {
            sum += u1[i];
        }
    }
    return sum;
}

int test_le_conditional(int *src, int threshold, int n) {
    int out[N];
    for (int i = 0; i < n; i++) {
        // Using conditional operator with <= comparison
        out[i] = (src[i] <= threshold) ? src[i] : threshold;
    }
    
    // Sum to prevent elimination
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += out[i];
    }
    return sum;
}

/* Test with different data patterns */
void initialize_data(int *a, int *b, short *s1, short *s2, 
                     char *cdata, unsigned short *u1, unsigned short *u2, int size) {
    for (int i = 0; i < size; i++) {
        a[i] = i - size/2;           // Mix of positive and negative
        b[i] = i % 100;
        s1[i] = (short)(i * 2);
        s2[i] = (short)(i * 3);
        cdata[i] = (char)(i % 128);
        u1[i] = (unsigned short)(i * 5);
        u2[i] = (unsigned short)(i * 4);
    }
}

int main() {
    // Allocate and initialize arrays
    int a[N], b[N], c[M], d[M];
    short s1[L], s2[L];
    char cdata[L];
    unsigned short u1[N], u2[N];
    char mask[M];
    
    initialize_data(a, b, s1, s2, cdata, u1, u2, N);
    
    // Initialize additional arrays
    for (int i = 0; i < M; i++) {
        c[i] = i * 3;
        d[i] = i * 2;
    }
    
    // Run all test functions
    int result1 = test_gt_int(a, b, N);
    unsigned int result2 = test_ge_short(s1, s2, L);
    long result3 = test_lt_char(cdata, 64, L);
    unsigned long result4 = test_le_mixed(c, d, mask, M);
    int result5 = test_all_comparisons(a, b, c, d, M);
    unsigned int result6 = test_gt_unsigned(u1, u2, N);
    int result7 = test_le_conditional(a, 50, N);
    
    // Combine results to prevent dead code elimination
    long final_result = (long)result1 + result2 + result3 + result4 + 
                       result5 + result6 + result7;
    
    // Use mask array to prevent optimization
    int mask_sum = 0;
    for (int i = 0; i < M; i++) {
        mask_sum += mask[i];
    }
    
    printf("Final result: %ld (mask sum: %d)\n", final_result, mask_sum);
    
    return 0;
}
