#include <stdio.h>
#include <math.h>
#include <stdint.h>

// Prevent optimization
extern void sink(int);
volatile int checksum = 0;

// Force compiler to materialize condition codes
#define FORCE_CC(cond) \
    __asm__ goto ("j%c0 %l0" : : "i" (cond) : : target); \
    target: checksum += __LINE__;

// Pattern 1: UNORDERED comparisons
__attribute__((noinline))
void test_unordered(void) {
    volatile double nan_val = 0.0 / 0.0;
    volatile double inf_val = 1.0 / 0.0;
    volatile double normal = 3.14;
    
    // Direct NaN comparisons that should generate UNORDERED
    if (nan_val != nan_val) {
        checksum += 1;
    }
    
    // Using isunordered()
    if (isunordered(nan_val, normal)) {
        checksum += 2;
    }
    
    // Complex expression with unordered
    volatile float f_nan = 0.0f / 0.0f;
    int result = (f_nan != f_nan) ? 100 : 200;
    sink(result);
    
    // Memory operand with unordered
    volatile double arr[3] = {nan_val, inf_val, normal};
    if (isunordered(arr[0], arr[2])) {
        checksum += 4;
    }
}

// Pattern 2: ORDERED comparisons
__attribute__((noinline))
void test_ordered(void) {
    volatile double nan_val = 0.0 / 0.0;
    volatile double val1 = 1.5;
    volatile double val2 = 2.5;
    
    // Direct ordered comparison
    if (val1 == val1) {  // Should be true for non-NaN
        checksum += 8;
    }
    
    // Using isordered()
    if (isordered(val1, val2)) {
        checksum += 16;
    }
    
    // Complex ordered expression
    long double ld1 = 3.14159L;
    long double ld2 = 2.71828L;
    if (isordered(ld1, ld2) && (ld1 > ld2)) {
        checksum += 32;
    }
    
    // Memory operand
    volatile struct {
        double a;
        double b;
    } s = {val1, nan_val};
    
    if (isordered(s.a, s.b)) {
        checksum += 64;  // Should not execute
    }
}

// Pattern 3: UNEQ (unordered or equal)
__attribute__((noinline))
void test_uneq(void) {
    volatile double a = 1.0;
    volatile double b = 1.0;
    volatile double nan = 0.0 / 0.0;
    
    // Should generate UNEQ: !(a < b) && !(a > b)
    if (!(a < b) && !(a > b)) {
        checksum += 128;
    }
    
    // With NaN operand (should also be true for unordered)
    if (!(nan < b) && !(nan > b)) {
        checksum += 256;
    }
    
    // Using inline assembly to force condition code
    __asm__ goto ("j%c0 %l0" : : "i" (UNORDERED) : : label1);
    label1: checksum += 512;
    
    // Complex ternary
    volatile float fa = 2.0f, fb = 2.0f;
    int r = (!(fa < fb) && !(fa > fb)) ? 1 : 0;
    sink(r);
}

// Pattern 4: UNGE (not less than)
__attribute__((noinline))
void test_unge(void) {
    volatile double x = 3.0;
    volatile double y = 2.0;
    volatile double z = 3.0;
    volatile double nan = 0.0 / 0.0;
    
    // UNGE: !(x < y)  (x >= y or unordered)
    if (!(x < y)) {
        checksum += 1024;
    }
    
    // Equal case
    if (!(z < x)) {
        checksum += 2048;
    }
    
    // With NaN
    if (!(nan < y)) {
        checksum += 4096;
    }
    
    // In expression
    volatile double arr[2] = {x, y};
    int result = !(arr[0] < arr[1]) ? 10 : 20;
    sink(result);
}

// Pattern 5: UNGT (not less than or equal)
__attribute__((noinline))
void test_ungt(void) {
    volatile double p = 5.0;
    volatile double q = 4.0;
    volatile double r = 5.0;
    
    // UNGT: !(p <= q)  (p > q or unordered)
    if (!(p <= q)) {
        checksum += 8192;
    }
    
    // Equal case should be false
    if (!(r <= p)) {
        checksum += 16384;  // Should not execute
    }
    
    // Complex with memory
    volatile struct {
        double m;
        double n;
    } data = {p, q};
    
    if (!(data.m <= data.n)) {
        checksum += 32768;
    }
}

// Pattern 6: UNLE (unordered or less than or equal)
__attribute__((noinline))
void test_unle(void) {
    volatile double u = 2.0;
    volatile double v = 3.0;
    volatile double w = 3.0;
    
    // UNLE: !(u > v)  (u <= v or unordered)
    if (!(u > v)) {
        checksum += 65536;
    }
    
    // Equal case
    if (!(w > v)) {
        checksum += 131072;
    }
    
    // With long double
    volatile long double ld_u = 2.0L;
    volatile long double ld_v = 3.0L;
    if (!(ld_u > ld_v)) {
        checksum += 262144;
    }
}

// Pattern 7: UNLT (unordered or less than)
__attribute__((noinline))
void test_unlt(void) {
    volatile double alpha = 1.0;
    volatile double beta = 2.0;
    volatile double gamma = 2.0;
    
    // UNLT: !(alpha >= beta)  (alpha < beta or unordered)
    if (!(alpha >= beta)) {
        checksum += 524288;
    }
    
    // Equal case should be false
    if (!(gamma >= beta)) {
        checksum += 1048576;  // Should not execute
    }
    
    // In complex expression
    volatile float f_alpha = 1.0f;
    volatile float f_beta = 2.0f;
    int res = (!(f_alpha >= f_beta)) ? 30 : 40;
    sink(res);
}

// Pattern 8: LTGT (less than or greater than - ordered and not equal)
__attribute__((noinline))
void test_ltgt(void) {
    volatile double m = 1.0;
    volatile double n = 2.0;
    volatile double o = 2.0;
    volatile double nan = 0.0 / 0.0;
    
    // LTGT: (m < n) || (m > n)  (ordered and not equal)
    if ((m < n) || (m > n)) {
        checksum += 2097152;
    }
    
    // Equal case should be false
    if ((n < o) || (n > o)) {
        checksum += 4194304;  // Should not execute
    }
    
    // With NaN (should be false)
    if ((nan < n) || (nan > n)) {
        checksum += 8388608;  // Should not execute
    }
    
    // Complex with memory operands
    volatile double mem[4] = {1.0, 2.0, 3.0, 4.0};
    if ((mem[0] < mem[1]) || (mem[0] > mem[1])) {
        checksum += 16777216;
    }
}

// Pattern 9: Mixed condition codes in complex expressions
__attribute__((noinline))
void test_mixed(void) {
    volatile double a = 0.0 / 0.0;  // NaN
    volatile double b = 1.0;
    volatile double c = 2.0;
    volatile double d = 2.0;
    
    // Complex nested conditionals
    int result = 0;
    
    if (isunordered(a, b)) {
        result += 1;  // UNORDERED
    }
    
    if (!(c < d) && !(c > d)) {
        result += 2;  // UNEQ
    }
    
    if (!(b >= c)) {
        result += 4;  // UNLT
    }
    
    if ((b < c) || (b > c)) {
        result += 8;  // LTGT
    }
    
    // Use inline assembly with specific condition codes
    __asm__ goto ("j%c0 %l0" : : "i" (ORDERED) : : mixed_label);
    mixed_label: result += 16;
    
    sink(result);
    checksum += result;
}

// Pattern 10: Force condition codes with __asm__ goto
__attribute__((noinline))
void test_asm_goto(void) {
    volatile double x = 1.0;
    volatile double y = 2.0;
    volatile double nan = 0.0 / 0.0;
    
    // Force UNORDERED
    if (isunordered(nan, x)) {
        __asm__ goto ("j%c0 %l0" : : "i" (UNORDERED) : : asm_label1);
    }
    asm_label1: checksum += 33554432;
    
    // Force UNGE
    if (!(y < x)) {
        __asm__ goto ("j%c0 %l0" : : "i" (UNGE) : : asm_label2);
    }
    asm_label2: checksum += 67108864;
    
    // Force LTGT
    if ((x < y) || (x > y)) {
        __asm__ goto ("j%c0 %l0" : : "i" (LTGT) : : asm_label3);
    }
    asm_label3: checksum += 134217728;
}

int main(void) {
    // Initialize checksum
    checksum = 0;
    
    // Call all test functions
    test_unordered();
    test_ordered();
    test_uneq();
    test_unge();
    test_ungt();
    test_unle();
    test_unlt();
    test_ltgt();
    test_mixed();
    test_asm_goto();
    
    // Print final checksum to ensure all code is live
    printf("Checksum: %d\n", checksum);
    
    return 0;
}
