/* Test program for integer-valued real function constant folding */
#include <math.h>
#include <stdio.h>
#include <complex.h>

/* Prevent premature constant folding */
volatile double vd = 3.14159;
volatile float vf = 2.71828f;
volatile int vi = 42;

/* Complex types */
volatile _Complex int vci = 3 + 4 * I;
volatile _Complex double vcd = 1.5 + 2.5 * I;

/* Test 1: Basic integer-valued real functions in constant contexts */
enum {
    ENUM_TRUNC = (int)trunc(5.9),
    ENUM_FLOOR = (int)floor(4.7),
    ENUM_CEIL = (int)ceil(4.1),
    ENUM_ROUND = (int)round(3.5),
    ENUM_NEARBYINT = (int)nearbyint(2.3),
    ENUM_RINT = (int)rint(6.8)
};

/* Array sizes using integer-valued functions */
char buffer1[(int)floor(10.5)];
char buffer2[(int)ceil(9.1)];
char buffer3[(int)trunc(15.9)];

/* Static assertions */
static_assert(trunc(5.9) == 5, "trunc failed");
static_assert(floor(4.7) == 4, "floor failed");
static_assert(ceil(4.1) == 5, "ceil failed");
static_assert(round(3.5) == 4, "round failed");

/* Global initializers with const variables */
const double cd = 7.89;
const float cf = 6.54f;
static double g1 = trunc(cd);
static float g2 = floor(cf);
static int g3 = round(3.14);

/* Test 2: Nested calls */
constexpr double test_nested(double x) {
    return floor(ceil(trunc(round(x))));
}

/* Test 3: Calls within conditional expressions */
constexpr double test_conditional(double a, double b, int cond) {
    return cond ? trunc(a) : floor(b);
}

/* Test 4: Complex number real/imag part extractors */
constexpr int test_complex_real(_Complex int z) {
    return __real__ z;
}

constexpr int test_complex_imag(_Complex int z) {
    return __imag__ z;
}

/* Test 5: Builtins with different argument counts */
constexpr long long test_builtins(double x) {
    return __builtin_llround(x) + __builtin_llrint(x);
}

/* Test 6: Mixed expressions with arithmetic */
constexpr double test_mixed(double x, double y) {
    return (trunc(x) * 2.0) / floor(y) + ceil(x + y) - nearbyint(x - y);
}

/* Test 7: Recursive depth testing */
template<int N>
struct RecursiveTest {
    static constexpr double value = trunc(RecursiveTest<N-1>::value);
};

template<>
struct RecursiveTest<0> {
    static constexpr double value = 12.345;
};

/* Test functions that will be called from main */
int test1_basic_functions() {
    int sum = 0;
    
    /* Force evaluation in non-const context */
    double d = vd;
    float f = vf;
    
    sum += (int)trunc(d + 1.0);
    sum += (int)floor(f * 2.0f);
    sum += (int)ceil(d - 0.5);
    sum += (int)round(f + 0.3f);
    sum += (int)nearbyint(d * 2.0);
    sum += (int)rint(f / 2.0f);
    
    return sum;
}

int test2_nested_and_conditional() {
    int sum = 0;
    double a = vd;
    double b = vf;
    int cond = vi & 1;
    
    /* Nested calls */
    sum += (int)floor(ceil(a));
    sum += (int)trunc(round(b));
    sum += (int)round(trunc(a + b));
    
    /* Conditional calls */
    sum += (int)(cond ? trunc(a * 2.0) : floor(b * 3.0));
    sum += (int)(!cond ? ceil(a / 2.0) : nearbyint(b * 4.0));
    
    /* Calls as arguments */
    sum += (int)round(trunc(a) + floor(b));
    
    return sum;
}

int test3_complex_parts() {
    int sum = 0;
    _Complex int ci = vci;
    _Complex double cd = vcd;
    
    /* Real/imag parts of complex integers */
    sum += __real__ ci;
    sum += __imag__ ci;
    
    /* Complex double with integer-valued extraction */
    sum += (int)trunc(creal(cd));
    sum += (int)floor(cimag(cd));
    
    return sum;
}

int test4_builtin_functions() {
    long long sum = 0;
    double d = vd;
    float f = vf;
    
    /* Builtins that return long long */
    sum += __builtin_llround(d);
    sum += __builtin_llrint(f);
    sum += __builtin_llround(d * 2.0);
    sum += __builtin_llrint(f * 3.0f);
    
    /* Test with integer arguments */
    sum += __builtin_llround(5.0);  /* Exact integer */
    sum += __builtin_llrint(-3.0);  /* Negative exact integer */
    
    return (int)(sum % 1000);  /* Reduce to int for checksum */
}

int test5_mixed_expressions() {
    int sum = 0;
    double x = vd;
    double y = vf;
    
    /* Arithmetic with integer-valued functions */
    sum += (int)((trunc(x) * 2) / floor(y + 1.0));
    sum += (int)(ceil(x * y) - nearbyint(x - y));
    sum += (int)(rint(x) * round(y));
    
    /* Comparisons that might fold */
    if (ceil(x) > floor(y)) sum += 10;
    if (trunc(x) == (int)x) sum += 20;
    if (round(y) != floor(y)) sum += 30;
    
    /* Type casts */
    sum += (int)rint(x * 10.0);
    sum += (int)trunc(y * 20.0f);
    
    return sum;
}

int test6_constant_expressions() {
    /* These should be folded at compile time */
    constexpr double c1 = test_nested(3.7);
    constexpr double c2 = test_conditional(4.2, 5.8, 1);
    constexpr int c3 = test_complex_real(3 + 4*I);
    constexpr int c4 = test_complex_imag(3 + 4*I);
    constexpr long long c5 = test_builtins(2.7);
    constexpr double c6 = test_mixed(1.2, 3.4);
    constexpr double c7 = RecursiveTest<3>::value;
    
    return (int)c1 + (int)c2 + c3 + c4 + (int)c5 + (int)c6 + (int)c7;
}

int test7_edge_cases() {
    int sum = 0;
    
    /* Large values */
    sum += (int)trunc(1e10 + 0.7);
    sum += (int)floor(1e10 - 0.3);
    
    /* Negative values */
    sum += (int)ceil(-3.7);
    sum += (int)round(-2.5);
    sum += (int)trunc(-4.9);
    sum += (int)floor(-5.1);
    
    /* Zero */
    sum += (int)trunc(0.0);
    sum += (int)ceil(0.0);
    
    /* Exact integers */
    sum += (int)floor(42.0);
    sum += (int)ceil(42.0);
    sum += (int)round(42.0);
    
    return sum;
}

/* Main driver that accumulates checksum */
int main() {
    int checksum = 0;
    
    printf("Testing integer-valued real function constant folding...\n");
    
    checksum += test1_basic_functions();
    checksum += test2_nested_and_conditional();
    checksum += test3_complex_parts();
    checksum += test4_builtin_functions();
    checksum += test5_mixed_expressions();
    checksum += test6_constant_expressions();
    checksum += test7_edge_cases();
    
    /* Add enum values */
    checksum += ENUM_TRUNC + ENUM_FLOOR + ENUM_CEIL + ENUM_ROUND + 
                ENUM_NEARBYINT + ENUM_RINT;
    
    /* Add global initializers */
    checksum += (int)g1 + (int)g2 + g3;
    
    /* Add array size indicators */
    checksum += sizeof(buffer1) + sizeof(buffer2) + sizeof(buffer3);
    
    printf("Result: %d\n", checksum);
    
    /* Verify some expected values */
    if (trunc(5.9) != 5) printf("ERROR: trunc(5.9) != 5\n");
    if (floor(4.7) != 4) printf("ERROR: floor(4.7) != 4\n");
    if (ceil(4.1) != 5) printf("ERROR: ceil(4.1) != 5\n");
    if (round(3.5) != 4) printf("ERROR: round(3.5) != 4\n");
    
    return 0;
}
