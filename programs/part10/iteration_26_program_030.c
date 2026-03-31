#include <stdio.h>
#include <stdlib.h>

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
int test_le(unsigned short *a, unsigned short *b, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

/* Mixed comparisons in one loop to potentially trigger multiple cases */
int test_mixed_comparisons(int *arr1, int *arr2, int *out, int n) {
    int total = 0;
    for (int i = 0; i < n; i++) {
        /* Use all four comparison operators in separate branches */
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

/* Conditional assignment using ternary operator */
void test_ternary_comparisons(int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        /* Each uses a different comparison operator */
        c[i] = (a[i] > b[i]) ? a[i] : b[i];      // GT_EXPR
        c[i] += (a[i] >= b[i]) ? 1 : 0;          // GE_EXPR
        c[i] += (a[i] < b[i]) ? 10 : 0;          // LT_EXPR
        c[i] += (a[i] <= b[i]) ? 100 : 0;        // LE_EXPR
    }
}

/* Test with different data types and signedness */
int test_signed_unsigned_mix(int *signed_arr, unsigned *unsigned_arr, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        /* Mix signed and unsigned comparisons */
        if (signed_arr[i] > (int)unsigned_arr[i]) {
            result += 1;
        }
        if ((unsigned)signed_arr[i] <= unsigned_arr[i]) {
            result += 2;
        }
    }
    return result;
}

/* Initialize arrays with varying patterns */
void init_arrays(int *a, int *b, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = i * 2 - n;  /* Mix of positive and negative */
        b[i] = i * 3 - n;
    }
}

void init_short_arrays(short *a, short *b, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (short)(i % 100);
        b[i] = (short)((i * 7) % 100);
    }
}

void init_char_arrays(char *a, char *b, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (char)(i % 128);
        b[i] = (char)((i * 3) % 128);
    }
}

void init_ushort_arrays(unsigned short *a, unsigned short *b, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = (unsigned short)(i % 65535);
        b[i] = (unsigned short)((i * 5) % 65535);
    }
}

int main() {
    /* Allocate and initialize arrays of different types and sizes */
    int arr1[N], arr2[N];
    short sarr1[M], sarr2[M];
    char carr1[L], carr2[L];
    unsigned short usarr1[M], usarr2[M];
    int mixed_out[N];
    int ternary_arr[N];
    unsigned unsigned_arr[N];
    
    init_arrays(arr1, arr2, N);
    init_short_arrays(sarr1, sarr2, M);
    init_char_arrays(carr1, carr2, L);
    init_ushort_arrays(usarr1, usarr2, M);
    
    /* Initialize unsigned array */
    for (int i = 0; i < N; i++) {
        unsigned_arr[i] = (unsigned)(i * 11) % 1000;
    }
    
    /* Execute all test functions to ensure all comparisons are evaluated */
    int total = 0;
    
    total += test_gt(arr1, arr2, N);           // GT_EXPR
    total += test_ge(sarr1, sarr2, M);         // GE_EXPR
    total += test_lt(carr1, carr2, L);         // LT_EXPR
    total += test_le(usarr1, usarr2, M);       // LE_EXPR
    
    total += test_mixed_comparisons(arr1, arr2, mixed_out, N);
    
    test_ternary_comparisons(arr1, arr2, ternary_arr, N);
    for (int i = 0; i < N; i++) {
        total += ternary_arr[i];
    }
    
    total += test_signed_unsigned_mix(arr1, unsigned_arr, N);
    
    /* Print result to prevent dead code elimination */
    printf("Total result: %d\n", total);
    
    /* Additional verification prints */
    printf("Mixed comparisons output[0]: %d\n", mixed_out[0]);
    printf("Ternary result[0]: %d\n", ternary_arr[0]);
    
    return 0;
}
