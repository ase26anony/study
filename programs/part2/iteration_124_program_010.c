#include <stdio.h>
#include <math.h>

// Global results array to prevent optimization
volatile int results[20] = {0};
volatile int idx = 0;

// Initialize volatile doubles with NaN and normal values
volatile double v_normal1 = 1.0;
volatile double v_normal2 = 3.14159;
volatile double v_normal3 = 2.71828;
volatile double v_nan = 0.0 / 0.0;  // Generate NaN
volatile double v_inf = 1.0 / 0.0;  // Generate Infinity

// Test UNORDERED and ORDERED conditions
void test_unordered_ordered(void) {
    // UNORDERED: x != x when x is NaN
    if (v_nan != v_nan) {
        results[idx++] = 1;  // UNORDERED true
    } else {
        results[idx++] = 0;
    }
    
    // ORDERED: x == x when x is not NaN
    if (v_normal1 == v_normal1) {
        results[idx++] = 1;  // ORDERED true
    } else {
        results[idx++] = 0;
    }
    
    // Mixed: ORDERED with normal values
    if (v_normal2 == v_normal3) {
        results[idx++] = 0;
    } else {
        results[idx++] = 1;  // ORDERED false
    }
}

// Test UNEQ (unordered or equal) and UNGE (not less than)
void test_uneq_unge(void) {
    // UNEQ: v_normal1 == v_normal2 (ordered equal comparison)
    if (v_normal1 == v_normal2) {
        results[idx++] = 1;  // UNEQ true
    } else {
        results[idx++] = 0;  // UNEQ false
    }
    
    // UNEQ with NaN operand
    if (v_nan == v_normal1) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  // UNEQ false (unordered)
    }
    
    // UNGE: v_normal1 >= v_nan (not less than, with NaN)
    if (v_normal1 >= v_nan) {
        results[idx++] = 1;  // UNGE true (unordered)
    } else {
        results[idx++] = 0;
    }
    
    // UNGE: v_normal2 >= v_normal1
    if (v_normal2 >= v_normal1) {
        results[idx++] = 1;  // UNGE true
    } else {
        results[idx++] = 0;
    }
}

// Test UNGT (not less than or equal), UNLE (unordered or less than or equal), UNLT (unordered or less than)
void test_ungt_unle_unlt(void) {
    // UNGT: v_normal1 > v_nan (not less than or equal, with NaN)
    if (v_normal1 > v_nan) {
        results[idx++] = 1;  // UNGT true (unordered)
    } else {
        results[idx++] = 0;
    }
    
    // UNGT: v_normal2 > v_normal1
    if (v_normal2 > v_normal1) {
        results[idx++] = 1;  // UNGT true
    } else {
        results[idx++] = 0;
    }
    
    // UNLE: v_nan <= v_normal1 (unordered or less than or equal)
    if (v_nan <= v_normal1) {
        results[idx++] = 1;  // UNLE true (unordered)
    } else {
        results[idx++] = 0;
    }
    
    // UNLE: v_normal1 <= v_normal2
    if (v_normal1 <= v_normal2) {
        results[idx++] = 1;  // UNLE true
    } else {
        results[idx++] = 0;
    }
    
    // UNLT: v_nan < v_normal1 (unordered or less than)
    if (v_nan < v_normal1) {
        results[idx++] = 1;  // UNLT true (unordered)
    } else {
        results[idx++] = 0;
    }
    
    // UNLT: v_normal1 < v_normal2
    if (v_normal1 < v_normal2) {
        results[idx++] = 1;  // UNLT true
    } else {
        results[idx++] = 0;
    }
}

// Test LTGT (less than or greater than - ordered not equal)
void test_ltgt(void) {
    // LTGT: v_normal1 != v_normal2 (ordered not equal)
    if (v_normal1 != v_normal2) {
        results[idx++] = 1;  // LTGT true
    } else {
        results[idx++] = 0;
    }
    
    // LTGT with builtin (explicit ordered comparison)
    if (__builtin_islessgreater(v_normal2, v_normal3)) {
        results[idx++] = 1;  // LTGT true
    } else {
        results[idx++] = 0;
    }
    
    // LTGT with NaN (should be false for ordered comparison)
    if (v_normal1 != v_nan) {
        results[idx++] = 1;  // This is UNEQ, not LTGT
    } else {
        results[idx++] = 0;
    }
}

// Additional tests using ternary operators
void test_ternary_operators(void) {
    // Use ternary to force condition code generation
    double temp;
    
    // UNORDERED in ternary
    temp = (v_nan != v_nan) ? 1.0 : 0.0;
    results[idx++] = (int)temp;
    
    // ORDERED in ternary
    temp = (v_normal1 == v_normal1) ? 1.0 : 0.0;
    results[idx++] = (int)temp;
    
    // UNGE in ternary
    temp = (v_normal1 >= v_nan) ? 1.0 : 0.0;
    results[idx++] = (int)temp;
    
    // LTGT in ternary
    temp = (v_normal1 != v_normal2) ? 1.0 : 0.0;
    results[idx++] = (int)temp;
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
    
    // Compute checksum to ensure all comparisons executed
    int checksum = 0;
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
        // Print each result to prevent optimization
        printf("result[%d] = %d\n", i, results[i]);
    }
    
    printf("Total checksum: %d\n", checksum);
    printf("Number of comparisons: %d\n", idx);
    
    return checksum;
}
