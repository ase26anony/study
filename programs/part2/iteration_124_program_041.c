#include <stdio.h>
#include <math.h>

// Global results array to prevent optimization
volatile int results[20] = {0};
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
    
    // ORDERED: v1 == v1 (normal number)
    if (v1 == v1) {
        results[idx++] = 1; // ORDERED true
    } else {
        results[idx++] = 0;
    }
    
    // Mixed ordered/unordered comparisons
    if (v1 != v2) { // UNORDERED or NE
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

void test_uneq_unge(void) {
    volatile double v1 = 2.0;
    volatile double v2 = 2.0;
    volatile double v3 = 0.0/0.0; // NaN
    volatile double v4 = 4.0;
    
    // UNEQ: v1 == v2 (equal, but unordered possible)
    if (v1 == v2) {
        results[idx++] = 1; // UNEQ true
    } else {
        results[idx++] = 0;
    }
    
    // UNGE: v1 >= v3 (v3 is NaN)
    if (v1 >= v3) {
        results[idx++] = 1; // UNGE true
    } else {
        results[idx++] = 0;
    }
    
    // UNGE: v4 >= v1 (normal ordered case)
    if (v4 >= v1) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

void test_ungt_unle_unlt(void) {
    volatile double v1 = 5.0;
    volatile double v2 = 0.0/0.0; // NaN
    volatile double v3 = 3.0;
    volatile double v4 = 7.0;
    
    // UNGT: v1 > v2 (v2 is NaN)
    if (v1 > v2) {
        results[idx++] = 1; // UNGT true
    } else {
        results[idx++] = 0;
    }
    
    // UNLE: v2 <= v3 (v2 is NaN)
    if (v2 <= v3) {
        results[idx++] = 1; // UNLE true
    } else {
        results[idx++] = 0;
    }
    
    // UNLT: v2 < v1 (v2 is NaN)
    if (v2 < v1) {
        results[idx++] = 1; // UNLT true
    } else {
        results[idx++] = 0;
    }
    
    // Additional ordered comparisons
    if (v3 < v1) { // Ordered less than
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    if (v1 > v3) { // Ordered greater than
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

void test_ltgt(void) {
    volatile double v1 = 8.0;
    volatile double v2 = 9.0;
    volatile double v3 = 8.0;
    volatile double v4 = 0.0/0.0; // NaN
    
    // LTGT: v1 != v2 (not equal and ordered)
    if (v1 != v2) {
        results[idx++] = 1; // LTGT true
    } else {
        results[idx++] = 0;
    }
    
    // LTGT: v1 != v3 (equal, so false)
    if (v1 != v3) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // Using __builtin_islessgreater for explicit LTGT
    if (__builtin_islessgreater(v1, v2)) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // Comparison with NaN (should be false for LTGT)
    if (v1 != v4) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

void test_mixed_conditions(void) {
    volatile double v1 = 10.0;
    volatile double v2 = nan("");
    volatile double v3 = 11.0;
    
    // Complex expression mixing conditions
    int r1 = (v1 == v1) ? 1 : 0; // ORDERED
    int r2 = (v2 != v2) ? 1 : 0; // UNORDERED
    int r3 = (v1 >= v2) ? 1 : 0; // UNGE
    int r4 = (v1 <= v2) ? 1 : 0; // UNLE
    int r5 = (v1 > v2) ? 1 : 0;  // UNGT
    int r6 = (v1 < v2) ? 1 : 0;  // UNLT
    int r7 = (v1 == v3) ? 1 : 0; // UNEQ
    int r8 = (v1 != v3) ? 1 : 0; // LTGT
    
    results[idx++] = r1;
    results[idx++] = r2;
    results[idx++] = r3;
    results[idx++] = r4;
    results[idx++] = r5;
    results[idx++] = r6;
    results[idx++] = r7;
    results[idx++] = r8;
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
    
    printf("Checksum: %d\n", checksum);
    printf("Number of comparisons: %d\n", idx);
    
    // Print individual results for debugging
    printf("Results: ");
    for (int i = 0; i < idx; i++) {
        printf("%d ", results[i]);
    }
    printf("\n");
    
    return 0;
}
