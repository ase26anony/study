#include <stdio.h>
#include <math.h>
#include <stdint.h>

// Global results array to prevent optimization
volatile int results[20] = {0};
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
    volatile double v2 = 0.0/0.0;  // NaN
    volatile double v3 = 3.0;
    volatile double v4 = -2.0;
    volatile double v5 = 0.0;
    
    // Force compiler to keep these variables
    (void)v1; (void)v2; (void)v3; (void)v4; (void)v5;
    
    // Run all test functions
    test_unordered();
    test_ordered();
    test_uneq();
    test_unge();
    test_ungt();
    test_unle();
    test_unlt();
    test_ltgt();
    
    // Compute checksum to ensure all code executed
    int checksum = 0;
    for (int i = 0; i < 20; i++) {
        checksum += results[i];
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum;
}

// Test UNORDERED condition code
void test_unordered(void) {
    volatile double nan_val = 0.0/0.0;
    volatile double normal = 1.0;
    
    // UNORDERED: x != x when x is NaN
    if (nan_val != nan_val) {
        results[idx++] = 1;  // true branch
    } else {
        results[idx++] = 0;  // false branch
    }
    
    // Another unordered test with different NaN source
    volatile double another_nan = nan_val * 2.0;
    if (another_nan != another_nan) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test ORDERED condition code  
void test_ordered(void) {
    volatile double normal1 = 2.5;
    volatile double normal2 = 3.7;
    
    // ORDERED: x == x when x is not NaN
    if (normal1 == normal1) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // Ordered comparison between two normal values
    if (normal1 < normal2) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test UNEQ condition code (unordered or equal)
void test_uneq(void) {
    volatile double a = 1.0;
    volatile double b = 1.0;
    volatile double nan_val = 0.0/0.0;
    
    // UNEQ: a == b (could be unordered)
    if (a == b) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // UNEQ with NaN operand
    if (a == nan_val) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test UNGE condition code (unordered or greater or equal)
void test_unge(void) {
    volatile double a = 2.0;
    volatile double b = 1.0;
    volatile double nan_val = 0.0/0.0;
    
    // UNGE: a >= b
    if (a >= b) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // UNGE with NaN operand
    if (a >= nan_val) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // UNGE with both NaN
    if (nan_val >= nan_val) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test UNGT condition code (unordered or greater than)
void test_ungt(void) {
    volatile double a = 3.0;
    volatile double b = 2.0;
    volatile double nan_val = 0.0/0.0;
    
    // UNGT: a > b
    if (a > b) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // UNGT with NaN operand
    if (a > nan_val) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test UNLE condition code (unordered or less or equal)
void test_unle(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double nan_val = 0.0/0.0;
    
    // UNLE: a <= b
    if (a <= b) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // UNLE with NaN operand
    if (nan_val <= b) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test UNLT condition code (unordered or less than)
void test_unlt(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double nan_val = 0.0/0.0;
    
    // UNLT: a < b
    if (a < b) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // UNLT with NaN operand
    if (nan_val < b) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test LTGT condition code (less than or greater than, but not equal and ordered)
void test_ltgt(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double c = 1.0;
    volatile double nan_val = 0.0/0.0;
    
    // LTGT: a != b (and ordered)
    if (a != b) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // LTGT with equal values (should be false)
    if (a != c) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // Using __builtin_islessgreater for explicit LTGT
    if (__builtin_islessgreater(a, b)) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // LTGT with NaN (should be false)
    if (__builtin_islessgreater(a, nan_val)) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}
