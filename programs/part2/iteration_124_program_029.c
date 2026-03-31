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
    idx = 0;
    
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

// Test UNORDERED condition code
void test_unordered(void) {
    volatile double nan_val = 0.0 / 0.0;  // Generate NaN
    volatile double normal_val = 3.14159;
    
    // UNORDERED: x != x when x is NaN
    if (nan_val != nan_val) {
        results[idx++] = 1;  // Should be true for NaN
    } else {
        results[idx++] = 0;
    }
    
    // Also test with normal value (should be false)
    if (normal_val != normal_val) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  // Should be false
    }
}

// Test ORDERED condition code  
void test_ordered(void) {
    volatile double nan_val = 0.0 / 0.0;
    volatile double normal_val = 2.71828;
    
    // ORDERED: x == x when x is not NaN
    if (normal_val == normal_val) {
        results[idx++] = 1;  // Should be true
    } else {
        results[idx++] = 0;
    }
    
    // Test with NaN (should be false)
    if (nan_val == nan_val) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  // Should be false
    }
}

// Test UNEQ condition code (unordered or equal)
void test_uneq(void) {
    volatile double a = 1.0;
    volatile double b = 1.0;
    volatile double nan_val = 0.0 / 0.0;
    
    // UNEQ: a == b (equal values)
    if (a == b) {
        results[idx++] = 1;  // Should be true
    } else {
        results[idx++] = 0;
    }
    
    // UNEQ with NaN (unordered case)
    if (a == nan_val) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  // Should be false (but unordered)
    }
}

// Test UNGE condition code (unordered or greater or equal)
void test_unge(void) {
    volatile double a = 5.0;
    volatile double b = 3.0;
    volatile double nan_val = 0.0 / 0.0;
    
    // UNGE: a >= b (greater case)
    if (a >= b) {
        results[idx++] = 1;  // Should be true
    } else {
        results[idx++] = 0;
    }
    
    // UNGE with NaN (unordered case)
    if (a >= nan_val) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  // Should be false (but unordered)
    }
}

// Test UNGT condition code (unordered or greater)
void test_ungt(void) {
    volatile double a = 5.0;
    volatile double b = 3.0;
    volatile double nan_val = 0.0 / 0.0;
    
    // UNGT: a > b (greater case)
    if (a > b) {
        results[idx++] = 1;  // Should be true
    } else {
        results[idx++] = 0;
    }
    
    // UNGT with NaN (unordered case)
    if (nan_val > a) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  // Should be false (but unordered)
    }
}

// Test UNLE condition code (unordered or less or equal)
void test_unle(void) {
    volatile double a = 3.0;
    volatile double b = 5.0;
    volatile double nan_val = 0.0 / 0.0;
    
    // UNLE: a <= b (less or equal case)
    if (a <= b) {
        results[idx++] = 1;  // Should be true
    } else {
        results[idx++] = 0;
    }
    
    // UNLE with NaN (unordered case)
    if (nan_val <= a) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  // Should be false (but unordered)
    }
}

// Test UNLT condition code (unordered or less)
void test_unlt(void) {
    volatile double a = 3.0;
    volatile double b = 5.0;
    volatile double nan_val = 0.0 / 0.0;
    
    // UNLT: a < b (less case)
    if (a < b) {
        results[idx++] = 1;  // Should be true
    } else {
        results[idx++] = 0;
    }
    
    // UNLT with NaN (unordered case)
    if (nan_val < a) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  // Should be false (but unordered)
    }
}

// Test LTGT condition code (less or greater, but not equal and ordered)
void test_ltgt(void) {
    volatile double a = 3.0;
    volatile double b = 5.0;
    volatile double c = 5.0;
    volatile double nan_val = 0.0 / 0.0;
    
    // LTGT: a != b (not equal, both ordered)
    if (a != b) {
        results[idx++] = 1;  // Should be true
    } else {
        results[idx++] = 0;
    }
    
    // LTGT: equal values (should be false)
    if (b != c) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  // Should be false
    }
    
    // LTGT with NaN (unordered, should be false)
    if (a != nan_val) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  // Should be false (unordered)
    }
    
    // Use __builtin_islessgreater for explicit LTGT
    if (__builtin_islessgreater(a, b)) {
        results[idx++] = 1;  // Should be true
    } else {
        results[idx++] = 0;
    }
}

// Test mixed comparisons to cover all condition codes
void test_mixed_comparisons(void) {
    volatile double v1 = 1.0;
    volatile double v2 = 0.0 / 0.0;  // NaN
    volatile double v3 = 3.0;
    volatile double v4 = 1.0;
    
    // Mix of comparisons that should generate different condition codes
    // Using ternary operator to force condition code generation
    results[idx++] = (v2 != v2) ? 1 : 0;      // UNORDERED
    results[idx++] = (v1 == v1) ? 1 : 0;      // ORDERED
    results[idx++] = (v1 == v4) ? 1 : 0;      // UNEQ (equal)
    results[idx++] = (v1 >= v2) ? 1 : 0;      // UNGE (with NaN)
    results[idx++] = (v1 > v2) ? 1 : 0;       // UNGT (with NaN)
    results[idx++] = (v2 <= v3) ? 1 : 0;      // UNLE (with NaN)
    results[idx++] = (v2 < v1) ? 1 : 0;       // UNLT (with NaN)
    results[idx++] = (v1 != v3) ? 1 : 0;      // LTGT (ordered, not equal)
    
    // Additional complex expression
    results[idx++] = (v1 < v3 && v3 > v1) ? 1 : 0;  // Multiple comparisons
}
