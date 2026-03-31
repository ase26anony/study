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

/* Additional tests with unsigned types */
unsigned test_gt_unsigned(unsigned *a, unsigned *b) {
    unsigned sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            sum += a[i] - b[i];
        }
    }
    return sum;
}

unsigned test_le_unsigned_char(unsigned char *a, unsigned char *b) {
    unsigned count = 0;
    for (int i = 0; i < M; i++) {
        count += (a[i] <= b[i]) ? 1 : 0;
    }
    return count;
}

/* Loop containing all four comparison operators */
int test_all_comparisons(int *a, int *b, int *c) {
    int results[4] = {0};
    
    for (int i = 0; i < N; i++) {
        // Each comparison in separate branch
        if (a[i] > b[i]) {
            results[0] += a[i];
        }
        if (a[i] >= b[i]) {
            results[1] += b[i];
        }
        if (a[i] < c[i]) {
            results[2] += c[i];
        }
        if (a[i] <= c[i]) {
            results[3] += a[i];
        }
    }
    
    return results[0] + results[1] + results[2] + results[3];
}

/* Helper to initialize arrays */
void init_arrays(int *a, int *b, int *c, 
                 short *sa, short *sb,
                 char *ca, char *cb,
                 unsigned *ua, unsigned *ub,
                 unsigned char *uca, unsigned char *ucb) {
    for (int i = 0; i < N; i++) {
        a[i] = i - N/2;          // Mixed positive/negative
        b[i] = i % 100;          // 0-99
        c[i] = N/2 - i;          // Reverse order
        sa[i] = (short)(i * 2);
        sb[i] = (short)(i * 3);
        ua[i] = i * 5;
        ub[i] = i * 3;
    }
    
    for (int i = 0; i < M; i++) {
        ca[i] = (char)(i - M/2);
        cb[i] = (char)(i % 64);
        uca[i] = (unsigned char)i;
        ucb[i] = (unsigned char)(i * 2);
    }
}

int main() {
    /* Declare arrays of different types and sizes */
    int a[N], b[N], c[N];
    short sa[N], sb[N];
    char ca[M], cb[M];
    unsigned ua[N], ub[N];
    unsigned char uca[M], ucb[M];
    
    /* Initialize with varied data */
    init_arrays(a, b, c, sa, sb, ca, cb, ua, ub, uca, ucb);
    
    /* Execute all test functions */
    int total = 0;
    
    total += test_gt_int(a, b);
    total += test_ge_short(sa, sb);
    total += test_lt_char(ca, cb);
    total += test_le_mixed(a, sb);
    total += test_gt_unsigned(ua, ub);
    total += test_le_unsigned_char(uca, ucb);
    total += test_all_comparisons(a, b, c);
    
    /* Use the result to prevent dead code elimination */
    printf("Total result: %d\n", total);
    
    return 0;
}
