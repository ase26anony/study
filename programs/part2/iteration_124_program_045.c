#include <stdio.h>
#include <math.h>
#include <stdint.h>

// Global results array to prevent optimization
volatile int results[20] = {0};
volatile int idx = 0;

// Test functions for different condition codes
void test_unordered_ordered(void) {
    volatile double v1 = 1.0;
    volatile double v2 = 0.0/0.0; // NaN
    volatile double v3 = 3.0;
    
    // UNORDERED: v2 != v2 (NaN comparison)
    if (v2 != v2) {
        results[idx++] = 1; // Should be true for NaN
    } else {
        results[idx++] = 0;
    }
    
    // ORDERED: v1 == v1 (non-NaN comparison)
    if (v1 == v1) {
        results[idx++] = 1; // Should be true
    } else {
        results[idx++] = 0;
    }
    
    // Mixed: ORDERED with potential NaN
    if (v1 == v3) {
        results[idx++] = 0; // False path
    } else {
        results[idx++] = 1; // True path - UNEQ condition
    }
}

void test_uneq_unge(void) {
    volatile double a = 2.5;
    volatile double b = 0.0/0.0; // NaN
    volatile double c = 2.5;
    volatile double d = 3.0;
    
    // UNEQ: a == c (equal, but unordered possible)
    if (a == c) {
        results[idx++] = 1; // True
    } else {
        results[idx++] = 0;
    }
    
    // UNGE: a >= b (with NaN operand)
    if (a >= b) {
        results[idx++] = 0; // False when b is NaN
    } else {
        results[idx++] = 1; // True path
    }
    
    // UNGE: d >= a (normal ordered comparison)
    if (d >= a) {
        results[idx++] = 1; // True
    } else {
        results[idx++] = 0;
    }
}

void test_ungt_unle_unlt(void) {
    volatile double x = 1.0;
    volatile double y = 0.0/0.0; // NaN
    volatile double z = 2.0;
    volatile double w = 1.0;
    
    // UNGT: x > y (with NaN operand)
    if (x > y) {
        results[idx++] = 0; // False when y is NaN
    } else {
        results[idx++] = 1; // True path
    }
    
    // UNLE: y <= z (with NaN operand)
    if (y <= z) {
        results[idx++] = 0; // False when y is NaN
    } else {
        results[idx++] = 1; // True path
    }
    
    // UNLT: y < x (with NaN operand)
    if (y < x) {
        results[idx++] = 0; // False when y is NaN
    } else {
        results[idx++] = 1; // True path
    }
    
    // UNLT: x < z (normal ordered comparison)
    if (x < z) {
        results[idx++] = 1; // True
    } else {
        results[idx++] = 0;
    }
}

void test_ltgt(void) {
    volatile double p = 1.0;
    volatile double q = 2.0;
    volatile double r = 1.0;
    volatile double s = 0.0/0.0; // NaN
    
    // LTGT: p != q (not equal and ordered)
    if (p != q) {
        results[idx++] = 1; // True
    } else {
        results[idx++] = 0;
    }
    
    // LTGT: p != r (equal, so false)
    if (p != r) {
        results[idx++] = 0; // False
    } else {
        results[idx++] = 1; // True path
    }
    
    // Using __builtin_islessgreater for explicit LTGT
    if (__builtin_islessgreater(p, q)) {
        results[idx++] = 1; // True
    } else {
        results[idx++] = 0;
    }
    
    // LTGT with NaN operand (should be false)
    if (p != s) {
        results[idx++] = 1; // True (p != NaN)
    } else {
        results[idx++] = 0;
    }
}

void test_mixed_conditions(void) {
    volatile double v = 5.0;
    volatile double nan1 = 0.0/0.0;
    volatile double nan2 = nan1 * 2.0; // Another NaN
    
    // Multiple conditions in complex expression
    int temp = 0;
    
    // Complex condition mixing ordered and unordered
    if ((v == v) && (nan1 != nan1)) {
        temp |= 1; // Both true
    }
    
    if ((v > 3.0) || (nan1 < v)) {
        temp |= 2; // First true, second false (unordered)
    }
    
    if ((nan1 >= nan2) && (v <= 10.0)) {
        temp |= 4; // First false (unordered), second true
    }
    
    results[idx++] = temp;
    
    // Ternary operator with floating comparison
    double t1 = (v > nan1) ? 1.0 : 2.0; // UNGT in condition
    double t2 = (nan1 <= v) ? 3.0 : 4.0; // UNLE in condition
    
    results[idx++] = (int)(t1 + t2);
}

int main(void) {
    // Initialize results
    for (int i = 0; i < 20; i++) {
        results[i] = 0;
    }
    idx = 0;
    
    // Run all test functions
    test_unordered_ordered();
    test_uneq_unge();
    test_ungt_unle_unlt();
    test_ltgt();
    test_mixed_conditions();
    
    // Compute checksum to ensure execution
    int checksum = 0;
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
        checksum ^= (results[i] << (i % 16));
    }
    
    printf("Results checksum: %d\n", checksum);
    printf("Number of tests executed: %d\n", idx);
    
    // Print individual results for debugging
    printf("Individual results: ");
    for (int i = 0; i < idx && i < 20; i++) {
        printf("%d ", results[i]);
    }
    printf("\n");
    
    return checksum != 0 ? 0 : 1;
}
