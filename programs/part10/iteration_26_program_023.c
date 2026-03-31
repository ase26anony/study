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

unsigned int test_ge_unsigned(unsigned short *a, unsigned short *b, int n) {
    unsigned int count = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            count++;
        }
    }
    return count;
}

long test_lt_long(long *a, long threshold, int n) {
    long sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] < threshold) {
            sum += a[i];
        }
    }
    return sum;
}

int test_le_char(char *a, char *b, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        result += (a[i] <= b[i]) ? 1 : 0;
    }
    return result;
}

// Mixed comparisons in a single loop
int test_mixed_comparisons(int *a, int *b, int *c, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        // Use all four comparison operators
        if (a[i] > b[i]) {
            sum += 1;
        }
        if (a[i] >= c[i]) {
            sum += 2;
        }
        if (b[i] < c[i]) {
            sum += 3;
        }
        if (b[i] <= a[i]) {
            sum += 4;
        }
    }
    return sum;
}

// Conditional assignment using comparisons
void test_conditional_assignment(short *src1, short *src2, short *dst, int n) {
    for (int i = 0; i < n; i++) {
        dst[i] = (src1[i] > src2[i]) ? src1[i] : src2[i];
    }
}

// Comparison with immediate values
int test_immediate_comparisons(int *arr, int n) {
    int count = 0;
    for (int i = 0; i < n; i++) {
        // Different comparison operators with constants
        if (arr[i] > 100) count++;
        if (arr[i] >= 50) count++;
        if (arr[i] < 0) count++;
        if (arr[i] <= -10) count++;
    }
    return count;
}

// Initialize arrays with varied data
void init_arrays(int *a, int *b, int *c, unsigned short *us1, unsigned short *us2,
                 long *long_arr, char *char1, char *char2, short *short1, short *short2,
                 int size) {
    for (int i = 0; i < size; i++) {
        a[i] = i - size/2;  // Mixed positive and negative
        b[i] = i % 100;
        c[i] = (i * 3) % 200;
        us1[i] = i % 256;
        us2[i] = (i * 2) % 256;
        long_arr[i] = i * 1000L;
        char1[i] = (i % 128) - 64;
        char2[i] = (i % 96) - 48;
        short1[i] = i - 100;
        short2[i] = i * 2 - 150;
    }
}

int main() {
    // Allocate and initialize arrays of different types and sizes
    int a[N], b[N], c[N];
    unsigned short us1[M], us2[M];
    long long_arr[L];
    char char1[N], char2[N];
    short short1[N], short2[N], short_dst[N];
    
    init_arrays(a, b, c, us1, us2, long_arr, char1, char2, short1, short2, N);
    
    // Seed random number generator
    srand(time(NULL));
    
    // Execute all test functions
    int total = 0;
    
    // Test greater-than with int
    total += test_gt_int(a, b, N);
    
    // Test greater-equal with unsigned short
    total += test_ge_unsigned(us1, us2, M);
    
    // Test less-than with long
    total += test_lt_long(long_arr, 25000L, L);
    
    // Test less-equal with char
    total += test_le_char(char1, char2, N);
    
    // Test mixed comparisons
    total += test_mixed_comparisons(a, b, c, N);
    
    // Test conditional assignment
    test_conditional_assignment(short1, short2, short_dst, N);
    for (int i = 0; i < N; i++) {
        total += short_dst[i];
    }
    
    // Test immediate comparisons
    total += test_immediate_comparisons(a, N);
    
    // Add some random variations
    for (int i = 0; i < 100; i++) {
        int idx = rand() % N;
        a[idx] = rand() % 1000 - 500;
        b[idx] = rand() % 1000 - 500;
    }
    
    // Run tests again with modified data
    total += test_gt_int(a, b, N);
    total += test_mixed_comparisons(a, b, c, N);
    
    printf("Total result: %d\n", total);
    printf("(This value varies based on input data)\n");
    
    return total != 0 ? 0 : 1;  // Ensure non-zero exit if all optimizations removed results
}
