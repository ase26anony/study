#include <stdio.h>
#include <math.h>
#include <stdint.h>

// Global results array to prevent optimization
volatile int results[32];
volatile int idx = 0;

// Test functions targeting specific condition codes
void test_unordered_ordered(void) {
    volatile double v1 = 1.0;
    volatile double v2 = 0.0/0.0; // NaN
    
    // UNORDERED: v2 != v2 (NaN comparison)
    if (v2 != v2) {
        results[idx++] = 1; // UNORDERED true
    } else {
        results[idx++] = 0;
    }
    
    // ORDERED: v1 == v1 (non-NaN comparison)
    if (v1 == v1) {
        results[idx++] = 1; // ORDERED true
    } else {
        results[idx++] = 0;
    }
    
    // Mixed: ORDERED with potential NaN
    if (v1 == v2) {
        results[idx++] = 0;
    } else {
        results[idx++] = 1; // ORDERED false (unordered)
    }
}

void test_uneq_unge(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double nan = 0.0/0.0;
    
    // UNEQ: a == b (ordered equal)
    if (a == b) {
        results[idx++] = 0;
    } else {
        results[idx++] = 1; // UNEQ false
    }
    
    // UNEQ with NaN operand
    if (a == nan) {
        results[idx++] = 0;
    } else {
        results[idx++] = 1; // UNEQ false (unordered)
    }
    
    // UNGE: a >= nan
    if (a >= nan) {
        results[idx++] = 0;
    } else {
        results[idx++] = 1; // UNGE false (unordered)
    }
    
    // UNGE: nan >= a
    if (nan >= a) {
        results[idx++] = 0;
    } else {
        results[idx++] = 1; // UNGE false (unordered)
    }
}

void test_ungt_unle_unlt(void) {
    volatile double x = 3.0;
    volatile double y = 5.0;
    volatile double nan1 = 0.0/0.0;
    volatile double nan2 = nan1 * 2.0; // Another NaN
    
    // UNGT: x > nan1
    if (x > nan1) {
        results[idx++] = 0;
    } else {
        results[idx++] = 1; // UNGT false (unordered)
    }
    
    // UNGT: nan1 > x
    if (nan1 > x) {
        results[idx++] = 0;
    } else {
        results[idx++] = 1; // UNGT false (unordered)
    }
    
    // UNLE: nan1 <= x
    if (nan1 <= x) {
        results[idx++] = 0;
    } else {
        results[idx++] = 1; // UNLE false (unordered)
    }
    
    // UNLE: x <= nan1
    if (x <= nan1) {
        results[idx++] = 0;
    } else {
        results[idx++] = 1; // UNLE false (unordered)
    }
    
    // UNLT: nan1 < x
    if (nan1 < x) {
        results[idx++] = 0;
    } else {
        results[idx++] = 1; // UNLT false (unordered)
    }
    
    // UNLT: x < nan1
    if (x < nan1) {
        results[idx++] = 0;
    } else {
        results[idx++] = 1; // UNLT false (unordered)
    }
    
    // UNLE with two NaNs
    if (nan1 <= nan2) {
        results[idx++] = 0;
    } else {
        results[idx++] = 1; // UNLE false (unordered)
    }
}

void test_ltgt(void) {
    volatile double p = 7.0;
    volatile double q = 9.0;
    volatile double nan = 0.0/0.0;
    
    // LTGT: p != q (ordered not equal)
    if (p != q) {
        results[idx++] = 1; // LTGT true
    } else {
        results[idx++] = 0;
    }
    
    // LTGT with NaN operand (should be false)
    if (p != nan) {
        results[idx++] = 1; // This is UNEQ, not LTGT
    } else {
        results[idx++] = 0;
    }
    
    // Using __builtin_islessgreater for explicit LTGT
    if (__builtin_islessgreater(p, q)) {
        results[idx++] = 1; // LTGT true
    } else {
        results[idx++] = 0;
    }
    
    // LTGT with NaN (should be false)
    if (__builtin_islessgreater(p, nan)) {
        results[idx++] = 0;
    } else {
        results[idx++] = 1; // LTGT false
    }
}

void test_mixed_conditions(void) {
    volatile double a = 1.5;
    volatile double b = 2.5;
    volatile double c = 1.5; // Equal to a
    volatile double nan = 0.0/0.0;
    
    // Generate various condition codes through ternary operators
    results[idx++] = (a < b) ? 1 : 0;   // UNLT or LT
    results[idx++] = (a <= c) ? 1 : 0;  // UNLE or LE
    results[idx++] = (a > b) ? 1 : 0;   // UNGT or GT
    results[idx++] = (a >= c) ? 1 : 0;  // UNGE or GE
    results[idx++] = (a == c) ? 1 : 0;  // UNEQ or EQ
    results[idx++] = (a != b) ? 1 : 0;  // LTGT or NE
    
    // With NaN
    results[idx++] = (a < nan) ? 1 : 0;   // UNLT
    results[idx++] = (nan <= a) ? 1 : 0;  // UNLE
    results[idx++] = (a > nan) ? 1 : 0;   // UNGT
    results[idx++] = (nan >= a) ? 1 : 0;  // UNGE
    results[idx++] = (a == nan) ? 1 : 0;  // UNEQ
    results[idx++] = (a != nan) ? 1 : 0;  // UNEQ (not LTGT!)
}

int main(void) {
    // Initialize results array
    for (int i = 0; i < 32; i++) {
        results[i] = -1;
    }
    idx = 0;
    
    // Run all tests to generate various condition codes
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
    
    // Print checksum to make execution observable
    printf("Result checksum: %d\n", checksum);
    printf("Tests executed: %d\n", idx);
    
    return checksum != 0 ? 0 : 1;
}
