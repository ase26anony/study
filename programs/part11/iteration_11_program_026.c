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

// UNORDERED comparisons with NaN
__attribute__((noinline))
static int test_unordered(void) {
    volatile double nan1 = make_nan();
    volatile double nan2 = make_nan();
    volatile double normal = 3.14;
    int result = 0;
    
    // Direct unordered comparisons
    if (nan1 != nan1) {
        result |= 1;
    }
    
    // Using isunordered()
    if (isunordered(nan1, normal)) {
        result |= 2;
    }
    
    // Complex unordered expression
    if (isunordered(nan1, nan2) && !(normal == normal)) {
        // This branch shouldn't be taken
        result |= 4;
    }
    
    // Inline assembly to force condition code use
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l[unordered_label]\n\t"
        : : "i"(UNORDERED) : "cc", "st", "st(1)" : unordered_label
    );
    
    return result;
    
unordered_label:
    result |= 8;
    return result;
}

// ORDERED comparisons
__attribute__((noinline))
static int test_ordered(void) {
    volatile double normal1 = 1.5;
    volatile double normal2 = 2.5;
    volatile double nan = make_nan();
    int result = 0;
    
    // Direct ordered comparisons
    if (normal1 == normal1) {
        result |= 1;
    }
    
    // Using isordered()
    if (isordered(normal1, normal2)) {
        result |= 2;
    }
    
    // Mixed ordered/unordered
    if (isordered(normal1, normal2) || !isunordered(nan, normal1)) {
        result |= 4;
    }
    
    // Inline assembly with ordered condition
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l[ordered_label]\n\t"
        : : "i"(ORDERED) : "cc", "st", "st(1)" : ordered_label
    );
    
    return result;
    
ordered_label:
    result |= 8;
    return result;
}

// UNEQ (unordered or equal)
__attribute__((noinline))
static int test_uneq(void) {
    volatile double a = 2.0;
    volatile double b = 2.0;
    volatile double nan = make_nan();
    int result = 0;
    
    // Generate UNEQ: !(a < b) && !(a > b)
    if (!(a < b) && !(a > b)) {
        result |= 1;
    }
    
    // Alternative: a == b (including NaN case)
    if (!(a != b)) {
        result |= 2;
    }
    
    // With NaN
    if (!(nan < nan) && !(nan > nan)) {
        result |= 4;
    }
    
    // Inline assembly
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l[uneq_label]\n\t"
        : : "i"(UNEQ) : "cc", "st", "st(1)" : uneq_label
    );
    
    return result;
    
uneq_label:
    result |= 8;
    return result;
}

// UNGE (not less than)
__attribute__((noinline))
static int test_unge(void) {
    volatile double a = 3.0;
    volatile double b = 2.0;
    volatile double c = 3.0;
    volatile double nan = make_nan();
    int result = 0;
    
    // Direct: !(a < b)
    if (!(a < b)) {
        result |= 1;
    }
    
    // Equal case: !(c < b)
    if (!(c < b)) {
        result |= 2;
    }
    
    // With NaN
    if (!(nan < a)) {
        result |= 4;
    }
    
    // Complex expression
    if (!(a < b) || (b >= a)) {
        result |= 16;
    }
    
    // Inline assembly
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l[unge_label]\n\t"
        : : "i"(UNGE) : "cc", "st", "st(1)" : unge_label
    );
    
    return result;
    
unge_label:
    result |= 32;
    return result;
}

// UNGT (not less than or equal)
__attribute__((noinline))
static int test_ungt(void) {
    volatile double a = 3.0;
    volatile double b = 2.0;
    volatile double c = 3.0;
    int result = 0;
    
    // Direct: !(a <= b)
    if (!(a <= b)) {
        result |= 1;
    }
    
    // Alternative: a > b
    if (a > b) {
        result |= 2;
    }
    
    // Complex: !(b >= a)
    if (!(b >= a)) {
        result |= 4;
    }
    
    // Inline assembly
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l[ungt_label]\n\t"
        : : "i"(UNGT) : "cc", "st", "st(1)" : ungt_label
    );
    
    return result;
    
ungt_label:
    result |= 8;
    return result;
}

// UNLE (unordered or less than or equal)
__attribute__((noinline))
static int test_unle(void) {
    volatile double a = 2.0;
    volatile double b = 3.0;
    volatile double c = 3.0;
    volatile double nan = make_nan();
    int result = 0;
    
    // Direct: !(a > b)
    if (!(a > b)) {
        result |= 1;
    }
    
    // Equal case: !(c > b)
    if (!(c > b)) {
        result |= 2;
    }
    
    // With NaN
    if (!(nan > a)) {
        result |= 4;
    }
    
    // Alternative: a <= b
    if (a <= b) {
        result |= 8;
    }
    
    // Inline assembly
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l[unle_label]\n\t"
        : : "i"(UNLE) : "cc", "st", "st(1)" : unle_label
    );
    
    return result;
    
unle_label:
    result |= 16;
    return result;
}

// UNLT (unordered or less than)
__attribute__((noinline))
static int test_unlt(void) {
    volatile double a = 2.0;
    volatile double b = 3.0;
    volatile double c = 3.0;
    volatile double nan = make_nan();
    int result = 0;
    
    // Direct: !(a >= b)
    if (!(a >= b)) {
        result |= 1;
    }
    
    // Alternative: a < b
    if (a < b) {
        result |= 2;
    }
    
    // With NaN
    if (!(nan >= a)) {
        result |= 4;
    }
    
    // Complex expression
    if ((a < b) || !(b <= a)) {
        result |= 8;
    }
    
    // Inline assembly
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l[unlt_label]\n\t"
        : : "i"(UNLT) : "cc", "st", "st(1)" : unlt_label
    );
    
    return result;
    
unlt_label:
    result |= 16;
    return result;
}

// LTGT (less than or greater than - not equal and ordered)
__attribute__((noinline))
static int test_ltgt(void) {
    volatile double a = 2.0;
    volatile double b = 3.0;
    volatile double c = 3.0;
    volatile double nan = make_nan();
    int result = 0;
    
    // Direct: (a < b) || (a > b)
    if ((a < b) || (a > b)) {
        result |= 1;
    }
    
    // Equal case (should be false)
    if ((c < b) || (c > b)) {
        result |= 2;
    }
    
    // With NaN (should be false)
    if ((nan < a) || (nan > a)) {
        result |= 4;
    }
    
    // Alternative: a != b (but not unordered)
    if ((a != b) && !isunordered(a, b)) {
        result |= 8;
    }
    
    // Inline assembly
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l[ltgt_label]\n\t"
        : : "i"(LTGT) : "cc", "st", "st(1)" : ltgt_label
    );
    
    return result;
    
ltgt_label:
    result |= 16;
    return result;
}

// Test with different types
__attribute__((noinline))
static int test_mixed_types(void) {
    volatile float f1 = 1.5f;
    volatile float f2 = 2.5f;
    volatile double d1 = 1.5;
    volatile double d2 = 2.5;
    volatile long double ld1 = 1.5L;
    volatile long double ld2 = 2.5L;
    int result = 0;
    
    // Float comparisons
    if (!(f1 < f2)) result |= 1;    // UNGE
    if (!(f1 > f2)) result |= 2;    // UNLE
    
    // Double comparisons
    if ((d1 < d2) || (d1 > d2)) result |= 4;  // LTGT
    
    // Long double comparisons
    if (!(ld1 <= ld2)) result |= 8;  // UNGT
    
    return result;
}

// Test with memory operands
__attribute__((noinline))
static int test_memory_operands(void) {
    volatile double arr[4] = {1.0, 2.0, 3.0, make_nan()};
    volatile struct {
        double x;
        double y;
    } point = {1.5, 2.5};
    
    int result = 0;
    
    // Array element comparisons
    if (!(arr[0] < arr[1])) result |= 1;      // UNGE
    if ((arr[1] < arr[2]) || (arr[1] > arr[2])) result |= 2;  // LTGT
    
    // Struct member comparisons
    if (!(point.x > point.y)) result |= 4;    // UNLE
    
    // NaN from array
    if (isunordered(arr[3], arr[0])) result |= 8;
    
    return result;
}

// Complex nested expressions
__attribute__((noinline))
static int test_complex_expressions(void) {
    volatile double x = 1.0;
    volatile double y = 2.0;
    volatile double z = make_nan();
    int result = 0;
    
    // Nested ternary with comparisons
    result = (x != x) ? 1 : ((y > x) ? 2 : 3);
    
    // Complex logical expression
    if ((!(x < y) && !(z == z)) || (isunordered(z, x) && !(y <= x))) {
        result += 10;
    }
    
    // Multiple condition codes combined
    int r1 = !(x > y) ? 100 : 200;      // UNLE
    int r2 = (x < y) || (x > y) ? 300 : 400;  // LTGT
    result += r1 + r2;
    
    return result;
}

int main(void) {
    // Initialize volatile seed
    volatile int seed = 42;
    
    // Call all test functions
    checksum += test_unordered();
    sink(checksum);
    
    checksum += test_ordered();
    sink(checksum);
    
    checksum += test_uneq();
    sink(checksum);
    
    checksum += test_unge();
    sink(checksum);
    
    checksum += test_ungt();
    sink(checksum);
    
    checksum += test_unle();
    sink(checksum);
    
    checksum += test_unlt();
    sink(checksum);
    
    checksum += test_ltgt();
    sink(checksum);
    
    checksum += test_mixed_types();
    sink(checksum);
    
    checksum += test_memory_operands();
    sink(checksum);
    
    checksum += test_complex_expressions();
    sink(checksum);
    
    // Print final checksum
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
