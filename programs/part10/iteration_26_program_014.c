#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

/* Test functions for each comparison operator */

int test_gt_int(int *a, int *b) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

int test_ge_short(short *a, short *b) {
    int count = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            count++;
        }
    }
    return count;
}

int test_lt_char(char *a, char *b) {
    int sum = 0;
    for (int i = 0; i < M; i++) {
        if (a[i] < b[i]) {
            sum += b[i];
        }
    }
    return sum;
}

int test_le_mixed(int *a, short *b) {
    int result = 0;
    for (int i = 0; i < N; i++) {
        result += (a[i] <= (int)b[i]) ? a[i] : 0;
    }
    return result;
}

/* Test with unsigned types */
unsigned test_gt_unsigned(unsigned *a, unsigned *b) {
    unsigned sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

/* Test with all four operators in one loop */
int test_all_comparisons(int *a, int *b, int *c, int *d) {
    int results[4] = {0};
    
    for (int i = 0; i < N; i++) {
        // Each comparison uses a different operator
        if (a[i] > b[i]) {
            results[0] += a[i];
        }
        
        if (a[i] >= c[i]) {
            results[1] += b[i];
        }
        
        if (b[i] < c[i]) {
            results[2] += c[i];
        }
        
        if (c[i] <= d[i]) {
            results[3] += d[i];
        }
    }
    
    return results[0] + results[1] + results[2] + results[3];
}

/* Helper to initialize arrays */
void init_arrays(int *arr_int, short *arr_short, char *arr_char, 
                 unsigned *arr_uint, int size) {
    for (int i = 0; i < size; i++) {
        arr_int[i] = (i % 3 == 0) ? i : -i;
        arr_short[i] = (short)(i * 2);
        arr_char[i] = (char)(i % 128);
        arr_uint[i] = (unsigned)(i * 3);
    }
}

int main() {
    /* Declare arrays of different types and sizes */
    int a_int[N], b_int[N], c_int[N], d_int[N];
    short a_short[N], b_short[N];
    char a_char[M], b_char[M];
    unsigned a_uint[N], b_uint[N];
    
    /* Initialize with varied data */
    init_arrays(a_int, a_short, a_char, a_uint, N);
    init_arrays(b_int, b_short, b_char, b_uint, N);
    
    /* Create some variation in the data */
    for (int i = 0; i < N; i++) {
        c_int[i] = i * 2;
        d_int[i] = i * 3;
        b_uint[i] = a_uint[i] + (i % 5);
    }
    
    for (int i = 0; i < M; i++) {
        b_char[i] = (char)(i % 64);
    }
    
    /* Execute all test functions */
    int total = 0;
    
    total += test_gt_int(a_int, b_int);
    total += test_ge_short(a_short, b_short);
    total += test_lt_char(a_char, b_char);
    total += test_le_mixed(a_int, b_short);
    total += (int)test_gt_unsigned(a_uint, b_uint);
    total += test_all_comparisons(a_int, b_int, c_int, d_int);
    
    /* Additional loops with different lengths and types */
    {
        char small_arr[64];
        char small_thresh[64];
        int small_sum = 0;
        
        for (int i = 0; i < 64; i++) {
            small_arr[i] = (char)(i - 32);
            small_thresh[i] = (char)(i % 20);
        }
        
        for (int i = 0; i < 64; i++) {
            if (small_arr[i] <= small_thresh[i]) {
                small_sum += small_arr[i];
            }
        }
        total += small_sum;
    }
    
    {
        short medium_arr[512];
        short medium_vals[512];
        int medium_count = 0;
        
        for (int i = 0; i < 512; i++) {
            medium_arr[i] = (short)(i % 100);
            medium_vals[i] = (short)(50 - (i % 100));
        }
        
        for (int i = 0; i < 512; i++) {
            medium_count += (medium_arr[i] < medium_vals[i]) ? 1 : 0;
        }
        total += medium_count;
    }
    
    printf("Total result: %d\n", total);
    return total > 0 ? 0 : 1;
}
