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
int test_lt(char *a, char threshold, int n) {
    int sum = 0;
    for (int i = 0; i < n; i++) {
        if (a[i] < threshold) {
            sum += a[i];
        }
    }
    return sum;
}

/* Less-than-or-equal (LE_EXPR) */
int test_le(unsigned short *a, unsigned short *b, int n) {
    int result = 0;
    for (int i = 0; i < n; i++) {
        result += (a[i] <= b[i]) ? 1 : 0;
    }
    return result;
}

/* Mixed comparisons in one loop to potentially trigger multiple cases */
int test_mixed_comparisons(int *arr1, int *arr2, int *arr3, int *arr4, int n) {
    int results[4] = {0};
    
    for (int i = 0; i < n; i++) {
        // GT_EXPR
        if (arr1[i] > arr2[i]) {
            results[0] += arr1[i];
        }
        
        // GE_EXPR
        if (arr1[i] >= arr3[i]) {
            results[1] += arr1[i];
        }
        
        // LT_EXPR
        if (arr1[i] < arr4[i]) {
            results[2] += arr1[i];
        }
        
        // LE_EXPR
        if (arr1[i] <= arr2[i]) {
            results[3] += arr1[i];
        }
    }
    
    return results[0] + results[1] + results[2] + results[3];
}

/* Conditional assignment using ternary operator */
void test_ternary_operators(int *src1, int *src2, int *dst, int n) {
    for (int i = 0; i < n; i++) {
        // Using all four comparison operators in ternary expressions
        dst[i] = (src1[i] > src2[i]) ? src1[i] : src2[i];
        dst[i] += (src1[i] >= src2[i]) ? 1 : 0;
        dst[i] += (src1[i] < src2[i]) ? -1 : 0;
        dst[i] += (src1[i] <= src2[i]) ? 2 : 0;
    }
}

/* Initialize arrays with varied data */
void init_arrays(int *arr1, int *arr2, short *sarr1, short *sarr2, 
                 char *carr, unsigned short *usarr1, unsigned short *usarr2, int size) {
    for (int i = 0; i < size; i++) {
        arr1[i] = i - size/2;  // Mix of positive and negative
        arr2[i] = i % 100;
        sarr1[i] = (short)(i * 2);
        sarr2[i] = (short)(i * 3 / 2);
        carr[i] = (char)(i % 128 - 64);
        usarr1[i] = (unsigned short)(i * 5);
        usarr2[i] = (unsigned short)(i * 3);
    }
}

int main() {
    /* Allocate and initialize arrays of different types and sizes */
    int arr1[N], arr2[N], arr3[N], arr4[N];
    short sarr1[M], sarr2[M];
    char carr[L];
    unsigned short usarr1[M], usarr2[M];
    int dst[N];
    
    srand(time(NULL));
    init_arrays(arr1, arr2, sarr1, sarr2, carr, usarr1, usarr2, N);
    
    /* Initialize additional arrays with different patterns */
    for (int i = 0; i < N; i++) {
        arr3[i] = rand() % 200 - 100;
        arr4[i] = rand() % 200 - 100;
    }
    
    for (int i = 0; i < L; i++) {
        carr[i] = (char)(rand() % 256 - 128);
    }
    
    /* Execute all test functions with different loop lengths and data types */
    int total = 0;
    
    // Test GT_EXPR with int arrays
    total += test_gt(arr1, arr2, N);
    
    // Test GE_EXPR with short arrays
    total += test_ge(sarr1, sarr2, M);
    
    // Test LT_EXPR with char array
    total += test_lt(carr, 0, L);
    
    // Test LE_EXPR with unsigned short arrays
    total += test_le(usarr1, usarr2, M);
    
    // Test mixed comparisons
    total += test_mixed_comparisons(arr1, arr2, arr3, arr4, N);
    
    // Test ternary operators
    test_ternary_operators(arr1, arr2, dst, N);
    for (int i = 0; i < N; i++) {
        total += dst[i];
    }
    
    /* Additional loops with different comparison patterns */
    
    // Loop with GT and LT comparisons on same data
    int gt_lt_sum = 0;
    for (int i = 0; i < M; i++) {
        if (sarr1[i] > sarr2[i]) {
            gt_lt_sum += sarr1[i];
        }
        if (sarr1[i] < sarr2[i]) {
            gt_lt_sum -= sarr2[i];
        }
    }
    total += gt_lt_sum;
    
    // Loop with GE and LE comparisons
    int ge_le_count = 0;
    for (int i = 0; i < N; i += 2) {
        ge_le_count += (arr1[i] >= arr2[i]) ? 1 : 0;
        ge_le_count += (arr1[i+1] <= arr2[i+1]) ? 1 : 0;
    }
    total += ge_le_count;
    
    // Nested conditional with comparisons
    int complex_result = 0;
    for (int i = 0; i < N; i++) {
        if (arr1[i] > 0) {
            if (arr2[i] < 100) {
                complex_result += arr1[i] * arr2[i];
            }
        } else if (arr1[i] <= -50) {
            complex_result -= arr2[i];
        }
    }
    total += complex_result;
    
    printf("Total result: %d\n", total);
    printf("(This value should be deterministic with fixed seed)\n");
    
    return total != 0 ? 0 : 1;  // Return 0 if computations were performed
}
