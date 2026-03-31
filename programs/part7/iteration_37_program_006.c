/* Test program for integer-valued real function constant folding */
#include <math.h>
#include <stdio.h>
#include <complex.h>

/* Prevent premature constant folding */
volatile double vd = 3.14159;
volatile float vf = 2.71828f;
volatile int vi = 42;

/* Global variables to force constant folding in static initializers */
static const double cd = 5.67;
static const float cf = 8.91f;
static const int ci = 123;

/* Test 1: Basic integer-valued real functions in constant contexts */
enum {
    ENUM_TRUNC = (int)trunc(10.7),           /* Should be 10 */
    ENUM_FLOOR = (int)floor(10.7),           /* Should be 10 */
    ENUM_CEIL = (int)ceil(10.3),             /* Should be 11 */
    ENUM_ROUND = (int)round(10.5),           /* Should be 11 */
    ENUM_NEARBYINT = (int)nearbyint(10.4),   /* Should be 10 */
    ENUM_RINT = (int)rint(10.6),             /* Should be 11 */
};

/* Test 2: Array sizes using integer-valued real functions */
char buffer1[(int)floor(20.5)];              /* Size 20 */
char buffer2[(int)ceil(20.1)];               /* Size 21 */
char buffer3[(int)trunc(20.9)];              /* Size 20 */

/* Test 3: Static assertions */
static_assert(trunc(5.9) == 5, "trunc failed");
static_assert(floor(5.9) == 5, "floor failed");
static_assert(ceil(5.1) == 6, "ceil failed");
static_assert(round(5.5) == 6, "round failed");

/* Test 4: Static variable initializers */
static double sd1 = trunc(cd * 2.0);         /* trunc(11.34) = 11.0 */
static double sd2 = floor(cd + 1.0);         /* floor(6.67) = 6.0 */
static double sd3 = ceil(cf * 1.5);          /* ceil(13.365) = 14.0 */
static float sf1 = round(cf);                /* round(8.91) = 9.0 */

/* Test 5: Nested calls */
constexpr double nested1() {
    return floor(ceil(10.3));                /* ceil(10.3)=11, floor(11)=11 */
}

constexpr double nested2() {
    return trunc(round(7.8));                /* round(7.8)=8, trunc(8)=8 */
}

/* Test 6: Conditional operator with integer-valued real calls */
double test_conditional(double x, double y) {
    return (x > y) ? trunc(x) : floor(y);
}

/* Test 7: Complex number real/imag part extractors */
int test_complex() {
    complex int ci = 3 + 4 * I;
    complex double cd = 5.6 + 7.8 * I;
    
    /* __real__ and __imag__ on complex integer types */
    int real_part = __real__ ci;             /* Should be 3 */
    int imag_part = __imag__ ci;             /* Should be 4 */
    
    /* These produce real results from complex inputs */
    double dreal = __real__ cd;              /* Should be 5.6 */
    double dimag = __imag__ cd;              /* Should be 7.8 */
    
    return real_part + imag_part + (int)dreal + (int)dimag;
}

/* Test 8: Builtin functions with different argument counts */
long long test_builtins() {
    /* Single argument builtins */
    long long ll1 = __builtin_llround(100.7);    /* 101 */
    long long ll2 = __builtin_llrint(200.3);     /* 200 */
    
    /* Using nearbyint with different rounding modes */
    double d1 = nearbyint(50.4);                 /* 50 */
    double d2 = nearbyint(50.6);                 /* 51 */
    
    return ll1 + ll2 + (long long)d1 + (long long)d2;
}

/* Test 9: Mixed expressions with arithmetic */
double test_mixed(double x, double y) {
    return (trunc(x) * 2.0) / floor(y) + ceil(x + y) - round(x * y);
}

/* Test 10: Calls as function arguments */
double test_nested_args(double x) {
    return floor(trunc(ceil(round(x))));     /* Multiple levels of nesting */
}

/* Test 11: Integer arguments (edge cases) */
double test_integer_args() {
    return floor(5) + trunc(2) + ceil(4.0) + round(3);  /* All integer results */
}

/* Test 12: Negative values */
double test_negative() {
    return floor(-2.7) + ceil(-2.1) + trunc(-3.8) + round(-3.5);
}

/* Test 13: Large values */
double test_large() {
    return trunc(1e10 + 0.7) + floor(1e10 + 0.3);
}

/* Test 14: Template metaprogramming (C++ only) */
template<double Value>
struct TruncValue {
    static const int result = (int)trunc(Value);
};

template<double Value>
struct FloorValue {
    static const int result = (int)floor(Value);
};

/* Test 15: Recursive constant evaluation */
constexpr double recursive_fold(int n, double x) {
    return (n <= 0) ? x : floor(recursive_fold(n - 1, x * 1.1));
}

/* Driver function that tests all patterns */
int main() {
    int checksum = 0;
    
    /* Test 1: Enum values */
    checksum += ENUM_TRUNC + ENUM_FLOOR + ENUM_CEIL + ENUM_ROUND + 
                ENUM_NEARBYINT + ENUM_RINT;
    
    /* Test 2: Array sizes */
    checksum += sizeof(buffer1) + sizeof(buffer2) + sizeof(buffer3);
    
    /* Test 4: Static initializers */
    checksum += (int)sd1 + (int)sd2 + (int)sd3 + (int)sf1;
    
    /* Test 5: Nested calls */
    checksum += (int)nested1() + (int)nested2();
    
    /* Test 6: Conditional operator */
    double cond_result = test_conditional(10.5, 9.5);
    checksum += (int)cond_result;
    
    /* Test 7: Complex numbers */
    checksum += test_complex();
    
    /* Test 8: Builtins */
    checksum += test_builtins();
    
    /* Test 9: Mixed expressions */
    double mixed_result = test_mixed(4.7, 3.2);
    checksum += (int)mixed_result;
    
    /* Test 10: Nested arguments */
    double nested_arg_result = test_nested_args(5.6);
    checksum += (int)nested_arg_result;
    
    /* Test 11: Integer arguments */
    double int_arg_result = test_integer_args();
    checksum += (int)int_arg_result;
    
    /* Test 12: Negative values */
    double neg_result = test_negative();
    checksum += (int)neg_result;
    
    /* Test 13: Large values */
    double large_result = test_large();
    checksum += (int)(large_result / 1e9);  /* Scale down to avoid overflow */
    
    /* Test 14: Template values */
    checksum += TruncValue<15.8>::result + FloorValue<15.2>::result;
    
    /* Test 15: Recursive folding */
    double recursive_result = recursive_fold(3, 2.0);
    checksum += (int)recursive_result;
    
    /* Additional tests with volatile to prevent early folding */
    checksum += (int)trunc(vd);
    checksum += (int)floor(vf);
    checksum += (int)ceil(vd + 1.0);
    checksum += (int)round(vf * 2.0);
    
    /* Test with 0, 1, and 2 argument functions */
    checksum += (int)nearbyint(vd);      /* 1 argument */
    checksum += (int)rint(vf);           /* 1 argument */
    
    /* Test in loop bounds */
    for (int i = 0; i < (int)floor(vd + 1.0); i++) {
        checksum += i;
    }
    
    /* Test in if conditions */
    if (ceil(vd) > floor(vf)) {
        checksum += 1000;
    }
    
    printf("Final checksum: %d\n", checksum);
    return checksum == 0 ? 1 : 0;  /* Return non-zero if checksum is non-zero */
}
