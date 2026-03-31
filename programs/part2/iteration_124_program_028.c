#include <stdio.h>
#include <math.h>
#include <stdint.h>

// Global results array to prevent optimization
volatile int results[32];
volatile int idx = 0;

// Helper to create NaN
static inline double make_nan(void) {
    return 0.0 / 0.0;
}

// Test UNORDERED and ORDERED conditions
void test_unordered_ordered(void) {
    volatile double v1 = 1.0;
    volatile double v2 = make_nan();
    volatile double v3 = 3.0;
    
    // UNORDERED: v2 != v2 (NaN != NaN)
    if (v2 != v2) {
        results[idx++] = 1;  // true branch
    } else {
        results[idx++] = 0;  // false branch
    }
    
    // ORDERED: v1 == v1 (normal == normal)
    if (v1 == v1) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // Mixed: ORDERED with potential NaN
    if (v1 == v3) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test UNEQ, UNGE, UNGT conditions
void test_uneq_unge_ungt(void) {
    volatile double a = 2.0;
    volatile double b = make_nan();
    volatile double c = 2.0;
    volatile double d = 3.0;
    
    // UNEQ: a == c (equal, but unordered possible)
    if (a == c) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // UNGE: a >= b (a >= NaN)
    if (a >= b) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // UNGT: a > b (a > NaN)
    if (a > b) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // UNGE with normal values
    if (d >= a) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test UNLE, UNLT conditions
void test_unle_unlt(void) {
    volatile double x = 1.0;
    volatile double y = make_nan();
    volatile double z = 3.0;
    
    // UNLE: y <= z (NaN <= 3.0)
    if (y <= z) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // UNLT: y < x (NaN < 1.0)
    if (y < x) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // UNLE with normal values
    if (x <= z) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // UNLT with normal values
    if (x < z) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Test LTGT (not equal and ordered) condition
void test_ltgt(void) {
    volatile double p = 2.0;
    volatile double q = 3.0;
    volatile double r = make_nan();
    
    // LTGT: p != q (ordered not equal)
    if (p != q) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // Using __builtin_islessgreater for explicit LTGT
    if (__builtin_islessgreater(p, q)) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // LTGT with NaN (should be false)
    if (p != r) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

// Additional tests with ternary operators
void test_ternary_ops(void) {
    volatile double m = 5.0;
    volatile double n = make_nan();
    volatile double o = 5.0;
    
    // Ternary with UNEQ
    int res1 = (m == o) ? 1 : 0;
    results[idx++] = res1;
    
    // Ternary with UNORDERED
    int res2 = (n != n) ? 1 : 0;
    results[idx++] = res2;
    
    // Ternary with UNGE
    int res3 = (m >= n) ? 1 : 0;
    results[idx++] = res3;
    
    // Ternary with UNLE
    int res4 = (n <= m) ? 1 : 0;
    results[idx++] = res4;
}

// Complex expression to force condition code generation
void test_complex_expressions(void) {
    volatile double a = 1.0;
    volatile double b = make_nan();
    volatile double c = 2.0;
    volatile double d = 3.0;
    
    // Complex condition mixing ordered and unordered
    if ((a < c) && (b != b)) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // Nested comparisons
    if ((a == a) || (b > d)) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // Multiple conditions
    if ((c != d) && (a <= c) && (b >= a)) {
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
    
    // Run all test functions
    test_unordered_ordered();
    test_uneq_unge_ungt();
    test_unle_unlt();
    test_ltgt();
    test_ternary_ops();
    test_complex_expressions();
    
    // Compute checksum to ensure execution
    int checksum = 0;
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Tests executed: %d\n", idx);
    
    return 0;
}
