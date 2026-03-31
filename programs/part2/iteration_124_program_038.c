#include <stdio.h>
#include <math.h>

// Global results array to prevent optimization
volatile int results[20];
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
    
    // ORDERED: v1 == v1 (normal comparison)
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
    
    // UNGE with reversed operands
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
    
    // UNGT: v1 > v2 (greater than with NaN operand)
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
    
    // Additional UNGT/UNLE combinations
    if (v4 > v2) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    if (v2 <= v4) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

void test_ltgt(void) {
    volatile double v1 = 1.0;
    volatile double v2 = 2.0;
    volatile double v3 = 3.0;
    volatile double nan_val = NAN;
    
    // LTGT: v1 != v2 (not equal and ordered)
    if (v1 != v2) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // LTGT with different values
    if (v2 != v3) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // LTGT with builtin (explicit ordered not-equal)
    if (__builtin_islessgreater(v1, v3)) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // LTGT that should be false
    if (v1 != v1) {  // This is UNORDERED, not LTGT
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

void test_mixed_conditions(void) {
    volatile double a = 5.0;
    volatile double b = 10.0;
    volatile double nan1 = 0.0/0.0;
    volatile double nan2 = NAN;
    
    // Mix of conditions in complex expressions
    int r1 = (a < b) ? 1 : 0;           // UNLT or LT
    int r2 = (nan1 >= a) ? 1 : 0;       // UNGE
    int r3 = (b > nan2) ? 1 : 0;        // UNGT
    int r4 = (nan1 <= nan2) ? 1 : 0;    // UNLE
    int r5 = (a == b) ? 1 : 0;          // UNEQ
    int r6 = (a != b) ? 1 : 0;          // LTGT
    
    results[idx++] = r1;
    results[idx++] = r2;
    results[idx++] = r3;
    results[idx++] = r4;
    results[idx++] = r5;
    results[idx++] = r6;
    
    // Use in if-else chain
    if (a < nan1) {
        results[idx++] = 1;
    } else if (nan2 >= b) {
        results[idx++] = 2;
    } else if (a != b) {
        results[idx++] = 3;
    } else {
        results[idx++] = 0;
    }
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
    
    // Compute checksum to ensure execution
    int checksum = 0;
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
        checksum ^= (i * 31);  // Mix in index
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Tests executed: %d\n", idx);
    
    return 0;
}
