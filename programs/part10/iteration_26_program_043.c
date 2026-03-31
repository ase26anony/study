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
    unsigned mask_sum = 0;
    for (int i = 0; i < N; i++) {
        mask_sum += (a[i] > b[i]) ? 1 : 0;
    }
    return mask_sum;
}

/* Test with all four operators in one loop */
int test_all_comparisons(int *a, int *b, int *c, int *d) {
    int total = 0;
    for (int i = 0; i < N; i++) {
        // Using all four comparison operators
        if (a[i] > b[i]) total += 1;
        if (a[i] >= c[i]) total += 2;
        if (b[i] < d[i]) total += 3;
        if (c[i] <= a[i]) total += 4;
    }
    return total;
}

/* Test with different loop lengths and data types */
int test_variable_lengths(char *data, int *thresholds) {
    int sum = 0;
    // Loop with char type
    for (int i = 0; i < 128; i++) {
        if (data[i] > 0) sum += data[i];
    }
    
    // Loop with int type
    for (int i = 0; i < 256; i++) {
        if (thresholds[i] <= 100) sum -= thresholds[i];
    }
    
    return sum;
}

/* Initialize arrays with varied data */
void init_arrays(int *a, int *b, short *s1, short *s2, char *c1, char *c2, 
                 unsigned *u1, unsigned *u2, int *c, int *d) {
    for (int i = 0; i < N; i++) {
        a[i] = i - N/2;           // Mixed positive/negative
        b[i] = i * 2 - N;
        s1[i] = (short)(i * 3);
        s2[i] = (short)(i * 2 + 5);
        c1[i] = (char)(i % 128 - 64);
        c2[i] = (char)(i % 64 - 32);
        u1[i] = i * 7;
        u2[i] = i * 5 + 3;
        c[i] = i * 4 - 100;
        d[i] = i * 3 + 50;
    }
}

int main() {
    /* Declare arrays of different types */
    int a[N], b[N], c[N], d[N];
    short s1[N], s2[N];
    char c1[N], c2[N];
    unsigned u1[N], u2[N];
    
    /* Initialize with varied data */
    init_arrays(a, b, s1, s2, c1, c2, u1, u2, c, d);
    
    /* Execute all test functions */
    int result = 0;
    
    result += test_gt_int(a, b);           // > comparison
    result += test_ge_short(s1, s2);       // >= comparison  
    result += test_lt_char(c1, c2);        // < comparison
    result += test_le_mixed(a, s1);        // <= comparison
    result += test_gt_unsigned(u1, u2);    // > with unsigned
    result += test_all_comparisons(a, b, c, d);  // All four in one loop
    result += test_variable_lengths(c1, a);      // Mixed lengths/types
    
    /* Use the result to prevent dead code elimination */
    printf("Final result: %d\n", result);
    
    /* Additional test with conditional assignment (ternary operator) */
    int out[N];
    for (int i = 0; i < N; i++) {
        // Using <= in ternary operator
        out[i] = (a[i] <= b[i]) ? a[i] : b[i];
    }
    
    /* Verify some outputs */
    int check = 0;
    for (int i = 0; i < N; i++) {
        check += out[i];
    }
    printf("Check sum: %d\n", check);
    
    return 0;
}
