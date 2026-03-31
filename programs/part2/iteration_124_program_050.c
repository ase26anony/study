#include <stdio.h>
#include <math.h>
#include <stdint.h>

// Global results array to prevent optimization
volatile int results[20];
volatile int idx = 0;

// Initialize volatile doubles with normal and NaN values
volatile double v_normal1 = 1.0;
volatile double v_normal2 = 3.14159;
volatile double v_normal3 = 2.71828;
volatile double v_nan1 = 0.0/0.0;  // NaN
volatile double v_nan2 = NAN;      // Another NaN

// Test UNORDERED and ORDERED conditions
void test_unordered_ordered(void) {
    // UNORDERED: v_nan1 != v_nan1 (true when NaN)
    if (v_nan1 != v_nan1) {
        results[idx++] = 1;  // UNORDERED true
    } else {
        results[idx++] = 0;
    }
    
    // ORDERED: v_normal1 == v_normal1 (true for non-NaN)
    if (v_normal1 == v_normal1) {
        results[idx++] = 1;  // ORDERED true
    } else {
        results[idx++] = 0;
    }
    
    // Mixed: ORDERED check with NaN
    if (v_nan1 == v_nan1) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  // Should be false (unordered)
    }
}

// Test UNEQ (unordered or equal) and UNGE (unordered or greater or equal)
void test_uneq_unge(void) {
    // UNEQ: v_normal1 == v_normal2 (false, but unordered case considered)
    if (v_normal1 == v_normal2) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  // UNEQ false
    }
    
    // UNEQ with NaN: v_normal1 == v_nan1
    if (v_normal1 == v_nan1) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  // UNEQ false (unordered)
    }
    
    // UNGE: v_normal1 >= v_nan1 (unordered case)
    if (v_normal1 >= v_nan1) {
        results[idx++] = 1;  // UNGE true (because unordered)
    } else {
        results[idx++] = 0;
    }
    
    // UNGE: v_nan1 >= v_normal1
    if (v_nan1 >= v_normal1) {
        results[idx++] = 1;  // UNGE true (unordered)
    } else {
        results[idx++] = 0;
    }
}

// Test UNGT (unordered or greater than), UNLE (unordered or less or equal), UNLT (unordered or less than)
void test_ungt_unle_unlt(void) {
    // UNGT: v_normal1 > v_nan1
    if (v_normal1 > v_nan1) {
        results[idx++] = 1;  // UNGT true (unordered)
    } else {
        results[idx++] = 0;
    }
    
    // UNGT: v_nan1 > v_normal1
    if (v_nan1 > v_normal1) {
        results[idx++] = 1;  // UNGT true (unordered)
    } else {
        results[idx++] = 0;
    }
    
    // UNLE: v_nan1 <= v_normal1
    if (v_nan1 <= v_normal1) {
        results[idx++] = 1;  // UNLE true (unordered)
    } else {
        results[idx++] = 0;
    }
    
    // UNLE: v_normal1 <= v_nan1
    if (v_normal1 <= v_nan1) {
        results[idx++] = 1;  // UNLE true (unordered)
    } else {
        results[idx++] = 0;
    }
    
    // UNLT: v_nan1 < v_normal1
    if (v_nan1 < v_normal1) {
        results[idx++] = 1;  // UNLT true (unordered)
    } else {
        results[idx++] = 0;
    }
    
    // UNLT: v_normal1 < v_nan1
    if (v_normal1 < v_nan1) {
        results[idx++] = 1;  // UNLT true (unordered)
    } else {
        results[idx++] = 0;
    }
}

// Test LTGT (less than or greater than, but not equal and ordered)
void test_ltgt(void) {
    // LTGT: v_normal1 != v_normal2 (ordered not-equal)
    if (v_normal1 != v_normal2) {
        results[idx++] = 1;  // LTGT true
    } else {
        results[idx++] = 0;
    }
    
    // LTGT with builtin (explicitly ordered comparison)
    if (__builtin_islessgreater(v_normal1, v_normal3)) {
        results[idx++] = 1;  // LTGT true
    } else {
        results[idx++] = 0;
    }
    
    // LTGT with NaN (should be false)
    if (v_normal1 != v_nan1) {
        results[idx++] = 1;  // This will execute (unordered)
    } else {
        results[idx++] = 0;
    }
}

// Additional tests with ternary operators to force condition code generation
void test_ternary_operators(void) {
    // Use ternary operator with floating comparisons
    // This often generates different code patterns
    results[idx++] = (v_nan1 != v_nan1) ? 1 : 0;      // UNORDERED
    results[idx++] = (v_normal1 == v_normal1) ? 1 : 0; // ORDERED
    results[idx++] = (v_normal1 >= v_nan1) ? 1 : 0;    // UNGE
    results[idx++] = (v_nan1 <= v_normal1) ? 1 : 0;    // UNLE
    results[idx++] = (v_normal1 != v_normal2) ? 1 : 0; // LTGT
}

int main(void) {
    // Reset index
    idx = 0;
    
    // Run all test functions
    test_unordered_ordered();
    test_uneq_unge();
    test_ungt_unle_unlt();
    test_ltgt();
    test_ternary_operators();
    
    // Compute checksum to ensure all code executed
    int checksum = 0;
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
    }
    
    // Print checksum to prevent dead code elimination
    printf("Checksum: %d\n", checksum);
    printf("Number of tests: %d\n", idx);
    
    return 0;
}
