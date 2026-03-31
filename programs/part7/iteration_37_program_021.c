/* Test program for integer-valued real function constant folding */
#include <stdio.h>
#include <math.h>
#include <complex.h>

/* Global volatile to prevent premature optimization */
volatile double g_input = 3.14159;
volatile double g_input2 = 2.71828;

/* Test 1: Basic integer-valued real functions in constant contexts */
static int test_basic_functions(void) {
    /* Array sizes using integer-valued real functions */
    char buffer1[(int)trunc(10.7)];
    char buffer2[(int)floor(9.2)];
    char buffer3[(int)ceil(8.1)];
    char buffer4[(int)round(7.6)];
    
    /* Static assertions */
    static_assert(trunc(5.9) == 5, "trunc failed");
    static_assert(floor(5.9) == 5, "floor failed");
    static_assert(ceil(5.1) == 6, "ceil failed");
    static_assert(round(5.5) == 6, "round failed");
    
    /* Enum values */
    enum {
        VAL_TRUNC = (int)trunc(100.7),
        VAL_FLOOR = (int)floor(99.2),
        VAL_CEIL = (int)ceil(98.1),
        VAL_ROUND = (int)round(97.6)
    };
    
    return VAL_TRUNC + VAL_FLOOR + VAL_CEIL + VAL_ROUND;
}

/* Test 2: Nested calls and complex expressions */
static int test_nested_calls(void) {
    double x = g_input;
    
    /* Nested integer-valued real function calls */
    double result1 = floor(ceil(x));
    double result2 = trunc(round(x * 2.0));
    double result3 = nearbyint(rint(x + 1.0));
    
    /* Complex expressions with multiple calls */
    double result4 = (trunc(x) * 2.0) / floor(x + 1.0);
    double result5 = ceil(result1) + floor(result2) - round(result3);
    
    /* Conditional expressions with integer-valued calls */
    double result6 = (x > 3.0) ? trunc(x) : floor(x);
    double result7 = (result4 > 2.0) ? ceil(result4) : round(result4);
    
    return (int)(result1 + result2 + result3 + result4 + result5 + result6 + result7);
}

/* Test 3: Builtin functions with different argument counts */
static int test_builtin_functions(void) {
    double x = g_input2;
    
    /* Builtins with different argument patterns */
    long long result1 = __builtin_llround(x);
    long long result2 = __builtin_llrint(x * 2.0);
    
    /* Mix with standard functions */
    double result3 = trunc(__builtin_llround(x) / 2.0);
    double result4 = floor(result1 + result2);
    
    return (int)(result1 + result2 + result3 + result4);
}

/* Test 4: Complex number operations */
static int test_complex_operations(void) {
    /* Complex integer type */
    _Complex int c1 = 3 + 4 * I;
    _Complex double c2 = 5.5 + 6.6 * I;
    
    /* Real/imag part extractors - these are integer-valued */
    int real_part1 = __real__ c1;
    int imag_part1 = __imag__ c1;
    double real_part2 = __real__ c2;
    double imag_part2 = __imag__ c2;
    
    /* Apply integer-valued functions to complex parts */
    double result1 = floor(__real__ c2);
    double result2 = ceil(__imag__ c2);
    double result3 = trunc(real_part2) + round(imag_part2);
    
    return real_part1 + imag_part1 + (int)result1 + (int)result2 + (int)result3;
}

/* Test 5: Template and constexpr contexts (C++ specific) */
#ifdef __cplusplus
template<typename T>
constexpr T template_floor(T x) {
    return floor(x);
}

template<typename T>
constexpr T template_ceil(T x) {
    return ceil(x);
}

static int test_template_constexpr(void) {
    constexpr double val1 = template_floor(10.7);
    constexpr double val2 = template_ceil(9.2);
    constexpr double val3 = trunc(template_floor(8.9));
    constexpr double val4 = round(template_ceil(7.4));
    
    /* Use in array size */
    char buffer[(int)val1 + (int)val2];
    
    return (int)(val1 + val2 + val3 + val4);
}
#endif

/* Test 6: Mixed argument types and values */
static int test_mixed_arguments(void) {
    /* Integer arguments */
    double r1 = floor(5);
    double r2 = trunc(2);
    
    /* Exact integer real arguments */
    double r3 = ceil(4.0);
    double r4 = round(6.0);
    
    /* Fractional arguments */
    double r5 = floor(4.7);
    double r6 = trunc(3.14159);
    
    /* Negative values */
    double r7 = round(-2.3);
    double r8 = ceil(-3.7);
    
    /* Large values */
    double r9 = floor(1e10 + 0.5);
    double r10 = trunc(1e15 - 0.2);
    
    /* Zero and one */
    double r11 = nearbyint(0.0);
    double r12 = rint(1.0);
    
    return (int)(r1 + r2 + r3 + r4 + r5 + r6 + r7 + r8 + r9 + r10 + r11 + r12);
}

/* Test 7: Recursive depth testing */
static int test_recursive_depth(void) {
    double x = g_input;
    
    /* Deeply nested calls to test recursion depth */
    double result1 = floor(ceil(trunc(round(nearbyint(rint(x))))));
    
    /* Mixed nesting with arithmetic */
    double result2 = trunc(floor(x) + ceil(x * 2) - round(x / 2));
    
    /* Conditional nested calls */
    double result3 = (x > 2.0) ? 
                     floor(ceil(x)) : 
                     trunc(round(x));
    
    /* Multiple argument positions */
    double result4 = floor(x) * ceil(x + 1.0) / trunc(x * 0.5);
    
    return (int)(result1 + result2 + result3 + result4);
}

/* Test 8: Loop bounds and conditional contexts */
static int test_loop_bounds(void) {
    int sum = 0;
    
    /* Use integer-valued calls in loop bounds */
    int limit1 = (int)floor(g_input + 5.0);
    int limit2 = (int)ceil(g_input2 * 2.0);
    
    for (int i = (int)trunc(1.0); i < limit1; i += (int)round(1.0)) {
        sum += i;
    }
    
    /* Conditional based on integer-valued calls */
    if (ceil(g_input) > floor(g_input2)) {
        sum += (int)trunc(g_input * 10.0);
    }
    
    /* Switch with computed cases */
    switch ((int)round(g_input)) {
        case 3: sum += 100; break;
        case 4: sum += 200; break;
        default: sum += 300;
    }
    
    return sum;
}

/* Main driver function */
int main(void) {
    int checksum = 0;
    
    printf("Starting integer-valued real function tests...\n");
    
    /* Run all tests and accumulate checksum */
    checksum += test_basic_functions();
    checksum += test_nested_calls();
    checksum += test_builtin_functions();
    checksum += test_complex_operations();
    
    #ifdef __cplusplus
    checksum += test_template_constexpr();
    #endif
    
    checksum += test_mixed_arguments();
    checksum += test_recursive_depth();
    checksum += test_loop_bounds();
    
    printf("Final checksum: %d\n", checksum);
    
    /* Verify with a simple computation using all patterns */
    volatile double verify_input = 2.5;
    double verify_result = 
        floor(ceil(verify_input)) + 
        trunc(round(verify_input * 2)) + 
        __builtin_llround(verify_input) +
        __real__ (3 + 4 * I);
    
    printf("Verification result: %.2f\n", verify_result);
    
    return (checksum > 0) ? 0 : 1;
}
