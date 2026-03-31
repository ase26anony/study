#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 256
#define M 128
#define L 512

// Test functions for each comparison operator

int test_gt_char(char *a, char *b, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

int test_ge_short(short *x, short *y, int n) {
    int count = 0;
    for (int i = 0; i < n; i++) {
        if (x[i] >= y[i]) {
            count++;
        }
    }
    return count;
}

int test_lt_int(int *arr, int threshold, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (arr[i] < threshold) {
            sum += arr[i];
        }
    }
    return sum;
}

int test_le_mixed(unsigned int *a, unsigned int *b, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        result += (a[i] <= b[i]) ? a[i] : b[i];
    }
    return result;
}

// Combined test with all four operators in one loop
int test_all_comparisons(int *src1, int *src2, int *out, int n) {
    int total = 0;
    for (int i = 0; i < n; i++) {
        // Using all four comparison operators in separate branches
        if (src1[i] > src2[i]) {
            out[i] = 1;
        } else if (src1[i] >= src2[i]) {
            out[i] = 2;
        } else if (src1[i] < src2[i]) {
            out[i] = 3;
        } else if (src1[i] <= src2[i]) {
            out[i] = 4;
        } else {
            out[i] = 0;
        }
        total += out[i];
    }
    return total;
}

// Additional tests with different data types and patterns

void test_gt_unsigned(unsigned short *a, unsigned short *b, unsigned short *out, int n) {
    for (int i = 0; i < n; i++) {
        out[i] = (a[i] > b[i]) ? a[i] : b[i];
    }
}

int test_le_char_signed(signed char *a, signed char *b, int n) {
    int diff = 0;
    for (int i = 0; i < n; i++) {
        diff += (a[i] <= b[i]) ? (b[i] - a[i]) : (a[i] - b[i]);
    }
    return diff;
}

void test_lt_ge_mixed(int *a, int *b, int *c, int n) {
    for (int i = 0; i < n; i++) {
        // Mix LT and GE in same loop
        c[i] = (a[i] < b[i]) ? -1 : ((a[i] >= b[i]) ? 1 : 0);
    }
}

int main() {
    // Initialize with different patterns to ensure comparisons are meaningful
    char char_arr1[N], char_arr2[N];
    short short_arr1[M], short_arr2[M];
    int int_arr1[L], int_arr2[L];
    unsigned int uint_arr1[N], uint_arr2[N];
    unsigned short ushort_arr1[M], ushort_arr2[M], ushort_out[M];
    signed char schar_arr1[N], schar_arr2[N];
    int out_arr[L];
    int mixed_arr1[L], mixed_arr2[L], mixed_out[L];
    
    srand(time(NULL));
    
    // Initialize arrays with various patterns
    for (int i = 0; i < N; i++) {
        char_arr1[i] = (char)(i - 128);  // Mix of negative and positive
        char_arr2[i] = (char)(rand() % 256 - 128);
        uint_arr1[i] = i * 3;
        uint_arr2[i] = i * 2 + 1;
        schar_arr1[i] = (signed char)(i % 256 - 128);
        schar_arr2[i] = (signed char)((i * 7) % 256 - 128);
    }
    
    for (int i = 0; i < M; i++) {
        short_arr1[i] = (short)(i * 2);
        short_arr2[i] = (short)(i * 2 + (i % 3));
        ushort_arr1[i] = (unsigned short)(i * 5);
        ushort_arr2[i] = (unsigned short)(i * 5 + 2);
    }
    
    for (int i = 0; i < L; i++) {
        int_arr1[i] = i * 3 - L/2;  // Center around zero
        int_arr2[i] = i * 2 - L/2;
        mixed_arr1[i] = rand() % 1000;
        mixed_arr2[i] = rand() % 1000;
    }
    
    int total = 0;
    
    // Execute all test functions
    total += test_gt_char(char_arr1, char_arr2, N);
    total += test_ge_short(short_arr1, short_arr2, M);
    total += test_lt_int(int_arr1, 0, L);  // Compare with threshold 0
    total += test_le_mixed(uint_arr1, uint_arr2, N);
    total += test_all_comparisons(mixed_arr1, mixed_arr2, out_arr, L);
    
    test_gt_unsigned(ushort_arr1, ushort_arr2, ushort_out, M);
    for (int i = 0; i < M; i++) {
        total += ushort_out[i];
    }
    
    total += test_le_char_signed(schar_arr1, schar_arr2, N);
    
    test_lt_ge_mixed(mixed_arr1, mixed_arr2, mixed_out, L);
    for (int i = 0; i < L; i++) {
        total += mixed_out[i];
    }
    
    printf("Total result: %d\n", total);
    printf("(This value should be consistent across runs)\n");
    
    return 0;
}
