#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

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

unsigned test_ge_short(short *a, short *b, int n) {
    unsigned count = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            count++;
        }
    }
    return count;
}

int test_lt_char(char *a, char *b, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            sum += b[i];
        }
    }
    return sum;
}

unsigned test_le_ushort(unsigned short *a, unsigned short *b, int n) {
    unsigned sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

/* Mixed operators in one loop to increase density */
int test_mixed_comparisons(int *a, int *b, int *c, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        // Use all four comparison operators
        if (a[i] > b[i]) {
            result += 1;
        }
        if (a[i] >= c[i]) {
            result += 2;
        }
        if (b[i] < c[i]) {
            result += 4;
        }
        if (b[i] <= a[i]) {
            result += 8;
        }
    }
    return result;
}

/* Conditional assignment using ternary operator */
void test_ternary_gt(int *src1, int *src2, int *dst, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] > src2[i]) ? src1[i] : src2[i];
    }
}

void test_ternary_le(short *src1, short *src2, short *dst, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] <= src2[i]) ? src1[i] : src2[i];
    }
}

/* Initialize arrays with varying patterns */
void init_arrays(int *a, int *b, int *c, 
                 short *sa, short *sb,
                 char *ca, char *cb,
                 unsigned short *usa, unsigned short *usb,
                 int n) {
    for (int i = 0; i < n; i++) {
        a[i] = i - n/2;           // Mixed positive/negative
        b[i] = (i * 3) % 127;     // Different pattern
        c[i] = n - i;             // Decreasing values
        
        sa[i] = (short)(i * 2);
        sb[i] = (short)(i * 2 + 1);
        
        ca[i] = (char)(i % 128);
        cb[i] = (char)((i + 64) % 128);
        
        usa[i] = (unsigned short)(i * 3);
        usb[i] = (unsigned short)(i * 5);
    }
}

int main() {
    /* Declare arrays of different types and sizes */
    int a[N], b[N], c[N];
    short sa[M], sb[M];
    char ca[N], cb[N];
    unsigned short usa[N], usb[N];
    int dst_int[N];
    short dst_short[M];
    
    /* Initialize all arrays */
    init_arrays(a, b, c, sa, sb, ca, cb, usa, usb, N);
    
    /* Execute all test functions */
    int total = 0;
    
    total += test_gt_int(a, b, N);
    total += test_ge_short(sa, sb, M);
    total += test_lt_char(ca, cb, N);
    total += test_le_ushort(usa, usb, N);
    total += test_mixed_comparisons(a, b, c, N);
    
    test_ternary_gt(a, b, dst_int, N);
    test_ternary_le(sa, sb, dst_short, M);
    
    /* Use results to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        total += dst_int[i];
    }
    for (int i = 0; i < M; i++) {
        total += dst_short[i];
    }
    
    printf("Total result: %d\n", total);
    
    return 0;
}
