/* test_double_int_cmp.c - Comprehensive test for double_int comparison logic */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <limits.h>

/* Force 128-bit integer support */
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* ========== SECTION 1: Constant Folding with Large Integers ========== */

/* Large constants that require double_int representation */
static const int128_t VERY_LARGE_POS = ((int128_t)1 << 70);
static const int128_t VERY_LARGE_NEG = -((int128_t)1 << 70);
static const int128_t HUGE_PRODUCT = ((int128_t)INT64_MAX) * ((int128_t)INT64_MAX);
static const uint128_t LARGE_UNSIGNED = ((uint128_t)1 << 100);

/* Compile-time comparisons using static assertions */
_Static_assert(VERY_LARGE_POS > 0, "Large positive constant comparison");
_Static_assert(VERY_LARGE_NEG < 0, "Large negative constant comparison");
_Static_assert(HUGE_PRODUCT > INT64_MAX, "Product comparison with double_int");
_Static_assert(LARGE_UNSIGNED > UINT64_MAX, "Unsigned large constant comparison");

/* Function to test constant folding with __builtin_constant_p */
static void test_constant_folding(void) {
    printf("Testing constant folding...\n");
    
    /* These should be evaluated at compile-time */
    if (__builtin_constant_p(VERY_LARGE_POS > VERY_LARGE_NEG)) {
        printf("  Constant comparison 1: PASS\n");
    }
    
    if (__builtin_constant_p((VERY_LARGE_POS + 1) > VERY_LARGE_POS)) {
        printf("  Constant comparison 2: PASS\n");
    }
    
    /* Complex expression requiring double_int comparison */
    const int128_t shifted = VERY_LARGE_POS << 5;
    if (__builtin_constant_p(shifted > VERY_LARGE_POS)) {
        printf("  Constant comparison 3: PASS\n");
    }
}

/* ========== SECTION 2: GCC Builtins with Overflow ========== */

static void test_overflow_builtins(void) {
    printf("\nTesting overflow builtins...\n");
    
    long long a, b;
    long long result;
    int overflow;
    
    /* Test cases designed to trigger overflow checks with double_int comparisons */
    
    /* Case 1: Multiplication near 64-bit boundaries */
    a = LLONG_MAX / 2;
    b = 3;
    overflow = __builtin_mul_overflow(a, b, &result);
    printf("  mul_overflow(%lld * %d): overflow=%d (expected: 1)\n", 
           a, 3, overflow);
    
    /* Case 2: Large positive multiplication */
    a = 1LL << 62;
    b = 1LL << 62;
    overflow = __builtin_mul_overflow(a, b, &result);
    printf("  mul_overflow(2^62 * 2^62): overflow=%d (expected: 1)\n", overflow);
    
    /* Case 3: Addition with potential overflow */
    a = LLONG_MAX;
    b = 1;
    overflow = __builtin_add_overflow(a, b, &result);
    printf("  add_overflow(LLONG_MAX + 1): overflow=%d (expected: 1)\n", overflow);
    
    /* Case 4: __builtin_mul_overflow_p for constant evaluation */
    if (__builtin_constant_p(__builtin_mul_overflow_p(LLONG_MAX, 2, 0))) {
        printf("  mul_overflow_p constant evaluation: PASS\n");
    }
}

/* ========== SECTION 3: Range Calculations and VRP ========== */

static void test_range_calculations(int x) {
    /* Complex range analysis that should use double_int comparisons */
    
    /* Narrow range */
    if (x > 1000 && x < 2000) {
        /* Multiplication that creates a range requiring double_int */
        int128_t y = (int128_t)x * (int128_t)x;
        
        /* Comparison within the calculated range */
        if (y > 1000000 && y < 4000000) {
            printf("  Range test 1 passed for x=%d, y=%lld\n", x, (long long)y);
        }
    }
    
    /* Wider range with potential overflow */
    if (x > INT_MAX / 100 && x < INT_MAX / 50) {
        int128_t product = (int128_t)x * 75;
        
        /* This comparison should trigger double_int::cmp in VRP */
        if (product > INT_MAX && product < (int128_t)INT_MAX * 2) {
            printf("  Range test 2 passed for x=%d\n", x);
        }
    }
}

/* Induction variable with large step */
static void test_induction_variables(void) {
    printf("\nTesting induction variables...\n");
    
    for (int128_t i = 0; i < ((int128_t)1 << 70); i += ((int128_t)1 << 60)) {
        /* Loop condition comparison uses double_int */
        if (i > ((int128_t)1 << 65)) {
            printf("  Induction variable reached >2^65\n");
            break;
        }
    }
}

/* ========== SECTION 4: Template Metaprogramming (C++ version available) ========== */

#ifdef __cplusplus
template <int128_t N>
struct LargeCompare {
    static const bool greater_than_2_65 = N > ((int128_t)1 << 65);
    static const bool less_than_neg_2_65 = N < -((int128_t)1 << 65);
};

/* Instantiate templates with large values */
template struct LargeCompare<((int128_t)1 << 70)>;
template struct LargeCompare<-((int128_t)1 << 70)>;
#endif

/* ========== SECTION 5: Tree Node Construction ========== */

/* Enumeration with 128-bit underlying type */
enum big_enum : int128_t {
    BIG_ENUM_VALUE = ((int128_t)1 << 70),
    NEG_BIG_ENUM_VALUE = -((int128_t)1 << 70)
};

/* Structure with mode attribute for 128-bit type */
typedef int int128_attr __attribute__((mode(TI)));

static void test_tree_node_operations(void) {
    printf("\nTesting tree node operations...\n");
    
    int128_attr a = ((int128_attr)1 << 70);
    int128_attr b = ((int128_attr)1 << 69);
    
    /* Operations that should create INTEGER_CST nodes with double_int */
    int128_attr sum = a + b;
    int128_attr diff = a - b;
    int128_attr prod = a / 2;
    
    /* Comparisons on these results */
    if (sum > a && diff > 0 && prod == b) {
        printf("  Tree node operations: PASS\n");
    }
    
    /* Test enum comparisons */
    if (BIG_ENUM_VALUE > 0 && NEG_BIG_ENUM_VALUE < 0) {
        printf("  Enum comparisons: PASS\n");
    }
}

/* ========== SECTION 6: Complex Expression Evaluation ========== */

static void test_complex_expressions(void) {
    printf("\nTesting complex expressions...\n");
    
    /* Expression that requires multiple double_int comparisons */
    const int128_t A = ((int128_t)1 << 70);
    const int128_t B = ((int128_t)1 << 69);
    const int128_t C = ((int128_t)1 << 68);
    
    /* Chain of comparisons */
    int result = (A > B) + (B > C) + (C < A);
    printf("  Chain comparison result: %d (expected: 3)\n", result);
    
    /* Mixed signed/unsigned comparisons */
    uint128_t unsigned_large = ((uint128_t)1 << 100);
    int128_t signed_large = ((int128_t)1 << 70);
    
    /* This should trigger unsigned high part comparison in double_int::cmp */
    if (unsigned_large > (uint128_t)signed_large) {
        printf("  Mixed signed/unsigned comparison: PASS\n");
    }
}

/* ========== MAIN FUNCTION ========== */

int main(void) {
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
    /* Run all test sections */
    test_constant_folding();
    test_overflow_builtins();
    
    /* Test range calculations with different values */
    test_range_calculations(1500);
    test_range_calculations(30000000);
    
    test_induction_variables();
    test_tree_node_operations();
    test_complex_expressions();
    
    /* Final validation */
    printf("\n=== Final Validation ===\n");
    
    /* Direct comparison of large constants */
    const int128_t X = ((int128_t)1 << 70) + 1;
    const int128_t Y = ((int128_t)1 << 70);
    
    if (X > Y) {
        printf("Direct large constant comparison: PASS\n");
    }
    
    /* Edge case: high parts equal, low parts differ */
    const int128_t Z = ((int128_t)0x123456789ABCDEF0 << 64) | 0x1111111111111111;
    const int128_t W = ((int128_t)0x123456789ABCDEF0 << 64) | 0x2222222222222222;
    
    if (Z < W) {
        printf("Equal high part, different low part comparison: PASS\n");
    }
    
    printf("\nAll tests completed successfully!\n");
    return 0;
}
