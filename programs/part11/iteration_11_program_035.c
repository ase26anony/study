#include <stdio.h>
#include <math.h>
#include <stdint.h>

// Prevent optimization
extern void sink(int);
volatile int checksum = 0;

// Force condition code usage via inline assembly
#define BRANCH_IF(cond) \
    __asm__ goto ("j%c0 %l[lbl]" : : "i" (cond) : : lbl); \
    return 0; lbl: return 1

// Pattern 1: UNORDERED comparisons
__attribute__((noinline)) 
int test_unordered(void) {
    volatile double nan = 0.0 / 0.0;
    volatile double inf = 1.0 / 0.0;
    volatile double normal = 3.14;
    
    int result = 0;
    
    // Direct unordered check
    if (nan != nan) {
        result |= 1;  // unordered
    }
    
    // Using isunordered()
    if (isunordered(nan, normal)) {
        result |= 2;
    }
    
    // Complex expression with unordered
    result |= (isunordered(nan, inf) && !isunordered(normal, normal)) ? 4 : 0;
    
    // Inline assembly to force condition code
    if (nan == nan) {  // This will be false for NaN
        __asm__ goto ("j%c0 %l[lbl1]" : : "i" (UNORDERED) : : lbl1);
        result |= 8;
        lbl1: ;
    }
    
    sink(result);
    checksum += result;
    return result;
}

// Pattern 2: ORDERED comparisons
__attribute__((noinline))
int test_ordered(void) {
    volatile float f1 = 1.0f;
    volatile float f2 = 2.0f;
    volatile float fnan = 0.0f / 0.0f;
    
    int result = 0;
    
    // Direct ordered check
    if (f1 == f1) {  // Always true for non-NaN
        result |= 1;
    }
    
    // Using isordered()
    if (isordered(f1, f2)) {
        result |= 2;
    }
    
    // Complex ordered expression
    result |= (isordered(f1, f2) || !isordered(fnan, f1)) ? 4 : 0;
    
    // Force ORDERED condition code with inline assembly
    if (f1 < f2) {
        __asm__ goto ("j%c0 %l[lbl2]" : : "i" (ORDERED) : : lbl2);
        result |= 8;
        lbl2: ;
    }
    
    sink(result);
    checksum += result;
    return result;
}

// Pattern 3: UNEQ (unordered or equal)
__attribute__((noinline))
int test_uneq(void) {
    volatile double a = 5.0;
    volatile double b = 5.0;
    volatile double c = 0.0 / 0.0;
    
    int result = 0;
    
    // Generate UNEQ: !(a > b) && !(a < b) which covers equal or unordered
    if (!(a > b) && !(a < b)) {
        result |= 1;
    }
    
    // Another UNEQ pattern
    if (a == b || a != a) {
        result |= 2;
    }
    
    // Using volatile array to force memory operands
    volatile double arr[3] = {1.0, 2.0, 0.0/0.0};
    if (!(arr[0] > arr[1]) && !(arr[0] < arr[1])) {
        result |= 4;
    }
    
    sink(result);
    checksum += result;
    return result;
}

// Pattern 4: UNGE (not less than) = !(a < b)
__attribute__((noinline))
int test_unge(void) {
    volatile float x = 3.0f;
    volatile float y = 2.0f;
    volatile float z = 0.0f / 0.0f;
    
    int result = 0;
    
    // Direct UNGE: !(x < y)
    if (!(x < y)) {
        result |= 1;
    }
    
    // Complex expression with UNGE
    result |= (!(x < y) && (x > 0)) ? 2 : 0;
    
    // With NaN
    if (!(z < x)) {  // NaN < x is false, so !(false) = true
        result |= 4;
    }
    
    // Force UNGE condition code
    if (x >= y) {
        __asm__ goto ("j%c0 %l[lbl3]" : : "i" (UNGE) : : lbl3);
        result |= 8;
        lbl3: ;
    }
    
    sink(result);
    checksum += result;
    return result;
}

// Pattern 5: UNGT (not less than or equal) = !(a <= b)
__attribute__((noinline))
int test_ungt(void) {
    volatile double p = 7.0;
    volatile double q = 5.0;
    volatile double r = 0.0 / 0.0;
    
    int result = 0;
    
    // Direct UNGT: !(p <= q)
    if (!(p <= q)) {
        result |= 1;
    }
    
    // Complex expression
    result |= (!(p <= q) || (r != r)) ? 2 : 0;
    
    // Nested ternary with UNGT
    int val = (!(p <= q)) ? 4 : ((!(q <= p)) ? 8 : 0);
    result |= val;
    
    sink(result);
    checksum += result;
    return result;
}

// Pattern 6: UNLE (unordered or less than or equal)
__attribute__((noinline))
int test_unle(void) {
    volatile long double ld1 = 3.14159265358979323846L;
    volatile long double ld2 = 2.71828182845904523536L;
    volatile long double ldnan = 0.0L / 0.0L;
    
    int result = 0;
    
    // UNLE: !(a > b)
    if (!(ld1 > ld2)) {
        result |= 1;
    }
    
    // Another UNLE pattern
    if (ld1 <= ld2 || ld1 != ld1) {
        result |= 2;
    }
    
    // Complex logical expression
    if ((!(ld1 > ld2) && (ld1 == ld1)) || ldnan != ldnan) {
        result |= 4;
    }
    
    sink(result);
    checksum += result;
    return result;
}

// Pattern 7: UNLT (unordered or less than)
__attribute__((noinline))
int test_unlt(void) {
    volatile double m = 1.5;
    volatile double n = 2.5;
    volatile double nan = 0.0 / 0.0;
    
    int result = 0;
    
    // UNLT: !(a >= b)
    if (!(m >= n)) {
        result |= 1;
    }
    
    // With NaN operand
    if (!(nan >= m)) {
        result |= 2;
    }
    
    // Complex expression
    result |= (!(m >= n) ? 4 : 0) | (!(n >= m) ? 8 : 0);
    
    // Force UNLT condition code
    if (m < n) {
        __asm__ goto ("j%c0 %l[lbl4]" : : "i" (UNLT) : : lbl4);
        result |= 16;
        lbl4: ;
    }
    
    sink(result);
    checksum += result;
    return result;
}

// Pattern 8: LTGT (less than or greater than - ordered and not equal)
__attribute__((noinline))
int test_ltgt(void) {
    volatile float u = 10.0f;
    volatile float v = 20.0f;
    volatile float w = 0.0f / 0.0f;
    
    int result = 0;
    
    // LTGT: (a < b) || (a > b)
    if ((u < v) || (u > v)) {
        result |= 1;
    }
    
    // Another LTGT pattern
    if (u != v && u == u && v == v) {
        result |= 2;
    }
    
    // With NaN (should be false)
    if ((w < u) || (w > u)) {
        result |= 4;
    }
    
    // Complex nested expression
    result |= ((u < v) || (u > v)) ? 8 : 0;
    result |= (!(u == v) && isordered(u, v)) ? 16 : 0;
    
    sink(result);
    checksum += result;
    return result;
}

// Pattern 9: Mixed condition codes in complex function
__attribute__((noinline))
int test_mixed(void) {
    volatile double a = 1.0;
    volatile double b = 2.0;
    volatile double c = 0.0 / 0.0;
    volatile double d = -1.0 / 0.0;  // -inf
    
    int result = 0;
    
    // Chain of comparisons
    result |= (a < b) ? 1 : 0;
    result |= !(a >= b) ? 2 : 0;
    result |= (c != c) ? 4 : 0;
    result |= !(c == c) ? 8 : 0;
    result |= (a == a && b == b) ? 16 : 0;
    result |= (!(a > b) && !(a < b)) ? 32 : 0;
    result |= ((d < a) || (d > a)) ? 64 : 0;
    result |= !(a <= b) ? 128 : 0;
    
    // Nested ternary with multiple condition codes
    int complex = (a != a) ? 256 : 
                  (!(a < b)) ? 512 :
                  ((a > b) || (b > a)) ? 1024 : 0;
    result |= complex;
    
    sink(result);
    checksum += result;
    return result;
}

// Pattern 10: Struct with volatile floating-point members
typedef struct {
    volatile float f;
    volatile double d;
    volatile long double ld;
} FloatStruct;

__attribute__((noinline))
int test_struct(void) {
    FloatStruct fs = {3.14f, 2.71828, 1.41421356237309504880L};
    FloatStruct fs_nan = {0.0f/0.0f, 0.0/0.0, 0.0L/0.0L};
    
    int result = 0;
    
    // Compare struct members
    if (!(fs.f < fs_nan.f)) {
        result |= 1;
    }
    
    if (fs.d == fs.d && fs_nan.d != fs_nan.d) {
        result |= 2;
    }
    
    if (!(fs.ld >= fs_nan.ld)) {
        result |= 4;
    }
    
    // Complex expression with struct members
    result |= ((fs.f < 4.0f) || (fs.f > 4.0f)) ? 8 : 0;
    result |= (!(fs.d <= fs_nan.d) && isordered(fs.d, fs_nan.d)) ? 16 : 0;
    
    sink(result);
    checksum += result;
    return result;
}

int main(void) {
    printf("Starting condition code tests...\n");
    
    // Initialize checksum
    checksum = 0;
    
    // Run all test patterns
    checksum += test_unordered();
    checksum += test_ordered();
    checksum += test_uneq();
    checksum += test_unge();
    checksum += test_ungt();
    checksum += test_unle();
    checksum += test_unlt();
    checksum += test_ltgt();
    checksum += test_mixed();
    checksum += test_struct();
    
    // Print final checksum to ensure all code is live
    printf("Final checksum: %d\n", checksum);
    
    return 0;
}
