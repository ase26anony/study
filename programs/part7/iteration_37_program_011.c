/* Test program for integer-valued real function constant folding */
#include <math.h>
#include <stdio.h>
#include <complex.h>

/* Prevent premature constant folding */
volatile double vd = 3.14159;
volatile float vf = 2.71828f;
volatile int vi = 10;

/* Complex types */
volatile double _Complex vc_d = 3.0 + 4.0 * I;
volatile int _Complex vc_i = 5 + 6 * I;

/* Test 1: Basic integer-valued real functions in constant contexts */
enum {
    VAL_TRUNC = (int)trunc(5.9),           /* Should be 5 */
    VAL_FLOOR = (int)floor(4.7),           /* Should be 4 */
    VAL_CEIL = (int)ceil(4.1),             /* Should be 5 */
    VAL_ROUND = (int)round(3.5),           /* Should be 4 */
    VAL_NEARBYINT = (int)nearbyint(2.3),   /* Implementation defined */
    VAL_RINT = (int)rint(6.8)              /* Implementation defined */
};

/* Test 2: Array sizes using integer-valued functions */
char buffer1[(int)floor(10.5)];            /* Size 10 */
char buffer2[(int)ceil(10.1)];             /* Size 11 */
char buffer3[(int)trunc(15.9)];            /* Size 15 */

/* Test 3: Static assertions */
static_assert(trunc(5.9) == 5, "trunc failed");
static_assert(floor(4.7) == 4, "floor failed");
static_assert(ceil(4.1) == 5, "ceil failed");
static_assert(round(3.5) == 4, "round failed");

/* Test 4: Global initializers */
static double g1 = trunc(7.8);
static double g2 = floor(6.3);
static double g3 = ceil(5.2);
static double g4 = round(9.6);
static double g5 = nearbyint(3.14);
static double g6 = rint(2.71);

/* Test 5: Nested calls */
static double test_nested(double x) {
    /* Nested integer-valued calls */
    return floor(ceil(trunc(round(x))));
}

/* Test 6: Calls in conditional expressions */
static double test_conditional(double x, double y) {
    /* Conditional operator with integer-valued calls */
    return (x > y) ? trunc(x) : floor(y);
}

/* Test 7: Builtins with different argument counts */
static long long test_builtins(double x) {
    /* Builtins that return long long */
    return __builtin_llround(x) + __builtin_llrint(x);
}

/* Test 8: Complex number real/imag parts */
static int test_complex_parts() {
    /* Extract real/imag parts from complex integer */
    int _Complex c = 3 + 4 * I;
    return __real__ c + __imag__ c;  /* Should be 7 */
}

/* Test 9: Mixed expressions with arithmetic */
static double test_mixed_arithmetic(double x, double y) {
    return (trunc(x) * 2.0) / floor(y) + ceil(x + y) - round(x - y);
}

/* Test 10: In comparisons */
static int test_comparisons(double a, double b) {
    return ceil(a) > floor(b);
}

/* Test 11: Type casts */
static int test_type_casts(double d) {
    return (int)rint(d) + (int)nearbyint(d * 2.0);
}

/* Test 12: Recursive depth testing */
static double test_deep_nesting(double x) {
    /* Deeply nested to test recursion depth */
    return trunc(round(floor(ceil(nearbyint(rint(x))))));
}

/* Test 13: With integer arguments */
static double test_integer_args(int i) {
    return floor(i) + ceil(i) + trunc(i);
}

/* Test 14: With exact integer real arguments */
static double test_exact_integers() {
    return ceil(4.0) + floor(4.0) + trunc(4.0);
}

/* Test 15: Negative values */
static double test_negative_values() {
    return round(-2.3) + floor(-2.3) + ceil(-2.3);
}

/* Test 16: Large values */
static double test_large_values() {
    return trunc(1e10) + floor(1e10 + 0.5) + ceil(1e10 - 0.5);
}

/* Test 17: In loop bounds (compile-time) */
template<int N>
struct ArrayWrapper {
    char data[N];
};

/* Test 18: constexpr functions (C++11) */
constexpr double constexpr_trunc(double x) {
    return trunc(x);
}

constexpr double constexpr_floor(double x) {
    return floor(x);
}

constexpr double constexpr_ceil(double x) {
    return ceil(x);
}

/* Test function that uses all patterns */
static int run_all_tests() {
    int checksum = 0;
    
    /* Use volatile to prevent pre-folding */
    double x = vd;
    double y = vf;
    int i = vi;
    
    /* Test 1: Basic functions */
    checksum += (int)trunc(x);
    checksum += (int)floor(y);
    checksum += (int)ceil(x + 1.0);
    checksum += (int)round(y - 0.5);
    checksum += (int)nearbyint(x * 2.0);
    checksum += (int)rint(y * 3.0);
    
    /* Test 5: Nested calls */
    checksum += (int)test_nested(x);
    
    /* Test 6: Conditional calls */
    checksum += (int)test_conditional(x, y);
    
    /* Test 7: Builtins */
    checksum += test_builtins(x) % 100;
    
    /* Test 8: Complex parts */
    checksum += test_complex_parts();
    
    /* Test 9: Mixed arithmetic */
    checksum += (int)test_mixed_arithmetic(x, y);
    
    /* Test 10: Comparisons */
    checksum += test_comparisons(x, y);
    
    /* Test 11: Type casts */
    checksum += test_type_casts(x);
    
    /* Test 12: Deep nesting */
    checksum += (int)test_deep_nesting(x);
    
    /* Test 13: Integer arguments */
    checksum += (int)test_integer_args(i);
    
    /* Test 14: Exact integers */
    checksum += (int)test_exact_integers();
    
    /* Test 15: Negative values */
    checksum += (int)test_negative_values();
    
    /* Test 16: Large values */
    checksum += (int)test_large_values();
    
    /* Test 18: constexpr */
    constexpr double ct = constexpr_trunc(5.9);
    constexpr double cf = constexpr_floor(4.7);
    constexpr double cc = constexpr_ceil(4.1);
    checksum += (int)ct + (int)cf + (int)cc;
    
    /* Additional nested conditional pattern */
    checksum += (x > 0) ? (int)trunc(x) : (int)floor(y);
    checksum += (i % 2) ? (int)ceil(x) : (int)round(y);
    
    /* Nested in arithmetic expression */
    checksum += (int)(trunc(x) * floor(y) / ceil(x + y));
    
    /* Multiple arguments extraction test */
    checksum += (int)copysign(trunc(x), floor(y));  /* 2-argument function */
    
    return checksum;
}

int main() {
    /* Force evaluation in constant contexts */
    ArrayWrapper<(int)floor(20.7)> arr1;
    ArrayWrapper<(int)ceil(15.2)> arr2;
    ArrayWrapper<(int)trunc(25.9)> arr3;
    
    /* Unused to avoid warnings */
    (void)buffer1;
    (void)buffer2;
    (void)buffer3;
    (void)g1; (void)g2; (void)g3; (void)g4; (void)g5; (void)g6;
    (void)arr1; (void)arr2; (void)arr3;
    
    /* Run tests and print checksum */
    int result = run_all_tests();
    printf("Result: %d\n", result);
    
    /* Verify some constant folded values */
    printf("Constants: %d %d %d %d\n", 
           VAL_TRUNC, VAL_FLOOR, VAL_CEIL, VAL_ROUND);
    
    return 0;
}
