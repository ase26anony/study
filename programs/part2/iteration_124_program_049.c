#include <stdio.h>
#include <math.h>
#include <stdint.h>

// Global results array to prevent optimization
volatile int results[32];
volatile int idx = 0;

// Test functions for different condition codes
void test_unordered(void) {
    volatile double v1 = 1.0;
    volatile double v2 = 0.0/0.0;  // NaN
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
    volatile double v2 = 0.0/0.0;  // NaN
    volatile double v3 = 3.0;
    volatile double v4 = 1.0;
    
    // UNEQ: v1 == v4 (equal with possible unordered)
    if (v1 == v4) {
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
    
    // UNGE: v3 >= v1 (normal ordered case)
    if (v3 >= v1) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

void test_ungt_unle_unlt(void) {
    volatile double v1 = 2.0;
    volatile double v2 = 0.0/0.0;  // NaN
    volatile double v3 = 3.0;
    volatile double v4 = 2.0;
    
    // UNGT: v1 > v2 (greater with NaN operand)
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
    
    // Additional UNGT with normal values
    if (v3 > v1) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

void test_ltgt(void) {
    volatile double v1 = 2.0;
    volatile double v2 = 3.0;
    volatile double v3 = 2.0;
    volatile double v4 = 0.0/0.0;  // NaN
    
    // LTGT: v1 != v2 (not equal and ordered)
    if (v1 != v2) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // LTGT: v1 != v3 (equal case, should be false)
    if (v1 != v3) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // Using __builtin_islessgreater for explicit LTGT
    if (__builtin_islessgreater(v1, v2)) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // LTGT with NaN (should be false)
    if (v1 != v4) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

void test_mixed_conditions(void) {
    volatile double a = 1.5;
    volatile double b = 2.5;
    volatile double nan1 = 0.0/0.0;
    volatile double nan2 = NAN;
    
    // Mix of conditions in complex expressions
    int r1 = (a < b) ? 1 : 0;           // UNLT or LT
    int r2 = (nan1 >= a) ? 1 : 0;       // UNGE
    int r3 = (b != nan2) ? 1 : 0;       // LTGT or UNEQ
    int r4 = (nan1 == nan2) ? 1 : 0;    // UNORDERED/ORDERED
    
    results[idx++] = r1;
    results[idx++] = r2;
    results[idx++] = r3;
    results[idx++] = r4;
    
    // Ternary operator with floating comparisons
    results[idx++] = (a > nan1) ? 5 : 6;    // UNGT
    results[idx++] = (nan1 <= b) ? 7 : 8;   // UNLE
}

int main(void) {
    // Initialize results array
    for (int i = 0; i < 32; i++) {
        results[i] = 0;
    }
    idx = 0;
    
    // Run all test functions
    test_unordered();
    test_uneq_unge();
    test_ungt_unle_unlt();
    test_ltgt();
    test_mixed_conditions();
    
    // Compute checksum to prevent optimization
    int checksum = 0;
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
        checksum ^= (results[i] << (i % 16));
    }
    
    // Print checksum to ensure execution
    printf("Checksum: %d\n", checksum);
    printf("Tests executed: %d\n", idx);
    
    return checksum != 0 ? 0 : 1;
}
