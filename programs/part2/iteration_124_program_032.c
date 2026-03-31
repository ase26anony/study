#include <stdio.h>
#include <math.h>
#include <stdint.h>

// Global results array to prevent optimization
volatile int results[32];
volatile int idx = 0;

// Volatile doubles to prevent constant folding
volatile double v_normal1 = 1.0;
volatile double v_normal2 = 2.0;
volatile double v_normal3 = 3.0;
volatile double v_nan = 0.0/0.0;  // Generate NaN
volatile double v_inf = 1.0/0.0;  // Generate INF

// Test UNORDERED and ORDERED conditions
void test_unordered_ordered(void) {
    // UNORDERED: (x != x) when x is NaN
    if (v_nan != v_nan) {
        results[idx++] = 1;  // UNORDERED true
    } else {
        results[idx++] = 0;  // UNORDERED false
    }
    
    // ORDERED: (x == x) when x is not NaN
    if (v_normal1 == v_normal1) {
        results[idx++] = 1;  // ORDERED true
    } else {
        results[idx++] = 0;  // ORDERED false
    }
    
    // Mixed: ORDERED with potential NaN
    if (v_normal1 == v_nan) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test UNEQ (unordered or equal)
void test_uneq(void) {
    // UNEQ: (a == b) with potential unordered
    if (v_normal1 == v_normal2) {
        results[idx++] = 1;  // UNEQ true
    } else {
        results[idx++] = 0;  // UNEQ false
    }
    
    // UNEQ with NaN operand
    if (v_normal1 == v_nan) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test UNGE (not less than) - unordered or greater than or equal
void test_unge(void) {
    // UNGE: (a >= b) with potential unordered
    if (v_normal1 >= v_normal2) {
        results[idx++] = 1;  // UNGE true
    } else {
        results[idx++] = 0;  // UNGE false
    }
    
    // UNGE with NaN operand
    if (v_normal1 >= v_nan) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test UNGT (not less than or equal) - unordered or greater than
void test_ungt(void) {
    // UNGT: (a > b) with potential unordered
    if (v_normal1 > v_normal2) {
        results[idx++] = 1;  // UNGT true
    } else {
        results[idx++] = 0;  // UNGT false
    }
    
    // UNGT with NaN operand
    if (v_normal1 > v_nan) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test UNLE (unordered or less than or equal)
void test_unle(void) {
    // UNLE: (a <= b) with potential unordered
    if (v_normal1 <= v_normal2) {
        results[idx++] = 1;  // UNLE true
    } else {
        results[idx++] = 0;  // UNLE false
    }
    
    // UNLE with NaN operand
    if (v_normal1 <= v_nan) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test UNLT (unordered or less than)
void test_unlt(void) {
    // UNLT: (a < b) with potential unordered
    if (v_normal1 < v_normal2) {
        results[idx++] = 1;  // UNLT true
    } else {
        results[idx++] = 0;  // UNLT false
    }
    
    // UNLT with NaN operand
    if (v_normal1 < v_nan) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test LTGT (less than or greater than - ordered and not equal)
void test_ltgt(void) {
    // LTGT: (a != b) for ordered values
    if (v_normal1 != v_normal2) {
        results[idx++] = 1;  // LTGT true
    } else {
        results[idx++] = 0;  // LTGT false
    }
    
    // Using __builtin_islessgreater for explicit LTGT
    if (__builtin_islessgreater(v_normal1, v_normal2)) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // LTGT with NaN (should be false)
    if (v_normal1 != v_nan) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Complex test mixing multiple conditions
void test_mixed_conditions(void) {
    volatile double a = v_normal1;
    volatile double b = v_normal2;
    volatile double c = v_nan;
    volatile double d = v_inf;
    
    // Chain of comparisons to force different condition codes
    if ((a < b) && (c >= d)) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    if ((a > c) || (b <= d)) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // Ternary operator with FP comparison
    int res = (a == b) ? 1 : ((a != c) ? 2 : 3);
    results[idx++] = res;
    
    // Nested comparisons
    if ((a != a) || (b == b)) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

int main(void) {
    // Initialize results array
    for (int i = 0; i < 32; i++) {
        results[i] = -1;
    }
    idx = 0;
    
    // Run all tests to generate various condition codes
    test_unordered_ordered();    // UNORDERED, ORDERED
    test_uneq();                 // UNEQ
    test_unge();                 // UNGE
    test_ungt();                 // UNGT
    test_unle();                 // UNLE
    test_unlt();                 // UNLT
    test_ltgt();                 // LTGT
    test_mixed_conditions();     // Mixed conditions
    
    // Compute checksum to ensure all code executed
    int checksum = 0;
    for (int i = 0; i < idx; i++) {
        checksum = (checksum * 31 + results[i]) & 0xFFFF;
    }
    
    printf("Results checksum: %d\n", checksum);
    printf("Number of tests executed: %d\n", idx);
    
    // Additional printf to force code generation
    printf("v_nan != v_nan: %d\n", v_nan != v_nan);
    printf("v_normal1 == v_normal1: %d\n", v_normal1 == v_normal1);
    printf("v_normal1 >= v_nan: %d\n", v_normal1 >= v_nan);
    printf("v_normal1 != v_normal2: %d\n", v_normal1 != v_normal2);
    
    return checksum != 0 ? 0 : 1;
}
