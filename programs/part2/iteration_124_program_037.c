#include <stdio.h>
#include <math.h>

// Global array to store comparison results (prevents dead code elimination)
volatile int results[20] = {0};
int result_idx = 0;

// Test functions for different condition codes
void test_unordered_ordered(void) {
    volatile double v1 = 1.0;
    volatile double v2 = 0.0/0.0;  // NaN
    volatile double v3 = 3.0;
    
    // UNORDERED: v2 != v2 (NaN comparison)
    if (v2 != v2) {
        results[result_idx++] = 1;  // UNORDERED true
    } else {
        results[result_idx++] = 0;
    }
    
    // ORDERED: v1 == v1 (normal number comparison)
    if (v1 == v1) {
        results[result_idx++] = 1;  // ORDERED true
    } else {
        results[result_idx++] = 0;
    }
    
    // UNORDERED with explicit NaN
    volatile double nan_val = NAN;
    if (nan_val != nan_val) {
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;
    }
}

void test_uneq_unge(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double nan_val = 0.0/0.0;
    
    // UNEQ: a == b (but with NaN possibility)
    if (a == b) {
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;
    }
    
    // UNGE: a >= nan_val (unordered greater or equal)
    if (a >= nan_val) {
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;
    }
    
    // Another UNGE variant
    if (nan_val >= a) {
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;
    }
}

void test_ungt_unle_unlt(void) {
    volatile double x = 5.0;
    volatile double y = 10.0;
    volatile double nan1 = NAN;
    volatile double nan2 = -NAN;
    
    // UNGT: x > nan1 (unordered greater than)
    if (x > nan1) {
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;
    }
    
    // UNLE: nan1 <= y (unordered less or equal)
    if (nan1 <= y) {
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;
    }
    
    // UNLT: nan2 < x (unordered less than)
    if (nan2 < x) {
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;
    }
    
    // Additional UNLE/UNLT with different NaN sources
    volatile double inf = 1.0/0.0;
    if (nan1 <= inf) {
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;
    }
}

void test_ltgt(void) {
    volatile double p = 7.0;
    volatile double q = 7.0;
    volatile double r = 8.0;
    volatile double nan_val = NAN;
    
    // LTGT: p != r (not equal and ordered)
    if (p != r) {
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;
    }
    
    // LTGT: p != q (equal values, should be false)
    if (p != q) {
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;
    }
    
    // Using __builtin_islessgreater for explicit LTGT
    if (__builtin_islessgreater(p, r)) {
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;
    }
    
    // LTGT with NaN (should be false)
    if (p != nan_val) {
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;
    }
}

void test_mixed_conditions(void) {
    volatile double a = 1.5;
    volatile double b = 2.5;
    volatile double c = NAN;
    volatile double d = -NAN;
    
    // Mix of conditions in complex expression
    int res1 = (a < b) ? 1 : 0;           // Potential UNLT
    int res2 = (c >= d) ? 1 : 0;          // Potential UNGE
    int res3 = (a == c) ? 1 : 0;          // Potential UNEQ
    int res4 = (b > c) ? 1 : 0;           // Potential UNGT
    int res5 = (c <= a) ? 1 : 0;          // Potential UNLE
    
    results[result_idx++] = res1;
    results[result_idx++] = res2;
    results[result_idx++] = res3;
    results[result_idx++] = res4;
    results[result_idx++] = res5;
    
    // Complex conditional with multiple comparisons
    if ((a != b) && (c == c)) {
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;
    }
}

int main(void) {
    // Initialize all test functions
    test_unordered_ordered();
    test_uneq_unge();
    test_ungt_unle_unlt();
    test_ltgt();
    test_mixed_conditions();
    
    // Compute checksum to ensure all comparisons executed
    int checksum = 0;
    for (int i = 0; i < result_idx; i++) {
        checksum += results[i];
        checksum *= 31;  // Simple mixing
    }
    
    printf("Result checksum: %d\n", checksum);
    printf("Total comparisons: %d\n", result_idx);
    
    return checksum != 0 ? 0 : 1;
}
