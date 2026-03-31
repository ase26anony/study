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
    VAL_TRUNC = (int)trunc(5.9),           /* Should be 5 */
    VAL_FLOOR = (int)floor(4.7),           /* Should be 4 */
    VAL_CEIL = (int)ceil(4.1),             /* Should be 5 */
    VAL_ROUND = (int)round(3.5),           /* Should be 4 */
    VAL_NEARBYINT = (int)nearbyint(2.3),   /* Should be 2 */
    VAL_RINT = (int)rint(6.8),             /* Should be 7 */
};

/* Test 2: Array sizes using integer-valued functions */
char buffer1[(int)floor(10.5)];            /* Size 10 */
char buffer2[(int)ceil(7.2)];              /* Size 8 */
char buffer3[(int)trunc(9.9)];             /* Size 9 */

/* Test 3: Static assertions */
static_assert(trunc(5.9) == 5, "trunc failed");
static_assert(floor(4.7) == 4, "floor failed");
static_assert(ceil(4.1) == 5, "ceil failed");
static_assert(round(3.5) == 4, "round failed");

/* Test 4: Global initializers with const variables */
const double cd = 8.6;
const float cf = 3.3f;
static double g1 = trunc(cd);              /* Should be 8.0 */
static float g2 = floor(cf);               /* Should be 3.0 */
static double g3 = ceil(cd);               /* Should be 9.0 */
static double g4 = round(cd);              /* Should be 9.0 */

/* Test 5: Nested calls */
static double test_nested(double x) {
    /* Nested integer-valued calls */
    return floor(ceil(trunc(round(x))));
}

/* Test 6: Calls within conditional expressions */
static double test_conditional(double x, double y, int flag) {
    return flag ? trunc(x) : floor(y);
}

/* Test 7: Builtin functions with different argument counts */
static long long test_builtins(double x) {
    /* __builtin_llround has 1 argument */
    long long r1 = __builtin_llround(x);
    /* __builtin_llrint has 1 argument */
    long long r2 = __builtin_llrint(x);
    return r1 + r2;
}

/* Test 8: Complex number real/imag parts */
static int test_complex(_Complex int ci, _Complex double cd) {
    /* __real__ and __imag__ on complex types */
    int real_part = __real__ ci;
    int imag_part = __imag__ ci;
    double dreal = __real__ cd;
    double dimag = __imag__ cd;
    
    /* Combine with integer-valued functions */
    return (int)trunc(dreal) + (int)floor(dimag) + real_part + imag_part;
}

/* Test 9: Mixed expressions with arithmetic */
static double test_mixed(double x, double y) {
    return (trunc(x) * 2.0) / floor(y) + ceil(x + y) - round(x * y);
}

/* Test 10: Template metaprogramming (C++ only) */
#ifdef __cplusplus
template<double X>
struct IntegerValue {
    static constexpr int truncated = static_cast<int>(trunc(X));
    static constexpr int floored = static_cast<int>(floor(X));
    static constexpr int ceiled = static_cast<int>(ceil(X));
};

constexpr int template_test() {
    return IntegerValue<7.8>::truncated + 
           IntegerValue<7.8>::floored + 
           IntegerValue<7.8>::ceiled;
}
#endif

/* Test 11: Large values and edge cases */
static double test_edge_cases() {
    double large = 1e15;
    double small = 1e-15;
    double negative = -3.7;
    
    return trunc(large) + floor(negative) + ceil(small) + round(0.0);
}

/* Test 12: Recursive depth testing */
static double test_recursive_depth(double x, int depth) {
    if (depth <= 0) return x;
    /* Create deep nesting of integer-valued calls */
    return test_recursive_depth(trunc(x), depth - 1) +
           test_recursive_depth(floor(x + 1.0), depth - 1);
}

/* Test 13: Inline assembly to prevent optimization */
static double test_volatile_mix() {
    double x = vd;
    float y = vf;
    
    /* Mix volatile and non-volatile in expressions */
    return trunc(x + y) + floor((double)vi) + ceil(vd * 0.5);
}

/* Test 14: Switch statement with integer-valued calls */
static double test_switch(double x, int choice) {
    switch (choice) {
        case 0: return trunc(x);
        case 1: return floor(x);
        case 2: return ceil(x);
        case 3: return round(x);
        default: return nearbyint(x);
    }
}

/* Test 15: Loop with integer-valued calls in bounds */
static int test_loop() {
    int sum = 0;
    for (int i = (int)floor(0.5); i < (int)ceil(10.3); i++) {
        sum += (int)trunc(i * 1.5);
    }
    return sum;
}

/* Main driver that exercises all tests */
int main() {
    int checksum = 0;
    
    /* Test 1: Enum values */
    checksum += VAL_TRUNC + VAL_FLOOR + VAL_CEIL + VAL_ROUND + VAL_NEARBYINT + VAL_RINT;
    
    /* Test 2: Array sizes (implicitly tested) */
    checksum += sizeof(buffer1) + sizeof(buffer2) + sizeof(buffer3);
    
    /* Test 4: Global initializers */
    checksum += (int)g1 + (int)g2 + (int)g3 + (int)g4;
    
    /* Test 5: Nested calls */
    checksum += (int)test_nested(3.7);
    
    /* Test 6: Conditional calls */
    checksum += (int)test_conditional(4.2, 5.8, 1);
    checksum += (int)test_conditional(4.2, 5.8, 0);
    
    /* Test 7: Builtins */
    checksum += (int)test_builtins(3.7);
    
    /* Test 8: Complex numbers */
    checksum += test_complex(3 + 4 * I, 1.5 + 2.5 * I);
    
    /* Test 9: Mixed expressions */
    checksum += (int)test_mixed(3.2, 4.7);
    
    /* Test 10: Template (C++ only) */
    #ifdef __cplusplus
    checksum += template_test();
    #endif
    
    /* Test 11: Edge cases */
    checksum += (int)test_edge_cases();
    
    /* Test 12: Recursive depth */
    checksum += (int)test_recursive_depth(5.3, 3);
    
    /* Test 13: Volatile mix */
    checksum += (int)test_volatile_mix();
    
    /* Test 14: Switch statement */
    for (int i = 0; i < 5; i++) {
        checksum += (int)test_switch(3.7, i);
    }
    
    /* Test 15: Loop */
    checksum += test_loop();
    
    printf("Result: %d\n", checksum);
    
    /* Additional compile-time tests */
    constexpr double ct1 = trunc(9.99);
    constexpr double ct2 = floor(9.01);
    constexpr double ct3 = ceil(8.1);
    constexpr double ct4 = round(6.5);
    
    static_assert(ct1 == 9.0, "compile-time trunc failed");
    static_assert(ct2 == 9.0, "compile-time floor failed");
    static_assert(ct3 == 9.0, "compile-time ceil failed");
    static_assert(ct4 == 7.0, "compile-time round failed");
    
    return 0;
}
