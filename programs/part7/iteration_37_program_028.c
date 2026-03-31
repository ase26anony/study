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
static const float cf = 9.87f;
static const int ci = 123;

/* Test 1: Basic integer-valued real functions in constant contexts */
enum TestEnum1 {
    VAL1 = (int)trunc(10.7),           /* Should be 10 */
    VAL2 = (int)floor(10.7),           /* Should be 10 */
    VAL3 = (int)ceil(10.3),            /* Should be 11 */
    VAL4 = (int)round(10.5),           /* Should be 11 */
    VAL5 = (int)nearbyint(10.2),       /* Should be 10 */
    VAL6 = (int)rint(10.8)             /* Should be 11 */
};

/* Test 2: Array sizes using integer-valued real functions */
char buffer1[(int)trunc(20.9)];        /* Size 20 */
char buffer2[(int)floor(15.1)];        /* Size 15 */
char buffer3[(int)ceil(12.01)];        /* Size 13 */

/* Test 3: Static assertions */
static_assert(trunc(5.9) == 5, "trunc failed");
static_assert(floor(5.9) == 5, "floor failed");
static_assert(ceil(5.1) == 6, "ceil failed");
static_assert(round(5.5) == 6, "round failed");

/* Test 4: Nested calls */
static const double nested1 = floor(ceil(7.3));      /* floor(8.0) = 8.0 */
static const double nested2 = trunc(round(6.7));     /* trunc(7.0) = 7.0 */
static const double nested3 = nearbyint(rint(9.4));  /* nearbyint(9.0) = 9.0 */

/* Test 5: Conditional operator with integer-valued real calls */
static double conditional1(int x) {
    return (x > 0) ? trunc(vd) : floor(vd);
}

static double conditional2(float x) {
    return (x < 0) ? ceil(x) : round(x);
}

/* Test 6: Built-in functions with different argument counts */
static long long test_builtins(void) {
    /* Single argument builtins */
    long long r1 = __builtin_llround(3.14159);
    long long r2 = __builtin_llrint(2.71828);
    
    /* Complex number real/imag parts */
    complex int ci = 3 + 4I;
    int real_part = __real__ ci;    /* Should be 3 */
    int imag_part = __imag__ ci;    /* Should be 4 */
    
    return r1 + r2 + real_part + imag_part;
}

/* Test 7: Mixed expressions with arithmetic */
static double mixed_expr(double x) {
    return (trunc(x) * 2.0) / floor(x + 1.0);
}

/* Test 8: Template metaprogramming (C++ only) */
#ifdef __cplusplus
template<typename T>
constexpr T template_test(T x) {
    return floor(x) + ceil(x) - trunc(x);
}

template<int N>
struct Factorial {
    enum { value = N * Factorial<N-1>::value };
};

template<>
struct Factorial<0> {
    enum { value = 1 };
};

/* Use integer-valued function in template argument */
constexpr int template_arg = (int)round(5.3);
template<int N = template_arg>
struct TestTemplate {
    enum { val = N * 2 };
};
#endif

/* Test 9: Complex number extraction functions */
static double complex_test(void) {
    complex double cd = 3.5 + 4.7I;
    double real_part = __real__ cd;    /* Should be 3.5 */
    double imag_part = __imag__ cd;    /* Should be 4.7 */
    
    /* Apply integer-valued functions to complex parts */
    return floor(real_part) + ceil(imag_part);
}

/* Test 10: Recursive depth testing */
static double recursive_depth(double x, int depth) {
    if (depth <= 0) return x;
    /* Nested calls to increase recursion depth */
    return floor(recursive_depth(ceil(x), depth - 1));
}

/* Test 11: Different argument types and values */
static void test_various_args(void) {
    /* Integer arguments */
    double r1 = floor(5);          /* 5.0 */
    double r2 = trunc(2);          /* 2.0 */
    
    /* Real arguments that are exact integers */
    double r3 = ceil(4.0);         /* 4.0 */
    double r4 = round(6.0);        /* 6.0 */
    
    /* Real arguments with fractional parts */
    double r5 = floor(4.7);        /* 4.0 */
    double r6 = trunc(-3.8);       /* -3.0 */
    
    /* Negative values */
    double r7 = round(-2.3);       /* -2.0 */
    double r8 = ceil(-2.3);        /* -2.0 */
    
    /* Large values */
    double r9 = trunc(1e10 + 0.7); /* 1e10 */
    double r10 = floor(1e10 - 0.3); /* 1e10 - 1 */
    
    /* Use results to prevent dead code elimination */
    volatile double sink = r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10;
    (void)sink;
}

/* Test 12: In loop bounds and conditions */
static int loop_test(void) {
    int sum = 0;
    int limit = (int)ceil(vd);  /* Should be 4 */
    
    for (int i = 0; i < limit; i++) {
        sum += (int)trunc(vf);  /* Should be 2 */
    }
    
    if ((int)round(cd) > (int)floor(cf)) {  /* 6 > 9? false */
        sum += 100;
    }
    
    return sum;
}

/* Main test driver */
int main(void) {
    int checksum = 0;
    
    /* Test 1: Enum values */
    checksum += VAL1 + VAL2 + VAL3 + VAL4 + VAL5 + VAL6;
    
    /* Test 2: Array sizes */
    checksum += sizeof(buffer1) + sizeof(buffer2) + sizeof(buffer3);
    
    /* Test 4: Nested calls */
    checksum += (int)nested1 + (int)nested2 + (int)nested3;
    
    /* Test 5: Conditional operators */
    checksum += (int)conditional1(vi);
    checksum += (int)conditional2(vf);
    
    /* Test 6: Builtins */
    checksum += test_builtins();
    
    /* Test 7: Mixed expressions */
    checksum += (int)mixed_expr(cd);
    
    /* Test 8: Template (C++ only) */
    #ifdef __cplusplus
    checksum += template_test(3.7);
    checksum += Factorial<5>::value;
    checksum += TestTemplate<>::val;
    #endif
    
    /* Test 9: Complex numbers */
    checksum += (int)complex_test();
    
    /* Test 10: Recursive depth */
    checksum += (int)recursive_depth(3.7, 3);
    
    /* Test 11: Various arguments */
    test_various_args();
    
    /* Test 12: Loops and conditions */
    checksum += loop_test();
    
    printf("Result: %d\n", checksum);
    
    /* Additional compile-time tests */
    constexpr double ct1 = trunc(7.89);
    constexpr double ct2 = floor(7.89);
    constexpr double ct3 = ceil(7.11);
    constexpr double ct4 = round(7.5);
    
    static_assert(ct1 == 7.0, "compile-time trunc failed");
    static_assert(ct2 == 7.0, "compile-time floor failed");
    static_assert(ct3 == 8.0, "compile-time ceil failed");
    static_assert(ct4 == 8.0, "compile-time round failed");
    
    return 0;
}
