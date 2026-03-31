#include <stdio.h>
#include <math.h>
#include <stdint.h>

// Global results array to prevent optimization
volatile int results[20] = {0};
volatile int idx = 0;

// Function to generate NaN
double make_nan() {
    return 0.0 / 0.0;
}

// Test UNORDERED and ORDERED conditions
void test_unordered_ordered(void) {
    volatile double v1 = 1.0;
    volatile double v2 = make_nan();
    volatile double v3 = 3.0;
    
    // UNORDERED: v2 != v2 (NaN comparison)
    if (v2 != v2) {
        results[idx++] = 1;  // UNORDERED true
    } else {
        results[idx++] = 0;
    }
    
    // ORDERED: v1 == v1 (normal comparison)
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
    volatile double b = make_nan();
    volatile double c = 2.5;
    volatile double d = 3.0;
    
    // UNEQ: a == c (equal, but unordered possible)
    if (a == c) {
        results[idx++] = 1;  // UNEQ true
    } else {
        results[idx++] = 0;
    }
    
    // UNGE: a >= b (greater or equal with NaN)
    if (a >= b) {
        results[idx++] = 1;  // UNGE true
    } else {
        results[idx++] = 0;
    }
    
    // UNGE: d >= a (normal ordered case)
    if (d >= a) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test UNGT, UNLE, and UNLT conditions
void test_ungt_unle_unlt(void) {
    volatile double x = 4.0;
    volatile double y = make_nan();
    volatile double z = 5.0;
    volatile double w = 3.0;
    
    // UNGT: x > y (greater than with NaN)
    if (x > y) {
        results[idx++] = 1;  // UNGT true
    } else {
        results[idx++] = 0;
    }
    
    // UNLE: y <= z (less or equal with NaN)
    if (y <= z) {
        results[idx++] = 1;  // UNLE true
    } else {
        results[idx++] = 0;
    }
    
    // UNLT: y < x (less than with NaN)
    if (y < x) {
        results[idx++] = 1;  // UNLT true
    } else {
        results[idx++] = 0;
    }
    
    // Normal ordered comparisons for completeness
    if (w < x) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    if (z > x) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test LTGT (not equal and ordered) condition
void test_ltgt(void) {
    volatile double p = 7.0;
    volatile double q = 8.0;
    volatile double r = make_nan();
    
    // LTGT: p != q (not equal, both ordered)
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
    
    // Comparison with NaN (should not trigger LTGT)
    if (p != r) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Additional test with complex expressions
void test_mixed_conditions(void) {
    volatile double m = 10.0;
    volatile double n = make_nan();
    volatile double o = 10.0;
    
    // Complex expression that might generate multiple condition codes
    if ((m == o) && (m > 5.0)) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    if ((n != n) || (m < 20.0)) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // Ternary operator with FP comparison
    int res = (m >= n) ? 1 : 0;
    results[idx++] = res;
    
    // Nested comparisons
    if (m < 15.0) {
        if (n > 5.0) {
            results[idx++] = 1;
        } else {
            results[idx++] = 0;
        }
    } else {
        results[idx++] = 0;
    }
}

int main(void) {
    // Initialize with various values including NaN
    volatile double v_normal = 42.0;
    volatile double v_nan = make_nan();
    volatile double v_zero = 0.0;
    
    // Force initialization to be used
    if (v_normal > v_zero) {
        results[idx++] = 1;
    }
    
    // Run all test functions
    test_unordered_ordered();
    test_uneq_unge();
    test_ungt_unle_unlt();
    test_ltgt();
    test_mixed_conditions();
    
    // Compute checksum to ensure all code executed
    int checksum = 0;
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
        checksum ^= (results[i] << (i % 16));
    }
    
    // Print checksum to prevent dead code elimination
    printf("Result checksum: %d\n", checksum);
    printf("Tests executed: %d\n", idx);
    
    return checksum != 0 ? 0 : 1;
}
