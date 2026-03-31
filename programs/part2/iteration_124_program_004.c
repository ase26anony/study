#include <stdio.h>
#include <math.h>

// Global results array to prevent optimization
volatile int results[20];
volatile int idx = 0;

// Test functions for different condition codes
void test_unordered(void) {
    volatile double v1 = 1.0;
    volatile double v2 = 0.0/0.0; // NaN
    volatile double v3 = 3.0;
    
    // UNORDERED: v2 != v2 (NaN comparison)
    if (v2 != v2) {
        results[idx++] = 1;  // true
    } else {
        results[idx++] = 0;  // false
    }
    
    // ORDERED: v1 == v1 (normal number comparison)
    if (v1 == v1) {
        results[idx++] = 1;  // true
    } else {
        results[idx++] = 0;  // false
    }
    
    // Additional UNORDERED test with explicit NaN
    volatile double nan_val = NAN;
    if (nan_val != nan_val) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

void test_uneq_unge(void) {
    volatile double v1 = 1.0;
    volatile double v2 = 0.0/0.0; // NaN
    volatile double v3 = 3.0;
    
    // UNEQ: v1 == v3 (equal comparison with possible unordered)
    if (v1 == v3) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // UNGE: v1 >= v2 (greater or equal with NaN operand)
    if (v1 >= v2) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // Another UNGE variant
    if (v3 >= v2) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

void test_ungt_unle_unlt(void) {
    volatile double v1 = 1.0;
    volatile double v2 = 0.0/0.0; // NaN
    volatile double v3 = 3.0;
    volatile double v4 = 2.0;
    
    // UNGT: v1 > v2 (greater than with NaN operand)
    if (v1 > v2) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // UNLE: v2 <= v3 (less or equal with NaN operand)
    if (v2 <= v3) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // UNLT: v2 < v1 (less than with NaN operand)
    if (v2 < v1) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // Additional UNGT test
    if (v4 > v2) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

void test_ltgt(void) {
    volatile double v1 = 1.0;
    volatile double v3 = 3.0;
    volatile double v4 = 2.0;
    
    // LTGT: v1 != v3 (not equal and ordered)
    if (v1 != v3) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // Another LTGT variant
    if (v1 != v4) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // Using __builtin_islessgreater for explicit LTGT
    if (__builtin_islessgreater(v1, v3)) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

void test_mixed_conditions(void) {
    volatile double v1 = 1.0;
    volatile double v2 = 0.0/0.0; // NaN
    volatile double v3 = 3.0;
    volatile double v4 = 1.0;
    
    // Mix of conditions in a single expression
    int r1 = (v1 == v4) ? 1 : 0;  // UNEQ
    int r2 = (v1 >= v2) ? 1 : 0;  // UNGE
    int r3 = (v2 < v3) ? 1 : 0;   // UNLT
    int r4 = (v1 != v3) ? 1 : 0;  // LTGT
    
    results[idx++] = r1;
    results[idx++] = r2;
    results[idx++] = r3;
    results[idx++] = r4;
    
    // Complex conditional with multiple comparisons
    if ((v1 > v2) && (v3 != v4)) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

int main(void) {
    // Initialize results array
    for (int i = 0; i < 20; i++) {
        results[i] = 0;
    }
    idx = 0;
    
    // Run all test functions
    test_unordered();
    test_uneq_unge();
    test_ungt_unle_unlt();
    test_ltgt();
    test_mixed_conditions();
    
    // Compute checksum to ensure execution
    int checksum = 0;
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
    }
    
    printf("Results checksum: %d\n", checksum);
    printf("Number of comparisons: %d\n", idx);
    
    return checksum;
}
