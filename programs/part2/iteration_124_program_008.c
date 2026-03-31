#include <stdio.h>
#include <math.h>
#include <stdint.h>

// Global results array to prevent optimization
volatile int results[32];
volatile int idx = 0;

// Initialize volatile doubles with normal and NaN values
volatile double v1 = 1.0;
volatile double v2 = 0.0 / 0.0;  // NaN
volatile double v3 = 3.0;
volatile double v4 = 2.0;
volatile double v5 = 0.0 / 0.0;  // Another NaN

// Test UNORDERED and ORDERED conditions
void test_unordered_ordered(void) {
    // UNORDERED: x != x when x is NaN
    if (v2 != v2) {
        results[idx++] = 1;  // UNORDERED true
    } else {
        results[idx++] = 0;
    }
    
    // ORDERED: x == x when x is not NaN
    if (v1 == v1) {
        results[idx++] = 1;  // ORDERED true
    } else {
        results[idx++] = 0;
    }
    
    // Another UNORDERED test with explicit NaN comparison
    volatile double nan_val = NAN;
    if (nan_val != nan_val) {
        results[idx++] = 1;  // UNORDERED true
    } else {
        results[idx++] = 0;
    }
}

// Test UNEQ (unordered or equal) and UNGE (not less than)
void test_uneq_unge(void) {
    // UNEQ: v1 == v4 (2.0 == 2.0) - equal case
    if (v1 == v4) {
        results[idx++] = 1;  // UNEQ true (equal)
    } else {
        results[idx++] = 0;
    }
    
    // UNEQ with NaN operand (unordered case)
    if (v1 == v2) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  // UNEQ false when unordered
    }
    
    // UNGE: v1 >= v2 (1.0 >= NaN) - unordered case
    if (v1 >= v2) {
        results[idx++] = 1;  // UNGE true when unordered
    } else {
        results[idx++] = 0;
    }
    
    // UNGE: v3 >= v1 (3.0 >= 1.0) - ordered greater case
    if (v3 >= v1) {
        results[idx++] = 1;  // UNGE true
    } else {
        results[idx++] = 0;
    }
}

// Test UNGT (not less or equal), UNLE (unordered or less or equal), UNLT (unordered or less than)
void test_ungt_unle_unlt(void) {
    // UNGT: v1 > v2 (1.0 > NaN) - unordered case
    if (v1 > v2) {
        results[idx++] = 1;  // UNGT true when unordered
    } else {
        results[idx++] = 0;
    }
    
    // UNGT: v3 > v1 (3.0 > 1.0) - ordered greater case
    if (v3 > v1) {
        results[idx++] = 1;  // UNGT true
    } else {
        results[idx++] = 0;
    }
    
    // UNLE: v2 <= v3 (NaN <= 3.0) - unordered case
    if (v2 <= v3) {
        results[idx++] = 1;  // UNLE true when unordered
    } else {
        results[idx++] = 0;
    }
    
    // UNLE: v1 <= v4 (1.0 <= 2.0) - ordered less case
    if (v1 <= v4) {
        results[idx++] = 1;  // UNLE true
    } else {
        results[idx++] = 0;
    }
    
    // UNLT: v2 < v1 (NaN < 1.0) - unordered case
    if (v2 < v1) {
        results[idx++] = 1;  // UNLT true when unordered
    } else {
        results[idx++] = 0;
    }
    
    // UNLT: v1 < v3 (1.0 < 3.0) - ordered less case
    if (v1 < v3) {
        results[idx++] = 1;  // UNLT true
    } else {
        results[idx++] = 0;
    }
}

// Test LTGT (not equal and ordered)
void test_ltgt(void) {
    // LTGT: v1 != v3 (1.0 != 3.0) - ordered not equal
    if (v1 != v3) {
        results[idx++] = 1;  // LTGT true
    } else {
        results[idx++] = 0;
    }
    
    // LTGT with NaN (should be false when unordered)
    if (v1 != v2) {
        results[idx++] = 1;  // Not LTGT when unordered
    } else {
        results[idx++] = 0;
    }
    
    // Use __builtin_islessgreater for explicit LTGT
    if (__builtin_islessgreater(v1, v3)) {
        results[idx++] = 1;  // LTGT true
    } else {
        results[idx++] = 0;
    }
    
    // LTGT with equal values (should be false)
    if (__builtin_islessgreater(v1, v1)) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  // LTGT false when equal
    }
}

// Additional tests with mixed conditions
void test_mixed_conditions(void) {
    // Use ternary operator to generate condition codes
    volatile double a = v1;
    volatile double b = v2;
    volatile double c = v3;
    
    // Multiple conditions in one expression
    int r1 = (a < b) ? 1 : 0;      // UNLT
    int r2 = (b <= c) ? 1 : 0;     // UNLE
    int r3 = (a > b) ? 1 : 0;      // UNGT
    int r4 = (b >= a) ? 1 : 0;     // UNGE
    int r5 = (a == a) ? 1 : 0;     // ORDERED
    int r6 = (b != b) ? 1 : 0;     // UNORDERED
    int r7 = (a != c) ? 1 : 0;     // LTGT
    
    results[idx++] = r1;
    results[idx++] = r2;
    results[idx++] = r3;
    results[idx++] = r4;
    results[idx++] = r5;
    results[idx++] = r6;
    results[idx++] = r7;
    
    // Complex condition mixing ordered and unordered
    if ((a < b) || (b > c) || (a == c)) {
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
    
    // Compute checksum to ensure execution and prevent optimization
    int checksum = 0;
    for (int i = 0; i < idx; i++) {
        checksum = (checksum * 31 + results[i]) & 0xFF;
    }
    
    // Print checksum to ensure observable behavior
    printf("Checksum: %d\n", checksum);
    printf("Tests executed: %d\n", idx);
    
    return checksum != 0 ? 0 : 1;
}
