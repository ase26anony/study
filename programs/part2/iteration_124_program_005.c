#include <stdio.h>
#include <math.h>
#include <stdint.h>

// Global results array to prevent optimization
volatile int results[32];
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
    volatile double v4 = -2.5;
    volatile double v5 = __builtin_nan("");  // Another NaN
    
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
    
    // Compute checksum to ensure execution
    int checksum = 0;
    for (int i = 0; i < idx; i++) {
        checksum = (checksum * 31 + results[i]) & 0xFF;
    }
    
    printf("Checksum: %d\n", checksum);
    return checksum;
}

// Test UNORDERED condition code
void test_unordered(void) {
    volatile double a = 0.0/0.0;  // NaN
    volatile double b = 2.0;
    
    // UNORDERED: a != a (true for NaN)
    if (a != a) {
        results[idx++] = 1;  // UNORDERED true
    } else {
        results[idx++] = 0;
    }
    
    // Another UNORDERED test
    if (__builtin_isunordered(a, b)) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test ORDERED condition code  
void test_ordered(void) {
    volatile double a = 1.5;
    volatile double b = 3.7;
    
    // ORDERED: a == a (true for non-NaN)
    if (a == a) {
        results[idx++] = 1;  // ORDERED true
    } else {
        results[idx++] = 0;
    }
    
    // Another ORDERED test
    if (__builtin_isordered(a, b)) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test UNEQ condition code (unordered or equal)
void test_uneq(void) {
    volatile double a = 2.0;
    volatile double b = 2.0;
    volatile double c = 0.0/0.0;  // NaN
    
    // UNEQ: a == b (equal values)
    if (a == b) {
        results[idx++] = 1;  // UNEQ true
    } else {
        results[idx++] = 0;
    }
    
    // UNEQ with NaN (unordered case)
    if (c == c) {  // false for NaN
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test UNGE condition code (unordered or greater or equal)
void test_unge(void) {
    volatile double a = 3.0;
    volatile double b = 2.0;
    volatile double c = 0.0/0.0;  // NaN
    
    // UNGE: a >= b (greater or equal)
    if (a >= b) {
        results[idx++] = 1;  // UNGE true
    } else {
        results[idx++] = 0;
    }
    
    // UNGE with NaN (unordered case)
    if (c >= a) {  // unordered -> true
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test UNGT condition code (unordered or greater)
void test_ungt(void) {
    volatile double a = 3.0;
    volatile double b = 2.0;
    volatile double c = 0.0/0.0;  // NaN
    
    // UNGT: a > b (greater)
    if (a > b) {
        results[idx++] = 1;  // UNGT true
    } else {
        results[idx++] = 0;
    }
    
    // UNGT with NaN (unordered case)
    if (c > a) {  // unordered -> true
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test UNLE condition code (unordered or less or equal)
void test_unle(void) {
    volatile double a = 2.0;
    volatile double b = 3.0;
    volatile double c = 0.0/0.0;  // NaN
    
    // UNLE: a <= b (less or equal)
    if (a <= b) {
        results[idx++] = 1;  // UNLE true
    } else {
        results[idx++] = 0;
    }
    
    // UNLE with NaN (unordered case)
    if (c <= a) {  // unordered -> true
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test UNLT condition code (unordered or less)
void test_unlt(void) {
    volatile double a = 2.0;
    volatile double b = 3.0;
    volatile double c = 0.0/0.0;  // NaN
    
    // UNLT: a < b (less)
    if (a < b) {
        results[idx++] = 1;  // UNLT true
    } else {
        results[idx++] = 0;
    }
    
    // UNLT with NaN (unordered case)
    if (c < a) {  // unordered -> true
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test LTGT condition code (less or greater, but not equal and ordered)
void test_ltgt(void) {
    volatile double a = 2.0;
    volatile double b = 3.0;
    volatile double c = 2.0;
    volatile double d = 0.0/0.0;  // NaN
    
    // LTGT: a != b (not equal, both ordered)
    if (a != b) {
        results[idx++] = 1;  // LTGT true
    } else {
        results[idx++] = 0;
    }
    
    // LTGT: a != c (equal, so false)
    if (a != c) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // LTGT with NaN (unordered, so false)
    if (d != a) {  // unordered -> false for LTGT
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
}
