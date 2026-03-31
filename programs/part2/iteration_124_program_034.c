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
        results[idx++] = 1; // UNORDERED true
    } else {
        results[idx++] = 0;
    }
    
    // ORDERED: v1 == v1 (non-NaN comparison)
    if (v1 == v1) {
        results[idx++] = 1; // ORDERED true
    } else {
        results[idx++] = 0;
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
    
    // UNGE with both NaN
    volatile double nan1 = NAN;
    volatile double nan2 = NAN;
    if (nan1 >= nan2) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

void test_ungt_unle_unlt(void) {
    volatile double v1 = 1.0;
    volatile double v2 = 0.0/0.0; // NaN
    volatile double v3 = 3.0;
    
    // UNGT: v1 > v2 (greater with NaN operand)
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
}

void test_ltgt(void) {
    volatile double v1 = 1.0;
    volatile double v3 = 3.0;
    volatile double v4 = 1.0;
    
    // LTGT: v1 != v3 (not equal and ordered)
    if (v1 != v3) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // LTGT using __builtin_islessgreater
    if (__builtin_islessgreater(v1, v3)) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // LTGT with equal values (should be false)
    if (v1 != v4) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

void test_mixed_conditions(void) {
    volatile double a = 2.5;
    volatile double b = NAN;
    volatile double c = 2.5;
    volatile double d = 3.5;
    
    // Mix of conditions in complex expressions
    int r1 = (a == b) ? 1 : 0;  // UNEQ
    int r2 = (a >= b) ? 1 : 0;  // UNGE
    int r3 = (b > a) ? 1 : 0;   // UNGT
    int r4 = (b <= d) ? 1 : 0;  // UNLE
    int r5 = (b < a) ? 1 : 0;   // UNLT
    int r6 = (a != c) ? 1 : 0;  // LTGT (false)
    int r7 = (a != d) ? 1 : 0;  // LTGT (true)
    
    results[idx++] = r1;
    results[idx++] = r2;
    results[idx++] = r3;
    results[idx++] = r4;
    results[idx++] = r5;
    results[idx++] = r6;
    results[idx++] = r7;
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
    }
    
    printf("Results checksum: %d\n", checksum);
    printf("Number of tests executed: %d\n", idx);
    
    // Print individual results for debugging
    printf("Individual results: ");
    for (int i = 0; i < idx; i++) {
        printf("%d ", results[i]);
    }
    printf("\n");
    
    return 0;
}
