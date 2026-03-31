#include <stdio.h>
#include <math.h>

// Global results array to prevent optimization
volatile int results[20] = {0};
int result_idx = 0;

// Test functions targeting specific condition codes
void test_unordered_ordered(void) {
    volatile double a = 1.0;
    volatile double b = 0.0 / 0.0;  // NaN
    
    // UNORDERED: a != a when a is NaN
    if (b != b) {  // Should be true for NaN
        results[result_idx++] = 1;  // UNORDERED true
    } else {
        results[result_idx++] = 0;
    }
    
    // ORDERED: a == a when a is not NaN
    if (a == a) {  // Should be true for normal number
        results[result_idx++] = 1;  // ORDERED true
    } else {
        results[result_idx++] = 0;
    }
    
    // More UNORDERED/ORDERED variations
    volatile double c = NAN;
    if (c == c) {  // false for NaN -> ORDERED false
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;  // ORDERED false
    }
    
    if (b == b) {  // false for NaN -> ORDERED false
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;
    }
}

void test_uneq_unge(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double nan = NAN;
    
    // UNEQ: a == b (unordered equal)
    if (a == b) {  // false for 1.0 == 2.0
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;  // UNEQ false
    }
    
    // UNEQ with NaN
    if (a == nan) {  // false (unordered)
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;  // UNEQ false
    }
    
    // UNGE: a >= b (unordered greater or equal)
    if (a >= nan) {  // false with NaN -> UNGE false
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;
    }
    
    if (nan >= a) {  // false with NaN -> UNGE false
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;
    }
    
    // Regular >= for comparison
    if (b >= a) {  // true for 2.0 >= 1.0
        results[result_idx++] = 1;  // UNGE true
    } else {
        results[result_idx++] = 0;
    }
}

void test_ungt_unle_unlt(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double nan = NAN;
    volatile double c = 3.0;
    
    // UNGT: a > b (unordered greater than)
    if (a > b) {  // false for 1.0 > 2.0
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;  // UNGT false
    }
    
    if (nan > a) {  // false with NaN -> UNGT false
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;
    }
    
    if (b > a) {  // true for 2.0 > 1.0
        results[result_idx++] = 1;  // UNGT true
    } else {
        results[result_idx++] = 0;
    }
    
    // UNLE: a <= b (unordered less or equal)
    if (a <= nan) {  // false with NaN -> UNLE false
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;
    }
    
    if (a <= b) {  // true for 1.0 <= 2.0
        results[result_idx++] = 1;  // UNLE true
    } else {
        results[result_idx++] = 0;
    }
    
    // UNLT: a < b (unordered less than)
    if (nan < a) {  // false with NaN -> UNLT false
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;
    }
    
    if (a < b) {  // true for 1.0 < 2.0
        results[result_idx++] = 1;  // UNLT true
    } else {
        results[result_idx++] = 0;
    }
    
    if (b < a) {  // false for 2.0 < 1.0
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;  // UNLT false
    }
}

void test_ltgt(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double c = 1.0;
    volatile double nan = NAN;
    
    // LTGT: a != b and ordered (not equal and ordered)
    if (a != b) {  // true for 1.0 != 2.0
        results[result_idx++] = 1;  // LTGT true
    } else {
        results[result_idx++] = 0;
    }
    
    // Using __builtin_islessgreater for explicit LTGT
    if (__builtin_islessgreater(a, b)) {  // true
        results[result_idx++] = 1;  // LTGT true
    } else {
        results[result_idx++] = 0;
    }
    
    if (__builtin_islessgreater(a, c)) {  // false for 1.0 != 1.0
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;  // LTGT false
    }
    
    // LTGT with NaN (should be false)
    if (a != nan) {  // true, but unordered
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;
    }
    
    if (__builtin_islessgreater(a, nan)) {  // false with NaN
        results[result_idx++] = 1;
    } else {
        results[result_idx++] = 0;  // LTGT false
    }
}

void test_mixed_conditions(void) {
    volatile double x = 5.0;
    volatile double y = 10.0;
    volatile double z = NAN;
    
    // Complex expression mixing conditions
    int r1 = (x < y) ? 1 : 0;           // UNLT true
    int r2 = (z > x) ? 1 : 0;           // UNGT false (NaN)
    int r3 = (x != z) ? 1 : 0;          // LTGT? (unordered)
    int r4 = (z == z) ? 1 : 0;          // ORDERED false (NaN)
    int r5 = (x == x) ? 1 : 0;          // ORDERED true
    
    results[result_idx++] = r1;
    results[result_idx++] = r2;
    results[result_idx++] = r3;
    results[result_idx++] = r4;
    results[result_idx++] = r5;
    
    // Nested conditionals
    if (x < y) {
        if (y > x) {
            results[result_idx++] = 1;  // Both UNLT and UNGT true
        } else {
            results[result_idx++] = 0;
        }
    }
    
    // Switch-like behavior using ternary
    results[result_idx++] = (x <= y) ? 1 : 0;   // UNLE true
    results[result_idx++] = (y >= x) ? 1 : 0;   // UNGE true
    results[result_idx++] = (x == y) ? 1 : 0;   // UNEQ false
}

int main(void) {
    // Initialize all test functions
    test_unordered_ordered();
    test_uneq_unge();
    test_ungt_unle_unlt();
    test_ltgt();
    test_mixed_conditions();
    
    // Compute checksum to ensure all code executed
    int checksum = 0;
    for (int i = 0; i < result_idx; i++) {
        checksum += results[i];
        // Also use in conditional to force condition code usage
        if (results[i] == 0) {
            checksum += 100;
        } else {
            checksum += 200;
        }
    }
    
    printf("Result checksum: %d\n", checksum);
    printf("Number of comparisons: %d\n", result_idx);
    
    // Additional volatile store to prevent optimization
    volatile int dummy = checksum;
    
    return checksum > 0 ? 0 : 1;
}
