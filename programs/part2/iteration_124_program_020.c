#include <stdio.h>
#include <math.h>
#include <stdint.h>

// Global results array to prevent optimization
volatile int results[20];
volatile int idx = 0;

// Function prototypes
void test_unordered(void);
void test_ordered(void);
void test_uneq(void);
void test_unge(void);
void test_ungt(void);
void test_unle(void);
void test_unlt(void);
void test_ltgt(void);
void test_mixed_comparisons(void);

int main(void) {
    // Initialize results array
    for (int i = 0; i < 20; i++) {
        results[i] = 0;
    }
    
    // Run all tests
    test_unordered();
    test_ordered();
    test_uneq();
    test_unge();
    test_ungt();
    test_unle();
    test_unlt();
    test_ltgt();
    test_mixed_comparisons();
    
    // Compute checksum to ensure execution
    int checksum = 0;
    for (int i = 0; i < 20; i++) {
        checksum += results[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum;
}

// Test UNORDERED condition (x != x when x is NaN)
void test_unordered(void) {
    volatile double nan_val = 0.0 / 0.0;  // Generate NaN
    volatile double normal_val = 1.0;
    
    // UNORDERED: Compare NaN with itself
    if (nan_val != nan_val) {
        results[idx++] = 1;  // This should be true for NaN
    } else {
        results[idx++] = 0;
    }
    
    // Another UNORDERED test with different NaN source
    volatile double nan_val2 = nan_val * 2.0;
    if (nan_val2 != nan_val2) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test ORDERED condition (x == x when x is not NaN)
void test_ordered(void) {
    volatile double normal_val = 2.0;
    volatile double nan_val = 0.0 / 0.0;
    
    // ORDERED: Compare normal value with itself
    if (normal_val == normal_val) {
        results[idx++] = 1;  // This should be true
    } else {
        results[idx++] = 0;
    }
    
    // ORDERED with potential NaN (should be false)
    if (nan_val == nan_val) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test UNEQ condition (unordered or equal)
void test_uneq(void) {
    volatile double a = 3.0;
    volatile double b = 3.0;
    volatile double nan_val = 0.0 / 0.0;
    
    // UNEQ: Equal values
    if (a == b) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // UNEQ: NaN comparison (unordered)
    if (nan_val == a) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test UNGE condition (unordered or greater or equal)
void test_unge(void) {
    volatile double a = 5.0;
    volatile double b = 3.0;
    volatile double nan_val = 0.0 / 0.0;
    
    // UNGE: Greater than
    if (a >= b) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // UNGE: Equal values
    if (a >= a) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // UNGE: With NaN (unordered)
    if (nan_val >= a) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test UNGT condition (unordered or greater than)
void test_ungt(void) {
    volatile double a = 7.0;
    volatile double b = 4.0;
    volatile double nan_val = 0.0 / 0.0;
    
    // UNGT: Greater than
    if (a > b) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // UNGT: With NaN (unordered)
    if (nan_val > a) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test UNLE condition (unordered or less or equal)
void test_unle(void) {
    volatile double a = 2.0;
    volatile double b = 5.0;
    volatile double nan_val = 0.0 / 0.0;
    
    // UNLE: Less than
    if (a <= b) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // UNLE: Equal values
    if (a <= a) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // UNLE: With NaN (unordered)
    if (nan_val <= a) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test UNLT condition (unordered or less than)
void test_unlt(void) {
    volatile double a = 1.0;
    volatile double b = 3.0;
    volatile double nan_val = 0.0 / 0.0;
    
    // UNLT: Less than
    if (a < b) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // UNLT: With NaN (unordered)
    if (nan_val < a) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test LTGT condition (less than or greater than, but not equal and ordered)
void test_ltgt(void) {
    volatile double a = 4.0;
    volatile double b = 6.0;
    volatile double c = 4.0;
    volatile double nan_val = 0.0 / 0.0;
    
    // LTGT: Less than (ordered)
    if (a != b) {  // This generates LTGT for floating point
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // LTGT: Greater than (ordered)
    if (b != a) {  // This generates LTGT for floating point
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // Not LTGT: Equal values
    if (a != c) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // Not LTGT: NaN comparison (unordered)
    if (nan_val != a) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Mixed comparisons to ensure all condition codes are generated
void test_mixed_comparisons(void) {
    volatile double v1 = 1.0;
    volatile double v2 = 2.0;
    volatile double v3 = 1.0;
    volatile double nan1 = 0.0 / 0.0;
    volatile double nan2 = nan1 * 2.0;
    
    // Use ternary operators to generate condition codes
    results[idx++] = (v1 < v2) ? 1 : 0;    // UNLT
    results[idx++] = (v1 <= v3) ? 1 : 0;   // UNLE
    results[idx++] = (v2 > v1) ? 1 : 0;    // UNGT
    results[idx++] = (v2 >= v1) ? 1 : 0;   // UNGE
    results[idx++] = (v1 == v3) ? 1 : 0;   // UNEQ
    results[idx++] = (v1 != v2) ? 1 : 0;   // LTGT
    
    // NaN comparisons
    results[idx++] = (nan1 == nan1) ? 1 : 0;   // ORDERED/UNORDERED
    results[idx++] = (nan1 != nan1) ? 1 : 0;   // UNORDERED
    results[idx++] = (v1 == nan1) ? 1 : 0;     // UNORDERED
    results[idx++] = (nan1 < v1) ? 1 : 0;      // UNORDERED
}
