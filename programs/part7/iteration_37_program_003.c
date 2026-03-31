/* Test program for integer-valued real function constant folding */
#include <stdio.h>
#include <math.h>
#include <complex.h>

/* Volatile variables to prevent premature folding */
volatile double vd = 3.14159;
volatile float vf = 2.71828f;
volatile int vi = 42;

/* Global constants for constant folding contexts */
const double cd = 4.5;
const float cf = -2.3f;
const int ci = 100;

/* Test 1: Basic integer-valued real function calls */
int test_basic_calls(void) {
    /* These should be folded during compilation */
    double d1 = trunc(cd);        /* 4.0 */
    double d2 = floor(cd);        /* 4.0 */
    double d3 = ceil(cd);         /* 5.0 */
    double d4 = round(cd);        /* 5.0 */
    double d5 = nearbyint(cd);    /* 4.0 */
    double d6 = rint(cd);         /* 4.0 */
    
    /* Mix with volatile to ensure some runtime evaluation */
    double d7 = trunc(vd);
    double d8 = floor(vd);
    
    /* Return checksum */
    return (int)(d1 + d2 + d3 + d4 + d5 + d6 + d7 + d8);
}

/* Test 2: Builtin functions with explicit integer returns */
long long test_builtin_calls(void) {
    /* These use __builtin functions that return long long */
    long long ll1 = __builtin_llround(cd);    /* 5 */
    long long ll2 = __builtin_llrint(cd);     /* 4 */
    
    /* With volatile */
    long long ll3 = __builtin_llround(vd);
    long long ll4 = __builtin_llrint(vd);
    
    return ll1 + ll2 + ll3 + ll4;
}

/* Test 3: Complex number real/imag parts */
int test_complex_parts(void) {
    /* Complex integer type */
    _Complex int ci = 3 + 4 * _Complex_I;
    
    /* These extract real/imag parts - integer-valued real operations */
    double r1 = __real__(ci);  /* 3.0 */
    double r2 = __imag__(ci);  /* 4.0 */
    
    /* Complex float */
    _Complex float cf = 1.5f + 2.5f * _Complex_I;
    float r3 = __real__(cf);   /* 1.5 */
    float r4 = __imag__(cf);   /* 2.5 */
    
    return (int)(r1 + r2 + r3 + r4);
}

/* Test 4: Nested calls to trigger recursive depth */
double test_nested_calls(void) {
    /* Multiple levels of nesting */
    double d1 = floor(ceil(cd));              /* floor(5.0) = 5.0 */
    double d2 = trunc(round(cf));             /* trunc(-2.0) = -2.0 */
    double d3 = nearbyint(rint(2.7));         /* nearbyint(3.0) = 3.0 */
    
    /* Deeper nesting */
    double d4 = floor(ceil(trunc(round(3.14))));  /* floor(ceil(trunc(3.0))) = 3.0 */
    
    /* With volatile in middle */
    double d5 = floor(ceil(vd));
    
    return d1 + d2 + d3 + d4 + d5;
}

/* Test 5: Calls in conditional expressions */
double test_conditional_calls(void) {
    /* Conditional operator with integer-valued calls */
    double d1 = (ci > 50) ? trunc(cd) : floor(cd);    /* trunc(4.5) = 4.0 */
    double d2 = (cf < 0) ? ceil(cf) : round(cf);      /* ceil(-2.3) = -2.0 */
    
    /* Nested conditionals */
    double d3 = (vi > 0) ? 
                ((vd > 3.0) ? nearbyint(vd) : rint(vd)) : 
                floor(vd);
    
    return d1 + d2 + d3;
}

/* Test 6: Calls as function arguments */
double test_argument_calls(double x) {
    /* Integer-valued calls as arguments to other calls */
    double d1 = trunc(floor(x));      /* trunc(floor(x)) */
    double d2 = round(ceil(x));       /* round(ceil(x)) */
    double d3 = nearbyint(rint(x));   /* nearbyint(rint(x)) */
    
    return d1 + d2 + d3;
}

/* Test 7: Mixed arithmetic with integer-valued calls */
double test_mixed_arithmetic(void) {
    /* Arithmetic expressions containing integer-valued calls */
    double d1 = trunc(cd) * 2.0;                 /* 4.0 * 2.0 = 8.0 */
    double d2 = floor(cf) / 2.0;                 /* -3.0 / 2.0 = -1.5 */
    double d3 = (ceil(vd) + round(2.3)) / 2.0;   /* (4.0 + 2.0) / 2.0 = 3.0 */
    
    /* More complex expression */
    double d4 = (trunc(5.9) * floor(3.2)) - (ceil(2.1) / nearbyint(4.6));
    /* (5.0 * 3.0) - (3.0 / 5.0) = 15.0 - 0.6 = 14.4 */
    
    return d1 + d2 + d3 + d4;
}

/* Test 8: Comparison expressions */
int test_comparisons(void) {
    /* Comparisons that should be folded */
    int b1 = trunc(cd) > floor(cd);      /* 4.0 > 4.0 = false (0) */
    int b2 = ceil(cf) == round(cf);      /* -2.0 == -2.0 = true (1) */
    int b3 = nearbyint(2.3) <= rint(2.7);/* 2.0 <= 3.0 = true (1) */
    
    /* Complex comparison */
    int b4 = (trunc(vd) * 2) < (floor(vd) + ceil(vd));
    
    return b1 + b2 + b3 + b4;
}

/* C++ specific tests (compile with g++) */
#ifdef __cplusplus
#include <type_traits>

/* Test 9: Template arguments and constexpr */
template<int N>
struct TestTemplate {
    static const int value = N;
};

constexpr int test_constexpr_calls() {
    /* constexpr context forces compile-time evaluation */
    constexpr double d1 = trunc(4.5);      /* 4.0 */
    constexpr double d2 = floor(4.5);      /* 4.0 */
    constexpr double d3 = ceil(4.5);       /* 5.0 */
    
    /* Use in template argument */
    constexpr int val = static_cast<int>(round(3.7));  /* 4 */
    
    return val + static_cast<int>(d1 + d2 + d3);
}

/* Test 10: Static assertions */
void test_static_asserts() {
    static_assert(trunc(5.9) == 5, "trunc should work at compile time");
    static_assert(floor(5.9) == 5, "floor should work at compile time");
    static_assert(ceil(5.1) == 6, "ceil should work at compile time");
    static_assert(round(5.5) == 6, "round should work at compile time");
}

#endif

/* Test 11: Array sizes (GNU extension) */
void test_array_sizes() {
    /* Using integer-valued calls in array sizes */
    char buffer1[(int)trunc(10.5)];        /* buffer1[10] */
    char buffer2[(int)floor(10.5)];        /* buffer2[10] */
    char buffer3[(int)ceil(10.5)];         /* buffer3[11] */
    char buffer4[(int)round(10.5)];        /* buffer4[11] */
    
    /* Use the arrays to prevent optimization */
    buffer1[0] = 'a';
    buffer2[0] = 'b';
    buffer3[0] = 'c';
    buffer4[0] = 'd';
}

/* Test 12: Enum values */
enum TestEnum {
    VAL1 = (int)trunc(10.5),      /* 10 */
    VAL2 = (int)floor(10.5),      /* 10 */
    VAL3 = (int)ceil(10.5),       /* 11 */
    VAL4 = (int)round(10.5)       /* 11 */
};

/* Test 13: Switch cases */
void test_switch_cases(int x) {
    switch(x) {
        case (int)trunc(1.5):   /* case 1: */
            break;
        case (int)floor(2.5):   /* case 2: */
            break;
        case (int)ceil(3.2):    /* case 4: */
            break;
        case (int)round(4.6):   /* case 5: */
            break;
    }
}

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    /* Run all tests and accumulate checksum */
    checksum += test_basic_calls();
    checksum += (int)test_builtin_calls();
    checksum += test_complex_parts();
    checksum += (int)test_nested_calls();
    checksum += (int)test_conditional_calls();
    checksum += (int)test_argument_calls(3.14);
    checksum += (int)test_mixed_arithmetic();
    checksum += test_comparisons();
    
    /* Test array sizes */
    test_array_sizes();
    
    /* Test switch cases */
    test_switch_cases(1);
    
    /* C++ specific tests */
    #ifdef __cplusplus
    checksum += test_constexpr_calls();
    test_static_asserts();
    
    /* Use template with integer-valued call result */
    TestTemplate<(int)trunc(7.8)> t;  /* TestTemplate<7> */
    checksum += t.value;
    #endif
    
    /* Use enum values */
    checksum += VAL1 + VAL2 + VAL3 + VAL4;
    
    printf("Result: %d\n", checksum);
    return 0;
}
