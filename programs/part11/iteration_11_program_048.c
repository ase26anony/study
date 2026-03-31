#include <stdio.h>
#include <math.h>
#include <stdint.h>

// Prevent optimization
extern void sink(int);
volatile int checksum = 0;

// Function to generate NaN
static double make_nan(void) {
    volatile double zero = 0.0;
    return zero / zero;
}

// Function to generate infinity
static double make_inf(void) {
    volatile double large = 1e308;
    return large * large;
}

// UNORDERED and ORDERED patterns
__attribute__((noinline))
void test_unordered_ordered(void) {
    volatile double nan1 = make_nan();
    volatile double nan2 = make_nan();
    volatile double normal = 3.14;
    volatile double inf = make_inf();
    
    int result = 0;
    
    // UNORDERED: using isunordered()
    if (isunordered(nan1, normal)) {
        result |= 1;
    }
    
    // UNORDERED: direct NaN comparison
    if (nan1 != nan1) {
        result |= 2;
    }
    
    // ORDERED: using isordered()
    if (isordered(normal, inf)) {
        result |= 4;
    }
    
    // ORDERED: direct comparison
    if (normal == normal) {
        result |= 8;
    }
    
    // Mixed unordered/ordered with inline asm
    volatile double a = nan1;
    volatile double b = normal;
    
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : "cc", "st", "st(1)"
        : unordered_label
    );
    
    // This should not be reached if jump taken
    result |= 16;
    goto after_label;
    
unordered_label:
    result |= 32;
    
after_label:
    checksum += result;
    sink(result);
}

// UNEQ (unordered or equal)
__attribute__((noinline))
void test_uneq(void) {
    volatile double a = make_nan();
    volatile double b = 5.0;
    volatile double c = 5.0;
    volatile double d = 6.0;
    
    int result = 0;
    
    // UNEQ: !(a > b) && !(a < b) which includes NaN case
    if (!(a > b) && !(a < b)) {
        result |= 1;
    }
    
    // UNEQ with equal values
    if (!(c > d) && !(c < d)) {
        result |= 2;
    }
    
    // Using inline asm to force condition code
    volatile double x = a;
    volatile double y = b;
    
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : "cc", "st", "st(1)"
        : uneq_label
    );
    
    result |= 4;
    goto after_uneq;
    
uneq_label:
    result |= 8;
    
after_uneq:
    checksum += result;
    sink(result);
}

// UNGE (!(a < b) - not less than, includes unordered)
__attribute__((noinline))
void test_unge(void) {
    volatile double a = make_nan();
    volatile double b = 10.0;
    volatile double c = 15.0;
    volatile double d = 10.0;
    
    int result = 0;
    
    // UNGE: !(a < b)
    if (!(a < b)) {
        result |= 1;
    }
    
    // UNGE with normal values
    if (!(c < d)) {
        result |= 2;
    }
    
    // Complex expression
    result += (a < b) ? 0 : 4;
    
    checksum += result;
    sink(result);
}

// UNGT (!(a <= b) - not less or equal, includes unordered)
__attribute__((noinline))
void test_ungt(void) {
    volatile double a = make_nan();
    volatile double b = 20.0;
    volatile double c = 25.0;
    volatile double d = 20.0;
    
    int result = 0;
    
    // UNGT: !(a <= b)
    if (!(a <= b)) {
        result |= 1;
    }
    
    // Using different floating types
    volatile float f1 = make_nan();
    volatile float f2 = 30.0f;
    if (!(f1 <= f2)) {
        result |= 2;
    }
    
    // Long double
    volatile long double ld1 = make_nan();
    volatile long double ld2 = 40.0L;
    if (!(ld1 <= ld2)) {
        result |= 4;
    }
    
    checksum += result;
    sink(result);
}

// UNLE (unordered or less or equal)
__attribute__((noinline))
void test_unle(void) {
    volatile double a = make_nan();
    volatile double b = 50.0;
    volatile double c = 45.0;
    volatile double d = 50.0;
    
    int result = 0;
    
    // UNLE: !(a > b)
    if (!(a > b)) {
        result |= 1;
    }
    
    // With equal values
    if (!(d > b)) {
        result |= 2;
    }
    
    // With less than
    if (!(c > b)) {
        result |= 4;
    }
    
    checksum += result;
    sink(result);
}

// UNLT (unordered or less than)
__attribute__((noinline))
void test_unlt(void) {
    volatile double a = make_nan();
    volatile double b = 60.0;
    volatile double c = 55.0;
    volatile double d = 60.0;
    
    int result = 0;
    
    // UNLT: !(a >= b)
    if (!(a >= b)) {
        result |= 1;
    }
    
    // With less than
    if (!(c >= b)) {
        result |= 2;
    }
    
    // Using memory operands
    volatile double arr[2] = {make_nan(), 70.0};
    if (!(arr[0] >= arr[1])) {
        result |= 4;
    }
    
    checksum += result;
    sink(result);
}

// LTGT (not equal and ordered)
__attribute__((noinline))
void test_ltgt(void) {
    volatile double a = 80.0;
    volatile double b = 85.0;
    volatile double c = 80.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    // LTGT: (a < b) || (a > b)
    if ((a < b) || (a > b)) {
        result |= 1;
    }
    
    // Should be false for equal values
    if ((c < a) || (c > a)) {
        result |= 2;
    }
    
    // Should be false for NaN
    if ((nan < a) || (nan > a)) {
        result |= 4;
    }
    
    // Complex ternary expression
    result += ((a < b) || (a > b)) ? 8 : 0;
    
    // Inline asm with condition code
    volatile double x = a;
    volatile double y = b;
    
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l0"
        : /* no outputs */
        : /* no inputs */
        : "cc", "st", "st(1)"
        : ltgt_label
    );
    
    result |= 16;
    goto after_ltgt;
    
ltgt_label:
    result |= 32;
    
after_ltgt:
    checksum += result;
    sink(result);
}

// Test with struct members
typedef struct {
    volatile double x;
    volatile double y;
} Point;

__attribute__((noinline))
void test_struct_comparisons(void) {
    Point p1 = {make_nan(), 100.0};
    Point p2 = {150.0, make_nan()};
    
    int result = 0;
    
    // UNORDERED with struct members
    if (isunordered(p1.x, p2.x)) {
        result |= 1;
    }
    
    // UNGE with struct members
    if (!(p1.y < p2.y)) {
        result |= 2;
    }
    
    // Complex expression mixing comparisons
    result += (p1.x != p1.x) ? 4 : ((p2.y > p1.y) ? 8 : 16);
    
    checksum += result;
    sink(result);
}

// Mixed type comparisons
__attribute__((noinline))
void test_mixed_types(void) {
    volatile float f_nan = 0.0f / 0.0f;
    volatile double d_nan = make_nan();
    volatile long double ld_nan = make_nan();
    
    volatile float f_val = 200.0f;
    volatile double d_val = 200.0;
    volatile long double ld_val = 200.0L;
    
    int result = 0;
    
    // Float comparisons
    if (isunordered(f_nan, f_val)) {
        result |= 1;
    }
    
    // Double comparisons
    if (!(d_nan <= d_val)) {  // UNGT
        result |= 2;
    }
    
    // Long double comparisons
    if (!(ld_nan >= ld_val)) {  // UNLT
        result |= 4;
    }
    
    // Cross-type comparisons (promotions will occur)
    if (isunordered(f_nan, d_val)) {
        result |= 8;
    }
    
    checksum += result;
    sink(result);
}

int main(void) {
    // Initialize checksum
    checksum = 0;
    
    // Call all test functions
    test_unordered_ordered();
    test_uneq();
    test_unge();
    test_ungt();
    test_unle();
    test_unlt();
    test_ltgt();
    test_struct_comparisons();
    test_mixed_types();
    
    // Print final checksum to ensure all code is live
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
