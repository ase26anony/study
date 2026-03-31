#include <stdio.h>
#include <math.h>
#include <stdint.h>

// Prevent optimization
extern void sink(int);
volatile int checksum = 0;

// Force condition code usage with inline assembly
#define USE_CC(cond) \
    __asm__ goto ("j%c0 %l0" : : "i" (cond) : : target); \
    target: checksum += __LINE__;

// Pattern 1: UNORDERED comparisons
__attribute__((noinline))
void test_unordered(void) {
    volatile double nan1 = 0.0/0.0;
    volatile double nan2 = nan1 * 2.0;
    volatile double normal = 3.14;
    
    // Direct unordered check
    if (isunordered(nan1, normal)) {
        USE_CC(UNORDERED);
    }
    
    // Self-comparison for unordered
    if (nan1 != nan1) {
        USE_CC(UNORDERED);
    }
    
    // Complex expression with unordered
    int r = (isunordered(nan1, nan2)) ? 100 : 200;
    sink(r);
    
    // Memory operand with unordered
    volatile double arr[3] = {nan1, normal, nan2};
    if (isunordered(arr[0], arr[2])) {
        USE_CC(UNORDERED);
    }
}

// Pattern 2: ORDERED comparisons
__attribute__((noinline))
void test_ordered(void) {
    volatile double nan = 0.0/0.0;
    volatile double inf = 1.0/0.0;
    volatile double normal1 = 2.71;
    volatile double normal2 = 3.14;
    
    // Direct ordered check
    if (isordered(normal1, normal2)) {
        USE_CC(ORDERED);
    }
    
    // Self-comparison for ordered
    if (normal1 == normal1) {
        USE_CC(ORDERED);
    }
    
    // Ordered with memory operand
    volatile struct { double a, b; } s = {normal1, inf};
    if (isordered(s.a, s.b)) {
        USE_CC(ORDERED);
    }
    
    // Mixed types for ordered
    volatile float f1 = 1.5f;
    volatile long double ld1 = 2.5L;
    if (isordered(f1, (double)ld1)) {
        USE_CC(ORDERED);
    }
}

// Pattern 3: UNEQ (unordered or equal)
__attribute__((noinline))
void test_uneq(void) {
    volatile double nan = 0.0/0.0;
    volatile double a = 1.0;
    volatile double b = 1.0;
    volatile double c = 2.0;
    
    // Generate UNEQ: !(a < b) && !(a > b) which is a >= b && a <= b
    // For unordered values, this also captures unordered case
    if (!(nan < nan) && !(nan > nan)) {
        USE_CC(UNEQ);
    }
    
    // Equal values
    if (!(a < b) && !(a > b)) {
        USE_CC(UNEQ);
    }
    
    // Complex expression
    volatile double arr[2] = {a, b};
    int r = (!(arr[0] < arr[1]) && !(arr[0] > arr[1])) ? 1 : 0;
    sink(r);
}

// Pattern 4: UNGE (not less than) = !(a < b)
__attribute__((noinline))
void test_unge(void) {
    volatile double a = 5.0;
    volatile double b = 3.0;
    volatile double c = 7.0;
    volatile double nan = 0.0/0.0;
    
    // Direct UNGE: !(a < b)
    if (!(a < b)) {
        USE_CC(UNGE);
    }
    
    // UNGE with nan (unordered)
    if (!(nan < b)) {
        USE_CC(UNGE);
    }
    
    // In ternary operator
    volatile double *ptr = &a;
    int r = (!(*ptr < b)) ? 10 : 20;
    sink(r);
    
    // Multiple comparisons
    if (!(a < b) && !(b < c)) {
        checksum += 1;
    }
}

// Pattern 5: UNGT (not less or equal) = !(a <= b)
__attribute__((noinline))
void test_ungt(void) {
    volatile double a = 8.0;
    volatile double b = 5.0;
    volatile double c = 8.0;  // equal case
    volatile double nan = 0.0/0.0;
    
    // Direct UNGT: !(a <= b)
    if (!(a <= b)) {
        USE_CC(UNGT);
    }
    
    // Equal values should not trigger
    if (!(c <= b)) {
        USE_CC(UNGT);
    }
    
    // With nan
    if (!(nan <= a)) {
        USE_CC(UNGT);
    }
    
    // In complex logical expression
    volatile struct { double x, y; } point = {a, b};
    if (!(point.x <= point.y) || (point.x > 0)) {
        checksum += 2;
    }
}

// Pattern 6: UNLE (unordered or less or equal)
__attribute__((noinline))
void test_unle(void) {
    volatile double a = 3.0;
    volatile double b = 5.0;
    volatile double c = 5.0;  // equal case
    volatile double nan = 0.0/0.0;
    
    // UNLE: !(a > b)
    if (!(a > b)) {
        USE_CC(UNLE);
    }
    
    // Equal case
    if (!(c > b)) {
        USE_CC(UNLE);
    }
    
    // With nan (unordered)
    if (!(nan > a)) {
        USE_CC(UNLE);
    }
    
    // Nested in expression
    int r = (!(a > b)) ? (!(b > 10.0) ? 1 : 2) : 3;
    sink(r);
}

// Pattern 7: UNLT (unordered or less than)
__attribute__((noinline))
void test_unlt(void) {
    volatile double a = 2.0;
    volatile double b = 4.0;
    volatile double c = 4.0;  // equal case
    volatile double nan = 0.0/0.0;
    
    // UNLT: !(a >= b)
    if (!(a >= b)) {
        USE_CC(UNLT);
    }
    
    // Equal case should not trigger UNLT
    if (!(c >= b)) {
        // This won't execute for equal values
        checksum += 100;
    }
    
    // With nan
    if (!(nan >= a)) {
        USE_CC(UNLT);
    }
    
    // Memory operand
    volatile double mem[2] = {a, b};
    if (!(mem[0] >= mem[1])) {
        USE_CC(UNLT);
    }
}

// Pattern 8: LTGT (less or greater, but not equal and ordered)
__attribute__((noinline))
void test_ltgt(void) {
    volatile double a = 3.0;
    volatile double b = 7.0;
    volatile double c = 7.0;  // equal case
    volatile double nan = 0.0/0.0;
    
    // LTGT: (a < b) || (a > b)
    if ((a < b) || (a > b)) {
        USE_CC(LTGT);
    }
    
    // Equal values should not trigger
    if ((c < b) || (c > b)) {
        checksum += 200;  // Shouldn't execute
    }
    
    // With nan (unordered shouldn't trigger LTGT)
    if ((nan < a) || (nan > a)) {
        checksum += 300;  // Might execute depending on NaN behavior
    }
    
    // Complex expression
    volatile double *p1 = &a, *p2 = &b;
    int r = ((*p1 < *p2) || (*p1 > *p2)) ? 5 : 6;
    sink(r);
}

// Pattern 9: Mixed condition codes in complex expressions
__attribute__((noinline))
void test_mixed(void) {
    volatile double nan = 0.0/0.0;
    volatile double x = 1.5;
    volatile double y = 2.5;
    volatile double z = 3.5;
    
    // Combine multiple condition codes
    int result = 0;
    
    if (isunordered(nan, x)) {
        result |= 1;  // UNORDERED
    }
    
    if (!(x < y)) {
        result |= 2;  // UNGE
    }
    
    if (!(y <= z)) {
        result |= 4;  // UNGT
    }
    
    if ((x < z) || (x > z)) {
        result |= 8;  // LTGT
    }
    
    sink(result);
    
    // Nested ternary with different condition codes
    volatile double arr[3] = {nan, x, y};
    int r = (!(arr[0] == arr[0])) ? 1 :  // UNORDERED check
            (!(arr[1] < arr[2])) ? 2 :   // UNGE
            (!(arr[2] > arr[1])) ? 3 :   // UNLE
            ((arr[1] < arr[2]) || (arr[1] > arr[2])) ? 4 :  // LTGT
            0;
    sink(r);
}

// Pattern 10: Long double operations
__attribute__((noinline))
void test_long_double(void) {
    volatile long double ld_nan = 0.0L/0.0L;
    volatile long double ld1 = 1.23456789L;
    volatile long double ld2 = 9.87654321L;
    
    // Long double unordered
    if (isunordered(ld_nan, ld1)) {
        USE_CC(UNORDERED);
    }
    
    // Long double ordered
    if (isordered(ld1, ld2)) {
        USE_CC(ORDERED);
    }
    
    // Long double comparisons generating various condition codes
    if (!(ld1 < ld2)) {
        USE_CC(UNGE);
    }
    
    if (!(ld1 <= ld2)) {
        USE_CC(UNGT);
    }
    
    if ((ld1 < ld2) || (ld1 > ld2)) {
        USE_CC(LTGT);
    }
}

int main(void) {
    // Initialize volatile seed
    volatile int seed = 42;
    checksum = seed;
    
    // Call all test patterns
    test_unordered();
    test_ordered();
    test_uneq();
    test_unge();
    test_ungt();
    test_unle();
    test_unlt();
    test_ltgt();
    test_mixed();
    test_long_double();
    
    // Print checksum to ensure all code is live
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
