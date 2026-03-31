#include <stdio.h>
#include <math.h>

// Global results array to prevent optimization
volatile int results[20];
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
    
    // Mixed ordered/unordered
    if (v1 == v2) {  // UNEQ when unordered
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

void test_uneq_unge(void) {
    volatile double v1 = 2.0;
    volatile double v2 = 0.0/0.0;  // NaN
    volatile double v3 = 2.0;
    volatile double v4 = 3.0;
    
    // UNEQ: v1 == v3 (equal, but unordered possible in general)
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
    
    // UNGE: v4 >= v1 (normal ordered case)
    if (v4 >= v1) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

void test_ungt_unle_unlt(void) {
    volatile double v1 = 5.0;
    volatile double v2 = 0.0/0.0;  // NaN
    volatile double v3 = 3.0;
    volatile double v4 = 7.0;
    
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
    
    // Normal ordered comparisons that can still generate condition codes
    if (v4 > v1) {  // UNGT in ordered case
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    if (v3 <= v1) {  // UNLE in ordered case
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    if (v3 < v4) {  // UNLT in ordered case
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
    
    // LTGT: v1 != v3 (equal case - false)
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
    
    // Mixed with NaN
    if (v1 != v4) {  // This should also generate condition codes
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

void test_mixed_conditions(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double nan = 0.0/0.0;
    
    // Use ternary operators to generate condition codes
    results[idx++] = (a < b) ? 1 : 0;      // UNLT
    results[idx++] = (a <= b) ? 1 : 0;     // UNLE  
    results[idx++] = (a > b) ? 1 : 0;      // UNGT
    results[idx++] = (a >= b) ? 1 : 0;     // UNGE
    results[idx++] = (a == b) ? 1 : 0;     // UNEQ
    results[idx++] = (a != b) ? 1 : 0;     // LTGT
    
    // With NaN
    results[idx++] = (a < nan) ? 1 : 0;    // UNLT with NaN
    results[idx++] = (nan == nan) ? 1 : 0; // UNORDERED/ORDERED test
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
    
    // Compute checksum to ensure all code executed
    int checksum = 0;
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
        // Also print to ensure observability
        printf("results[%d] = %d\n", i, results[i]);
    }
    
    printf("Total checksum: %d\n", checksum);
    printf("Number of tests executed: %d\n", idx);
    
    return checksum != 0 ? 0 : 1;
}
