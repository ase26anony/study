/* Test program for integer-valued real function constant folding */
#include <math.h>
#include <stdio.h>
#include <complex.h>

/* Prevent premature constant folding */
volatile double vd = 3.14159;
volatile float vf = 2.71828f;
volatile int vi = 10;

/* Complex types */
volatile double _Complex vcd = 3.0 + 4.0 * I;
volatile float _Complex vcf = 1.5f + 2.5f * I;

/* Test 1: Basic integer-valued real functions in constant contexts */
static int test1(void) {
    /* These should be folded by fold-const.cc */
    const double d1 = trunc(5.9);
    const double d2 = floor(5.9);
    const double d3 = ceil(5.1);
    const double d4 = round(5.5);
    const double d5 = nearbyint(5.3);
    const double d6 = rint(5.7);
    
    /* Use in array size (C99 VLA or C++ array) */
    char arr1[(int)trunc(10.5)];
    char arr2[(int)floor(10.5)];
    
    return (int)(d1 + d2 + d3 + d4 + d5 + d6) + sizeof(arr1) + sizeof(arr2);
}

/* Test 2: Nested calls */
static int test2(void) {
    /* Nested integer-valued real calls */
    double d1 = floor(ceil(3.7));
    double d2 = trunc(round(2.3));
    double d3 = nearbyint(rint(4.9));
    double d4 = ceil(floor(6.2));
    
    /* More complex nesting */
    double d5 = trunc(floor(ceil(round(5.6))));
    
    return (int)(d1 + d2 + d3 + d4 + d5);
}

/* Test 3: Conditional expressions with integer-valued calls */
static int test3(void) {
    /* Use volatile to prevent front-end folding */
    double x = vd;
    double y = vf;
    
    /* Conditional operator with integer-valued calls */
    double d1 = (x > 3.0) ? trunc(x) : floor(y);
    double d2 = (y < 2.0) ? ceil(y) : round(x);
    double d3 = (vi > 5) ? nearbyint(x) : rint(y);
    
    /* Nested conditional */
    double d4 = (x > y) ? ((vi % 2) ? trunc(x) : floor(x)) : ceil(y);
    
    return (int)(d1 + d2 + d3 + d4);
}

/* Test 4: Builtin functions with explicit integer returns */
static int test4(void) {
    /* __builtin_llround and __builtin_llrint return long long */
    long long ll1 = __builtin_llround(3.14);
    long long ll2 = __builtin_llround(2.718);
    long long ll3 = __builtin_llrint(1.618);
    long long ll4 = __builtin_llrint(0.577);
    
    /* Use in constant expressions */
    enum { 
        VAL1 = (int)__builtin_llround(10.1),
        VAL2 = (int)__builtin_llrint(20.2)
    };
    
    return (int)(ll1 + ll2 + ll3 + ll4) + VAL1 + VAL2;
}

/* Test 5: Complex number real/imag parts */
static int test5(void) {
    /* __real__ and __imag__ on complex types */
    double _Complex cd = 3.0 + 4.0 * I;
    float _Complex cf = 1.5f + 2.5f * I;
    
    /* These extract real/imag parts as integer-valued real expressions */
    double d1 = __real__ cd;
    double d2 = __imag__ cd;
    float f1 = __real__ cf;
    float f2 = __imag__ cf;
    
    /* Combine with other integer-valued calls */
    double d3 = trunc(__real__ cd);
    double d4 = floor(__imag__ cd);
    
    return (int)(d1 + d2 + f1 + f2 + d3 + d4);
}

/* Test 6: Integer-valued calls in arithmetic expressions */
static int test6(void) {
    double x = vd;
    double y = vf;
    
    /* Complex arithmetic with integer-valued calls */
    double d1 = (trunc(x) * 2.0) / floor(y);
    double d2 = ceil(x) + round(y) - nearbyint(x);
    double d3 = rint(x) * trunc(y) / 2.0;
    
    /* Comparisons that might be folded */
    int cmp1 = (ceil(x) > floor(y));
    int cmp2 = (trunc(x) == round(y));
    int cmp3 = (nearbyint(x) <= rint(y));
    
    return (int)(d1 + d2 + d3) + cmp1 + cmp2 + cmp3;
}

/* Test 7: Mixed argument types */
static int test7(void) {
    /* Integer arguments */
    double d1 = floor(5);
    double d2 = trunc(2);
    
    /* Real arguments that are exact integers */
    double d3 = ceil(4.0);
    double d4 = round(6.0);
    
    /* Real arguments with fractional parts */
    double d5 = floor(4.7);
    double d6 = trunc(3.14159);
    
    /* Negative values */
    double d7 = round(-2.3);
    double d8 = ceil(-3.7);
    double d9 = floor(-4.2);
    
    /* Large values */
    double d10 = trunc(1e10 + 0.7);
    double d11 = floor(1e10 - 0.3);
    
    return (int)(d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8 + d9 + d10 + d11);
}

/* Test 8: Calls with different numbers of arguments */
static int test8(void) {
    /* Most math functions take 1 argument, but test various cases */
    double d1 = trunc(vd);          /* 1 arg */
    double d2 = floor(vd);          /* 1 arg */
    
    /* Some builtins might have optional second args */
    /* For example, some implementations have rounding mode arguments */
    
    return (int)(d1 + d2);
}

/* C++ specific tests (if compiled as C++) */
#ifdef __cplusplus
#include <type_traits>

constexpr double cpp_test1() {
    /* constexpr math functions */
    return trunc(5.9) + floor(5.9) + ceil(5.1);
}

template<int N>
struct TestTemplate {
    static const int value = (int)trunc(N * 1.5);
};

static int test_cpp(void) {
    /* constexpr function evaluation */
    constexpr double d1 = cpp_test1();
    
    /* Template argument */
    constexpr int val = TestTemplate<10>::value;
    
    /* static_assert with integer-valued calls */
    static_assert(trunc(5.9) == 5, "trunc should work in static_assert");
    static_assert(floor(5.9) == 5, "floor should work in static_assert");
    static_assert(ceil(5.1) == 6, "ceil should work in static_assert");
    
    /* Array size in C++ */
    char arr[(int)round(15.5)];
    
    return (int)d1 + val + sizeof(arr);
}
#endif

/* Main driver */
int main(void) {
    int checksum = 0;
    
    /* Run all tests */
    checksum += test1();
    checksum += test2();
    checksum += test3();
    checksum += test4();
    checksum += test5();
    checksum += test6();
    checksum += test7();
    checksum += test8();
    
#ifdef __cplusplus
    checksum += test_cpp();
#endif
    
    /* Use results to prevent dead code elimination */
    printf("Result: %d\n", checksum);
    
    /* Additional verification */
    printf("Verification:\n");
    printf("  trunc(5.9) = %.0f (expected 5)\n", trunc(5.9));
    printf("  floor(5.9) = %.0f (expected 5)\n", floor(5.9));
    printf("  ceil(5.1) = %.0f (expected 6)\n", ceil(5.1));
    printf("  round(5.5) = %.0f (expected 6)\n", round(5.5));
    printf("  __builtin_llround(3.14) = %lld\n", __builtin_llround(3.14));
    
    return checksum != 0 ? 0 : 1;
}
