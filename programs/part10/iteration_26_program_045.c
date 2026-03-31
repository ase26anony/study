#include <stdio.h>
#include <stdlib.h>

#define N 256
#define M 128

/* Test functions for each comparison operator */

/* Greater-than (GT_EXPR) */
int test_gt(int *a, int *b) {
    int sum = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

/* Greater-than-or-equal (GE_EXPR) */
int test_ge(short *a, short *b) {
    int count = 0;
    for (int i = 0; i < N; i++) {
        if (a[i] >= b[i]) {
            count++;
        }
    }
    return count;
}

/* Less-than (LT_EXPR) */
int test_lt(char *a, char *b) {
    int sum = 0;
    for (int i = 0; i < M; i++) {
        if (a[i] < b[i]) {
            sum += b[i];
        }
    }
    return sum;
}

/* Less-than-or-equal (LE_EXPR) */
int test_le(unsigned short *a, unsigned short *b) {
    int sum = 0;
    for (int i = 0; i < M; i++) {
        if (a[i] <= b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

/* Mixed comparisons in one loop to potentially trigger multiple cases */
int test_mixed_comparisons(int *a, int *b, int *c, int *d) {
    int result = 0;
    for (int i = 0; i < N; i++) {
        // Use all four comparison operators in conditional expressions
        int val1 = (a[i] > b[i]) ? a[i] : b[i];      // GT_EXPR
        int val2 = (c[i] >= d[i]) ? c[i] : d[i];     // GE_EXPR  
        int val3 = (a[i] < c[i]) ? a[i] : c[i];      // LT_EXPR
        int val4 = (b[i] <= d[i]) ? b[i] : d[i];     // LE_EXPR
        
        result += val1 + val2 + val3 + val4;
    }
    return result;
}

/* Another variant using different data types */
void test_vector_masks(char *src1, char *src2, char *dst) {
    for (int i = 0; i < N; i++) {
        // Generate masks using comparisons
        char mask1 = (src1[i] > src2[i]) ? 0xFF : 0x00;    // GT_EXPR
        char mask2 = (src1[i] >= 0) ? 0xFF : 0x00;         // GE_EXPR
        char mask3 = (src2[i] < 64) ? 0xFF : 0x00;         // LT_EXPR
        char mask4 = (src1[i] <= src2[i]) ? 0xFF : 0x00;   // LE_EXPR
        
        dst[i] = mask1 & mask2 & mask3 & mask4;
    }
}

int main() {
    // Initialize arrays with different patterns
    int a_int[N], b_int[N], c_int[N], d_int[N];
    short a_short[N], b_short[N];
    char a_char[M], b_char[M];
    unsigned short a_ushort[M], b_ushort[M];
    char src1[N], src2[N], dst[N];
    
    // Fill arrays with varying data to ensure comparisons are meaningful
    for (int i = 0; i < N; i++) {
        a_int[i] = i;
        b_int[i] = N - i;
        c_int[i] = i * 2;
        d_int[i] = i / 2;
        
        a_short[i] = (short)(i - 128);
        b_short[i] = (short)(i * 2 - 128);
        
        src1[i] = (char)(i % 128);
        src2[i] = (char)((i * 3) % 128);
    }
    
    for (int i = 0; i < M; i++) {
        a_char[i] = (char)(i - 64);
        b_char[i] = (char)(128 - i);
        
        a_ushort[i] = (unsigned short)(i * 10);
        b_ushort[i] = (unsigned short)(i * 5 + 100);
    }
    
    // Execute all test functions
    int total = 0;
    
    total += test_gt(a_int, b_int);
    total += test_ge(a_short, b_short);
    total += test_lt(a_char, b_char);
    total += test_le(a_ushort, b_ushort);
    total += test_mixed_comparisons(a_int, b_int, c_int, d_int);
    
    test_vector_masks(src1, src2, dst);
    
    // Use dst array to prevent dead code elimination
    for (int i = 0; i < N; i++) {
        total += dst[i];
    }
    
    printf("Result: %d\n", total);
    return 0;
}
