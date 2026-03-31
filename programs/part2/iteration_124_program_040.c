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

int main(void) {
    // Initialize volatile doubles with mix of normal and NaN values
    volatile double v1 = 1.0;
    volatile double v2 = 0.0 / 0.0;  // NaN
    volatile double v3 = 3.0;
    volatile double v4 = -2.0;
    volatile double v5 = NAN;  // Another NaN
    
    // Force compiler to keep these variables
    (void)v1; (void)v2; (void)v3; (void)v4; (void)v5;
    
    // Run all tests
    test_unordered();
    test_ordered();
    test_uneq();
    test_unge();
    test_ungt();
    test_unle();
    test_unlt();
    test_ltgt();
    
    // Compute checksum to ensure all comparisons executed
    int checksum = 0;
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return 0;
}

// Test UNORDERED condition (x != x when x is NaN)
void test_unordered(void) {
    volatile double nan_val = 0.0 / 0.0;
    volatile double normal_val = 5.0;
    
    // UNORDERED: nan_val != nan_val should be true
    if (nan_val != nan_val) {
        results[idx++] = 1;  // UNORDERED true
    } else {
        results[idx++] = 0;
    }
    
    // Also test with normal value (should be false)
    if (normal_val != normal_val) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  // UNORDERED false
    }
}

// Test ORDERED condition (x == x when x is not NaN)
void test_ordered(void) {
    volatile double nan_val = NAN;
    volatile double normal_val = 7.0;
    
    // ORDERED: normal_val == normal_val should be true
    if (normal_val == normal_val) {
        results[idx++] = 1;  // ORDERED true
    } else {
        results[idx++] = 0;
    }
    
    // With NaN (should be false)
    if (nan_val == nan_val) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  // ORDERED false
    }
}

// Test UNEQ condition (== with possible NaN)
void test_uneq(void) {
    volatile double a = 1.0;
    volatile double b = 1.0;
    volatile double nan_val = NAN;
    
    // UNEQ: a == b (both ordered)
    if (a == b) {
        results[idx++] = 1;  // UNEQ true
    } else {
        results[idx++] = 0;
    }
    
    // With NaN operand
    if (a == nan_val) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  // UNEQ false with NaN
    }
}

// Test UNGE condition (>= with possible NaN)
void test_unge(void) {
    volatile double a = 5.0;
    volatile double b = 3.0;
    volatile double nan_val = 0.0 / 0.0;
    
    // UNGE: a >= b (ordered)
    if (a >= b) {
        results[idx++] = 1;  // UNGE true
    } else {
        results[idx++] = 0;
    }
    
    // With NaN operand
    if (a >= nan_val) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  // UNGE false with NaN
    }
    
    // Reverse with NaN first
    if (nan_val >= a) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  // Another UNGE false
    }
}

// Test UNGT condition (> with possible NaN)
void test_ungt(void) {
    volatile double a = 5.0;
    volatile double b = 3.0;
    volatile double nan_val = NAN;
    
    // UNGT: a > b (ordered)
    if (a > b) {
        results[idx++] = 1;  // UNGT true
    } else {
        results[idx++] = 0;
    }
    
    // With NaN operand
    if (a > nan_val) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  // UNGT false with NaN
    }
}

// Test UNLE condition (<= with possible NaN)
void test_unle(void) {
    volatile double a = 3.0;
    volatile double b = 5.0;
    volatile double nan_val = 0.0 / 0.0;
    
    // UNLE: a <= b (ordered)
    if (a <= b) {
        results[idx++] = 1;  // UNLE true
    } else {
        results[idx++] = 0;
    }
    
    // With NaN operand
    if (a <= nan_val) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  // UNLE false with NaN
    }
}

// Test UNLT condition (< with possible NaN)
void test_unlt(void) {
    volatile double a = 3.0;
    volatile double b = 5.0;
    volatile double nan_val = NAN;
    
    // UNLT: a < b (ordered)
    if (a < b) {
        results[idx++] = 1;  // UNLT true
    } else {
        results[idx++] = 0;
    }
    
    // With NaN operand
    if (a < nan_val) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  // UNLT false with NaN
    }
}

// Test LTGT condition (not equal and ordered)
void test_ltgt(void) {
    volatile double a = 5.0;
    volatile double b = 3.0;
    volatile double c = 5.0;
    volatile double nan_val = 0.0 / 0.0;
    
    // LTGT: a != b (both ordered)
    if (a != b) {
        results[idx++] = 1;  // LTGT true
    } else {
        results[idx++] = 0;
    }
    
    // Equal values (should be false)
    if (a != c) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  // LTGT false
    }
    
    // With NaN operand (should be false for LTGT)
    if (a != nan_val) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  // LTGT false with NaN
    }
    
    // Use __builtin_islessgreater for explicit LTGT
    #ifdef __GNUC__
    if (__builtin_islessgreater(a, b)) {
        results[idx++] = 1;  // Explicit LTGT
    } else {
        results[idx++] = 0;
    }
    #endif
}
