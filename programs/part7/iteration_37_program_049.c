/* Test program for integer-valued real function constant folding */
#include <math.h>
#include <stdio.h>
#include <complex.h>

/* Prevent premature constant folding */
volatile double vd = 3.14159;
volatile float vf = 2.71828f;
volatile int vi = 42;

/* Test 1: Basic integer-valued real functions in constant contexts */
static int test_basic_functions(void) {
    /* Use in static initializers */
    static const double d1 = trunc(5.9);
    static const double d2 = floor(5.9);
    static const double d3 = ceil(5.1);
    static const double d4 = round(5.5);
    static const double d5 = nearbyint(5.3);
    static const double d6 = rint(5.7);
    
    /* Use in array sizes */
    char arr1[(int)trunc(10.5)];
    char arr2[(int)floor(10.5)];
    char arr3[(int)ceil(10.1)];
    
    /* Static assertions */
    static_assert(trunc(5.9) == 5, "trunc failed");
    static_assert(floor(5.9) == 5, "floor failed");
    static_assert(ceil(5.1) == 6, "ceil failed");
    static_assert(round(5.5) == 6, "round failed");
    
    return (int)(d1 + d2 + d3 + d4 + d5 + d6 + 
                 sizeof(arr1) + sizeof(arr2) + sizeof(arr3));
}

/* Test 2: Nested calls and complex expressions */
static int test_nested_calls(void) {
    /* Nested calls */
    double n1 = floor(ceil(4.3));
    double n2 = trunc(round(3.7));
    double n3 = nearbyint(rint(2.9));
    
    /* Calls as arguments to other calls */
    double n4 = round(trunc(6.8));
    double n5 = ceil(floor(7.2));
    
    /* Arithmetic with integer-valued real functions */
    double n6 = (trunc(8.9) * 2) / floor(4.5);
    double n7 = ceil(3.2) + floor(2.8) - round(1.5);
    
    /* Comparisons that should fold */
    int cmp1 = (ceil(4.1) > floor(3.9)) ? 1 : 0;
    int cmp2 = (trunc(5.5) == 5) ? 1 : 0;
    
    return (int)(n1 + n2 + n3 + n4 + n5 + n6 + n7 + cmp1 + cmp2);
}

/* Test 3: Conditional expressions with integer-valued real calls */
static int test_conditional_calls(void) {
    /* Use volatile to prevent front-end folding */
    double x = vd;
    double y = vf;
    
    /* Conditional operator with integer-valued real calls */
    double c1 = (x > 0) ? trunc(x) : floor(y);
    double c2 = (y < 0) ? ceil(y) : round(x);
    double c3 = (vi % 2) ? nearbyint(x) : rint(y);
    
    /* Nested conditional with calls */
    double c4 = (x > y) ? 
                (trunc(x) > floor(y) ? ceil(x) : round(y)) :
                (floor(y) > trunc(x) ? nearbyint(x) : rint(y));
    
    return (int)(c1 + c2 + c3 + c4);
}

/* Test 4: Builtin functions with different argument counts */
static int test_builtin_functions(void) {
    /* Builtins that return long long */
    long long ll1 = __builtin_llround(9.7);
    long long ll2 = __builtin_llrint(8.3);
    
    /* Complex number real/imag parts */
    complex int ci = 3 + 4 * I;
    double cr = __real__(ci);
    double ci_imag = __imag__(ci);
    
    /* Mix with other integer-valued functions */
    double m1 = trunc(cr) + floor(ci_imag);
    double m2 = round(__real__(ci * 2));
    
    return (int)(ll1 + ll2 + m1 + m2);
}

/* Test 5: Various argument types and values */
static int test_various_arguments(void) {
    /* Integer arguments */
    double a1 = floor(5);
    double a2 = trunc(2);
    
    /* Real arguments that are exact integers */
    double a3 = ceil(4.0);
    double a4 = round(6.0);
    
    /* Real arguments with fractional parts */
    double a5 = floor(4.7);
    double a6 = trunc(3.14159);
    
    /* Negative values */
    double a7 = round(-2.3);
    double a8 = ceil(-3.7);
    double a9 = floor(-4.2);
    
    /* Large values */
    double a10 = trunc(1e10 + 0.7);
    double a11 = floor(1e10 - 0.3);
    
    /* Zero */
    double a12 = nearbyint(0.0);
    double a13 = rint(-0.0);
    
    return (int)(a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9 + a10 + a11 + a12 + a13);
}

/* Test 6: Template and constexpr contexts (C++ specific) */
#ifdef __cplusplus
template<typename T>
constexpr T template_floor(T x) {
    return floor(x);
}

template<typename T>
constexpr T template_round(T x) {
    return round(x);
}

static int test_template_constexpr(void) {
    /* Use in template arguments */
    constexpr double t1 = template_floor(9.8);
    constexpr double t2 = template_round(7.3);
    
    /* Use in constexpr calculations */
    constexpr double t3 = trunc(template_floor(8.9));
    constexpr double t4 = ceil(template_round(6.2));
    
    /* Array size from template function */
    char tarr1[(int)template_floor(15.7)];
    char tarr2[(int)template_round(12.3)];
    
    /* Static assertion with template function */
    static_assert(template_floor(10.9) == 10, "template floor failed");
    static_assert(template_round(10.5) == 11, "template round failed");
    
    return (int)(t1 + t2 + t3 + t4 + sizeof(tarr1) + sizeof(tarr2));
}
#endif

/* Test 7: Mixed expressions in loop bounds */
static int test_loop_bounds(void) {
    int sum = 0;
    
    /* Use integer-valued real functions in loop bounds */
    for (int i = (int)trunc(0.0); i < (int)floor(10.3); i++) {
        sum += i;
    }
    
    for (int i = (int)ceil(1.1); i <= (int)round(5.4); i++) {
        sum += i * 2;
    }
    
    /* Nested loop with different bounds */
    int limit = (int)nearbyint(3.7);
    for (int i = 0; i < limit; i++) {
        for (int j = (int)floor(0.5); j < (int)trunc(2.9); j++) {
            sum += i * j;
        }
    }
    
    return sum;
}

/* Test 8: Switch cases from integer-valued real calls */
static int test_switch_cases(void) {
    double val = vd;
    int result = 0;
    
    switch ((int)trunc(val)) {
        case 3:
            result += 10;
            break;
        case 4:
            result += 20;
            break;
        default:
            result += 30;
    }
    
    switch ((int)round(val)) {
        case 3:
            result += 100;
            break;
        case 4:
            result += 200;
            break;
        default:
            result += 300;
    }
    
    return result;
}

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    /* Run all tests */
    checksum += test_basic_functions();
    checksum += test_nested_calls();
    checksum += test_conditional_calls();
    checksum += test_builtin_functions();
    checksum += test_various_arguments();
    
    #ifdef __cplusplus
    checksum += test_template_constexpr();
    #endif
    
    checksum += test_loop_bounds();
    checksum += test_switch_cases();
    
    /* Add some direct calls to ensure they're processed */
    checksum += (int)trunc(100.7);
    checksum += (int)floor(100.2);
    checksum += (int)ceil(99.9);
    checksum += (int)round(50.5);
    
    /* Complex expression that should trigger folding */
    double complex_expr = trunc(floor(ceil(round(nearbyint(rint(25.7))))));
    checksum += (int)complex_expr;
    
    printf("Result: %d\n", checksum);
    
    /* Verify at runtime */
    if (checksum != 0) {
        return 0;  /* Success */
    }
    return 1;  /* Should not happen */
}
