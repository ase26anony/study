#include <stdio.h>
#include <math.h>
#include <stdint.h>

// Global results array to prevent optimization
volatile int results[20];
volatile int idx = 0;

// Helper to produce NaN
static double get_nan(void) {
    return 0.0 / 0.0;
}

// Test UNORDERED and ORDERED conditions
void test_unordered_ordered(void) {
    volatile double v1 = 1.0;
    volatile double v2 = get_nan();
    volatile double v3 = 3.0;
    
    // UNORDERED: v2 != v2 (NaN != NaN)
    if (v2 != v2) {
        results[idx++] = 1;  // UNORDERED true
    } else {
        results[idx++] = 0;
    }
    
    // ORDERED: v1 == v1 (normal == normal)
    if (v1 == v1) {
        results[idx++] = 1;  // ORDERED true
    } else {
        results[idx++] = 0;
    }
    
    // Mixed: ORDERED with potential NaN
    if (v1 == v3) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test UNEQ and UNGE conditions
void test_uneq_unge(void) {
    volatile double a = 2.5;
    volatile double b = get_nan();
    volatile double c = 2.5;
    volatile double d = 3.5;
    
    // UNEQ: a == c (ordered equal)
    if (a == c) {
        results[idx++] = 1;  // UNEQ true
    } else {
        results[idx++] = 0;
    }
    
    // UNGE: a >= b (with NaN)
    if (a >= b) {
        results[idx++] = 1;  // UNGE true
    } else {
        results[idx++] = 0;
    }
    
    // UNGE: d >= a (normal values)
    if (d >= a) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test UNGT, UNLE, UNLT conditions
void test_ungt_unle_unlt(void) {
    volatile double x = 1.0;
    volatile double y = get_nan();
    volatile double z = 3.0;
    
    // UNGT: x > y (with NaN)
    if (x > y) {
        results[idx++] = 1;  // UNGT true
    } else {
        results[idx++] = 0;
    }
    
    // UNLE: y <= z (with NaN)
    if (y <= z) {
        results[idx++] = 1;  // UNLE true
    } else {
        results[idx++] = 0;
    }
    
    // UNLT: y < x (with NaN)
    if (y < x) {
        results[idx++] = 1;  // UNLT true
    } else {
        results[idx++] = 0;
    }
    
    // Additional ordered comparisons
    if (x < z) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test LTGT (not equal and ordered) condition
void test_ltgt(void) {
    volatile double p = 1.0;
    volatile double q = 2.0;
    volatile double r = get_nan();
    
    // LTGT: p != q (ordered not equal)
    if (p != q) {
        results[idx++] = 1;  // LTGT true
    } else {
        results[idx++] = 0;
    }
    
    // Using __builtin_islessgreater for explicit LTGT
    if (__builtin_islessgreater(p, q)) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // LTGT with NaN (should be false)
    if (p != r) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Complex test mixing multiple conditions
void test_mixed_conditions(void) {
    volatile double a = 1.0;
    volatile double b = get_nan();
    volatile double c = 1.0;
    volatile double d = 2.0;
    
    // Chain of comparisons to force different condition codes
    int count = 0;
    
    // ORDERED check
    if (a == a) {
        count++;
    }
    
    // UNORDERED check
    if (b != b) {
        count++;
    }
    
    // UNEQ
    if (a == c) {
        count++;
    }
    
    // UNGE with NaN
    if (a >= b) {
        count++;
    }
    
    // UNGT with NaN
    if (d > b) {
        count++;
    }
    
    // UNLE with NaN
    if (b <= d) {
        count++;
    }
    
    // UNLT with NaN
    if (b < a) {
        count++;
    }
    
    // LTGT
    if (a != d) {
        count++;
    }
    
    results[idx++] = count;
}

int main(void) {
    // Initialize results array
    for (int i = 0; i < 20; i++) {
        results[i] = -1;
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
    }
    
    // Print checksum to prevent dead code elimination
    printf("Checksum: %d\n", checksum);
    printf("Number of tests: %d\n", idx);
    
    return 0;
}
