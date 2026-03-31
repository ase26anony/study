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
    unsigned int mask_sum = 0;
    for (int i = 0; i < n; i++) {
        mask_sum += (a[i] <= b[i]) ? a[i] : 0;
    }
    return (int)mask_sum;
}

/* Mixed comparisons in one loop to potentially trigger multiple cases */
int test_mixed_comparisons(int *a, int *b, int *c, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        // Using ternary operators which generate comparison trees
        int gt_val = (a[i] > b[i]) ? 1 : 0;
        int ge_val = (a[i] >= c[i]) ? 2 : 0;
        int lt_val = (b[i] < c[i]) ? 3 : 0;
        int le_val = (b[i] <= a[i]) ? 4 : 0;
        
        result += gt_val + ge_val + lt_val + le_val;
    }
    return result;
}

/* Test with different data types and signedness */
short test_signed_unsigned(unsigned short *a, short *b, int n) {
    short diff = 0;
    for (int i = 0; i < n; i++) {
        // Mix signed and unsigned comparisons
        if ((short)a[i] > b[i]) {
            diff += a[i] - b[i];
        }
        if (a[i] <= (unsigned short)b[i]) {
            diff -= 1;
        }
    }
    return diff;
}

/* Test with array initialization */
void init_arrays(int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = n - i;
        c[i] = (i % 2 == 0) ? i : -i;
    }
}

void init_short_arrays(short *a, short *b, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (short)(i * 2);
        b[i] = (short)(i * 3 - n);
    }
}

void init_char_arrays(char *a, char *b, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (char)(i - 64);
        b[i] = (char)(128 - i);
    }
}

void init_unsigned_arrays(unsigned int *a, unsigned int *b, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = i * 3;
        b[i] = i * 2 + 100;
    }
}

int main() {
    /* Initialize random seed for variability */
    srand(time(NULL));
    
    /* Allocate and initialize arrays of different types and sizes */
    int arr1[N], arr2[N], arr3[N];
    short sarr1[M], sarr2[M];
    char carr1[L], carr2[L];
    unsigned int uarr1[N], uarr2[N];
    unsigned short usarr1[M];
    short sarr3[M];
    
    init_arrays(arr1, arr2, arr3, N);
    init_short_arrays(sarr1, sarr2, M);
    init_char_arrays(carr1, carr2, L);
    init_unsigned_arrays(uarr1, uarr2, N);
    
    /* Initialize mixed signed/unsigned arrays */
    for (int i = 0; i < M; i++) {
        usarr1[i] = (unsigned short)(i * 5);
        sarr3[i] = (short)(i * 4 - M/2);
    }
    
    /* Execute all test functions to ensure all comparisons are evaluated */
    int total_result = 0;
    
    total_result += test_gt(arr1, arr2, N);           // GT_EXPR
    total_result += test_ge(sarr1, sarr2, M);         // GE_EXPR
    total_result += test_lt(carr1, carr2, L);         // LT_EXPR
    total_result += test_le(uarr1, uarr2, N);         // LE_EXPR
    total_result += test_mixed_comparisons(arr1, arr2, arr3, N);
    total_result += test_signed_unsigned(usarr1, sarr3, M);
    
    /* Additional loops with different iteration counts and data types */
    
    /* Loop with GT and LE in same loop body */
    int alt_result = 0;
    for (int i = 0; i < 128; i++) {
        if (arr1[i] > arr3[i]) {
            alt_result += arr1[i];
        }
        if (arr2[i] <= arr1[i]) {
            alt_result -= arr2[i];
        }
    }
    total_result += alt_result;
    
    /* Loop with GE and LT in same loop body using different data widths */
    short short_result = 0;
    for (int i = 0; i < 64; i++) {
        if (sarr1[i] >= sarr2[i]) {
            short_result += sarr1[i];
        }
        if (sarr3[i] < (short)(i * 2)) {
            short_result -= sarr3[i];
        }
    }
    total_result += short_result;
    
    /* Final output to prevent dead code elimination */
    printf("Total result: %d\n", total_result);
    
    /* Additional check to ensure all loops execute */
    if (total_result != 0) {
        printf("All comparison tests executed successfully.\n");
    }
    
    return 0;
}
