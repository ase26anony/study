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

// UNORDERED condition (unord)
__attribute__((noinline))
static int test_unordered(void) {
    volatile double nan1 = make_nan();
    volatile double nan2 = make_nan();
    volatile double normal = 3.14;
    int result = 0;
    
    // Direct unordered comparison
    if (isunordered(nan1, normal)) {
        result |= 1;
    }
    
    // Self-comparison for unordered
    if (nan1 != nan1) {
        result |= 2;
    }
    
    // Inline assembly to force condition code
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l0"
        : : : "st", "st(1)" : unordered_label
    );
    
    result |= 4;
    goto end;
    
unordered_label:
    result |= 8;
    
end:
    sink(result);
    return result;
}

// ORDERED condition (ord)
__attribute__((noinline))
static int test_ordered(void) {
    volatile double a = 1.5;
    volatile double b = 2.5;
    volatile double nan = make_nan();
    int result = 0;
    
    // Direct ordered comparison
    if (isordered(a, b)) {
        result |= 1;
    }
    
    // Self-comparison for ordered
    if (a == a && b == b) {
        result |= 2;
    }
    
    // Inline assembly to force condition code
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l0"
        : : : "st", "st(1)" : ordered_label
    );
    
    result |= 4;
    goto end2;
    
ordered_label:
    result |= 8;
    
end2:
    sink(result);
    return result;
}

// UNEQ condition (ueq) - unordered or equal
__attribute__((noinline))
static int test_uneq(void) {
    volatile double a = 2.0;
    volatile double b = 2.0;
    volatile double nan = make_nan();
    int result = 0;
    
    // Generate UNEQ: !(a != b) which is a == b or unordered
    if (!(a != b)) {
        result |= 1;
    }
    
    // Using ternary with unordered
    result |= (isunordered(nan, a) || nan == a) ? 2 : 0;
    
    // Complex expression
    volatile double arr[3] = {1.0, 2.0, make_nan()};
    if (!(arr[0] != arr[1]) || !(arr[2] != arr[2])) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

// UNGE condition (nlt) - not less than (greater or equal or unordered)
__attribute__((noinline))
static int test_unge(void) {
    volatile double a = 3.0;
    volatile double b = 2.0;
    volatile double nan = make_nan();
    int result = 0;
    
    // Generate UNGE: !(a < b)
    if (!(a < b)) {
        result |= 1;
    }
    
    // With NaN
    if (!(nan < b)) {
        result |= 2;
    }
    
    // Using long double
    volatile long double ld1 = 5.0L;
    volatile long double ld2 = 3.0L;
    if (!(ld1 < ld2)) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

// UNGT condition (nle) - not less or equal (greater or unordered)
__attribute__((noinline))
static int test_ungt(void) {
    volatile double a = 5.0;
    volatile double b = 3.0;
    volatile double nan = make_nan();
    int result = 0;
    
    // Generate UNGT: !(a <= b)
    if (!(a <= b)) {
        result |= 1;
    }
    
    // Complex expression with memory operand
    volatile struct {
        double x;
        double y;
    } point = {4.0, 2.0};
    
    if (!(point.x <= point.y)) {
        result |= 2;
    }
    
    // Float type
    volatile float f1 = 10.0f;
    volatile float f2 = 5.0f;
    if (!(f1 <= f2)) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

// UNLE condition (ule) - unordered or less or equal
__attribute__((noinline))
static int test_unle(void) {
    volatile double a = 2.0;
    volatile double b = 3.0;
    volatile double nan = make_nan();
    int result = 0;
    
    // Generate UNLE: !(a > b)
    if (!(a > b)) {
        result |= 1;
    }
    
    // With NaN operand
    if (!(nan > b)) {
        result |= 2;
    }
    
    // Nested in ternary
    result |= (!(a > b)) ? 4 : 0;
    
    sink(result);
    return result;
}

// UNLT condition (ult) - unordered or less than
__attribute__((noinline))
static int test_unlt(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double nan = make_nan();
    int result = 0;
    
    // Generate UNLT: !(a >= b)
    if (!(a >= b)) {
        result |= 1;
    }
    
    // Complex logical expression
    if (!(a >= b) && (a < b || isunordered(a, b))) {
        result |= 2;
    }
    
    // Array access
    volatile double arr[2] = {1.5, 2.5};
    if (!(arr[0] >= arr[1])) {
        result |= 4;
    }
    
    sink(result);
    return result;
}

// LTGT condition (une) - not equal and ordered
__attribute__((noinline))
static int test_ltgt(void) {
    volatile double a = 2.0;
    volatile double b = 3.0;
    volatile double c = 2.0;
    int result = 0;
    
    // Generate LTGT: (a < b) || (a > b)
    if ((a < b) || (a > b)) {
        result |= 1;
    }
    
    // Equal values should not trigger
    if (!((c < a) || (c > a))) {
        result |= 2;
    }
    
    // With ordered check
    if ((a < b || a > b) && isordered(a, b)) {
        result |= 4;
    }
    
    // Inline assembly forcing condition code
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l0"
        : : : "st", "st(1)" : ltgt_label
    );
    
    result |= 8;
    goto end3;
    
ltgt_label:
    result |= 16;
    
end3:
    sink(result);
    return result;
}

// Combined test with complex expressions
__attribute__((noinline))
static int test_combined(void) {
    volatile double nan = make_nan();
    volatile double inf = make_inf();
    volatile double normal = 42.0;
    int result = 0;
    
    // Complex nested expression
    result = (nan != nan) ? 
             ((normal > 0) ? 1 : 2) :
             ((!(normal < 10)) ? 4 : 8);
    
    // Multiple comparisons
    if ((!(normal >= 50)) && ((normal < 30) || (normal > 30))) {
        result |= 16;
    }
    
    // Memory operands in struct
    volatile struct {
        float f;
        double d;
        long double ld;
    } data = {3.14f, 2.718, 1.414L};
    
    if (!(data.f >= 4.0f) && !(data.d <= 2.0)) {
        result |= 32;
    }
    
    sink(result);
    return result;
}

int main(void) {
    // Initialize volatile seed
    volatile int seed = 0x12345678;
    
    // Call all test functions
    checksum ^= test_unordered();
    checksum ^= test_ordered();
    checksum ^= test_uneq();
    checksum ^= test_unge();
    checksum ^= test_ungt();
    checksum ^= test_unle();
    checksum ^= test_unlt();
    checksum ^= test_ltgt();
    checksum ^= test_combined();
    
    // Print checksum to ensure all code is live
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
