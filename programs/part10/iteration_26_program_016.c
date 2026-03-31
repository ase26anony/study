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
        mask_sum += (a[i] <= b[i]) ? 1 : 0;
    }
    return (int)mask_sum;
}

/* Mixed comparisons in one loop to potentially trigger multiple cases */
int test_mixed_comparisons(int *a, int *b, int *c, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        // Using ternary operators which generate comparison trees
        int gt_val = (a[i] > b[i]) ? a[i] : 0;
        int ge_val = (a[i] >= c[i]) ? 1 : 0;
        int lt_val = (b[i] < c[i]) ? b[i] : 0;
        int le_val = (c[i] <= a[i]) ? 1 : 0;
        
        result += gt_val + ge_val + lt_val + le_val;
    }
    return result;
}

/* Test with different data types and loop lengths */
short test_gt_short(short *a, short threshold, int n) {
    short sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] > threshold) {
            sum += a[i];
        }
    }
    return sum;
}

int test_le_int_mixed(int *a, int *b, int n) {
    int out[N];
    for (int i = 0; i < n; i++) {
        out[i] = (a[i] <= b[i]) ? a[i] : b[i];
    }
    
    // Compute checksum to prevent elimination
    int sum = 0;
    for (int i = 0; i < n; i++) {
        sum += out[i];
    }
    return sum;
}

/* Initialize arrays with varied data */
void init_arrays(int *a, int *b, int *c, short *sa, short *sb, 
                 char *ca, char *cb, unsigned int *ua, unsigned int *ub, int n) {
    for (int i = 0; i < n; i++) {
        a[i] = i - n/2;               // Mixed positive/negative
        b[i] = (i * 3) % 100;
        c[i] = (i * 7) % 50;
        sa[i] = (short)((i * 5) % 256);
        sb[i] = (short)((i * 11) % 256);
        ca[i] = (char)(i % 128);
        cb[i] = (char)((i * 3) % 128);
        ua[i] = (unsigned int)(i * 2);
        ub[i] = (unsigned int)(i * 2 + 1);
    }
}

int main() {
    /* Declare arrays of different types */
    int a[N], b[N], c[N];
    short sa[M], sb[M];
    char ca[L], cb[L];
    unsigned int ua[N], ub[N];
    
    /* Initialize with varied data */
    init_arrays(a, b, c, sa, sb, ca, cb, ua, ub, N);
    
    /* Seed for reproducibility */
    srand(42);
    
    /* Add some randomness */
    for (int i = 0; i < N; i++) {
        if (i % 7 == 0) a[i] = rand() % 100;
        if (i % 5 == 0) b[i] = rand() % 100;
    }
    
    /* Execute all test functions */
    int total = 0;
    
    total += test_gt(a, b, N);           // GT_EXPR case
    total += test_ge(sa, sb, M);         // GE_EXPR case
    total += test_lt(ca, cb, L);         // LT_EXPR case (with swap)
    total += test_le(ua, ub, N);         // LE_EXPR case (with swap)
    
    total += test_mixed_comparisons(a, b, c, N);  // Mixed comparisons
    
    total += test_gt_short(sa, 50, M);   // GT_EXPR with short
    total += test_le_int_mixed(a, b, N); // LE_EXPR with output array
    
    /* Additional loop with all comparisons in different forms */
    int mixed_result = 0;
    for (int i = 0; i < N; i++) {
        // Direct comparisons that should generate the tree codes
        mixed_result += (a[i] > b[i]) ? 1 : 0;    // GT_EXPR
        mixed_result += (a[i] >= c[i]) ? 1 : 0;   // GE_EXPR  
        mixed_result += (b[i] < c[i]) ? 1 : 0;    // LT_EXPR
        mixed_result += (c[i] <= a[i]) ? 1 : 0;   // LE_EXPR
    }
    total += mixed_result;
    
    printf("Total result: %d\n", total);
    
    return 0;
}
