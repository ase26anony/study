/* test_double_int_cmp.c - Comprehensive test for double_int comparison logic */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include <limits.h>

/* Force 128-bit integer support */
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* ========== SECTION 1: Constant Folding with Large Integers ========== */

/* Static assertions with large constants */
static_assert(((int128_t)1 << 70) > 0, "Large positive 128-bit constant");
static_assert(((int128_t)1 << 70) > ((int128_t)1 << 69), "Comparison of large 128-bit constants");
static_assert(((int128_t)1 << 120) != ((int128_t)1 << 119), "Inequality of very large constants");

/* Template-like macro for compile-time comparisons */
#define COMPILE_TIME_CMP(a, b) ((a) > (b) ? 1 : ((a) < (b) ? -1 : 0))

/* Force constant evaluation with __builtin_constant_p */
void test_constant_folding(void) {
    const int128_t huge1 = ((int128_t)1 << 100);
    const int128_t huge2 = ((int128_t)1 << 99) + ((int128_t)1 << 98);
    
    /* These comparisons should be evaluated at compile time */
    if (__builtin_constant_p(huge1 > huge2)) {
        printf("Constant folding test 1: %s\n", 
               huge1 > huge2 ? "PASS" : "FAIL");
    }
    
    if (__builtin_constant_p(huge1 != huge2)) {
        printf("Constant folding test 2: %s\n",
               huge1 != huge2 ? "PASS" : "FAIL");
    }
    
    /* Test with negative large numbers */
    const int128_t neg_huge = -((int128_t)1 << 100);
    const int128_t pos_huge = ((int128_t)1 << 99);
    
    if (__builtin_constant_p(neg_huge < pos_huge)) {
        printf("Constant folding test 3: %s\n",
               neg_huge < pos_huge ? "PASS" : "FAIL");
    }
}

/* ========== SECTION 2: GCC Builtins with Overflow ========== */

void test_overflow_builtins(void) {
    long long a, b;
    long long res;
    int overflow;
    
    /* Test multiplication that overflows 64-bit */
    a = LLONG_MAX;
    b = 2;
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("Overflow test 1: %s (overflow=%d)\n", 
           overflow ? "PASS" : "FAIL", overflow);
    
    /* Test with constants that should be evaluated at compile time */
    const long long c1 = LLONG_MAX / 2;
    const long long c2 = 3;
    
    if (__builtin_constant_p(__builtin_mul_overflow_p(c1, c2, (long long)0))) {
        printf("Overflow test 2: Compile-time overflow detection\n");
    }
    
    /* Test addition overflow */
    long long d = LLONG_MAX;
    long long e = 1;
    overflow = __builtin_add_overflow(d, e, &res);
    printf("Overflow test 3: %s (overflow=%d)\n",
           overflow ? "PASS" : "FAIL", overflow);
    
    /* Test subtraction underflow */
    long long f = LLONG_MIN;
    long long g = 1;
    overflow = __builtin_sub_overflow(f, g, &res);
    printf("Overflow test 4: %s (overflow=%d)\n",
           overflow ? "PASS" : "FAIL", overflow);
}

/* ========== SECTION 3: Range Calculations and VRP ========== */

void test_range_analysis(int x) {
    /* Create complex range conditions */
    if (x > 1000 && x < 2000) {
        /* This multiplication's range analysis uses double_int comparisons */
        int128_t y = (int128_t)x * (int128_t)x;
        
        /* Further comparisons on the result */
        if (y > 1000000 && y < 4000000) {
            printf("Range test 1: x=%d, y=%lld (truncated)\n", x, (long long)y);
        }
    }
    
    /* Test with larger ranges */
    if (x > 1000000 && x < 10000000) {
        int128_t z = (int128_t)x * (int128_t)x;
        
        /* Nested comparisons to force range analysis */
        if (z > (int128_t)1000000000000LL && 
            z < (int128_t)100000000000000LL) {
            printf("Range test 2: x=%d\n", x);
        }
    }
}

/* Loop with induction variable analysis */
void test_induction_variables(void) {
    for (int128_t i = 0; i < ((int128_t)1 << 30); i += (1 << 20)) {
        /* The loop analysis compares the induction variable against bounds */
        if (i > ((int128_t)1 << 29)) {
            printf("Induction test: i=%lld (truncated)\n", (long long)i);
            break;
        }
    }
}

/* ========== SECTION 4: Template Metaprogramming (C++ style in C) ========== */

/* Simulate template metaprogramming using macros and static functions */
struct LargeCompare {
    static int compare_128(int128_t a, int128_t b) {
        return (a > b) ? 1 : ((a < b) ? -1 : 0);
    }
};

/* Force compile-time evaluation of comparisons */
#define EVAL_COMPARE(a, b) \
    do { \
        static const int result = LargeCompare::compare_128(a, b); \
        printf("Template test: compare(%lld, %lld) = %d\n", \
               (long long)(a), (long long)(b), result); \
    } while(0)

/* ========== SECTION 5: Tree Node Construction ========== */

/* Use mode attribute for 128-bit integers */
typedef int int128_attr __attribute__((mode(TI)));
typedef unsigned int uint128_attr __attribute__((mode(TI)));

void test_tree_nodes(void) {
    int128_attr a = ((int128_attr)1 << 70);
    int128_attr b = ((int128_attr)1 << 69);
    
    /* Operations that require magnitude comparison */
    int128_attr div_result = a / b;
    int128_attr mod_result = a % b;
    
    printf("Tree node test: a/b=%lld, a%%b=%lld\n",
           (long long)div_result, (long long)mod_result);
    
    /* Comparisons that should create INTEGER_CST nodes */
    if (a > b) {
        printf("Tree node comparison: a > b\n");
    }
    
    /* Test with division by large constants */
    int128_attr c = ((int128_attr)1 << 100) / ((int128_attr)1 << 50);
    printf("Tree node division: result=%lld\n", (long long)c);
}

/* ========== SECTION 6: Complex Expressions ========== */

void test_complex_expressions(void) {
    /* Create expressions that combine multiple operations */
    const int128_t base = ((int128_t)1 << 80);
    const int128_t offset = ((int128_t)1 << 79);
    
    /* Chain of comparisons */
    int result = (base > offset) && 
                 (base + offset > base) &&
                 (base - offset < base) &&
                 (base * 2 > base);
    
    printf("Complex expression test: %s\n", result ? "PASS" : "FAIL");
    
    /* Test with mixed signed/unsigned comparisons */
    uint128_t u1 = (uint128_t)1 << 90;
    int128_t s1 = -((int128_t)1 << 90);
    
    /* This should trigger unsigned comparison in double_int::cmp */
    if (u1 > (uint128_t)s1) {
        printf("Mixed signed/unsigned test: PASS\n");
    }
}

/* ========== MAIN FUNCTION ========== */

int main(void) {
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
    /* Section 1: Constant Folding */
    printf("1. Constant Folding Tests:\n");
    test_constant_folding();
    printf("\n");
    
    /* Section 2: Overflow Builtins */
    printf("2. Overflow Builtin Tests:\n");
    test_overflow_builtins();
    printf("\n");
    
    /* Section 3: Range Analysis */
    printf("3. Range Analysis Tests:\n");
    test_range_analysis(1500);  /* Mid-range value */
    test_range_analysis(5000000); /* Larger value */
    test_induction_variables();
    printf("\n");
    
    /* Section 4: Template-like Comparisons */
    printf("4. Template-style Comparisons:\n");
    EVAL_COMPARE(((int128_t)1 << 65), ((int128_t)1 << 64));
    EVAL_COMPARE(((int128_t)1 << 70), ((int128_t)1 << 71));
    printf("\n");
    
    /* Section 5: Tree Node Tests */
    printf("5. Tree Node Construction Tests:\n");
    test_tree_nodes();
    printf("\n");
    
    /* Section 6: Complex Expressions */
    printf("6. Complex Expression Tests:\n");
    test_complex_expressions();
    printf("\n");
    
    /* Additional edge cases */
    printf("7. Edge Case Tests:\n");
    
    /* Test with maximum 128-bit values */
    const uint128_t max_u128 = ~(uint128_t)0;
    const uint128_t almost_max = max_u128 - 1;
    
    if (__builtin_constant_p(max_u128 > almost_max)) {
        printf("Edge case 1: Max > AlmostMax = %s\n",
               max_u128 > almost_max ? "PASS" : "FAIL");
    }
    
    /* Test with high parts only differing */
    const int128_t high_diff1 = ((int128_t)0x12345678 << 64) | 0x1;
    const int128_t high_diff2 = ((int128_t)0x12345679 << 64) | 0xFFFFFFFFFFFFFFFFULL;
    
    printf("Edge case 2: High part comparison %s\n",
           high_diff1 < high_diff2 ? "PASS" : "FAIL");
    
    /* Test with equal high parts, different low parts */
    const int128_t same_high1 = ((int128_t)0x55555555 << 64) | 0x1;
    const int128_t same_high2 = ((int128_t)0x55555555 << 64) | 0x2;
    
    printf("Edge case 3: Low part comparison %s\n",
           same_high1 < same_high2 ? "PASS" : "FAIL");
    
    printf("\n=== All tests completed ===\n");
    
    return 0;
}
