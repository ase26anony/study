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
char buffer2[(int)ceil(9.1)];
char buffer3[(int)trunc(8.9)];

/* Test 3: Static assertions */
static_assert(trunc(5.9) == 5, "trunc failed");
static_assert(floor(5.9) == 5, "floor failed");
static_assert(ceil(5.1) == 6, "ceil failed");
static_assert(round(5.5) == 6, "round failed");

/* Test 4: Global initializers */
static double g1 = trunc(7.8);
static double g2 = floor(7.8);
static double g3 = ceil(7.2);
static double g4 = round(7.5);
static double g5 = nearbyint(7.3);
static double g6 = rint(7.7);

/* Test 5: Nested calls */
static double nested1 = floor(ceil(5.3));
static double nested2 = trunc(round(4.7));
static double nested3 = round(trunc(6.2));
static double nested4 = ceil(floor(3.8));

/* Test 6: Conditional operator with integer-valued calls */
static double cond1 = (vi > 5) ? trunc(4.7) : floor(4.2);
static double cond2 = (vi < 5) ? ceil(3.1) : round(3.6);

/* Test 7: Builtins with different argument counts */
static long long blt1 = __builtin_llround(9.7);
static long long blt2 = __builtin_llrint(9.3);

/* Test 8: Complex part extraction */
static double complex_part1 = __real__(vc_i);
static double complex_part2 = __imag__(vc_i);
static double complex_part3 = __real__(vc_d);
static double complex_part4 = __imag__(vc_d);

/* Test 9: Arithmetic with integer-valued calls */
static double arith1 = trunc(5.9) * 2.0;
static double arith2 = floor(4.7) / ceil(2.1);
static double arith3 = round(3.5) + nearbyint(2.3);

/* Test 10: Comparisons with integer-valued calls */
static int cmp1 = (ceil(4.1) > floor(3.9));
static int cmp2 = (trunc(5.5) == round(5.5));
static int cmp3 = (rint(6.7) <= nearbyint(6.7));

/* C++ specific tests (if compiled as C++) */
#ifdef __cplusplus
constexpr double cpp_trunc(double x) { return trunc(x); }
constexpr double cpp_floor(double x) { return floor(x); }
constexpr double cpp_ceil(double x) { return ceil(x); }
constexpr double cpp_round(double x) { return round(x); }

template<int N>
struct TestTemplate {
    static const int value = (int)trunc(N * 1.5);
};

constexpr int template_val = TestTemplate<5>::value;
#endif

/* Test functions that return values based on integer-valued real functions */
int test_basic_functions(void) {
    double d = vd;
    float f = vf;
    
    /* Various integer-valued real function calls */
    double r1 = trunc(d);
    double r2 = floor(d + 1.0);
    double r3 = ceil(f);
    double r4 = round(d - 0.5);
    double r5 = nearbyint(d * 2.0);
    double r6 = rint(f * 3.0);
    
    /* Nested calls */
    double r7 = floor(ceil(d));
    double r8 = trunc(round(f));
    double r9 = round(trunc(d));
    
    /* Conditional calls */
    double r10 = (d > 3.0) ? trunc(d) : floor(d);
    double r11 = (f < 3.0) ? ceil(f) : round(f);
    
    /* As function arguments */
    double r12 = trunc(floor(ceil(d)));
    
    return (int)(r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 + r11 + r12);
}

int test_builtin_functions(void) {
    double d = vd;
    float f = vf;
    
    /* Builtin functions */
    long long r1 = __builtin_llround(d);
    long long r2 = __builtin_llrint(f);
    
    /* Complex part extraction */
    double _Complex c1 = vc_d;
    int _Complex c2 = vc_i;
    
    double r3 = __real__(c1);
    double r4 = __imag__(c1);
    double r5 = __real__(c2);
    double r6 = __imag__(c2);
    
    /* Mixed with other operations */
    double r7 = __real__(c1) + __imag__(c1);
    double r8 = trunc(__real__(c2)) + floor(__imag__(c2));
    
    return (int)(r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8);
}

int test_constant_contexts(void) {
    /* Force evaluation in constant contexts */
    const double cd = 2.5;
    const float cf = 3.5f;
    
    /* These should be folded during compilation */
    const double cr1 = trunc(cd);
    const double cr2 = floor(cd + 1.0);
    const double cr3 = ceil(cf);
    const double cr4 = round(cd * 2.0);
    
    /* In expressions */
    const int cr5 = (int)trunc(cd * 3.0);
    const int cr6 = (int)floor(cf * 2.0);
    
    /* In comparisons */
    const int cr7 = (trunc(cd) == 2);
    const int cr8 = (ceil(cf) == 4);
    
    return cr1 + cr2 + cr3 + cr4 + cr5 + cr6 + cr7 + cr8;
}

int test_edge_cases(void) {
    double d = vd;
    
    /* Edge cases */
    double r1 = trunc(0.0);      /* Exact zero */
    double r2 = floor(1.0);      /* Exact integer */
    double r3 = ceil(-2.3);      /* Negative value */
    double r4 = round(-2.5);     /* Negative half case */
    double r5 = nearbyint(1e10); /* Large value */
    double r6 = rint(-1e10);     /* Large negative */
    
    /* Exact integer arguments */
    double r7 = trunc(5);
    double r8 = floor(5);
    double r9 = ceil(5);
    
    /* Very small fractional parts */
    double r10 = trunc(5.0000001);
    double r11 = floor(5.9999999);
    double r12 = ceil(4.0000001);
    
    return (int)(r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 + r11 + r12);
}

#ifdef __cplusplus
int test_cpp_features(void) {
    /* C++ constexpr functions */
    constexpr double ce1 = cpp_trunc(3.7);
    constexpr double ce2 = cpp_floor(3.7);
    constexpr double ce3 = cpp_ceil(3.2);
    constexpr double ce4 = cpp_round(3.5);
    
    /* In template arguments */
    constexpr int tval = template_val;
    
    /* In static_assert */
    static_assert(cpp_trunc(4.9) == 4, "C++ trunc failed");
    static_assert(cpp_floor(4.9) == 4, "C++ floor failed");
    static_assert(cpp_ceil(4.1) == 5, "C++ ceil failed");
    static_assert(cpp_round(4.5) == 5, "C++ round failed");
    
    return (int)(ce1 + ce2 + ce3 + ce4 + tval);
}
#endif

int main(void) {
    int checksum = 0;
    
    /* Run all test functions */
    checksum += test_basic_functions();
    checksum += test_builtin_functions();
    checksum += test_constant_contexts();
    checksum += test_edge_cases();
    
    #ifdef __cplusplus
    checksum += test_cpp_features();
    #endif
    
    /* Use the global/static variables to prevent dead code elimination */
    checksum += (int)(g1 + g2 + g3 + g4 + g5 + g6);
    checksum += (int)(nested1 + nested2 + nested3 + nested4);
    checksum += (int)(cond1 + cond2);
    checksum += (int)blt1 + (int)blt2;
    checksum += (int)(complex_part1 + complex_part2 + complex_part3 + complex_part4);
    checksum += (int)(arith1 + arith2 + arith3);
    checksum += cmp1 + cmp2 + cmp3;
    
    /* Use enum values */
    checksum += ENUM_TRUNC + ENUM_FLOOR + ENUM_CEIL + ENUM_ROUND + ENUM_NEARBYINT + ENUM_RINT;
    
    /* Use array sizes */
    checksum += sizeof(buffer1) + sizeof(buffer2) + sizeof(buffer3);
    
    printf("Result: %d\n", checksum);
    return 0;
}
