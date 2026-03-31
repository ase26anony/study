#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 256
#define M 128
#define L 512

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

unsigned int test_ge_unsigned(unsigned short *a, unsigned short *b, int n) {
    unsigned int count = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            count++;
        }
    }
    return count;
}

short test_lt_short(short *a, short *b, short *out, int n) {
    short sum = 0;
    for (int i = 0; i < n; i++) {
        out[i] = (a[i] < b[i]) ? a[i] : b[i];
        sum += out[i];
    }
    return sum;
}

char test_le_char(char *a, char *b, char threshold, int n) {
    char result = 0;
    for (int i = 0; i < n; i++) {
        result |= (a[i] <= b[i]) ? a[i] : b[i];
    }
    return result;
}

/* Mixed comparisons in one loop to hit multiple cases */
int test_mixed_comparisons(int *a, int *b, int *c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        // Use all four comparison operators
        if (a[i] > b[i]) {
            sum += 1;
        }
        if (a[i] >= b[i]) {
            sum += 2;
        }
        if (a[i] < c[i]) {
            sum += 3;
        }
        if (a[i] <= c[i]) {
            sum += 4;
        }
    }
    return sum;
}

/* Test with different data patterns */
void init_arrays(int *a, int *b, int *c, unsigned short *us1, unsigned short *us2,
                 short *s1, short *s2, short *out, char *ch1, char *ch2, int size) {
    for (int i = 0; i < size; i++) {
        a[i] = i - size/2;          /* Mixed positive/negative */
        b[i] = i % 100;             /* Limited range */
        c[i] = size/2 - i;          /* Reverse pattern */
        
        us1[i] = i * 2;
        us2[i] = i * 2 + 1;
        
        s1[i] = (i % 2 == 0) ? i : -i;
        s2[i] = (i % 3 == 0) ? i * 2 : -i * 2;
        
        ch1[i] = (char)(i % 128);
        ch2[i] = (char)((i + 64) % 128);
    }
}

int main() {
    /* Allocate and initialize arrays of different types */
    int *a_int = (int*)malloc(N * sizeof(int));
    int *b_int = (int*)malloc(N * sizeof(int));
    int *c_int = (int*)malloc(N * sizeof(int));
    
    unsigned short *us_arr1 = (unsigned short*)malloc(M * sizeof(unsigned short));
    unsigned short *us_arr2 = (unsigned short*)malloc(M * sizeof(unsigned short));
    
    short *s_arr1 = (short*)malloc(L * sizeof(short));
    short *s_arr2 = (short*)malloc(L * sizeof(short));
    short *s_out = (short*)malloc(L * sizeof(short));
    
    char *ch_arr1 = (char*)malloc(N * sizeof(char));
    char *ch_arr2 = (char*)malloc(N * sizeof(char));
    
    /* Initialize with different patterns */
    init_arrays(a_int, b_int, c_int, us_arr1, us_arr2, 
                s_arr1, s_arr2, s_out, ch_arr1, ch_arr2, N);
    
    /* Execute all test functions */
    int total = 0;
    
    /* Greater than (GT_EXPR) - should trigger BIT_NOT_EXPR + BIT_AND_EXPR */
    total += test_gt_int(a_int, b_int, N);
    
    /* Greater or equal (GE_EXPR) - should trigger BIT_NOT_EXPR + BIT_IOR_EXPR */
    total += test_ge_unsigned(us_arr1, us_arr2, M);
    
    /* Less than (LT_EXPR) - should trigger BIT_NOT_EXPR + BIT_AND_EXPR + swap */
    total += test_lt_short(s_arr1, s_arr2, s_out, L);
    
    /* Less or equal (LE_EXPR) - should trigger BIT_NOT_EXPR + BIT_IOR_EXPR + swap */
    total += test_le_char(ch_arr1, ch_arr2, 64, N);
    
    /* Mixed comparisons - may trigger multiple cases in one vectorized loop */
    total += test_mixed_comparisons(a_int, b_int, c_int, N/2);
    
    /* Additional tests with different loop lengths and types */
    for (int i = 0; i < 100; i++) {
        /* Simple loop with <= comparison */
        int count = 0;
        for (int j = 0; j <= i; j++) {  // Note: loop condition uses <=
            if (a_int[j % N] <= b_int[j % N]) {
                count++;
            }
        }
        total += count;
    }
    
    printf("Total result: %d\n", total);
    printf("(This value should be consistent across runs)\n");
    
    /* Cleanup */
    free(a_int);
    free(b_int);
    free(c_int);
    free(us_arr1);
    free(us_arr2);
    free(s_arr1);
    free(s_arr2);
    free(s_out);
    free(ch_arr1);
    free(ch_arr2);
    
    return 0;
}
