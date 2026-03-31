#include <stdio.h>
#include <math.h>
#include <stdint.h>

// Global results array to prevent optimization
volatile int results[32];
volatile int result_idx = 0;

// Function to produce NaN
double make_nan() {
    return 0.0 / 0.0;
}

// Test UNORDERED (x != x when x is NaN) and ORDERED (x == x when x is not NaN)
void test_unordered_ordered(void) {
    volatile double v1 = 1.0;
    volatile double v2 = make_nan();
    volatile double v3 = 3.0;
    
    // UNORDERED: v2 != v2 (true when v2 is NaN)
    if (v2 != v2) {
        results[result_idx++] = 1;  // UNORDERED true
    } else {
        results[result_idx++] = 0;
    }
    
    // ORDERED: v1 == v1 (true when v1 is not NaN)
    if (v1 == v1) {
        results[result_idx++] = 1;  // ORDERED true
    } else {
        results[result_idx++] = 0;
    }
    
    // Mixed: ORDERED with NaN (false)
    if (v2 == v2) {
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;  // ORDERED false
    }
}

// Test UNEQ (== with possible NaN) and UNGE (>= with possible NaN)
void test_uneq_unge(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double nan_val = make_nan();
    
    // UNEQ: a == b (false, but generates UNEQ condition code)
    if (a == b) {
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;  // UNEQ false
    }
    
    // UNEQ with NaN operand
    if (a == nan_val) {
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;  // UNEQ false (unordered)
    }
    
    // UNGE: a >= nan_val (false when unordered)
    if (a >= nan_val) {
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;  // UNGE false (unordered)
    }
    
    // UNGE: normal ordered case
    if (b >= a) {
        results[result_idx++] = 1;  // UNGE true
    } else {
        results[result_idx++] = 0;
    }
}

// Test UNGT (> with possible NaN), UNLE (<= with possible NaN), UNLT (< with possible NaN)
void test_ungt_unle_unlt(void) {
    volatile double x = 5.0;
    volatile double y = 3.0;
    volatile double nan1 = make_nan();
    volatile double nan2 = make_nan();
    
    // UNGT: x > y (true when ordered)
    if (x > y) {
        results[result_idx++] = 1;  // UNGT true
    } else {
        results[result_idx++] = 0;
    }
    
    // UNGT with NaN (false when unordered)
    if (x > nan1) {
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;  // UNGT false
    }
    
    // UNLE: y <= x (true when ordered)
    if (y <= x) {
        results[result_idx++] = 1;  // UNLE true
    } else {
        results[result_idx++] = 0;
    }
    
    // UNLE with NaN (false when unordered)
    if (nan1 <= x) {
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;  // UNLE false
    }
    
    // UNLT: y < x (true when ordered)
    if (y < x) {
        results[result_idx++] = 1;  // UNLT true
    } else {
        results[result_idx++] = 0;
    }
    
    // UNLT with NaN (false when unordered)
    if (nan1 < x) {
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;  // UNLT false
    }
}

// Test LTGT (not equal and ordered, or "less than or greater than")
void test_ltgt(void) {
    volatile double p = 7.0;
    volatile double q = 8.0;
    volatile double r = 7.0;
    volatile double nan_val = make_nan();
    
    // LTGT: p != q (true, and ordered)
    if (p != q) {
        results[result_idx++] = 1;  // LTGT true
    } else {
        results[result_idx++] = 0;
    }
    
    // LTGT: p != r (false, equal)
    if (p != r) {
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;  // LTGT false
    }
    
    // LTGT with NaN (false when unordered)
    if (p != nan_val) {
        results[result_idx++] = 1;  // This might still be true, but generates LTGT
    } else {
        results[result_idx++] = 0;
    }
    
    // Using __builtin_islessgreater for explicit LTGT
    if (__builtin_islessgreater(p, q)) {
        results[result_idx++] = 1;  // LTGT true
    } else {
        results[result_idx++] = 0;
    }
    
    if (__builtin_islessgreater(p, nan_val)) {
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;  // LTGT false (unordered)
    }
}

// Additional test with ternary operators to generate different code patterns
void test_ternary_mixed(void) {
    volatile double a = 2.0;
    volatile double b = 4.0;
    volatile double nan_val = make_nan();
    
    // Use ternary operator with various conditions
    int res1 = (a < b) ? 1 : 0;        // UNLT ordered
    int res2 = (a < nan_val) ? 1 : 0;  // UNLT unordered
    int res3 = (b >= a) ? 1 : 0;       // UNGE ordered
    int res4 = (b >= nan_val) ? 1 : 0; // UNGE unordered
    
    results[result_idx++] = res1;
    results[result_idx++] = res2;
    results[result_idx++] = res3;
    results[result_idx++] = res4;
}

int main(void) {
    // Initialize results array
    for (int i = 0; i < 32; i++) {
        results[i] = -1;
    }
    result_idx = 0;
    
    // Run all tests to generate various condition codes
    test_unordered_ordered();
    test_uneq_unge();
    test_ungt_unle_unlt();
    test_ltgt();
    test_ternary_mixed();
    
    // Compute checksum to ensure all code executes
    int checksum = 0;
    for (int i = 0; i < result_idx; i++) {
        checksum += results[i];
    }
    
    printf("Result checksum: %d\n", checksum);
    printf("Number of tests executed: %d\n", result_idx);
    
    // Print individual results for debugging
    printf("Results: ");
    for (int i = 0; i < result_idx; i++) {
        printf("%d ", results[i]);
    }
    printf("\n");
    
    return 0;
}
