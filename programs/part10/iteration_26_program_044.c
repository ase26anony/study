#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

/* Test functions for each comparison operator */

int test_gt_char(char *a, char *b, char *out) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            out[i] = a[i];
            sum += 1;
        } else {
            out[i] = b[i];
        }
    }
    return sum;
}

int test_ge_short(short *a, short *b, short *out) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            out[i] = a[i];
            sum += 2;
        } else {
            out[i] = b[i];
        }
    }
    return sum;
}

int test_lt_int(int *a, int *b, int *out) {
    int sum = 0;
    for (int i = 0; i < M; i++) {
        if (a[i] < b[i]) {
            out[i] = a[i];
            sum += 3;
        } else {
            out[i] = b[i];
        }
    }
    return sum;
}

int test_le_mixed(unsigned int *a, int *b, int *out) {
    int sum = 0;
    for (int i = 0; i < M; i++) {
        if ((unsigned int)a[i] <= (unsigned int)b[i]) {
            out[i] = a[i];
            sum += 4;
        } else {
            out[i] = b[i];
        }
    }
    return sum;
}

/* Loop with all four operators in separate branches */
int test_all_operators(int *a, int *b, int *c, int *out) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            out[i] = 1;
            sum += 1;
        } else if (a[i] >= c[i]) {
            out[i] = 2;
            sum += 2;
        } else if (b[i] < c[i]) {
            out[i] = 3;
            sum += 3;
        } else if (a[i] <= b[i]) {
            out[i] = 4;
            sum += 4;
        } else {
            out[i] = 0;
        }
    }
    return sum;
}

/* Conditional assignment using ternary operator */
int test_ternary_gt(int *a, int *b, int *out) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        out[i] = (a[i] > b[i]) ? a[i] : b[i];
        sum += out[i];
    }
    return sum;
}

/* Mixed data types and comparisons */
int test_mixed_types(char *a, short *b, int *c, int *out) {
    int sum = 0;
    for (int i = 0; i < M; i++) {
        if (a[i] > (char)b[i]) {
            out[i] = 1;
            sum += 1;
        }
        if ((short)c[i] >= b[i]) {
            out[i] += 2;
            sum += 2;
        }
        if (a[i] < (char)c[i]) {
            out[i] += 3;
            sum += 3;
        }
        if ((unsigned char)a[i] <= (unsigned char)b[i]) {
            out[i] += 4;
            sum += 4;
        }
    }
    return sum;
}

int main() {
    /* Initialize arrays with different patterns */
    char a_char[N], b_char[N];
    short a_short[N], b_short[N];
    int a_int[M], b_int[M], c_int[M];
    unsigned int a_uint[M], b_uint[M];
    
    char out_char[N];
    short out_short[N];
    int out_int1[M], out_int2[M], out_int3[N], out_int4[M];
    
    /* Fill arrays with varying data to ensure comparisons are meaningful */
    for (int i = 0; i < N; i++) {
        a_char[i] = (i % 3) * 10 - 15;
        b_char[i] = (i % 5) * 7 - 10;
        a_short[i] = (i % 7) * 100 - 300;
        b_short[i] = (i % 11) * 70 - 200;
        if (i < M) {
            a_int[i] = i * 2 - 100;
            b_int[i] = i * 3 - 150;
            c_int[i] = i * 5 - 200;
            a_uint[i] = i * 10;
            b_uint[i] = i * 8 + 50;
        }
    }
    
    /* Call all test functions to ensure all comparison operators are used */
    int total_sum = 0;
    
    total_sum += test_gt_char(a_char, b_char, out_char);
    total_sum += test_ge_short(a_short, b_short, out_short);
    total_sum += test_lt_int(a_int, b_int, out_int1);
    total_sum += test_le_mixed(a_uint, b_int, out_int2);
    total_sum += test_all_operators(a_int, b_int, c_int, out_int3);
    total_sum += test_ternary_gt(a_int, b_int, out_int1);
    total_sum += test_mixed_types(a_char, a_short, a_int, out_int4);
    
    /* Use results to prevent dead code elimination */
    printf("Total sum: %d\n", total_sum);
    
    /* Also print a sample value to verify execution */
    printf("Sample outputs: %d, %d, %d\n", 
           out_char[10], out_short[20], out_int1[30]);
    
    return 0;
}
