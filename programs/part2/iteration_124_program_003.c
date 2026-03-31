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

int main(void) {
    // Initialize with volatile to prevent constant folding
    volatile double v1 = 1.0;
    volatile double v2 = 0.0/0.0;  // NaN
    volatile double v3 = 3.0;
    volatile double v4 = 1.0;
    volatile double v5 = NAN;      // Another NaN
    
    // Force compiler to keep these variables
    asm volatile("" : : "r"(v1), "r"(v2), "r"(v3), "r"(v4), "r"(v5));
    
    // Test all condition codes
    test_unordered();
    test_ordered();
    test_uneq();
    test_unge();
    test_ungt();
    test_unle();
    test_unlt();
    test_ltgt();
    
    // Additional mixed tests to ensure coverage
    volatile double a = 2.0;
    volatile double b = NAN;
    volatile double c = 2.0;
    
    // UNORDERED: x != x when x is NaN
    if (b != b) {
        results[idx++] = 1;  // UNORDERED true
    } else {
        results[idx++] = 0;
    }
    
    // ORDERED: x == x when x is not NaN
    if (a == a) {
        results[idx++] = 1;  // ORDERED true
    } else {
        results[idx++] = 0;
    }
    
    // UNEQ: a == b (with potential NaN)
    if (a == b) {
        results[idx++] = 1;  // UNEQ true (false due to NaN)
    } else {
        results[idx++] = 0;
    }
    
    // UNGE: a >= b (with b as NaN)
    if (a >= b) {
        results[idx++] = 1;  // UNGE true (false due to NaN)
    } else {
        results[idx++] = 0;
    }
    
    // UNGT: a > b (with b as NaN)
    if (a > b) {
        results[idx++] = 1;  // UNGT true (false due to NaN)
    } else {
        results[idx++] = 0;
    }
    
    // UNLE: b <= a (with b as NaN)
    if (b <= a) {
        results[idx++] = 1;  // UNLE true (false due to NaN)
    } else {
        results[idx++] = 0;
    }
    
    // UNLT: b < a (with b as NaN)
    if (b < a) {
        results[idx++] = 1;  // UNLT true (false due to NaN)
    } else {
        results[idx++] = 0;
    }
    
    // LTGT: a != c (both ordered, equal values)
    if (a != c) {
        results[idx++] = 1;  // LTGT false
    } else {
        results[idx++] = 0;  // LTGT true (values equal)
    }
    
    // LTGT with __builtin_islessgreater
    if (__builtin_islessgreater(a, c)) {
        results[idx++] = 1;  // false
    } else {
        results[idx++] = 0;  // true
    }
    
    // Compute checksum to ensure all code executed
    int checksum = 0;
    for (int i = 0; i < idx; i++) {
        checksum += results[i];
    }
    
    printf("Checksum: %d\n", checksum);
    printf("Total comparisons: %d\n", idx);
    
    return checksum;
}

void test_unordered(void) {
    volatile double nan_val = NAN;
    volatile double normal_val = 5.0;
    
    // UNORDERED: nan_val != nan_val
    if (nan_val != nan_val) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // Another UNORDERED test
    volatile double inf_val = 1.0/0.0;
    if (inf_val != inf_val) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

void test_ordered(void) {
    volatile double x = 10.0;
    volatile double y = 20.0;
    
    // ORDERED: x == x
    if (x == x) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // ORDERED in comparison context
    if (x < y) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

void test_uneq(void) {
    volatile double a = 15.0;
    volatile double b = NAN;
    volatile double c = 15.0;
    
    // UNEQ: a == b (with NaN)
    if (a == b) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // UNEQ: a == c (ordered, equal)
    if (a == c) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

void test_unge(void) {
    volatile double x = 25.0;
    volatile double nan_val = NAN;
    volatile double y = 30.0;
    
    // UNGE: x >= nan_val
    if (x >= nan_val) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // UNGE: x >= y (ordered, false)
    if (x >= y) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

void test_ungt(void) {
    volatile double a = 35.0;
    volatile double b = NAN;
    volatile double c = 40.0;
    
    // UNGT: a > b (with NaN)
    if (a > b) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // UNGT: a > c (ordered, false)
    if (a > c) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

void test_unle(void) {
    volatile double nan_val = NAN;
    volatile double x = 45.0;
    volatile double y = 50.0;
    
    // UNLE: nan_val <= x
    if (nan_val <= x) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // UNLE: x <= y (ordered, true)
    if (x <= y) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

void test_unlt(void) {
    volatile double nan_val = NAN;
    volatile double a = 55.0;
    volatile double b = 60.0;
    
    // UNLT: nan_val < a
    if (nan_val < a) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // UNLT: a < b (ordered, true)
    if (a < b) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}

void test_ltgt(void) {
    volatile double x = 65.0;
    volatile double y = 70.0;
    volatile double z = 65.0;
    
    // LTGT: x != y (ordered, not equal)
    if (x != y) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // LTGT: x != z (ordered, equal)
    if (x != z) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    // LTGT using __builtin_islessgreater
    if (__builtin_islessgreater(x, y)) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
    
    if (__builtin_islessgreater(x, z)) {
        results[idx++] = 1;
    } else {
        results[idx++] = 0;
    }
}
