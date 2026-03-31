#include <stdio.h>
#include <math.h>
#include <stdint.h>

// Prevent optimization
extern void sink(int);
volatile int checksum = 0;

// Function to generate NaN
static inline double make_nan() {
    volatile double zero = 0.0;
    return zero / zero;
}

// Function to generate infinity
static inline double make_inf() {
    volatile double large = 1e308;
    return large * large;
}

// Pattern 1: UNORDERED comparisons
__attribute__((noinline))
void test_unordered_ordered() {
    volatile double nan1 = make_nan();
    volatile double nan2 = make_nan();
    volatile double normal = 3.14159;
    volatile double inf = make_inf();
    
    int result = 0;
    
    // UNORDERED patterns
    if (isunordered(nan1, normal)) {
        result |= 1;
    }
    
    if (nan1 != nan1) {  // Should be true for NaN
        result |= 2;
    }
    
    // ORDERED patterns
    if (isordered(normal, inf)) {
        result |= 4;
    }
    
    if (normal == normal) {  // Should be true for non-NaN
        result |= 8;
    }
    
    // Complex expression with unordered
    result |= (isunordered(nan1, nan2) ? 16 : 0);
    
    sink(result);
    checksum += result;
}

// Pattern 2: UNEQ (unordered or equal)
__attribute__((noinline))
void test_uneq() {
    volatile double a = 5.0;
    volatile double b = 5.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    // Direct unordered or equal comparisons
    if (!(a < b) && !(a > b)) {  // a == b or unordered
        result |= 1;
    }
    
    // Using ternary with NaN
    result |= (nan == nan ? 2 : 0);  // false for NaN
    
    // Complex expression
    volatile double arr[3] = {1.0, 2.0, make_nan()};
    if (!(arr[0] < arr[1]) && !(arr[0] > arr[1])) {
        result |= 4;
    }
    
    sink(result);
    checksum += result;
}

// Pattern 3: UNGE (not less than)
__attribute__((noinline))
void test_unge() {
    volatile double x = 10.0;
    volatile double y = 5.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    // !(x < y) -> x >= y or unordered
    if (!(x < y)) {
        result |= 1;
    }
    
    // With NaN
    if (!(nan < y)) {
        result |= 2;
    }
    
    // Nested in larger expression
    result |= (!(x < y) ? 4 : 0) | (!(y < x) ? 8 : 0);
    
    sink(result);
    checksum += result;
}

// Pattern 4: UNGT (not less than or equal)
__attribute__((noinline))
void test_ungt() {
    volatile double a = 7.0;
    volatile double b = 3.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    // !(a <= b) -> a > b or unordered
    if (!(a <= b)) {
        result |= 1;
    }
    
    // Using memory operand
    volatile struct {
        double f1;
        double f2;
    } s = {15.0, 10.0};
    
    if (!(s.f1 <= s.f2)) {
        result |= 2;
    }
    
    // With NaN
    result |= (!(nan <= b) ? 4 : 0);
    
    sink(result);
    checksum += result;
}

// Pattern 5: UNLE (unordered or less than or equal)
__attribute__((noinline))
void test_unle() {
    volatile double p = 2.0;
    volatile double q = 8.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    // !(p > q) -> p <= q or unordered
    if (!(p > q)) {
        result |= 1;
    }
    
    // Complex expression
    result |= (!(q > p) ? 2 : 0) | (!(nan > p) ? 4 : 0);
    
    sink(result);
    checksum += result;
}

// Pattern 6: UNLT (unordered or less than)
__attribute__((noinline))
void test_unlt() {
    volatile double m = 1.0;
    volatile double n = 9.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    // !(m >= n) -> m < n or unordered
    if (!(m >= n)) {
        result |= 1;
    }
    
    // Different types
    volatile float f1 = 1.5f;
    volatile float f2 = 2.5f;
    if (!(f1 >= f2)) {
        result |= 2;
    }
    
    // Long double
    volatile long double ld1 = 3.14159L;
    volatile long double ld2 = 2.71828L;
    if (!(ld1 >= ld2)) {
        result |= 4;
    }
    
    sink(result);
    checksum += result;
}

// Pattern 7: LTGT (not equal and ordered)
__attribute__((noinline))
void test_ltgt() {
    volatile double u = 4.0;
    volatile double v = 4.0;
    volatile double w = 6.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    // (u < w) || (u > w) -> not equal and ordered when u != w
    if ((u < w) || (u > w)) {
        result |= 1;
    }
    
    // With equal values (should be false)
    if ((u < v) || (u > v)) {
        result |= 2;  // Should not be taken
    }
    
    // Complex nested expression
    result |= (((u < w) || (u > w)) ? 4 : 0);
    
    sink(result);
    checksum += result;
}

// Pattern 8: Mixed condition codes in inline assembly
__attribute__((noinline))
void test_asm_condition_codes() {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    // Use __asm__ goto to force condition code usage
    // UNORDERED
    if (isunordered(a, nan)) {
        asm goto ("j%c0 %l0" : : "i" (0) : : label1);
        // Not taken
        result |= 1;
        label1:;
    }
    
    // ORDERED
    if (isordered(a, b)) {
        asm goto ("j%c0 %l0" : : "i" (0) : : label2);
        // Not taken
        result |= 2;
        label2:;
    }
    
    // UNGE
    if (!(a < b)) {
        asm goto ("j%c0 %l0" : : "i" (0) : : label3);
        // Not taken
        result |= 4;
        label3:;
    }
    
    sink(result);
    checksum += result;
}

// Pattern 9: Complex nested comparisons
__attribute__((noinline))
void test_complex_nested() {
    volatile double x = 1.0;
    volatile double y = 2.0;
    volatile double z = 3.0;
    volatile double nan = make_nan();
    
    int result = 0;
    
    // Complex expression mixing multiple condition codes
    result = (x != x) ? 1 : 
             ((y > z) ? 2 : 
             ((!(x < y)) ? 3 : 
             ((isunordered(z, nan)) ? 4 : 5)));
    
    // Another complex expression
    volatile double arr[4] = {1.0, make_nan(), 3.0, 4.0};
    if ((!(arr[0] >= arr[1])) && ((arr[2] < arr[3]) || (arr[2] > arr[3]))) {
        result |= 8;
    }
    
    sink(result);
    checksum += result;
}

// Pattern 10: Memory operands with different addressing modes
__attribute__((noinline))
void test_memory_operands() {
    volatile struct Point {
        double x;
        double y;
    } points[3] = {{1.0, 2.0}, {make_nan(), 4.0}, {5.0, 6.0}};
    
    volatile double global_array[5] = {0.0, 1.0, make_nan(), 3.0, 4.0};
    
    int result = 0;
    
    // Array access with index
    if (!(global_array[1] < global_array[4])) {
        result |= 1;
    }
    
    // Struct member access
    if (isunordered(points[0].x, points[1].x)) {
        result |= 2;
    }
    
    // Pointer dereference
    volatile double *ptr = &global_array[2];
    if (!(*ptr == *ptr)) {  // Should be true for NaN
        result |= 4;
    }
    
    // Complex addressing
    if ((points[2].x < points[0].y) || (points[2].x > points[0].y)) {
        result |= 8;
    }
    
    sink(result);
    checksum += result;
}

int main() {
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
    test_asm_condition_codes();
    test_complex_nested();
    test_memory_operands();
    
    // Print checksum to ensure all code is live
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
