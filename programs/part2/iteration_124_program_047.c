#include <stdio.h>
#include <math.h>

// Global results array to prevent optimization
volatile int results[20];
volatile int idx = 0;

// Function prototypes
void test_unordered(void);
void test_uneq_unge(void);
void test_ungt_unle_unlt(void);
void test_ltgt(void);
void test_mixed_ordered(void);

// Volatile doubles to prevent constant folding
volatile double v1 = 1.0;
volatile double v2 = 0.0/0.0;  // NaN
volatile double v3 = 3.0;
volatile double v4 = -2.5;
volatile double v5 = 0.0;
volatile double v6 = 1.0/0.0;  // Infinity

void test_unordered(void) {
    // UNORDERED: x != x when x is NaN
    if (v2 != v2) {  // Should be true for NaN
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // ORDERED: x == x when x is not NaN
    if (v1 == v1) {  // Should be true for normal number
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // Another UNORDERED test with explicit NaN comparison
    volatile double local_nan = v2;
    if (local_nan != local_nan) {  // UNORDERED
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

void test_uneq_unge(void) {
    // UNEQ: == comparison with potential NaN
    // Using volatile to prevent optimization
    volatile double a = v1;
    volatile double b = v3;
    if (a == b) {  // UNEQ (unordered or equal)
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // UNGE: >= comparison with NaN operand
    // v1 >= v2 where v2 is NaN
    if (v1 >= v2) {  // UNGE (unordered or greater or equal)
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // Another UNGE test with different operands
    if (v3 >= v2) {  // UNGE
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

void test_ungt_unle_unlt(void) {
    // UNGT: > comparison with NaN operand
    if (v1 > v2) {  // UNGT (unordered or greater than)
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // UNLE: <= comparison with NaN operand
    if (v2 <= v3) {  // UNLE (unordered or less or equal)
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // UNLT: < comparison with NaN operand
    if (v2 < v1) {  // UNLT (unordered or less than)
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // Additional UNGT test with infinity
    if (v6 > v1) {  // UNGT (infinity > 1.0)
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

void test_ltgt(void) {
    // LTGT: != comparison with ordered operands
    // Using non-NaN values to ensure ordered comparison
    if (v1 != v3) {  // LTGT (less than or greater than)
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // Another LTGT test with different values
    if (v4 != v5) {  // LTGT
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // Using __builtin_islessgreater for explicit LTGT
    if (__builtin_islessgreater(v1, v3)) {  // Explicit LTGT
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

void test_mixed_ordered(void) {
    // Mixed comparisons to trigger various condition codes
    volatile double a = v1;
    volatile double b = v2;  // NaN
    volatile double c = v3;
    
    // Complex expression that might generate multiple condition codes
    if ((a < b) || (c > a)) {  // UNLT or UNGT
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // Ternary operator with floating comparison
    int res = (b >= a) ? 1 : 0;  // UNGE in ternary context
    results[idx++] = res;
    
    // Nested comparisons
    if ((a == b) && (c != a)) {  // UNEQ and LTGT
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

int main(void) {
    // Initialize results array
    for (int i = 0; i < 20; i++) {
        results[i] = -1;
    }
    idx = 0;
    
    // Run all test functions
    test_unordered();
    test_uneq_unge();
    test_ungt_unle_unlt();
    test_ltgt();
    test_mixed_ordered();
    
    // Compute checksum to ensure all code executed
    int checksum = 0;
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
    }
    
    // Print checksum to prevent dead code elimination
    printf("Checksum: %d\n", checksum);
    printf("Number of tests: %d\n", idx);
    
    // Additional printf to force evaluation of all comparisons
    for (int i = 0; i < idx; i++) {
        printf("Result[%d] = %d\n", i, results[i]);
    }
    
    return 0;
}
