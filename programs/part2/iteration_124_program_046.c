#include <stdio.h>
#include <math.h>

// Global results array to prevent optimization
volatile int results[20] = {0};
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
    
    // ORDERED: v1 == v1 (normal number)
    if (v1 == v1) {
        results[idx++] = 1;  // true
    } else {
        results[idx++] = 0;  // false
    }
    
    // UNORDERED with explicit NaN
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
    
    // UNEQ: v1 == v3 (equal comparison, unordered possible)
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
    
    // UNGE: v2 >= v1 (reverse with NaN first)
    if (v2 >= v1) {
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
    
    // UNLT: v2 < v1 (less with NaN operand)
    if (v2 < v1) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // Additional UNLE/UNLT with different operand orders
    if (v4 <= v2) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    if (v2 < v4) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

void test_ltgt(void) {
    volatile double v1 = 1.0;
    volatile double v2 = 3.0;
    volatile double v3 = 1.0;
    volatile double v4 = 0.0/0.0; // NaN
    
    // LTGT: v1 != v2 (not equal, both ordered)
    if (v1 != v2) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // LTGT: v1 != v3 (equal values, should be false)
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
    volatile double a = 5.0;
    volatile double b = 2.0;
    volatile double nan1 = 0.0/0.0;
    volatile double nan2 = NAN;
    
    // Mix of conditions in complex expressions
    int r1 = (a > b) ? 1 : 0;           // Ordered comparison
    int r2 = (nan1 == nan1) ? 1 : 0;    // ORDERED check
    int r3 = (a != nan1) ? 1 : 0;       // LTGT with NaN
    int r4 = (nan1 >= a) ? 1 : 0;       // UNGE
    int r5 = (b <= nan2) ? 1 : 0;       // UNLE
    
    results[idx++] = r1;
    results[idx++] = r2;
    results[idx++] = r3;
    results[idx++] = r4;
    results[idx++] = r5;
    
    // Use in switch-like logic
    volatile double x = 2.5;
    volatile double y = 2.5;
    volatile double z = NAN;
    
    // Multiple comparisons that should generate different condition codes
    if (x == y) {           // UNEQ
        results[idx++] = 10;
    }
    if (x != z) {           // LTGT (with NaN)
        results[idx++] = 11;
    }
    if (z < x) {            // UNLT
        results[idx++] = 12;
    }
    if (x >= z) {           // UNGE
        results[idx++] = 13;
    }
}

int main(void) {
    // Reset index
    idx = 0;
    
    // Run all test functions
    test_unordered();
    test_uneq_unge();
    test_ungt_unle_unlt();
    test_ltgt();
    test_mixed_conditions();
    
    // Compute checksum to ensure execution and prevent optimization
    int checksum = 0;
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
        checksum ^= (results[i] << (i % 16));
    }
    
    // Print checksum to make execution observable
    printf("Checksum: %d\n", checksum);
    printf("Number of tests executed: %d\n", idx);
    
    return 0;
}
