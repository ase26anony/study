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

int test_ge_short(short *a, short *b, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            sum += b[i];
        }
    }
    return sum;
}

int test_lt_int(int *a, int *b, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            sum += a[i] - b[i];
        }
    }
    return sum;
}

int test_le_mixed(unsigned char *a, unsigned char *b, int n) {
    int count = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
            count++;
        }
    }
    return count;
}

// Function using all four operators in separate branches
int test_all_operators(int *arr1, int *arr2, int *thresholds, int n) {
    int results[4] = {0};
    
    for (int i = 0; i < n; i++) {
        // Each comparison could be vectorized separately
        if (arr1[i] > thresholds[0]) {
            results[0] += arr1[i];
        }
        if (arr2[i] >= thresholds[1]) {
            results[1] += arr2[i];
        }
        if (arr1[i] < thresholds[2]) {
            results[2] += arr1[i];
        }
        if (arr2[i] <= thresholds[3]) {
            results[3] += arr2[i];
        }
    }
    
    return results[0] + results[1] + results[2] + results[3];
}

// Function using conditional operator (?:) with comparisons
void test_conditional_operator(short *src1, short *src2, char *mask, int n) {
    for (int i = 0; i < n; i++) {
        // These generate comparison operations that need to be vectorized
        mask[i] = (src1[i] > src2[i]) ? 1 : 0;
    }
}

// Additional test with different data types and loop lengths
unsigned long test_variable_lengths(int *data, int *limits, int *lengths, int num_segments) {
    unsigned long total = 0;
    
    for (int seg = 0; seg < num_segments; seg++) {
        int len = lengths[seg];
        int limit = limits[seg];
        
        // Mix of comparison operators
        for (int i = 0; i < len; i++) {
            if (data[i] > limit) {
                total += data[i];
            } else if (data[i] <= limit / 2) {
                total -= data[i];
            }
        }
        data += len;
    }
    
    return total;
}

int main() {
    // Initialize with deterministic but varied data
    srand(42);
    
    // Arrays of different types and sizes
    char char_arr1[N], char_arr2[N];
    short short_arr1[M], short_arr2[M];
    int int_arr1[L], int_arr2[L];
    unsigned char uchar_arr1[N], uchar_arr2[N];
    int all_ops_arr1[M], all_ops_arr2[M], thresholds[4];
    short cond_src1[N], cond_src2[N];
    char mask[N];
    
    // Variable length test data
    int var_data[512];
    int var_limits[8] = {100, 200, 50, 150, 75, 125, 175, 225};
    int var_lengths[8] = {32, 64, 48, 96, 56, 72, 88, 104};
    
    // Initialize arrays with mixed values
    for (int i = 0; i < N; i++) {
        char_arr1[i] = (i % 3) * 10 - 15;
        char_arr2[i] = (i % 5) * 7 - 10;
        uchar_arr1[i] = i % 256;
        uchar_arr2[i] = (i * 3) % 256;
        cond_src1[i] = i * 2;
        cond_src2[i] = i * 3;
    }
    
    for (int i = 0; i < M; i++) {
        short_arr1[i] = i * 10;
        short_arr2[i] = i * 7 + 5;
        all_ops_arr1[i] = i * 3 - M/2;
        all_ops_arr2[i] = i * 4 - M/2;
    }
    
    for (int i = 0; i < L; i++) {
        int_arr1[i] = i * 5 - L/2;
        int_arr2[i] = i * 3 - L/2;
    }
    
    for (int i = 0; i < 4; i++) {
        thresholds[i] = i * 50;
    }
    
    for (int i = 0; i < 512; i++) {
        var_data[i] = rand() % 300;
    }
    
    // Execute all test functions
    int result = 0;
    
    // Test each comparison operator separately
    result += test_gt_char(char_arr1, char_arr2, N);
    result += test_ge_short(short_arr1, short_arr2, M);
    result += test_lt_int(int_arr1, int_arr2, L);
    result += test_le_mixed(uchar_arr1, uchar_arr2, N);
    
    // Test function with all operators
    result += test_all_operators(all_ops_arr1, all_ops_arr2, thresholds, M);
    
    // Test conditional operator
    test_conditional_operator(cond_src1, cond_src2, mask, N);
    for (int i = 0; i < N; i++) {
        result += mask[i];
    }
    
    // Test variable lengths
    result += test_variable_lengths(var_data, var_limits, var_lengths, 8);
    
    printf("Final result: %d\n", result);
    
    // Additional loop with mixed signed/unsigned comparisons
    unsigned int usum = 0;
    int isum = 0;
    unsigned short ushort_arr[N];
    int int_arr[N];
    
    for (int i = 0; i < N; i++) {
        ushort_arr[i] = i * 2;
        int_arr[i] = i * 3 - N/2;
    }
    
    // Mix of signed and unsigned comparisons
    for (int i = 0; i < N; i++) {
        // Unsigned comparison
        if (ushort_arr[i] > (unsigned short)(N/2)) {
            usum += ushort_arr[i];
        }
        // Signed comparison
        if (int_arr[i] <= 0) {
            isum -= int_arr[i];
        }
    }
    
    printf("Unsigned sum: %u, Signed sum: %d\n", usum, isum);
    
    return 0;
}
