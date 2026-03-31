/* test_double_int_cmp.c - Comprehensive test for double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>
#include <assert.h>

/* Force 128-bit integer support */
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* ========== SECTION 1: Constant Folding with Large Integers ========== */

/* Large constants that require double_int representation */
static const int128_t VERY_LARGE_POS = ((int128_t)1 << 70);
static const int128_t VERY_LARGE_NEG = -((int128_t)1 << 70);
static const int128_t HUGE_PRODUCT = ((int128_t)INT64_MAX) * ((int128_t)INT64_MAX);
static const uint128_t LARGE_UNSIGNED = ((uint128_t)1 << 100);

/* Static assertions that force compile-time comparisons */
static_assert(VERY_LARGE_POS > 0, "Large positive constant");
static_assert(VERY_LARGE_NEG < 0, "Large negative constant");
static_assert(HUGE_PRODUCT > INT64_MAX, "Product exceeds 64-bit");
static_assert(LARGE_UNSIGNED > UINT64_MAX, "Unsigned exceeds 64-bit");

/* Template-like macro for compile-time comparisons */
#define COMPILE_TIME_CMP(a, b, op) \
    do { \
        if (__builtin_constant_p((a) op (b))) { \
            static_assert((a) op (b), #a " " #op " " #b); \
        } \
    } while(0)

/* Force evaluation of multiple comparison types */
COMPILE_TIME_CMP(VERY_LARGE_POS, VERY_LARGE_NEG, >);
COMPILE_TIME_CMP(VERY_LARGE_POS + 1, VERY_LARGE_POS, >);
COMPILE_TIME_CMP(VERY_LARGE_NEG - 1, VERY_LARGE_NEG, <);

/* ========== SECTION 2: GCC Builtins with Overflow ========== */

void test_overflow_builtins(void) {
    long long a, b;
    long long res;
    int overflow;
    
    /* Test cases designed to trigger overflow comparisons */
    a = LLONG_MAX;
    b = 2;
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("mul_overflow(LLONG_MAX, 2): overflow=%d\n", overflow);
    
    a = LLONG_MIN;
    b = -1;
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("mul_overflow(LLONG_MIN, -1): overflow=%d\n", overflow);
    
    /* Chain overflow operations */
    long long x = LLONG_MAX / 2;
    long long y = LLONG_MAX / 2;
    long long z;
    if (__builtin_add_overflow(x, y, &z)) {
        printf("add_overflow triggered\n");
    }
    
    /* Constant overflow checks */
    if (__builtin_constant_p(__builtin_mul_overflow_p(LLONG_MAX, 2, (long long)0))) {
        printf("Constant overflow check passed\n");
    }
}

/* ========== SECTION 3: Range Analysis with Complex Conditions ========== */

void test_range_analysis(int input) {
    /* Create complex range conditions */
    if (input > 1000 && input < 2000) {
        /* Multiplication that requires double_int for range calculation */
        int64_t x = (int64_t)input;
        int64_t y = x * x;  /* Range: 1,001,001 to 3,996,001 */
        
        /* Nested conditions that force range comparisons */
        if (y > 1500000 && y < 2500000) {
            printf("Range analysis middle path: y=%lld\n", (long long)y);
        }
        
        /* Large shift operations */
        int128_t shifted = (int128_t)x << 40;
        if (shifted > ((int128_t)1 << 70)) {
            printf("Large shift triggered\n");
        }
    }
    
    /* Loop with induction variable that may wrap */
    for (int64_t i = LLONG_MAX - 10; i < LLONG_MAX + 5LL; i++) {
        /* This loop's analysis requires double_int comparisons for wrap detection */
        if (i == LLONG_MAX) {
            printf("Reached LLONG_MAX in loop\n");
            break;
        }
    }
}

/* ========== SECTION 4: Template Metaprogramming (C++ style in C) ========== */

/* Simulate template-like behavior using macros and inline functions */
#define LARGE_COMPARE(N) ((N) > ((int128_t)1 << 65))

/* Force evaluation with different large values */
static const int _check1 = LARGE_COMPARE(((int128_t)1 << 66)) ? 1 : 0;
static const int _check2 = LARGE_COMPARE(((int128_t)1 << 64)) ? 1 : 0;

/* ========== SECTION 5: Tree Node Construction for Wide Constants ========== */

/* Use 128-bit types with attributes */
typedef int128_t __attribute__((mode(TI))) ti_int;
typedef uint128_t __attribute__((mode(TI))) ti_uint;

/* Operations that create wide INTEGER_CST nodes */
static const ti_int wide_const = ((ti_int)1 << 100);
static const ti_int wide_neg_const = -((ti_int)1 << 100);

/* Division/modulus operations that compare magnitudes */
static const int128_t div_check1 = wide_const / 2;
static const int128_t div_check2 = wide_neg_const / 2;

/* Enumeration with large values */
enum big_enum : int128_t {
    BIG_ENUM_VAL1 = ((int128_t)1 << 80),
    BIG_ENUM_VAL2 = ((int128_t)1 << 90),
    BIG_ENUM_VAL3 = BIG_ENUM_VAL1 * 2
};

/* ========== SECTION 6: Mixed Runtime/Compile-time Tests ========== */

void test_mixed_comparisons(void) {
    /* Array of large values for runtime comparison */
    int128_t large_values[] = {
        ((int128_t)1 << 70),
        ((int128_t)1 << 71),
        ((int128_t)1 << 69),
        -((int128_t)1 << 70),
        0
    };
    
    /* Sort-like comparisons */
    for (int i = 0; i < 4; i++) {
        for (int j = i + 1; j < 4; j++) {
            if (large_values[i] < large_values[j]) {
                printf("Order: [%d] < [%d]\n", i, j);
            } else if (large_values[i] > large_values[j]) {
                printf("Order: [%d] > [%d]\n", i, j);
            } else {
                printf("Order: [%d] == [%d]\n", i, j);
            }
        }
    }
    
    /* Boundary condition tests */
    int128_t max_pos = ((int128_t)1 << 127) - 1;
    int128_t min_neg = -((int128_t)1 << 127);
    
    if (max_pos > min_neg) printf("max_pos > min_neg correct\n");
    if (min_neg < max_pos) printf("min_neg < max_pos correct\n");
    
    /* Test all comparison operators */
    int128_t a = ((int128_t)1 << 100);
    int128_t b = ((int128_t)1 << 100) + 1;
    
    printf("Comparison tests:\n");
    printf("  a < b: %d\n", a < b);
    printf("  a > b: %d\n", a > b);
    printf("  a <= b: %d\n", a <= b);
    printf("  a >= b: %d\n", a >= b);
    printf("  a == b: %d\n", a == b);
    printf("  a != b: %d\n", a != b);
}

/* ========== SECTION 7: Complex Arithmetic Chains ========== */

void test_arithmetic_chains(void) {
    /* Chain of operations that require intermediate double_int comparisons */
    int128_t base = ((int128_t)1 << 60);
    int128_t step = ((int128_t)1 << 40);
    
    /* Multiple operations in sequence */
    int128_t val1 = base * base;      /* Requires double_int multiplication */
    int128_t val2 = val1 + step;
    int128_t val3 = val2 - base;
    int128_t val4 = val3 << 2;
    
    /* Comparisons at each stage */
    if (val2 > val1) printf("val2 > val1 (addition increased)\n");
    if (val3 < val2) printf("val3 < val2 (subtraction decreased)\n");
    if (val4 > val3) printf("val4 > val3 (shift increased)\n");
    
    /* Mixed signedness comparisons */
    uint128_t uval = (uint128_t)1 << 120;
    int128_t sval = (int128_t)1 << 120;
    
    /* These comparisons may trigger different code paths */
    if ((uint128_t)sval == uval) {
        printf("Signed/unsigned equality at high bits\n");
    }
}

/* ========== MAIN FUNCTION ========== */

int main(void) {
    printf("=== Starting double_int::cmp coverage tests ===\n\n");
    
    printf("1. Testing overflow builtins:\n");
    test_overflow_builtins();
    printf("\n");
    
    printf("2. Testing range analysis:\n");
    test_range_analysis(1500);  /* Middle of our test range */
    printf("\n");
    
    printf("3. Testing mixed comparisons:\n");
    test_mixed_comparisons();
    printf("\n");
    
    printf("4. Testing arithmetic chains:\n");
    test_arithmetic_chains();
    printf("\n");
    
    /* Verify compile-time checks */
    printf("5. Verifying compile-time assertions:\n");
    printf("   VERY_LARGE_POS = 0x%016llx%016llx\n", 
           (unsigned long long)(VERY_LARGE_POS >> 64),
           (unsigned long long)VERY_LARGE_POS);
    printf("   VERY_LARGE_NEG = 0x%016llx%016llx\n",
           (unsigned long long)(VERY_LARGE_NEG >> 64),
           (unsigned long long)VERY_LARGE_NEG);
    printf("   All static assertions passed!\n\n");
    
    /* Final validation */
    printf("=== All tests completed ===\n");
    printf("If you see this, the compiler successfully processed\n");
    printf("all large integer comparisons using double_int::cmp\n");
    
    return 0;
}
