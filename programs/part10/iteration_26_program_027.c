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
    int results[4] = {0};
    
    for (int i = 0; i < n; i++) {
        /* All four comparison operators in separate branches */
        if (a[i] > b[i]) {
            results[0] += a[i];
        }
        
        if (a[i] >= c[i]) {
            results[1] += b[i];
        }
        
        if (b[i] < c[i]) {
            results[2] += c[i];
        }
        
        if (c[i] <= a[i]) {
            results[3] += i;
        }
    }
    
    return results[0] + results[1] + results[2] + results[3];
}

/* Conditional assignment using ternary operator */
void test_ternary_gt(int *src1, int *src2, int *dst, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] > src2[i]) ? src1[i] : src2[i];
    }
}

/* Conditional assignment with LE operator */
void test_ternary_le(unsigned short *src1, unsigned short *src2, unsigned short *dst, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] <= src2[i]) ? src1[i] : src2[i];
    }
}

/* Initialize arrays with varying patterns */
void init_arrays(int *a, int *b, int *c, char *ca, char *cb, 
                 short *sa, short *sb, unsigned int *ua, unsigned int *ub,
                 unsigned short *usa, unsigned short *usb, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = i;
        b[i] = n - i;
        c[i] = i * 2;
        ca[i] = (char)(i % 128);
        cb[i] = (char)((i + 64) % 128);
        sa[i] = (short)(i * 3);
        sb[i] = (short)(i * 2);
        ua[i] = (unsigned int)(i * 5);
        ub[i] = (unsigned int)(i * 3);
        usa[i] = (unsigned short)(i % 65535);
        usb[i] = (unsigned short)((i + 32768) % 65535);
    }
}

int main() {
    /* Allocate and initialize arrays of different types and sizes */
    int a[N], b[N], c[N];
    char ca[M], cb[M];
    short sa[L], sb[L];
    unsigned int ua[N], ub[N];
    unsigned short usa[M], usb[M];
    int dst1[N];
    unsigned short dst2[M];
    
    srand(time(NULL));
    
    /* Initialize with different patterns */
    init_arrays(a, b, c, ca, cb, sa, sb, ua, ub, usa, usb, N);
    
    int total = 0;
    
    /* Call each test function with different data types and loop lengths */
    total += test_gt(a, b, N);           // GT_EXPR with int arrays
    total += test_ge(sa, sb, L);         // GE_EXPR with short arrays
    total += test_lt(ca, cb, M);         // LT_EXPR with char arrays
    total += test_le(ua, ub, N);         // LE_EXPR with unsigned int arrays
    
    /* Test mixed comparisons */
    total += test_mixed_comparisons(a, b, c, N);
    
    /* Test ternary conditional assignments */
    test_ternary_gt(a, b, dst1, N);
    test_ternary_le(usa, usb, dst2, M);
    
    /* Use results to prevent dead code elimination */
    for (int i = 0; i < N; i++) {
        total += dst1[i];
    }
    
    for (int i = 0; i < M; i++) {
        total += dst2[i];
    }
    
    printf("Total result: %d\n", total);
    printf("(This value varies based on array contents)\n");
    
    return 0;
}
