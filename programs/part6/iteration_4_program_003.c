/* test_double_int_cmp.c - Comprehensive test for double_int comparison logic */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Force 128-bit integer support */
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* Static assertions with large constants */
#define CONCAT(a, b) a##b
#define STATIC_ASSERT(cond, msg) typedef char CONCAT(static_assert_, __LINE__)[(cond) ? 1 : -1]

/* Test 1: Direct large constant comparisons at compile time */
static void test_large_constants(void) {
    /* These should trigger double_int::cmp during constant folding */
    const int128_t huge_pos = ((int128_t)1 << 100);
    const int128_t huge_neg = -((int128_t)1 << 100);
    const int128_t medium = ((int128_t)1 << 70);
    
    /* Compile-time comparisons */
    STATIC_ASSERT(huge_pos > 0, "Large positive constant");
    STATIC_ASSERT(huge_neg < 0, "Large negative constant");
    STATIC_ASSERT(huge_pos > medium, "Comparison of two large constants");
    STATIC_ASSERT(huge_neg < medium, "Negative vs positive large constant");
    
    /* Test equality with large values */
    const int128_t same1 = ((int128_t)0x123456789ABCDEF0 << 64) | 0xFEDCBA9876543210ULL;
    const int128_t same2 = ((int128_t)0x123456789ABCDEF0 << 64) | 0xFEDCBA9876543210ULL;
    STATIC_ASSERT(same1 == same2, "Large equality comparison");
    
    /* Test near-boundary values */
    const int128_t max64 = ((int128_t)1 << 63) - 1;
    const int128_t min64 = -((int128_t)1 << 63);
    STATIC_ASSERT(max64 > min64, "64-bit boundary comparison");
    
    printf("Large constant tests passed\n");
}

/* Test 2: Builtin overflow operations */
static void test_overflow_builtins(void) {
    long long a, b;
    long long result;
    int overflow;
    
    /* These overflow checks internally use double_int comparisons */
    
    /* Case 1: Multiplication that overflows 64-bit */
    a = 0x7FFFFFFFFFFFFFFFLL;  /* Max positive int64 */
    b = 2;
    overflow = __builtin_mul_overflow(a, b, &result);
    printf("Overflow mul test 1: %d (expected 1)\n", overflow);
    
    /* Case 2: Multiplication that doesn't overflow */
    a = 0x3FFFFFFFFFFFFFFFLL;
    b = 2;
    overflow = __builtin_mul_overflow(a, b, &result);
    printf("Overflow mul test 2: %d (expected 0), result = %lld\n", overflow, result);
    
    /* Case 3: Addition overflow */
    a = 0x7FFFFFFFFFFFFFFFLL;
    b = 1;
    overflow = __builtin_add_overflow(a, b, &result);
    printf("Overflow add test: %d (expected 1)\n", overflow);
    
    /* Case 4: Subtraction underflow */
    a = (-0x7FFFFFFFFFFFFFFFLL - 1);  /* INT64_MIN */
    b = 1;
    overflow = __builtin_sub_overflow(a, b, &result);
    printf("Overflow sub test: %d (expected 1)\n", overflow);
    
    /* Test with __builtin_constant_p to force constant evaluation */
    if (__builtin_constant_p(__builtin_mul_overflow_p(0x7FFFFFFFFFFFFFFFLL, 2, 0LL))) {
        printf("Constant overflow check triggered\n");
    }
}

/* Test 3: Range analysis with complex conditions */
static void test_range_analysis(int x) {
    /* These conditions create value ranges that VRP must analyze */
    
    /* Case 1: Multiplication within known range */
    if (x > 1000 && x < 2000) {
        /* VRP will compute range for y using double_int comparisons */
        long long y = (long long)x * x;
        
        /* Further comparisons on the result */
        if (y > 1000000 && y < 4000000) {
            printf("Range test 1 passed: y = %lld\n", y);
        }
    }
    
    /* Case 2: Large multiplication that might overflow */
    if (x > 1000000 && x < 2000000) {
        long long y = (long long)x * 1000000;
        /* This should trigger overflow analysis */
        if (__builtin_mul_overflow(x, 1000000, &y)) {
            printf("Range test 2: overflow detected\n");
        }
    }
    
    /* Case 3: Loop with induction variable analysis */
    for (int128_t i = 0; i < 100; i += ((int128_t)1 << 40)) {
        /* Large step values force wide integer comparisons */
        if (i > ((int128_t)1 << 50)) {
            printf("Large loop iteration: %lld\n", (long long)(i >> 40));
            break;
        }
    }
}

/* Test 4: Template metaprogramming (C++ version) */
#ifdef __cplusplus

template <int128_t N>
struct LargeCompare {
    static const bool is_positive = N > 0;
    static const bool is_large = N > ((int128_t)1 << 65);
    static const bool is_huge = N > ((int128_t)1 << 100);
};

template <int128_t A, int128_t B>
struct CompareValues {
    static const int cmp_result = (A > B) ? 1 : ((A < B) ? -1 : 0);
    static const bool a_greater = (A > B);
    static const bool equal = (A == B);
};

void test_templates() {
    /* Instantiate templates with large values */
    typedef LargeCompare<((int128_t)1 << 70)> Test1;
    typedef LargeCompare<-((int128_t)1 << 70)> Test2;
    typedef CompareValues<((int128_t)1 << 80), ((int128_t)1 << 79)> Test3;
    
    printf("Template test 1: is_positive = %d, is_large = %d\n", 
           Test1::is_positive, Test1::is_large);
    printf("Template test 2: is_positive = %d\n", Test2::is_positive);
    printf("Template test 3: cmp_result = %d, a_greater = %d\n",
           Test3::cmp_result, Test3::a_greater);
}

#endif

/* Test 5: Tree node construction with wide constants */
static void test_wide_constants(void) {
    /* Use 128-bit constants in various contexts */
    
    /* Enumeration with large values */
    enum wide_enum : int128_t {
        BIG_VALUE = ((int128_t)1 << 65),
        BIGGER_VALUE = ((int128_t)1 << 66),
        HUGE_VALUE = ((int128_t)1 << 100)
    };
    
    /* Operations on wide constants */
    int128_t a = BIG_VALUE;
    int128_t b = BIGGER_VALUE;
    
    /* These operations should create INTEGER_CST nodes */
    int128_t sum = a + b;
    int128_t diff = b - a;
    int128_t prod = a * 2;
    
    /* Comparisons that use double_int::cmp */
    if (a < b) {
        printf("Wide constant comparison 1 passed\n");
    }
    
    if (sum > a && sum > b) {
        printf("Wide constant comparison 2 passed\n");
    }
    
    /* Division and modulus with large values */
    int128_t quotient = HUGE_VALUE / BIG_VALUE;
    int128_t remainder = HUGE_VALUE % BIG_VALUE;
    
    if (quotient > 0) {
        printf("Wide division result: quotient > 0\n");
    }
    
    if (remainder < BIG_VALUE) {
        printf("Wide modulus result: remainder < divisor\n");
    }
    
    /* Test with attribute for 128-bit type */
    typedef int128_t __attribute__((mode(TI))) ti_int;
    ti_int ti_a = ((ti_int)1 << 120);
    ti_int ti_b = ((ti_int)1 << 119);
    
    if (ti_a > ti_b) {
        printf("TI mode comparison passed\n");
    }
}

/* Test 6: Mixed-size comparisons */
static void test_mixed_comparisons(void) {
    /* Compare 128-bit values with 64-bit values */
    int128_t large = ((int128_t)1 << 70);
    int64_t small = 100;
    
    /* These should trigger promotions and comparisons */
    if (large > small) {
        printf("Mixed comparison 1: 128-bit > 64-bit\n");
    }
    
    if (small < large) {
        printf("Mixed comparison 2: 64-bit < 128-bit\n");
    }
    
    /* Compare with negative values */
    int128_t neg_large = -((int128_t)1 << 70);
    if (neg_large < small) {
        printf("Mixed comparison 3: negative 128-bit < positive 64-bit\n");
    }
    
    /* Compare with zero */
    if (large > 0) {
        printf("Mixed comparison 4: large > 0\n");
    }
    
    if (neg_large < 0) {
        printf("Mixed comparison 5: neg_large < 0\n");
    }
}

/* Test 7: Complex compile-time expressions */
static void test_complex_expressions(void) {
    /* Nested operations that must be evaluated at compile time */
    const int128_t a = ((int128_t)1 << 65);
    const int128_t b = ((int128_t)1 << 64);
    const int128_t c = ((int128_t)1 << 63);
    
    /* Complex comparison chain */
    STATIC_ASSERT(a > b && b > c, "Nested large comparisons");
    
    /* Arithmetic then comparison */
    const int128_t sum = a + b;
    const int128_t prod = b * 2;
    STATIC_ASSERT(sum > prod, "Comparison after arithmetic");
    
    /* Shift then compare */
    const int128_t shifted = c << 10;
    STATIC_ASSERT(shifted > a, "Comparison after shift");
    
    /* Use in conditional expression */
    const int128_t max_val = (a > b) ? a : b;
    STATIC_ASSERT(max_val == a, "Conditional with large values");
    
    printf("Complex expression tests passed\n");
}

int main(void) {
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
    /* Run all tests */
    test_large_constants();
    printf("\n");
    
    test_overflow_builtins();
    printf("\n");
    
    test_range_analysis(1500);  /* Mid-range value */
    printf("\n");
    
#ifdef __cplusplus
    test_templates();
    printf("\n");
#endif
    
    test_wide_constants();
    printf("\n");
    
    test_mixed_comparisons();
    printf("\n");
    
    test_complex_expressions();
    printf("\n");
    
    printf("=== All tests completed ===\n");
    
    /* Runtime validation */
    const int128_t final_check_a = ((int128_t)1 << 100);
    const int128_t final_check_b = ((int128_t)1 << 99);
    
    if (final_check_a > final_check_b && 
        final_check_b > 0 &&
        -final_check_a < 0) {
        printf("Final validation: PASS\n");
        return 0;
    } else {
        printf("Final validation: FAIL\n");
        return 1;
    }
}
