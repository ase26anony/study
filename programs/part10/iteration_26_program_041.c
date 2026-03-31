#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 256
#define M 128
#define L 512

// Test functions for each comparison operator

int test_gt_int(int *a, int *b, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

unsigned int test_ge_unsigned(unsigned short *x, unsigned short *y, int n) {
    unsigned int count = 0;
    for (int i = 0; i < n; i++) {
        if (x[i] >= y[i]) {
            count += 1;
        }
    }
    return count;
}

short test_lt_short(short *arr, short threshold, int n) {
    short result = 0;
    for (int i = 0; i < n; i++) {
        if (arr[i] < threshold) {
            result += arr[i];
        }
    }
    return result;
}

char test_le_char(char *data1, char *data2, int n) {
    char diff = 0;
    for (int i = 0; i < n; i++) {
        if (data1[i] <= data2[i]) {
            diff += data1[i] - data2[i];
        }
    }
    return diff;
}

// Mixed comparisons in one loop to potentially trigger multiple cases
int test_mixed_comparisons(int *a, int *b, int *c, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        // Using all four comparison operators in conditional expressions
        int val = (a[i] > b[i]) ? 1 : 0;      // GT_EXPR
        val += (a[i] >= c[i]) ? 2 : 0;        // GE_EXPR  
        val += (b[i] < c[i]) ? 4 : 0;         // LT_EXPR
        val += (b[i] <= a[i]) ? 8 : 0;        // LE_EXPR
        result += val;
    }
    return result;
}

// Conditional assignment using comparisons
void test_conditional_assignment(int *src1, int *src2, int *dst, int n) {
    for (int i = 0; i < n; i++) {
        // Using > and <= in ternary operations
        dst[i] = (src1[i] > src2[i]) ? src1[i] : src2[i];
        dst[i] += (src1[i] <= 0) ? 100 : 0;
    }
}

// Test with unsigned char for different vector width
unsigned char test_uchar_comparisons(unsigned char *a, unsigned char *b, int n) {
    unsigned char mask = 0;
    for (int i = 0; i < n; i++) {
        // Mix of comparisons
        if (a[i] > b[i]) mask |= 0x01;
        if (a[i] >= b[i]) mask |= 0x02;
        if (a[i] < b[i]) mask |= 0x04;
        if (a[i] <= b[i]) mask |= 0x08;
    }
    return mask;
}

int main() {
    // Initialize with different patterns to ensure comparisons are meaningful
    int arr1[N], arr2[N], arr3[N];
    unsigned short us_arr1[M], us_arr2[M];
    short s_arr[L];
    char c_arr1[N], c_arr2[N];
    unsigned char uc_arr1[N], uc_arr2[N];
    int dst[N];
    
    srand(time(NULL));
    
    // Initialize arrays with varied data
    for (int i = 0; i < N; i++) {
        arr1[i] = rand() % 1000;
        arr2[i] = rand() % 1000;
        arr3[i] = rand() % 1000;
        c_arr1[i] = (char)(rand() % 256 - 128);
        c_arr2[i] = (char)(rand() % 256 - 128);
        uc_arr1[i] = rand() % 256;
        uc_arr2[i] = rand() % 256;
    }
    
    for (int i = 0; i < M; i++) {
        us_arr1[i] = rand() % 65535;
        us_arr2[i] = rand() % 65535;
    }
    
    for (int i = 0; i < L; i++) {
        s_arr[i] = (short)(rand() % 65536 - 32768);
    }
    
    // Call all test functions to ensure execution
    int total = 0;
    
    total += test_gt_int(arr1, arr2, N);           // GT_EXPR
    total += test_ge_unsigned(us_arr1, us_arr2, M); // GE_EXPR
    total += test_lt_short(s_arr, 0, L);           // LT_EXPR
    total += test_le_char(c_arr1, c_arr2, N);      // LE_EXPR
    total += test_mixed_comparisons(arr1, arr2, arr3, N); // All four
    
    test_conditional_assignment(arr1, arr2, dst, N);
    
    // Use dst to prevent dead code elimination
    for (int i = 0; i < 10; i++) {
        total += dst[i];
    }
    
    total += test_uchar_comparisons(uc_arr1, uc_arr2, N);
    
    printf("Total result: %d\n", total);
    printf("(This value varies due to random initialization)\n");
    
    return 0;
}
