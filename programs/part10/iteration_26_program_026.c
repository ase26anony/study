#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

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

unsigned test_ge_short(short *x, short *y, short threshold, int n) {
    unsigned count = 0;
    for (int i = 0; i < n; i++) {
        if (x[i] >= y[i]) {
            count += (x[i] >= threshold) ? 1 : 0;
        }
    }
    return count;
}

int test_lt_char(char *src1, char *src2, char *out, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        // Using ternary operator with less-than
        out[i] = (src1[i] < src2[i]) ? src1[i] : 0;
        sum += out[i];
    }
    return sum;
}

unsigned test_le_mixed(short *a, int *b, unsigned *mask, int n) {
    unsigned active = 0;
    for (int i = 0; i < n; i++) {
        // Less-than-or-equal with mixed types (promotion happens)
        mask[i] = (a[i] <= b[i]) ? 1 : 0;
        active += mask[i];
    }
    return active;
}

/* Combined test using all four operators in one loop */
int test_all_comparisons(int *arr1, int *arr2, int *results, int n) {
    int total = 0;
    for (int i = 0; i < n; i++) {
        int r = 0;
        // All four comparison operators in separate conditionals
        if (arr1[i] > arr2[i]) {
            r |= 0x1;  // GT_EXPR
        }
        if (arr1[i] >= arr2[i]) {
            r |= 0x2;  // GE_EXPR
        }
        if (arr1[i] < arr2[i]) {
            r |= 0x4;  // LT_EXPR
        }
        if (arr1[i] <= arr2[i]) {
            r |= 0x8;  // LE_EXPR
        }
        results[i] = r;
        total += r;
    }
    return total;
}

/* Unsigned comparisons to ensure different type handling */
uint32_t test_unsigned_gt(uint16_t *a, uint16_t *b, int n) {
    uint32_t sum = 0;
    for (int i = 0; i < n; i++) {
        // Unsigned greater-than
        sum += (a[i] > b[i]) ? a[i] : b[i];
    }
    return sum;
}

int test_le_with_swap(int8_t *a, int8_t *b, int8_t *out, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        // Pattern that might trigger operand swapping
        out[i] = (a[i] <= b[i]) ? a[i] : b[i];
        sum += out[i];
    }
    return sum;
}

/* Initialize arrays with varied data */
void init_arrays(int *a, int *b, short *s1, short *s2, char *c1, char *c2, 
                 uint16_t *u1, uint16_t *u2, int8_t *i1, int8_t *i2, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = i - n/2;  // Mix of positive and negative
        b[i] = i % 100;
        s1[i] = (i * 3) % 256;
        s2[i] = (i * 7) % 256;
        c1[i] = (i % 128) - 64;
        c2[i] = (i % 96) - 48;
        u1[i] = i * 2;
        u2[i] = i * 3;
        i1[i] = (i % 128) - 64;
        i2[i] = (i % 96) - 48;
    }
}

int main() {
    /* Allocate arrays with different sizes to trigger different vectorization factors */
    int a[N], b[N], results[N];
    short s1[M], s2[M];
    char c1[L], c2[L], out_char[L];
    uint16_t u1[N], u2[N];
    int8_t i1[M], i2[M], out_int8[M];
    unsigned mask[M];
    
    /* Initialize all arrays */
    init_arrays(a, b, s1, s2, c1, c2, u1, u2, i1, i2, N);
    
    /* Execute all test functions */
    int sum1 = test_gt_int(a, b, N);
    unsigned count1 = test_ge_short(s1, s2, 50, M);
    int sum2 = test_lt_char(c1, c2, out_char, L);
    unsigned active = test_le_mixed(s1, a, mask, M);
    int total = test_all_comparisons(a, b, results, N);
    uint32_t usum = test_unsigned_gt(u1, u2, N);
    int sum3 = test_le_with_swap(i1, i2, out_int8, M);
    
    /* Combine results to prevent dead code elimination */
    int final_result = sum1 + count1 + sum2 + active + total + usum + sum3;
    
    printf("Final result: %d\n", final_result);
    printf("Breakdown: gt_int=%d, ge_short=%u, lt_char=%d, le_mixed=%u\n",
           sum1, count1, sum2, active);
    printf("all_comparisons=%d, unsigned_gt=%u, le_with_swap=%d\n",
           total, usum, sum3);
    
    return 0;
}
