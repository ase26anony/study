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
int test_le(unsigned int *a, unsigned int *b, int n) {
    unsigned int mask_sum = 0;
    for (int i = 0; i < n; i++) {
        mask_sum += (a[i] <= b[i]) ? 1 : 0;
    }
    return mask_sum;
}

/* Mixed comparisons in one loop to potentially trigger multiple cases */
int test_mixed_comparisons(int *a, int *b, int *c, int n) {
    int results[4] = {0};
    
    for (int i = 0; i < n; i++) {
        /* GT_EXPR */
        if (a[i] > b[i]) {
            results[0] += a[i];
        }
        
        /* GE_EXPR */
        if (a[i] >= c[i]) {
            results[1] += b[i];
        }
        
        /* LT_EXPR */
        if (b[i] < c[i]) {
            results[2] += c[i];
        }
        
        /* LE_EXPR */
        if (c[i] <= a[i]) {
            results[3] += 1;
        }
    }
    
    return results[0] + results[1] + results[2] + results[3];
}

/* Conditional assignment using ternary operator */
void test_ternary_operators(int *src1, int *src2, int *dst, int n) {
    for (int i = 0; i < n; i++) {
        /* Using all four comparison operators in ternaries */
        dst[i] = (src1[i] > src2[i]) ? src1[i] : 
                 (src1[i] >= src2[i]) ? src1[i] + src2[i] :
                 (src1[i] < src2[i]) ? src2[i] :
                 (src1[i] <= src2[i]) ? src1[i] - src2[i] : 0;
    }
}

/* Different data types and loop lengths */
int test_various_types() {
    int total = 0;
    
    /* char arrays with GT/GE */
    char char_arr1[M], char_arr2[M];
    for (int i = 0; i < M; i++) {
        char_arr1[i] = i - 64;
        char_arr2[i] = i % 128;
    }
    
    for (int i = 0; i < M; i++) {
        if (char_arr1[i] > char_arr2[i]) {  /* GT_EXPR */
            total += char_arr1[i];
        }
        if (char_arr1[i] >= char_arr2[i]) { /* GE_EXPR */
            total += char_arr2[i];
        }
    }
    
    /* short arrays with LT/LE */
    short short_arr1[N], short_arr2[N];
    for (int i = 0; i < N; i++) {
        short_arr1[i] = i * 2;
        short_arr2[i] = i * 3;
    }
    
    for (int i = 0; i < N; i++) {
        if (short_arr1[i] < short_arr2[i]) {  /* LT_EXPR */
            total += short_arr1[i];
        }
        if (short_arr1[i] <= short_arr2[i]) { /* LE_EXPR */
            total += short_arr2[i];
        }
    }
    
    return total;
}

int main() {
    /* Initialize arrays with different patterns */
    int arr1[N], arr2[N], arr3[N];
    short sarr1[M], sarr2[M];
    char carr1[L], carr2[L];
    unsigned int uarr1[N], uarr2[N];
    int dst[N];
    
    /* Fill arrays with varied data */
    for (int i = 0; i < N; i++) {
        arr1[i] = i * 3;
        arr2[i] = i * 2 + 1;
        arr3[i] = i * 4 - 2;
        uarr1[i] = i * 5;
        uarr2[i] = i * 3 + 10;
    }
    
    for (int i = 0; i < M; i++) {
        sarr1[i] = i - 50;
        sarr2[i] = i * 2;
    }
    
    for (int i = 0; i < L; i++) {
        carr1[i] = i % 100;
        carr2[i] = 100 - (i % 100);
    }
    
    /* Execute all test functions */
    int result = 0;
    
    result += test_gt(arr1, arr2, N);           /* GT_EXPR */
    result += test_ge(sarr1, sarr2, M);         /* GE_EXPR */
    result += test_lt(carr1, carr2, L);         /* LT_EXPR */
    result += test_le(uarr1, uarr2, N);         /* LE_EXPR */
    
    result += test_mixed_comparisons(arr1, arr2, arr3, N);
    
    test_ternary_operators(arr1, arr2, dst, N);
    for (int i = 0; i < N; i++) {
        result += dst[i];
    }
    
    result += test_various_types();
    
    printf("Final result: %d\n", result);
    
    return 0;
}
