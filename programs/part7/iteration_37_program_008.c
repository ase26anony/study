/* Test program for integer-valued real function constant folding */
#include <stdio.h>
#include <math.h>
#include <complex.h>

/* Prevent premature constant folding */
volatile double vd = 3.14159;
volatile float vf = 2.71828f;
volatile int vi = 42;

/* Global variables to force constant folding in static initializers */
static double g1 = trunc(4.7);
static double g2 = floor(4.7);
static double g3 = ceil(4.7);
static double g4 = round(4.7);
static double g5 = nearbyint(4.7);
static double g6 = rint(4.7);

/* Complex types for __real__ and __imag__ */
static complex int ci = 3 + 4 * I;
static complex long cl = 5L + 6L * I;

/* Test 1: Basic integer-valued real functions in constant contexts */
int test_basic_functions(void) {
    /* Force constant folding with const variables */
    const double cd = 5.9;
    const float cf = -2.3f;
    
    /* Array sizes using integer-valued real functions */
    char buffer1[(int)trunc(10.5)];
    char buffer2[(int)floor(10.5)];
    char buffer3[(int)ceil(10.5)];
    
    /* Static assertions */
    static_assert(trunc(5.9) == 5, "trunc failed");
    static_assert(floor(5.9) == 5, "floor failed");
    static_assert(ceil(5.9) == 6, "ceil failed");
    static_assert(round(5.9) == 6, "round failed");
    
    /* Enum values */
    enum { 
        E_TRUNC = (int)trunc(cd),
        E_FLOOR = (int)floor(cd),
        E_CEIL = (int)ceil(cd),
        E_ROUND = (int)round(cf)
    };
    
    return E_TRUNC + E_FLOOR + E_CEIL + E_ROUND;
}

/* Test 2: Nested calls to integer-valued real functions */
int test_nested_calls(void) {
    const double x = 7.8;
    const double y = -3.2;
    
    /* Nested calls */
    double r1 = floor(ceil(x));      /* floor(8.0) = 8.0 */
    double r2 = trunc(round(y));     /* trunc(-3.0) = -3.0 */
    double r3 = round(trunc(x));     /* round(7.0) = 7.0 */
    double r4 = nearbyint(rint(y));  /* nearbyint(-3.0) = -3.0 */
    
    /* Multiple levels of nesting */
    double r5 = floor(ceil(trunc(round(9.7))));  /* floor(ceil(trunc(10.0))) = 10.0 */
    
    /* Use in arithmetic expressions */
    double r6 = (trunc(x) * 2) / floor(y + 4.0);  /* (7 * 2) / 1 = 14 */
    
    return (int)(r1 + r2 + r3 + r4 + r5 + r6);
}

/* Test 3: Conditional expressions with integer-valued real functions */
int test_conditional_calls(void) {
    const double a = 5.6;
    const double b = -2.4;
    const int cond = vi > 0;
    
    /* Conditional operator with integer-valued real calls */
    double r1 = cond ? trunc(a) : floor(b);
    double r2 = !cond ? ceil(a) : round(b);
    
    /* Nested conditional with calls */
    double r3 = (a > b) ? floor(a) : ceil(b);
    double r4 = (cond ? trunc : floor)(a);  /* Function pointer in conditional */
    
    /* Conditional with builtins */
    long long r5 = cond ? __builtin_llround(a) : __builtin_llround(b);
    long long r6 = __builtin_llrint(cond ? a : b);
    
    return (int)(r1 + r2 + r3 + r4) + (int)(r5 + r6);
}

/* Test 4: Builtin functions with different argument counts */
int test_builtin_variants(void) {
    /* Builtins that may have different numbers of arguments */
    double r1 = __builtin_trunc(4.7);
    double r2 = __builtin_floor(4.7);
    double r3 = __builtin_ceil(4.7);
    double r4 = __builtin_round(4.7);
    
    /* Long return variants */
    long r5 = __builtin_lround(4.7);
    long long r6 = __builtin_llround(4.7);
    long r7 = __builtin_lrint(4.7);
    long long r8 = __builtin_llrint(4.7);
    
    /* Complex part extraction */
    double r9 = __real__ ci;
    double r10 = __imag__ ci;
    double r11 = __real__ cl;
    double r12 = __imag__ cl;
    
    return (int)(r1 + r2 + r3 + r4 + r9 + r10 + r11 + r12) + 
           (int)(r5 + r6 + r7 + r8);
}

/* Test 5: Integer-valued real functions in loop bounds */
int test_loop_bounds(void) {
    int sum = 0;
    
    /* Loop bound from integer-valued real function */
    const int limit1 = (int)floor(vd + 1.0);  /* floor(4.14159) = 4 */
    for (int i = 0; i < limit1; i++) {
        sum += i;
    }
    
    /* Dynamic loop bound with integer-valued real call */
    int limit2 = (int)ceil(vf * 2.0);  /* ceil(5.43656) = 6 */
    for (int i = 0; i < limit2; i++) {
        sum += i * 2;
    }
    
    /* Nested loop with different bounds */
    int limit3 = (int)trunc(vd * 2.0);  /* trunc(6.28318) = 6 */
    for (int i = 0; i < limit3; i++) {
        for (int j = 0; j < (int)round(vf + 1.0); j++) {  /* round(3.71828) = 4 */
            sum += i * j;
        }
    }
    
    return sum;
}

/* Test 6: Mixed expressions with comparisons */
int test_comparisons(void) {
    const double a = 5.6;
    const double b = -2.4;
    int result = 0;
    
    /* Direct comparisons */
    if (ceil(a) > floor(b)) result += 1;
    if (trunc(a) != round(b)) result += 2;
    if (nearbyint(a) == 6.0) result += 4;
    if (rint(b) == -2.0) result += 8;
    
    /* Complex comparisons */
    if (__real__ ci == 3 && __imag__ ci == 4) result += 16;
    if (__real__ cl == 5L && __imag__ cl == 6L) result += 32;
    
    /* Comparison with arithmetic */
    if ((trunc(a) * 2) > floor(b + 10.0)) result += 64;
    
    return result;
}

/* Test 7: Template and constexpr for C++ (if compiled as C++) */
#ifdef __cplusplus
template<typename T>
constexpr T template_floor(T x) {
    return floor(x);
}

template<typename T>
constexpr T template_ceil(T x) {
    return ceil(x);
}

int test_template_constexpr(void) {
    constexpr double d1 = template_floor(9.9);
    constexpr double d2 = template_ceil(9.1);
    constexpr int arr1[(int)d1] = {0};
    constexpr int arr2[(int)d2] = {0};
    
    static_assert(d1 == 9.0, "template floor failed");
    static_assert(d2 == 10.0, "template ceil failed");
    
    return (int)(d1 + d2) + sizeof(arr1) + sizeof(arr2);
}
#endif

/* Test 8: Very large and edge case values */
int test_edge_cases(void) {
    /* Large values */
    double r1 = trunc(1e20 + 0.7);
    double r2 = floor(1e20 + 0.7);
    double r3 = ceil(1e20 + 0.7);
    double r4 = round(1e20 + 0.7);
    
    /* Exact integers */
    double r5 = trunc(100.0);
    double r6 = floor(100.0);
    double r7 = ceil(100.0);
    double r8 = round(100.0);
    
    /* Negative values */
    double r9 = trunc(-5.9);
    double r10 = floor(-5.9);
    double r11 = ceil(-5.9);
    double r12 = round(-5.9);
    
    /* Zero */
    double r13 = trunc(0.0);
    double r14 = floor(0.0);
    double r15 = ceil(0.0);
    double r16 = round(0.0);
    
    return (int)(r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + 
                 r9 + r10 + r11 + r12 + r13 + r14 + r15 + r16);
}

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    /* Run all tests and accumulate results */
    checksum += test_basic_functions();
    checksum += test_nested_calls();
    checksum += test_conditional_calls();
    checksum += test_builtin_variants();
    checksum += test_loop_bounds();
    checksum += test_comparisons();
    checksum += test_edge_cases();
    
    #ifdef __cplusplus
    checksum += test_template_constexpr();
    #endif
    
    /* Add global initializers */
    checksum += (int)(g1 + g2 + g3 + g4 + g5 + g6);
    
    printf("Result: %d\n", checksum);
    
    /* Verify some results at runtime */
    if (trunc(5.9) != 5) return 1;
    if (floor(5.9) != 5) return 1;
    if (ceil(5.9) != 6) return 1;
    if (round(5.9) != 6) return 1;
    if (__real__ ci != 3) return 1;
    if (__imag__ ci != 4) return 1;
    
    return 0;
}
