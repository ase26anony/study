#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

/* Test functions for each comparison operator */

int test_gt_int(int *a, int *b, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {  // GT_EXPR
            sum += a[i];
        }
    }
    return sum;
}

unsigned int test_ge_short(short *a, short *b, int n) {
    unsigned int count = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {  // GE_EXPR
            count++;
        }
    }
    return count;
}

char test_lt_char(char *a, char threshold, int n) {
    char sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] < threshold) {  // LT_EXPR
            sum += a[i];
        }
    }
    return sum;
}

int test_le_mixed(int *a, unsigned short *b, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] <= (int)b[i]) {  // LE_EXPR
            result |= 1 << (i & 0xF);
        }
    }
    return result;
}

/* Loop with all four operators in separate branches */
int test_all_operators(int *a, int *b, int *c, int n) {
    int results[4] = {0};
    
    for (int i = 0; i < n; i++) {
        // All four comparisons in one loop
        if (a[i] > b[i]) {     // GT_EXPR
            results[0] += a[i];
        }
        if (a[i] >= c[i]) {    // GE_EXPR
            results[1] += b[i];
        }
        if (b[i] < a[i]) {     // LT_EXPR
            results[2] += c[i];
        }
        if (c[i] <= b[i]) {    // LE_EXPR
            results[3] += a[i];
        }
    }
    
    return results[0] + results[1] + results[2] + results[3];
}

/* Unsigned comparisons to ensure different type handling */
unsigned int test_unsigned_gt(unsigned char *a, unsigned char *b, int n) {
    unsigned int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {  // GT_EXPR on unsigned
            sum += a[i];
        }
    }
    return sum;
}

/* Conditional assignment using ternary operator */
void test_ternary_lt_le(int *src, int *dst, int threshold, int n) {
    for (int i = 0; i < n; i++) {
        // Both LT and LE in ternary forms
        dst[i] = (src[i] < threshold) ? src[i] * 2 : src[i];      // LT_EXPR
        dst[i] += (src[i] <= threshold) ? 1 : 0;                  // LE_EXPR
    }
}

/* Initialize arrays with varying patterns */
void init_arrays(int *a, int *b, int *c, 
                 short *sa, short *sb,
                 char *ca, unsigned char *ua, unsigned char *ub,
                 unsigned short *us, int size) {
    for (int i = 0; i < size; i++) {
        a[i] = i - size/2;                    // Mixed positive/negative
        b[i] = (i * 3) % 100;
        c[i] = size - i;
        sa[i] = (short)(i * 2);
        sb[i] = (short)(i * 2 + 1);
        ca[i] = (char)(i % 128 - 64);
        ua[i] = (unsigned char)(i % 256);
        ub[i] = (unsigned char)((i * 7) % 256);
        us[i] = (unsigned short)(i % 1000);
    }
}

int main() {
    /* Declare arrays of different types and sizes */
    int arr1[N], arr2[N], arr3[N];
    short sarr1[M], sarr2[M];
    char carr1[N];
    unsigned char uarr1[N], uarr2[N];
    unsigned short usarr[M];
    int dst[N];
    
    /* Initialize all arrays */
    init_arrays(arr1, arr2, arr3, sarr1, sarr2, carr1, uarr1, uarr2, usarr, N);
    
    int total = 0;
    
    /* Execute all test functions */
    total += test_gt_int(arr1, arr2, N);           // > comparison
    total += test_ge_short(sarr1, sarr2, M);       // >= comparison
    total += test_lt_char(carr1, 0, N);            // < comparison
    total += test_le_mixed(arr1, usarr, N);        // <= comparison
    total += test_all_operators(arr1, arr2, arr3, N); // All four in one loop
    total += test_unsigned_gt(uarr1, uarr2, N);    // Unsigned >
    
    test_ternary_lt_le(arr1, dst, 50, N);          // Ternary with < and <=
    
    /* Use dst to prevent elimination */
    for (int i = 0; i < N; i++) {
        total += dst[i];
    }
    
    printf("Total result: %d\n", total);
    printf("(This value is non-deterministic due to array initialization)\n");
    
    return 0;
}
