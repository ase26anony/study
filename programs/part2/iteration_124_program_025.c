#include <stdio.h>
#include <math.h>
#include <stdint.h>

// Global results array to prevent optimization
volatile int results[20] = {0};
volatile int idx = 0;

// Test functions for different condition codes
void test_unordered_ordered(void) {
    volatile double v1 = 1.0;
    volatile double v2 = 0.0/0.0;  // NaN
    volatile double v3 = 3.0;
    
    // UNORDERED: v2 != v2 (NaN comparison)
    if (v2 != v2) {
        results[idx++] = 1;  // UNORDERED true
    } else {
        results[idx++] = 0;
    }
    
    // ORDERED: v1 == v1 (non-NaN comparison)
    if (v1 == v1) {
        results[idx++] = 1;  // ORDERED true
    } else {
        results[idx++] = 0;
    }
    
    // Mixed: v1 != v2 (unordered possible)
    if (v1 != v2) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

void test_uneq_unge(void) {
    volatile double v1 = 2.0;
    volatile double v2 = 0.0/0.0;  // NaN
    volatile double v3 = 2.0;
    
    // UNEQ: v1 == v3 (equal, but unordered possible)
    if (v1 == v3) {
        results[idx++] = 1;  // UNEQ true
    } else {
        results[idx++] = 0;
    }
    
    // UNGE: v1 >= v2 (greater or equal with NaN)
    if (v1 >= v2) {
        results[idx++] = 1;  // UNGE true
    } else {
        results[idx++] = 0;
    }
    
    // UNGE: v2 >= v1 (reverse with NaN)
    if (v2 >= v1) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

void test_ungt_unle_unlt(void) {
    volatile double v1 = 5.0;
    volatile double v2 = 0.0/0.0;  // NaN
    volatile double v3 = 3.0;
    
    // UNGT: v1 > v2 (greater with NaN)
    if (v1 > v2) {
        results[idx++] = 1;  // UNGT true
    } else {
        results[idx++] = 0;
    }
    
    // UNLE: v2 <= v3 (less or equal with NaN)
    if (v2 <= v3) {
        results[idx++] = 1;  // UNLE true
    } else {
        results[idx++] = 0;
    }
    
    // UNLT: v2 < v1 (less with NaN)
    if (v2 < v1) {
        results[idx++] = 1;  // UNLT true
    } else {
        results[idx++] = 0;
    }
}

void test_ltgt(void) {
    volatile double v1 = 4.0;
    volatile double v2 = 4.0;
    volatile double v3 = 5.0;
    volatile double v4 = 0.0/0.0;  // NaN
    
    // LTGT: v1 != v2 (not equal but ordered - false case)
    if (v1 != v2) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  // LTGT false
    }
    
    // LTGT: v1 != v3 (not equal and ordered - true case)
    if (v1 != v3) {
        results[idx++] = 1;  // LTGT true
    } else {
        results[idx++] = 0;
    }
    
    // Using __builtin_islessgreater for explicit LTGT
    if (__builtin_islessgreater(v1, v3)) {
        results[idx++] = 1;  // LTGT true
    } else {
        results[idx++] = 0;
    }
    
    // LTGT with NaN (should be false)
    if (__builtin_islessgreater(v1, v4)) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  // LTGT false with NaN
    }
}

void test_mixed_conditions(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double nan1 = 0.0/0.0;
    volatile double nan2 = nan1 * 2.0;  // Another NaN
    
    // Multiple conditions in complex expression
    if ((a < b) && (nan1 == nan1)) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // Ternary operator with floating comparison
    int res = (a > nan1) ? 1 : 0;
    results[idx++] = res;
    
    // Nested comparisons
    if ((a <= b) || (nan1 >= a)) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // Compare two NaNs
    if (nan1 == nan2) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    if (nan1 != nan2) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

int main(void) {
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
        checksum ^= (results[i] << (i % 16));
    }
    
    printf("Results checksum: %d\n", checksum);
    printf("Total comparisons: %d\n", idx);
    
    return checksum != 0 ? 0 : 1;
}
