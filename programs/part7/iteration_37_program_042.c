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
    /* These should be folded during constant folding pass */
    const double d1 = trunc(5.9);
    const double d2 = floor(4.7);
    const double d3 = ceil(3.2);
    const double d4 = round(6.5);
    const double d5 = nearbyint(2.3);
    const double d6 = rint(7.8);
    
    /* Use in static assertions (compile-time evaluation) */
    static_assert(trunc(5.9) == 5, "trunc should work at compile time");
    static_assert(floor(4.7) == 4, "floor should work at compile time");
    static_assert(ceil(3.2) == 4, "ceil should work at compile time");
    
    return (int)(d1 + d2 + d3 + d4 + d5 + d6);
}

/* Test 2: Builtin functions with integer return types */
static int test2(void) {
    /* These builtins return long long but take real arguments */
    long long ll1 = __builtin_llround(3.14);
    long long ll2 = __builtin_llround(-2.7);
    long long ll3 = __builtin_llrint(5.5);
    long long ll4 = __builtin_llrint(-3.5);
    
    /* Use in array size (requires constant folding) */
    char buffer1[(int)floor(10.5)];
    char buffer2[(int)ceil(9.2)];
    
    return (int)(ll1 + ll2 + ll3 + ll4 + sizeof(buffer1) + sizeof(buffer2));
}

/* Test 3: Nested calls to exercise recursive depth */
static int test3(void) {
    /* Nested integer-valued real function calls */
    double d1 = floor(ceil(3.7));
    double d2 = trunc(round(4.2));
    double d3 = ceil(floor(5.9));
    double d4 = round(trunc(6.4));
    
    /* Multiple levels of nesting */
    double d5 = floor(ceil(trunc(2.8)));
    double d6 = trunc(round(floor(3.6)));
    
    return (int)(d1 + d2 + d3 + d4 + d5 + d6);
}

/* Test 4: Conditional expressions with integer-valued calls */
static int test4(void) {
    /* Use volatile to prevent front-end constant folding */
    double x = vd;
    float y = vf;
    
    /* Conditional operator with integer-valued real calls */
    double d1 = (x > 0) ? trunc(x) : floor(y);
    double d2 = (y < 0) ? ceil(y) : round(x);
    
    /* Nested conditional with calls */
    double d3 = (x > y) ? trunc(ceil(x)) : floor(round(y));
    
    return (int)(d1 + d2 + d3);
}

/* Test 5: Complex number real/imag parts */
static int test5(void) {
    /* Extract real and imaginary parts from complex numbers */
    double _Complex cd = 3.0 + 4.0 * I;
    float _Complex cf = 1.5f + 2.5f * I;
    
    /* __real__ and __imag__ on complex integer types are integer-valued */
    double r1 = __real__(cd);
    double i1 = __imag__(cd);
    float r2 = __real__(cf);
    float i2 = __imag__(cf);
    
    /* Combine with other integer-valued functions */
    double d1 = trunc(__real__(cd));
    double d2 = floor(__imag__(cf));
    
    return (int)(r1 + i1 + r2 + i2 + d1 + d2);
}

/* Test 6: Calls with different numbers of arguments */
static int test6(void) {
    /* Test calls with 0, 1, and 2 arguments */
    double d1 = trunc(5.9);          /* 1 argument */
    
    /* Some builtins might have optional arguments */
    /* Use fmax/fmin which take 2 arguments and are integer-valued for integer inputs */
    double d2 = fmax(3.0, 5.0);      /* 2 arguments, integer-valued for integer args */
    double d3 = fmin(2.0, 4.0);      /* 2 arguments */
    
    /* Nested with different arg counts */
    double d4 = floor(fmax(3.5, 4.2));
    double d5 = ceil(fmin(1.8, 2.9));
    
    return (int)(d1 + d2 + d3 + d4 + d5);
}

/* Test 7: Integer-valued calls in arithmetic expressions */
static int test7(void) {
    double x = vd;
    
    /* Arithmetic with integer-valued real calls */
    double d1 = (trunc(x) * 2) / floor(x + 1.0);
    double d2 = ceil(x) + floor(x) - round(x);
    double d3 = nearbyint(x) * rint(x);
    
    /* Comparisons that might be folded */
    int cmp1 = (ceil(x) > floor(x + 1.0));
    int cmp2 = (trunc(x) == round(x - 0.5));
    
    return (int)(d1 + d2 + d3 + cmp1 + cmp2);
}

/* Test 8: Large values and edge cases */
static int test8(void) {
    /* Large values */
    double d1 = trunc(1e10 + 0.7);
    double d2 = floor(1e20 - 0.3);
    
    /* Exact integers */
    double d3 = ceil(4.0);
    double d4 = round(5.0);
    
    /* Negative values */
    double d5 = trunc(-3.7);
    double d6 = floor(-2.3);
    double d7 = ceil(-1.8);
    double d8 = round(-4.5);
    
    return (int)(d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8);
}

/* Test 9: Template and constexpr contexts (C++ specific) */
#ifdef __cplusplus
template<typename T>
constexpr T test9_helper(T x) {
    return trunc(ceil(floor(x)));
}

static int test9(void) {
    constexpr double d1 = test9_helper(3.7);
    constexpr double d2 = test9_helper(-2.4);
    
    /* Use in template parameter */
    struct Test {
        char buffer[(int)d1];
    };
    
    return (int)(d1 + d2 + sizeof(Test));
}
#else
static int test9(void) {
    /* C version */
    double d1 = trunc(ceil(floor(3.7)));
    double d2 = trunc(ceil(floor(-2.4)));
    
    char buffer[(int)d1];
    
    return (int)(d1 + d2 + sizeof(buffer));
}
#endif

/* Test 10: Mixed types and casts */
static int test10(void) {
    float f = vf;
    double d = vd;
    
    /* Mixed float/double calls */
    double d1 = trunc((double)f);
    double d2 = floor((float)d);
    
    /* Explicit casts */
    int i1 = (int)rint(d);
    long l1 = (long)round(f);
    
    /* Casts within expressions */
    double d3 = (double)((int)trunc(d));
    float f3 = (float)((long)ceil(f));
    
    return (int)(d1 + d2 + i1 + l1 + d3 + f3);
}

/* Main driver that calls all tests */
int main(void) {
    int checksum = 0;
    
    checksum += test1();
    checksum += test2();
    checksum += test3();
    checksum += test4();
    checksum += test5();
    checksum += test6();
    checksum += test7();
    checksum += test8();
    checksum += test9();
    checksum += test10();
    
    printf("Result: %d\n", checksum);
    
    /* Additional compile-time tests */
    enum { 
        E1 = (int)trunc(10.7),
        E2 = (int)floor(9.2),
        E3 = (int)ceil(8.3),
        E4 = (int)round(7.6)
    };
    
    /* Force evaluation in dead code to ensure all paths are processed */
    if (0) {
        static double unused1 = nearbyint(100.1);
        static float unused2 = rint(200.2f);
        static long long unused3 = __builtin_llround(300.3);
    }
    
    return checksum > 0 ? 0 : 1;
}
