#include <stdio.h>
#include <math.h>

// Global array to store comparison results
volatile int results[20];
int result_idx = 0;

// Test function for UNORDERED and ORDERED conditions
void test_unordered_ordered(void) {
    volatile double v1 = 1.0;
    volatile double v2 = 0.0/0.0; // NaN
    volatile double v3 = 3.0;
    
    // UNORDERED: v2 != v2 (NaN comparison)
    if (v2 != v2) {
        results[result_idx++] = 1; // UNORDERED true
    } else {
        results[result_idx++] = 0; // UNORDERED false
    }
    
    // ORDERED: v1 == v1 (normal number comparison)
    if (v1 == v1) {
        results[result_idx++] = 1; // ORDERED true
    } else {
        results[result_idx++] = 0; // ORDERED false
    }
    
    // Mixed: ORDERED with potential NaN
    if (v1 == v2) { // This will be UNEQ in assembly
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;
    }
}

// Test function for UNEQ, UNGE conditions
void test_uneq_unge(void) {
    volatile double a = 2.5;
    volatile double b = 2.5;
    volatile double nan_val = 0.0/0.0;
    
    // UNEQ: a == b (with potential unordered)
    if (a == b) {
        results[result_idx++] = 1; // UNEQ true
    } else {
        results[result_idx++] = 0; // UNEQ false
    }
    
    // UNGE: a >= nan_val
    if (a >= nan_val) {
        results[result_idx++] = 1; // UNGE true
    } else {
        results[result_idx++] = 0; // UNGE false
    }
    
    // Another UNGE variant
    if (nan_val >= a) {
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;
    }
}

// Test function for UNGT, UNLE, UNLT conditions
void test_ungt_unle_unlt(void) {
    volatile double x = 3.14159;
    volatile double y = 2.71828;
    volatile double nan_val = 0.0/0.0;
    
    // UNGT: x > nan_val
    if (x > nan_val) {
        results[result_idx++] = 1; // UNGT true
    } else {
        results[result_idx++] = 0; // UNGT false
    }
    
    // UNLE: nan_val <= y
    if (nan_val <= y) {
        results[result_idx++] = 1; // UNLE true
    } else {
        results[result_idx++] = 0; // UNLE false
    }
    
    // UNLT: nan_val < x
    if (nan_val < x) {
        results[result_idx++] = 1; // UNLT true
    } else {
        results[result_idx++] = 0; // UNLT false
    }
    
    // Regular comparison that might generate LTGT
    if (x > y) {
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;
    }
}

// Test function specifically for LTGT condition
void test_ltgt(void) {
    volatile double p = 7.5;
    volatile double q = 7.5;
    volatile double r = 8.5;
    
    // LTGT: p != r (not equal and ordered)
    if (p != r) {
        results[result_idx++] = 1; // LTGT true
    } else {
        results[result_idx++] = 0; // LTGT false
    }
    
    // Another LTGT variant
    if (p != q) {
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;
    }
    
    // Use __builtin_islessgreater for explicit LTGT
    if (__builtin_islessgreater(p, r)) {
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;
    }
}

// Additional test with mixed conditions
void test_mixed_conditions(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double nan1 = 0.0/0.0;
    volatile double nan2 = nan("");
    
    // Various comparisons that should generate different condition codes
    if (a < b) results[result_idx++] = 1; else results[result_idx++] = 0;
    if (a <= b) results[result_idx++] = 1; else results[result_idx++] = 0;
    if (a > b) results[result_idx++] = 1; else results[result_idx++] = 0;
    if (a >= b) results[result_idx++] = 1; else results[result_idx++] = 0;
    if (a == b) results[result_idx++] = 1; else results[result_idx++] = 0;
    if (a != b) results[result_idx++] = 1; else results[result_idx++] = 0;
    
    // NaN comparisons
    if (nan1 == nan2) results[result_idx++] = 1; else results[result_idx++] = 0;
    if (nan1 != nan2) results[result_idx++] = 1; else results[result_idx++] = 0;
    if (nan1 < a) results[result_idx++] = 1; else results[result_idx++] = 0;
    if (a < nan1) results[result_idx++] = 1; else results[result_idx++] = 0;
}

int main(void) {
    // Initialize results array
    for (int i = 0; i < 20; i++) {
        results[i] = 0;
    }
    
    // Run all test functions
    test_unordered_ordered();
    test_uneq_unge();
    test_ungt_unle_unlt();
    test_ltgt();
    test_mixed_conditions();
    
    // Compute checksum to ensure all comparisons are executed
    int checksum = 0;
    for (int i = 0; i < result_idx; i++) {
        checksum += results[i];
        checksum *= 31; // Simple mixing
    }
    
    printf("Result checksum: %d\n", checksum);
    printf("Number of comparisons: %d\n", result_idx);
    
    return 0;
}
