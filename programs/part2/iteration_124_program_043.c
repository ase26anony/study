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
        results[idx++] = 1; // UNORDERED true
    } else {
        results[idx++] = 0;
    }
    
    // ORDERED: v1 == v1 (normal number comparison)
    if (v1 == v1) {
        results[idx++] = 1; // ORDERED true
    } else {
        results[idx++] = 0;
    }
    
    // Additional UNORDERED test with explicit NaN
    volatile double v4 = NAN;
    if (v4 != v4) {
        results[idx++] = 1; // UNORDERED true
    } else {
        results[idx++] = 0;
    }
}

void test_uneq_unge(void) {
    volatile double v1 = 1.0;
    volatile double v2 = 2.0;
    volatile double v3 = 0.0/0.0; // NaN
    
    // UNEQ: v1 == v2 (but with unordered possibility)
    if (v1 == v2) {
        results[idx++] = 1; // Would be UNEQ if unordered
    } else {
        results[idx++] = 0;
    }
    
    // UNGE: v1 >= v3 where v3 is NaN
    if (v1 >= v3) {
        results[idx++] = 1; // UNGE true when unordered
    } else {
        results[idx++] = 0;
    }
    
    // Another UNGE variant
    if (v3 >= v1) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

void test_ungt_unle_unlt(void) {
    volatile double v1 = 1.0;
    volatile double v2 = 2.0;
    volatile double v3 = 0.0/0.0; // NaN
    volatile double v4 = 3.0;
    
    // UNGT: v1 > v3 where v3 is NaN
    if (v1 > v3) {
        results[idx++] = 1; // UNGT true when unordered
    } else {
        results[idx++] = 0;
    }
    
    // UNLE: v3 <= v4 where v3 is NaN
    if (v3 <= v4) {
        results[idx++] = 1; // UNLE true when unordered
    } else {
        results[idx++] = 0;
    }
    
    // UNLT: v3 < v1 where v3 is NaN
    if (v3 < v1) {
        results[idx++] = 1; // UNLT true when unordered
    } else {
        results[idx++] = 0;
    }
    
    // Additional UNGT with both NaN
    volatile double v5 = 0.0/0.0;
    if (v3 > v5) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

void test_ltgt(void) {
    volatile double v1 = 1.0;
    volatile double v2 = 2.0;
    volatile double v3 = 3.0;
    
    // LTGT: v1 != v2 (ordered not equal)
    if (v1 != v2) {
        results[idx++] = 1; // LTGT true
    } else {
        results[idx++] = 0;
    }
    
    // Another LTGT with different values
    if (v2 != v3) {
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
    volatile double v2 = 2.0;
    volatile double nan1 = 0.0/0.0;
    volatile double nan2 = NAN;
    
    // Mix of conditions in ternary operators
    int r1 = (v1 < nan1) ? 1 : 0;  // UNLT
    int r2 = (nan1 <= v2) ? 1 : 0; // UNLE
    int r3 = (v1 > nan2) ? 1 : 0;  // UNGT
    int r4 = (nan2 >= v2) ? 1 : 0; // UNGE
    int r5 = (v1 == v2) ? 1 : 0;   // UNEQ
    int r6 = (nan1 == nan2) ? 1 : 0; // UNORDERED/UNEQ
    
    results[idx++] = r1;
    results[idx++] = r2;
    results[idx++] = r3;
    results[idx++] = r4;
    results[idx++] = r5;
    results[idx++] = r6;
    
    // Complex expression with multiple conditions
    if ((v1 < nan1) || (v2 > nan2) || (v1 != v2)) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

int main(void) {
    // Initialize with various values including NaN
    volatile double init_nan = 0.0/0.0;
    (void)init_nan; // Use it to prevent optimization
    
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
    }
    
    // Print checksum to prevent dead code elimination
    printf("Checksum: %d\n", checksum);
    printf("Total comparisons: %d\n", idx);
    
    return checksum != 0 ? 0 : 1;
}
