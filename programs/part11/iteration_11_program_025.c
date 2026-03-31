#include <stdio.h>
#include <math.h>
#include <stdint.h>

// Prevent optimization
extern void sink(int);
volatile int checksum = 0;

// Function to generate NaN
double make_nan() {
    volatile double zero = 0.0;
    return zero / zero;
}

// Function to generate infinity
double make_inf() {
    volatile double large = 1e308;
    return large * large;
}

// UNORDERED pattern
__attribute__((noinline))
int test_unordered(volatile double a, volatile double b) {
    int result = 0;
    
    // Direct NaN comparison for unordered
    if (a != a) {
        result |= 1;
    }
    
    // Using isunordered()
    if (isunordered(a, b)) {
        result |= 2;
    }
    
    // Complex expression with unordered
    result |= (isunordered(a, b) && (a < b)) ? 4 : 0;
    
    // Inline assembly to force condition code use
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l0"
        : : : "st", "st(1)" : unordered_label
    );
    
    result |= 8;
    goto end;
    
unordered_label:
    result |= 16;
    
end:
    sink(result);
    return result;
}

// ORDERED pattern
__attribute__((noinline))
int test_ordered(volatile double a, volatile double b) {
    int result = 0;
    
    // Direct NaN comparison for ordered
    if (a == a) {
        result |= 1;
    }
    
    // Using isordered()
    if (isordered(a, b)) {
        result |= 2;
    }
    
    // Complex expression with ordered
    result |= (isordered(a, b) || (a > b)) ? 4 : 0;
    
    // Inline assembly to force condition code use
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l0"
        : : : "st", "st(1)" : ordered_label
    );
    
    result |= 8;
    goto end;
    
ordered_label:
    result |= 16;
    
end:
    sink(result);
    return result;
}

// UNEQ pattern (unordered or equal)
__attribute__((noinline))
int test_uneq(volatile double a, volatile double b) {
    int result = 0;
    
    // Generate UNEQ: !(a < b) && !(a > b) which includes unordered
    if (!(a < b) && !(a > b)) {
        result |= 1;
    }
    
    // Alternative: a == b || isunordered(a, b)
    if (a == b || isunordered(a, b)) {
        result |= 2;
    }
    
    // Complex ternary expression
    result |= (!(a < b) && !(a > b)) ? 4 : 8;
    
    // Inline assembly
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l0"
        : : : "st", "st(1)" : uneq_label
    );
    
    result |= 16;
    goto end;
    
uneq_label:
    result |= 32;
    
end:
    sink(result);
    return result;
}

// UNGE pattern (not less than)
__attribute__((noinline))
int test_unge(volatile double a, volatile double b) {
    int result = 0;
    
    // UNGE: !(a < b)
    if (!(a < b)) {
        result |= 1;
    }
    
    // Alternative: a >= b || isunordered(a, b)
    if (a >= b || isunordered(a, b)) {
        result |= 2;
    }
    
    // Nested expression
    result |= (!(a < b) && (a != b)) ? 4 : 8;
    
    // Inline assembly
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l0"
        : : : "st", "st(1)" : unge_label
    );
    
    result |= 16;
    goto end;
    
unge_label:
    result |= 32;
    
end:
    sink(result);
    return result;
}

// UNGT pattern (not less than or equal)
__attribute__((noinline))
int test_ungt(volatile double a, volatile double b) {
    int result = 0;
    
    // UNGT: !(a <= b)
    if (!(a <= b)) {
        result |= 1;
    }
    
    // Alternative: a > b || isunordered(a, b)
    if (a > b || isunordered(a, b)) {
        result |= 2;
    }
    
    // Complex expression
    result |= (!(a <= b) || (a != a)) ? 4 : 8;
    
    // Inline assembly
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l0"
        : : : "st", "st(1)" : ungt_label
    );
    
    result |= 16;
    goto end;
    
ungt_label:
    result |= 32;
    
end:
    sink(result);
    return result;
}

// UNLE pattern (unordered or less than or equal)
__attribute__((noinline))
int test_unle(volatile double a, volatile double b) {
    int result = 0;
    
    // UNLE: !(a > b)
    if (!(a > b)) {
        result |= 1;
    }
    
    // Alternative: a <= b || isunordered(a, b)
    if (a <= b || isunordered(a, b)) {
        result |= 2;
    }
    
    // Nested in ternary
    result |= (!(a > b) ? 4 : 8);
    
    // Inline assembly
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l0"
        : : : "st", "st(1)" : unle_label
    );
    
    result |= 16;
    goto end;
    
unle_label:
    result |= 32;
    
end:
    sink(result);
    return result;
}

// UNLT pattern (unordered or less than)
__attribute__((noinline))
int test_unlt(volatile double a, volatile double b) {
    int result = 0;
    
    // UNLT: !(a >= b)
    if (!(a >= b)) {
        result |= 1;
    }
    
    // Alternative: a < b || isunordered(a, b)
    if (a < b || isunordered(a, b)) {
        result |= 2;
    }
    
    // Complex logical expression
    result |= (!(a >= b) && (b == b)) ? 4 : 8;
    
    // Inline assembly
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l0"
        : : : "st", "st(1)" : unlt_label
    );
    
    result |= 16;
    goto end;
    
unlt_label:
    result |= 32;
    
end:
    sink(result);
    return result;
}

// LTGT pattern (less than or greater than - ordered and not equal)
__attribute__((noinline))
int test_ltgt(volatile double a, volatile double b) {
    int result = 0;
    
    // LTGT: (a < b) || (a > b)
    if ((a < b) || (a > b)) {
        result |= 1;
    }
    
    // Alternative: !(a == b) && isordered(a, b)
    if (!(a == b) && isordered(a, b)) {
        result |= 2;
    }
    
    // Complex expression
    result |= ((a < b) || (a > b)) ? 4 : 8;
    
    // Inline assembly
    __asm__ goto (
        "fucomip %%st(1), %%st\n\t"
        "j%c0 %l0"
        : : : "st", "st(1)" : ltgt_label
    );
    
    result |= 16;
    goto end;
    
ltgt_label:
    result |= 32;
    
end:
    sink(result);
    return result;
}

// Test with different types
__attribute__((noinline))
int test_mixed_types() {
    int result = 0;
    
    volatile float f1 = 1.0f;
    volatile float f2 = make_nan();
    volatile double d1 = 2.0;
    volatile double d2 = make_inf();
    volatile long double ld1 = 3.0L;
    volatile long double ld2 = make_nan();
    
    // Float comparisons
    if (isunordered(f1, f2)) result |= 1;
    if (!(f1 < f2)) result |= 2;
    
    // Double comparisons
    if (!(d1 > d2)) result |= 4;
    if ((d1 < d2) || (d1 > d2)) result |= 8;
    
    // Long double comparisons
    if (isordered(ld1, ld2)) result |= 16;
    if (!(ld1 >= ld2)) result |= 32;
    
    sink(result);
    return result;
}

// Test with memory operands
__attribute__((noinline))
int test_memory_operands() {
    int result = 0;
    
    volatile double arr[4] = {1.0, make_nan(), 3.0, make_inf()};
    volatile struct {
        double x;
        double y;
    } point = {make_nan(), 2.0};
    
    // Array element comparisons
    if (isunordered(arr[0], arr[1])) result |= 1;
    if (!(arr[2] < arr[3])) result |= 2;
    
    // Struct member comparisons
    if (!(point.x > point.y)) result |= 4;
    if ((point.x < point.y) || (point.x > point.y)) result |= 8;
    
    // Complex addressing
    volatile double *ptr = &arr[1];
    if (isordered(*ptr, *(ptr + 1))) result |= 16;
    
    sink(result);
    return result;
}

int main() {
    // Initialize with various floating-point values
    volatile double nan_val = make_nan();
    volatile double inf_val = make_inf();
    volatile double normal1 = 1.5;
    volatile double normal2 = 2.5;
    volatile double zero = 0.0;
    
    // Call all test functions
    checksum += test_unordered(nan_val, normal1);
    checksum += test_ordered(normal1, normal2);
    checksum += test_uneq(nan_val, nan_val);
    checksum += test_uneq(normal1, normal1);
    checksum += test_unge(nan_val, normal1);
    checksum += test_unge(normal2, normal1);
    checksum += test_ungt(nan_val, normal1);
    checksum += test_ungt(normal2, normal1);
    checksum += test_unle(nan_val, normal1);
    checksum += test_unle(normal1, normal2);
    checksum += test_unlt(nan_val, normal1);
    checksum += test_unlt(normal1, normal2);
    checksum += test_ltgt(normal1, normal2);
    checksum += test_ltgt(nan_val, normal1);
    
    // Additional tests
    checksum += test_mixed_types();
    checksum += test_memory_operands();
    
    // More complex scenarios
    volatile double a = nan_val;
    volatile double b = normal1;
    volatile double c = normal2;
    
    // Nested comparisons in complex expressions
    int complex_result = (a != a) ? 1 : ((b < c) ? 2 : 3);
    complex_result += (!(b > c) && (c == c)) ? 4 : 8;
    complex_result += ((b < c) || (b > c)) ? 16 : 32;
    
    checksum += complex_result;
    
    // Print final checksum
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
