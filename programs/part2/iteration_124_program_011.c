#include <stdio.h>
#include <math.h>
#include <stdint.h>

// Global results array to prevent optimization
volatile int results[32];
volatile int idx = 0;

// Initialize volatile doubles with NaN and normal values
volatile double v1 = 1.0;
volatile double v2 = 0.0/0.0;  // NaN
volatile double v3 = 3.0;
volatile double v4 = 2.0;
volatile double v5 = -1.0;

// Test UNORDERED and ORDERED conditions
void test_unordered_ordered(void) {
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
    
    // Another UNORDERED with explicit NaN
    volatile double nan_val = NAN;
    if (nan_val != nan_val) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test UNEQ (unordered or equal) and UNGE (not less than)
void test_uneq_unge(void) {
    // UNEQ: v1 == v3 (false, but generates UNEQ condition code)
    if (v1 == v3) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  // Should be false
    }
    
    // UNGE: v1 >= v2 (v2 is NaN, so unordered)
    if (v1 >= v2) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // UNGE with normal values
    if (v3 >= v1) {
        results[idx++] = 1;  // True: 3.0 >= 1.0
    } else {
        results[idx++] = 0;
    }
}

// Test UNGT (not less than or equal), UNLE (unordered or less than or equal), UNLT (unordered or less than)
void test_ungt_unle_unlt(void) {
    // UNGT: v1 > v2 (v2 is NaN)
    if (v1 > v2) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // UNLE: v2 <= v3 (v2 is NaN)
    if (v2 <= v3) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // UNLT: v2 < v1 (v2 is NaN)
    if (v2 < v1) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // UNLE with normal values
    if (v1 <= v4) {
        results[idx++] = 1;  // True: 1.0 <= 2.0
    } else {
        results[idx++] = 0;
    }
    
    // UNLT with normal values
    if (v5 < v1) {
        results[idx++] = 1;  // True: -1.0 < 1.0
    } else {
        results[idx++] = 0;
    }
}

// Test LTGT (less than or greater than - ordered and not equal)
void test_ltgt(void) {
    // LTGT: v1 != v3 (ordered not equal)
    if (v1 != v3) {
        results[idx++] = 1;  // True: 1.0 != 3.0
    } else {
        results[idx++] = 0;
    }
    
    // LTGT using __builtin_islessgreater
    if (__builtin_islessgreater(v1, v4)) {
        results[idx++] = 1;  // True: 1.0 != 2.0
    } else {
        results[idx++] = 0;
    }
    
    // LTGT with NaN (should be false)
    if (v1 != v2) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test mixed conditions in complex expressions
void test_mixed_conditions(void) {
    // Complex expression that might generate various condition codes
    volatile double a = v1;
    volatile double b = v2;  // NaN
    volatile double c = v3;
    
    // Ternary with floating comparison
    int r1 = (a < b) ? 1 : 0;  // UNLT
    int r2 = (b > c) ? 1 : 0;  // UNGT
    int r3 = (a == a) ? 1 : 0; // ORDERED
    int r4 = (b != b) ? 1 : 0; // UNORDERED
    
    results[idx++] = r1;
    results[idx++] = r2;
    results[idx++] = r3;
    results[idx++] = r4;
    
    // Nested comparisons
    if ((a > b) && (c < a)) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

int main(void) {
    // Initialize results array
    for (int i = 0; i < 32; i++) {
        results[i] = -1;
    }
    idx = 0;
    
    // Run all test functions
    test_unordered_ordered();
    test_uneq_unge();
    test_ungt_unle_unlt();
    test_ltgt();
    test_mixed_conditions();
    
    // Compute checksum to prevent optimization
    int checksum = 0;
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
    }
    
    // Print results to ensure execution
    printf("Checksum: %d\n", checksum);
    printf("Number of tests: %d\n", idx);
    
    // Print individual results for debugging
    for (int i = 0; i < idx; i++) {
        printf("result[%d] = %d\n", i, results[i]);
    }
    
    return 0;
}
