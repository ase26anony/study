/* Test program for integer-valued real function constant folding */
#include <math.h>
#include <stdio.h>
#include <complex.h>

/* Prevent premature constant folding */
volatile double vd = 3.14159;
volatile float vf = 2.71828f;
volatile int vi = 10;

/* Complex types */
volatile double _Complex vc_d = 3.0 + 4.0*I;
volatile int _Complex vc_i = 5 + 6*I;

/* Test 1: Basic integer-valued real functions in constant contexts */
enum {
    ENUM_TRUNC = (int)trunc(5.9),
    ENUM_FLOOR = (int)floor(5.9),
    ENUM_CEIL = (int)ceil(5.1),
    ENUM_ROUND = (int)round(5.5),
    ENUM_NEARBYINT = (int)nearbyint(5.3),
    ENUM_RINT = (int)rint(5.7)
};

/* Test 2: Array sizes using integer-valued functions */
char buffer1[(int)floor(10.5)];
char buffer2[(int)ceil(10.1)];
char buffer3[(int)trunc(15.9)];

/* Test 3: Static assertions */
static_assert(trunc(5.9) == 5, "trunc failed");
static_assert(floor(5.9) == 5, "floor failed");
static_assert(ceil(5.1) == 6, "ceil failed");
static_assert(round(5.5) == 6, "round failed");

/* Test 4: Global initializers with integer-valued functions */
static double g1 = trunc(7.8);
static double g2 = floor(7.8);
static double g3 = ceil(7.2);
static double g4 = round(7.5);
static double g5 = nearbyint(7.3);
static double g6 = rint(7.7);

/* Test 5: Complex part extraction */
static double g7 = __real__(3.0 + 4.0*I);
static double g8 = __imag__(3.0 + 4.0*I);
static int g9 = __real__(5 + 6*I);
static int g10 = __imag__(5 + 6*I);

/* Test 6: Builtins with long long return */
#ifdef __GNUC__
static long long g11 = __builtin_llround(9.7);
static long long g12 = __builtin_llrint(9.3);
#endif

/* Test functions with different patterns */
static int test_nested_calls(void) {
    /* Nested calls: floor(ceil(x)) */
    double x = vd;
    double result = floor(ceil(x * 2.0));
    return (int)result;
}

static int test_conditional_calls(void) {
    /* Conditional operator with integer-valued calls */
    double a = vd;
    double b = vf;
    double result = (vi > 5) ? trunc(a) : floor(b);
    return (int)result;
}

static int test_multi_arg_calls(void) {
    /* Some builtins may have multiple arguments */
    double x = vd;
    double y = vf;
    
    /* Hypothetical: some builtins might have 2 args */
    /* Using fmax/fmin which are integer-valued for integer inputs */
    double result1 = fmax(trunc(x), floor(y));
    double result2 = fmin(ceil(x), round(y));
    
    return (int)(result1 + result2);
}

static int test_complex_parts(void) {
    /* Complex part extraction */
    double _Complex c1 = vc_d;
    int _Complex c2 = vc_i;
    
    double real1 = __real__(c1);
    double imag1 = __imag__(c1);
    int real2 = __real__(c2);
    int imag2 = __imag__(c2);
    
    return (int)(real1 + imag1 + real2 + imag2);
}

static int test_arithmetic_with_calls(void) {
    /* Arithmetic expressions with integer-valued calls */
    double x = vd;
    double y = vf;
    
    double result = (trunc(x) * 2.0) / floor(y) + ceil(x + 1.5) - round(y * 2.0);
    return (int)result;
}

static int test_comparisons_with_calls(void) {
    /* Comparisons with integer-valued calls */
    double a = vd;
    double b = vf;
    
    int cmp1 = ceil(a) > floor(b);
    int cmp2 = trunc(a) == round(b);
    int cmp3 = nearbyint(a) <= rint(b);
    
    return cmp1 + cmp2 * 2 + cmp3 * 4;
}

static int test_type_casts(void) {
    /* Type casts of integer-valued calls */
    double x = vd;
    
    int i1 = (int)trunc(x);
    int i2 = (int)floor(x + 1.5);
    int i3 = (int)ceil(x * 2.0);
    int i4 = (int)round(x - 0.5);
    
    return i1 + i2 + i3 + i4;
}

static int test_negative_values(void) {
    /* Negative values */
    double x = -vd;
    double y = -vf;
    
    double r1 = trunc(x);
    double r2 = floor(y);
    double r3 = ceil(x * 1.5);
    double r4 = round(y * 2.0);
    
    return (int)(r1 + r2 + r3 + r4);
}

static int test_exact_integers(void) {
    /* Arguments that are exact integers */
    double x = 5.0;
    double y = -3.0;
    
    double r1 = trunc(x);
    double r2 = floor(y);
    double r3 = ceil(x + 0.0);
    double r4 = round(y - 0.0);
    
    return (int)(r1 + r2 + r3 + r4);
}

static int test_large_values(void) {
    /* Large values */
    double large = 1e10;
    double r1 = trunc(large + 0.7);
    double r2 = floor(large - 0.3);
    double r3 = ceil(large + 0.1);
    double r4 = round(large + 0.5);
    
    /* Use modulo to keep result small */
    return ((int)r1 + (int)r2 + (int)r3 + (int)r4) % 1000;
}

/* C++ specific tests (compile with C++) */
#ifdef __cplusplus
constexpr double cpp_test1() {
    return trunc(5.9) + floor(5.9) + ceil(5.1);
}

template<int N>
struct TestTemplate {
    static const int value = (int)trunc(N * 1.5);
};

constexpr int template_val = TestTemplate<10>::value;
#endif

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    /* Accumulate results from all test functions */
    checksum += test_nested_calls();
    checksum += test_conditional_calls();
    checksum += test_multi_arg_calls();
    checksum += test_complex_parts();
    checksum += test_arithmetic_with_calls();
    checksum += test_comparisons_with_calls();
    checksum += test_type_casts();
    checksum += test_negative_values();
    checksum += test_exact_integers();
    checksum += test_large_values();
    
    /* Add enum values */
    checksum += ENUM_TRUNC + ENUM_FLOOR + ENUM_CEIL + ENUM_ROUND + 
                ENUM_NEARBYINT + ENUM_RINT;
    
    /* Add global initializers (cast to int) */
    checksum += (int)g1 + (int)g2 + (int)g3 + (int)g4 + 
                (int)g5 + (int)g6 + (int)g7 + (int)g8 + 
                g9 + g10;
    
#ifdef __GNUC__
    checksum += (int)(g11 % 100) + (int)(g12 % 100);
#endif
    
#ifdef __cplusplus
    checksum += (int)cpp_test1() + template_val;
#endif
    
    /* Add buffer sizes */
    checksum += sizeof(buffer1) + sizeof(buffer2) + sizeof(buffer3);
    
    printf("Result: %d\n", checksum);
    return 0;
}
