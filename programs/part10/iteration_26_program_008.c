#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define N 256
#define M 128
#define L 512

/* Test functions for each comparison operator */

// Greater-than (>)
int test_gt(int *a, int *b, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] > b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

// Greater-than-or-equal (>=)
short test_ge(short *a, short *b, int n) {
    short sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] >= b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

// Less-than (<)
char test_lt(char *a, char *b, int n) {
    char sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] < b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

// Less-than-or-equal (<=)
unsigned int test_le(unsigned int *a, unsigned int *b, int n) {
    unsigned int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] <= b[i]) {
            sum += a[i];
        }
    }
    return sum;
}

// Mixed comparisons in one loop to potentially trigger multiple cases
int test_mixed_comparisons(int *a, int *b, int *c, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        // Using ternary operators with different comparisons
        int val1 = (a[i] > b[i]) ? a[i] : b[i];
        int val2 = (a[i] >= c[i]) ? a[i] : c[i];
        int val3 = (b[i] < c[i]) ? b[i] : c[i];
        int val4 = (b[i] <= a[i]) ? b[i] : a[i];
        
        result += val1 + val2 + val3 + val4;
    }
    return result;
}

// Test with different data types and loop lengths
void test_various_types() {
    // Arrays of different types and sizes
    int arr1_int[N], arr2_int[N];
    short arr1_short[M], arr2_short[M];
    char arr1_char[L], arr2_char[L];
    unsigned int arr1_uint[N], arr2_uint[N];
    
    // Initialize with varied data
    for (int i = 0; i < N; i++) {
        arr1_int[i] = i - N/2;  // Mix of positive and negative
        arr2_int[i] = i % 100;
        arr1_uint[i] = i * 2;
        arr2_uint[i] = i * 3;
    }
    
    for (int i = 0; i < M; i++) {
        arr1_short[i] = (i * 3) % 32767;
        arr2_short[i] = (i * 5) % 32767;
    }
    
    for (int i = 0; i < L; i++) {
        arr1_char[i] = (i % 128) - 64;  // Signed char range
        arr2_char[i] = (i % 96) - 32;
    }
    
    // Execute all test functions
    int res1 = test_gt(arr1_int, arr2_int, N);
    short res2 = test_ge(arr1_short, arr2_short, M);
    char res3 = test_lt(arr1_char, arr2_char, L);
    unsigned int res4 = test_le(arr1_uint, arr2_uint, N);
    
    // Additional test with mixed comparisons
    int arr3_int[N];
    for (int i = 0; i < N; i++) {
        arr3_int[i] = (i * 7) % 200;
    }
    int res5 = test_mixed_comparisons(arr1_int, arr2_int, arr3_int, N);
    
    // Use results to prevent dead code elimination
    printf("Results: %d, %d, %d, %u, %d\n", 
           res1, (int)res2, (int)res3, res4, res5);
}

// Additional test with conditional store (mask generation)
void test_mask_generation() {
    int src[N], dst[N], threshold[N];
    
    for (int i = 0; i < N; i++) {
        src[i] = i * 2;
        threshold[i] = i * 3;
    }
    
    // Loop with > comparison generating mask
    for (int i = 0; i < N; i++) {
        dst[i] = (src[i] > threshold[i]) ? src[i] : 0;
    }
    
    // Loop with <= comparison generating mask
    int dst2[N];
    for (int i = 0; i < N; i++) {
        dst2[i] = (src[i] <= threshold[i]) ? src[i] : -1;
    }
    
    // Use results
    int sum = 0;
    for (int i = 0; i < N; i++) {
        sum += dst[i] + dst2[i];
    }
    printf("Mask test sum: %d\n", sum);
}

// Test with nested comparisons
void test_nested_comparisons() {
    int a[N], b[N], c[N];
    
    for (int i = 0; i < N; i++) {
        a[i] = i;
        b[i] = i * 2;
        c[i] = i * 3;
    }
    
    int count = 0;
    for (int i = 0; i < N; i++) {
        // Multiple comparisons in one loop
        if (a[i] < b[i] && b[i] > c[i]) {
            count++;
        }
        if (a[i] >= b[i] || b[i] <= c[i]) {
            count--;
        }
    }
    
    printf("Nested comparison count: %d\n", count);
}

int main() {
    // Seed random number generator for varied data
    srand(time(NULL));
    
    printf("Starting vectorization comparison tests...\n");
    
    // Run all tests
    test_various_types();
    test_mask_generation();
    test_nested_comparisons();
    
    printf("Tests completed.\n");
    
    return 0;
}
