#include <stdio.h>
#include <math.h>
#include <stdint.h>

// Prevent optimization
extern void sink(int);
volatile int checksum = 0;

// Force condition code usage with inline assembly
#define USE_CC(cond) \
    __asm__ goto ("j%c0 %l[lbl]" : : "i" (cond) : : lbl); \
    return 0; lbl: return 1

// Pattern 1: UNORDERED/ORDERED with NaN
__attribute__((noinline)) 
int test_unordered_ordered(void) {
    volatile double nan_val = 0.0 / 0.0;
    volatile double normal_val = 3.14;
    volatile double inf_val = 1.0 / 0.0;
    
    int result = 0;
    
    // UNORDERED: NaN != NaN
    if (nan_val != nan_val) {
        result |= 1;
    }
    
    // ORDERED: normal == normal
    if (normal_val == normal_val) {
        result |= 2;
    }
    
    // Mixed unordered/ordered comparisons
    volatile double a = nan_val;
    volatile double b = normal_val;
    
    // This should generate unordered comparison
    if (isunordered(a, b)) {
        result |= 4;
    }
    
    // This should generate ordered comparison  
    if (isordered(b, b)) {
        result |= 8;
    }
    
    // Force condition code usage with inline assembly
    __asm__ goto ("fucomip %%st(1), %%st(0); jp %l[lbl1]" : : : : lbl1);
    result |= 16;
    goto skip1;
lbl1:
    result |= 32;
skip1:
    
    return result;
}

// Pattern 2: UNEQ (unordered or equal)
__attribute__((noinline))
int test_uneq(void) {
    volatile double x = 0.0 / 0.0;  // NaN
    volatile double y = 5.0;
    volatile double z = 5.0;
    
    int result = 0;
    
    // UNEQ: !(a < b) && !(a > b)  (unordered or equal)
    if (!(x < y) && !(x > y)) {  // x is NaN, so both comparisons false
        result |= 1;
    }
    
    if (!(y < z) && !(y > z)) {  // y == z
        result |= 2;
    }
    
    // Using inline assembly to force condition code
    volatile double a = 2.0;
    volatile double b = 2.0;
    __asm__ volatile ("fldl %1\n\t"
                      "fldl %2\n\t"
                      "fucomip %%st(1), %%st(0)\n\t"
                      "fstp %%st(0)\n\t"
                      : : "m" (a), "m" (b) : "st", "st(1)", "cc");
    
    if (USE_CC(uneq)) {
        result |= 4;
    }
    
    return result;
}

// Pattern 3: UNGE (!(a < b) - not less than)
__attribute__((noinline))
int test_unge(void) {
    volatile float f1 = 10.5f;
    volatile float f2 = 5.2f;
    volatile float f3 = 10.5f;
    volatile float f4 = 15.8f;
    
    int result = 0;
    
    // UNGE: !(a < b)  (a >= b or unordered)
    if (!(f1 < f2)) {  // 10.5 < 5.2 is false, so true
        result |= 1;
    }
    
    if (!(f1 < f3)) {  // 10.5 < 10.5 is false, so true
        result |= 2;
    }
    
    if (!(f1 < f4)) {  // 10.5 < 15.8 is true, so false
        // not taken
    } else {
        result |= 4;
    }
    
    // With NaN
    volatile double nan = 0.0 / 0.0;
    volatile double val = 7.0;
    
    if (!(nan < val)) {  // NaN < 7.0 is false, so true (unordered)
        result |= 8;
    }
    
    return result;
}

// Pattern 4: UNGT (!(a <= b) - not less than or equal)
__attribute__((noinline))
int test_ungt(void) {
    volatile double d1 = 8.9;
    volatile double d2 = 3.1;
    volatile double d3 = 8.9;
    
    int result = 0;
    
    // UNGT: !(a <= b)  (a > b or unordered)
    if (!(d1 <= d2)) {  // 8.9 <= 3.1 is false, so true
        result |= 1;
    }
    
    if (!(d1 <= d3)) {  // 8.9 <= 8.9 is true, so false
        // not taken
    } else {
        result |= 2;
    }
    
    // Complex expression with ternary
    result |= (!(d1 <= d2)) ? 4 : 0;
    result |= (!(d2 <= d1)) ? 8 : 0;
    
    // With memory operand from array
    volatile double arr[3] = {1.0, 2.0, 3.0};
    if (!(arr[0] <= arr[2])) {
        // not taken
    } else {
        result |= 16;
    }
    
    return result;
}

// Pattern 5: UNLE (unordered or less than or equal)
__attribute__((noinline))
int test_unle(void) {
    volatile long double ld1 = 3.14159265358979323846L;
    volatile long double ld2 = 2.71828182845904523536L;
    volatile long double ld3 = 3.14159265358979323846L;
    
    int result = 0;
    
    // UNLE: (a <= b) || (a != a) || (b != b)
    if (ld2 <= ld1) {  // 2.7 <= 3.14 is true
        result |= 1;
    }
    
    if (ld1 <= ld3) {  // 3.14 <= 3.14 is true
        result |= 2;
    }
    
    // With NaN using islessequal
    volatile double nan = 0.0 / 0.0;
    if (!isgreater(nan, ld1)) {  // Equivalent to unordered or less than or equal
        result |= 4;
    }
    
    // Complex nested expression
    result |= (ld2 <= ld1) ? 8 : ((ld1 <= ld2) ? 0 : 16);
    
    return result;
}

// Pattern 6: UNLT (unordered or less than)
__attribute__((noinline))
int test_unlt(void) {
    volatile float f[4] = {1.0f, 2.0f, 3.0f, 4.0f};
    volatile float *fp = f;
    
    int result = 0;
    
    // UNLT: (a < b) || (a != a) || (b != b)
    if (fp[0] < fp[2]) {  // 1.0 < 3.0 is true
        result |= 1;
    }
    
    if (fp[1] < fp[1]) {  // 2.0 < 2.0 is false
        // not taken
    } else {
        result |= 2;
    }
    
    // Using !(a >= b) which is equivalent to a < b or unordered
    if (!(fp[0] >= fp[3])) {  // !(1.0 >= 4.0) = true
        result |= 4;
    }
    
    // With struct member
    struct {
        volatile double x;
        volatile double y;
    } s = {5.0, 10.0};
    
    if (s.x < s.y) {
        result |= 8;
    }
    
    return result;
}

// Pattern 7: LTGT (not equal and ordered)
__attribute__((noinline))
int test_ltgt(void) {
    volatile double a = 7.5;
    volatile double b = 3.2;
    volatile double c = 7.5;
    
    int result = 0;
    
    // LTGT: (a < b) || (a > b)  (not equal and ordered)
    if ((a < b) || (a > b)) {  // 7.5 > 3.2 is true
        result |= 1;
    }
    
    if ((a < c) || (a > c)) {  // Both false, so false
        // not taken
    } else {
        result |= 2;
    }
    
    // Equivalent form: !(a == b) && (a == a) && (b == b)
    volatile double nan = 0.0 / 0.0;
    if (!(a == b) && (a == a) && (b == b)) {
        result |= 4;
    }
    
    if (!(nan == a) && (nan == nan) && (a == a)) {  // nan == nan is false
        // not taken
    } else {
        result |= 8;
    }
    
    // Force with inline assembly
    __asm__ volatile ("fldl %1\n\t"
                      "fldl %2\n\t"
                      "fucomip %%st(1), %%st(0)\n\t"
                      "fstp %%st(0)\n\t"
                      : : "m" (a), "m" (b) : "st", "st(1)", "cc");
    
    if (USE_CC(ltgt)) {
        result |= 16;
    }
    
    return result;
}

// Pattern 8: Mixed condition codes in complex expression
__attribute__((noinline))
int test_mixed(void) {
    volatile double x = 0.0 / 0.0;  // NaN
    volatile double y = 2.5;
    volatile double z = 2.5;
    volatile double w = 5.0;
    
    int result = 0;
    
    // Complex expression combining multiple condition codes
    result = (x != x) ? 1 : 
             ((y == z) ? 2 :
             ((!(y < w)) ? 4 :
             ((!(w <= y)) ? 8 :
             ((y <= z) ? 16 :
             ((y < w) || (y > w) ? 32 : 64)))));
    
    // Nested with logical operators
    if ((isunordered(x, y) || (y == z)) && !(w < y)) {
        result |= 128;
    }
    
    // Memory operands from different locations
    volatile struct {
        double d1;
        double d2;
    } data = {1.0, 2.0};
    
    volatile double arr[2] = {3.0, 4.0};
    
    if (!(data.d1 < arr[0]) && (data.d2 <= arr[1])) {
        result |= 256;
    }
    
    return result;
}

int main(void) {
    // Initialize volatile seed
    volatile int seed = 42;
    
    // Call all pattern functions
    checksum += test_unordered_ordered();
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
    
    checksum += test_mixed();
    sink(checksum);
    
    // Print final checksum to ensure all code is live
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
