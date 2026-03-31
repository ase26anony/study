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

unsigned int test_ge_short(short *a, short *b) {
    unsigned int count = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            count++;
        }
    }
    return count;
}

int test_lt_char(char *a, char *b, char *out) {
    int sum = 0;
    for (int i = 0; i < M; i++) {
        out[i] = (a[i] < b[i]) ? 1 : 0;
        sum += out[i];
    }
    return sum;
}

unsigned short test_le_mixed(short *a, int *b, unsigned short *out) {
    unsigned short total = 0;
    for (int i = 0; i < N; i += 2) {
        // Using two different comparisons in same loop
        if (a[i] <= b[i]) {
            out[i] = 1;
            total++;
        } else {
            out[i] = 0;
        }
        
        if (a[i+1] <= b[i+1]) {
            out[i+1] = 2;
            total += 2;
        } else {
            out[i+1] = 0;
        }
    }
    return total;
}

/* Combined test with all four operators in one loop */
int test_all_comparisons(int *a, int *b, int *c, int *d) {
    int results[4] = {0};
    
    for (int i = 0; i < N; i++) {
        // GT_EXPR
        if (a[i] > b[i]) {
            results[0] += a[i];
        }
        
        // GE_EXPR  
        if (a[i] >= b[i]) {
            results[1] += b[i];
        }
        
        // LT_EXPR
        if (c[i] < d[i]) {
            results[2] += c[i];
        }
        
        // LE_EXPR
        if (c[i] <= d[i]) {
            results[3] += d[i];
        }
    }
    
    return results[0] + results[1] + results[2] + results[3];
}

/* Test with unsigned types */
unsigned int test_unsigned_comparisons(unsigned char *a, unsigned char *b) {
    unsigned int sum = 0;
    for (int i = 0; i < M; i++) {
        // Mix of comparisons with unsigned
        if (a[i] > b[i]) sum += a[i];
        if (a[i] >= b[i]) sum += b[i];
        if (a[i] < b[i]) sum += 1;
        if (a[i] <= b[i]) sum += 2;
    }
    return sum;
}

/* Helper to initialize arrays */
void init_arrays(int *a, int *b, short *sa, short *sb, char *ca, char *cb, 
                 unsigned char *ua, unsigned char *ub) {
    for (int i = 0; i < N; i++) {
        a[i] = (i * 3) % 100;
        b[i] = (i * 7) % 100;
        sa[i] = (short)((i * 5) % 256);
        sb[i] = (short)((i * 11) % 256);
        
        if (i < M) {
            ca[i] = (char)((i * 13) % 128);
            cb[i] = (char)((i * 17) % 128);
            ua[i] = (unsigned char)((i * 19) % 256);
            ub[i] = (unsigned char)((i * 23) % 256);
        }
    }
}

int main() {
    /* Declare arrays of different types */
    int a[N], b[N], c[N], d[N];
    short sa[N], sb[N];
    char ca[M], cb[M];
    unsigned char ua[M], ub[M];
    char out_char[M];
    unsigned short out_short[N];
    
    /* Initialize with varying data */
    init_arrays(a, b, sa, sb, ca, cb, ua, ub);
    
    /* Copy for second set */
    for (int i = 0; i < N; i++) {
        c[i] = a[i] + 50;
        d[i] = b[i] + 25;
    }
    
    /* Execute all test functions */
    int total = 0;
    
    total += test_gt_int(a, b);
    total += test_ge_short(sa, sb);
    total += test_lt_char(ca, cb, out_char);
    total += test_le_mixed(sa, a, out_short);
    total += test_all_comparisons(a, b, c, d);
    total += test_unsigned_comparisons(ua, ub);
    
    /* Use results to prevent dead code elimination */
    printf("Total result: %d\n", total);
    
    /* Also print some array values to ensure execution */
    printf("Sample outputs: %d %u %d %u\n", 
           out_char[10], out_short[20], 
           (int)ua[30], (unsigned)ub[40]);
    
    return 0;
}
