#include <stdio.h>
#include <math.h>

// Global array to store comparison results
int results[20] = {0};
int idx = 0;

// Test functions targeting specific condition codes
void test_unordered(void) {
    volatile double v1 = 1.0;
    volatile double v2 = 0.0/0.0;  // NaN
    volatile double v3 = 3.0;
    
    // UNORDERED: v2 != v2 (NaN != NaN)
    if (v2 != v2) {
        results[idx++] = 1;  // Should be true
    } else {
        results[idx++] = 0;
    }
    
    // ORDERED: v1 == v1 (normal == normal)
    if (v1 == v1) {
        results[idx++] = 1;  // Should be true
    } else {
        results[idx++] = 0;
    }
    
    // UNORDERED with different NaN source
    volatile double v4 = NAN;
    if (v4 != v4) {
        results[idx++] = 1;  // Should be true
    } else {
        results[idx++] = 0;
    }
}

void test_uneq_unge(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double nan_val = NAN;
    
    // UNEQ: a == b (but with unordered possibility)
    // Using volatile to prevent constant folding
    volatile double c = 1.0;
    volatile double d = 1.0;
    if (c == d) {
        results[idx++] = 1;  // Should be true
    } else {
        results[idx++] = 0;
    }
    
    // UNGE: a >= nan_val
    if (a >= nan_val) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  // Should be false (unordered)
    }
    
    // UNGE with both normal values
    if (b >= a) {
        results[idx++] = 1;  // Should be true
    } else {
        results[idx++] = 0;
    }
}

void test_ungt_unle_unlt(void) {
    volatile double x = 5.0;
    volatile double y = 3.0;
    volatile double nan1 = NAN;
    volatile double nan2 = 0.0/0.0;
    
    // UNGT: x > nan1
    if (x > nan1) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  // Should be false (unordered)
    }
    
    // UNLE: nan2 <= y
    if (nan2 <= y) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  // Should be false (unordered)
    }
    
    // UNLT: nan1 < x
    if (nan1 < x) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  // Should be false (unordered)
    }
    
    // Normal comparisons that should generate condition codes
    if (x > y) {
        results[idx++] = 1;  // Should be true
    } else {
        results[idx++] = 0;
    }
    
    if (y < x) {
        results[idx++] = 1;  // Should be true
    } else {
        results[idx++] = 0;
    }
}

void test_ltgt(void) {
    volatile double p = 7.0;
    volatile double q = 8.0;
    volatile double r = 7.0;
    volatile double nan = NAN;
    
    // LTGT: p != q (not equal and ordered)
    if (p != q) {
        results[idx++] = 1;  // Should be true
    } else {
        results[idx++] = 0;
    }
    
    // LTGT: p != r (equal case)
    if (p != r) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  // Should be false
    }
    
    // Using __builtin_islessgreater for explicit LTGT
    if (__builtin_islessgreater(p, q)) {
        results[idx++] = 1;  // Should be true
    } else {
        results[idx++] = 0;
    }
    
    // LTGT with NaN (should be false)
    if (p != nan) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;  // Should be false (unordered)
    }
}

void test_mixed_conditions(void) {
    volatile double a = 10.0;
    volatile double b = 20.0;
    volatile double c = 10.0;
    volatile double nan = -NAN;  // Negative NaN
    
    // Mix of comparisons in complex expressions
    int res1 = (a < b) ? 1 : 0;
    int res2 = (b > a) ? 1 : 0;
    int res3 = (a == c) ? 1 : 0;
    int res4 = (nan == nan) ? 1 : 0;
    int res5 = (a != nan) ? 1 : 0;
    
    results[idx++] = res1;
    results[idx++] = res2;
    results[idx++] = res3;
    results[idx++] = res4;
    results[idx++] = res5;
    
    // Complex conditional with multiple comparisons
    if ((a < b) && (b > a) && (a != nan)) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

int main(void) {
    // Run all test functions
    test_unordered();
    test_uneq_unge();
    test_ungt_unle_unlt();
    test_ltgt();
    test_mixed_conditions();
    
    // Compute checksum to ensure all code executed
    int checksum = 0;
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
        checksum = (checksum * 31) & 0xFF;
    }
    
    printf("Results checksum: %d\n", checksum);
    printf("Total comparisons: %d\n", idx);
    
    // Print individual results for debugging
    printf("Individual results: ");
    for (int i = 0; i < idx; i++) {
        printf("%d ", results[i]);
    }
    printf("\n");
    
    return 0;
}
