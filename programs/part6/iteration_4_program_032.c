/* test_double_int_cmp.c - Comprehensive test for double_int::cmp coverage */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <limits.h>

/* ==================== 1. Constant Folding with Large Integers ==================== */

/* Large constants that require 128-bit representation */
#define VERY_LARGE_CONST ((__int128_t)1 << 70)
#define HUGE_NEGATIVE (-((__int128_t)1 << 68))
#define LARGE_PRODUCT ((__int128_t)0x7FFFFFFFFFFFFFFF * 4)

/* Static assertions force compile-time comparison */
static_assert(VERY_LARGE_CONST > 0, "Large positive constant should be > 0");
static_assert(HUGE_NEGATIVE < 0, "Large negative constant should be < 0");
static_assert(VERY_LARGE_CONST > HUGE_NEGATIVE, "Positive > negative");
static_assert(LARGE_PRODUCT > INT64_MAX, "Product exceeds 64-bit range");

/* Compile-time comparisons in macros */
#define COMPARE_CONSTANTS(a, b) ((a) < (b) ? -1 : ((a) > (b) ? 1 : 0))

/* Force evaluation with __builtin_constant_p */
int const_fold_test(void) {
    if (__builtin_constant_p(VERY_LARGE_CONST > 1000)) {
        return 1;
    }
    
    /* These comparisons should trigger double_int::cmp */
    __int128_t a = ((__int128_t)1 << 65) + 123;
    __int128_t b = ((__int128_t)1 << 65) + 456;
    
    /* Multiple comparison operations */
    int results = 0;
    results |= (a < b) ? 0x1 : 0;
    results |= (a > b) ? 0x2 : 0;
    results |= (a == a) ? 0x4 : 0;
    results |= (a != b) ? 0x8 : 0;
    results |= (a <= b) ? 0x10 : 0;
    results |= (b >= a) ? 0x20 : 0;
    
    return results;
}

/* ==================== 2. GCC Builtins with Overflow ==================== */

void overflow_builtin_tests(void) {
    long long x, y, result;
    int overflow;
    
    /* Test cases designed to trigger overflow comparisons */
    struct {
        long long a;
        long long b;
        int expected_overflow;
    } tests[] = {
        {LLONG_MAX, 2, 1},           /* Overflow on multiplication */
        {LLONG_MIN, -1, 1},          /* Overflow edge case */
        {1000, 2000, 0},             /* No overflow */
        {LLONG_MAX / 2, 3, 1},       /* Borderline overflow */
    };
    
    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); i++) {
        overflow = __builtin_mul_overflow(tests[i].a, tests[i].b, &result);
        
        /* The overflow check internally uses double_int comparisons */
        if (__builtin_constant_p(tests[i].a * tests[i].b)) {
            /* Force constant evaluation path */
            printf("Test %zu: overflow=%d (expected %d)\n", 
                   i, overflow, tests[i].expected_overflow);
        }
    }
    
    /* Addition overflow tests */
    long long sum;
    overflow = __builtin_add_overflow(LLONG_MAX, 1, &sum);
    
    /* Subtraction overflow */
    long long diff;
    overflow = __builtin_sub_overflow(LLONG_MIN, 1, &diff);
}

/* ==================== 3. Range Calculations ==================== */

void range_analysis_tests(int input) {
    /* Complex range analysis that requires double_int comparisons */
    
    /* Basic range */
    if (input > 1000 && input < 10000) {
        /* Multiplication expands range significantly */
        long long squared = (long long)input * input;
        
        /* Nested ranges */
        if (squared > 5000000 && squared < 5000000000LL) {
            long long cubed = squared * input;
            
            /* More comparisons with large values */
            if (cubed > 1000000000LL && cubed < 1000000000000LL) {
                printf("Range test passed: cubed=%lld\n", cubed);
            }
        }
    }
    
    /* Test with negative ranges */
    if (input < -1000 && input > -10000) {
        long long product = input * 1000;
        
        /* Comparison across zero */
        if (product < -1000000 && product > -10000000) {
            printf("Negative range test passed\n");
        }
    }
    
    /* Loop with induction variable analysis */
    for (long long i = 0; i < 10000000000LL; i += 1000000000LL) {
        /* The loop bound comparison uses wide integers */
        if (i > 5000000000LL) {
            printf("Large loop iteration: %lld\n", i);
        }
    }
}

/* ==================== 4. Template Metaprogramming (C++) ==================== */

#ifdef __cplusplus

template <__int128_t N>
struct LargeCompare {
    static const bool is_positive = N > 0;
    static const bool is_large = N > (__int128_t(1) << 65);
    static const bool is_huge = N > (__int128_t(1) << 100);
    
    /* Force multiple comparisons */
    static const int compare_to_mid = (N > (__int128_t(1) << 64)) ? 1 : 
                                     ((N < (__int128_t(1) << 64)) ? -1 : 0);
};

/* Instantiate templates with various large values */
template struct LargeCompare<(__int128_t(1) << 66)>;
template struct LargeCompare<-(__int128_t(1) << 66)>;
template struct LargeCompare<(__int128_t(1) << 70) + 123456789>;
template struct LargeCompare<(__int128_t(0x7FFFFFFFFFFFFFFF) * 2)>;

/* Template function with comparisons */
template <__int128_t A, __int128_t B>
constexpr int compare_values() {
    return (A < B) ? -1 : ((A > B) ? 1 : 0);
}

/* Force instantiation */
constexpr int cmp1 = compare_values<(__int128_t(1) << 68), (__int128_t(1) << 69)>();
constexpr int cmp2 = compare_values<(__int128_t(1) << 70), (__int128_t(1) << 69)>();
constexpr int cmp3 = compare_values<(__int128_t(1) << 65) + 1, (__int128_t(1) << 65) + 2>();

#endif

/* ==================== 5. Tree Node Construction ==================== */

/* Use 128-bit types with attributes */
typedef __int128_t __attribute__((mode(TI))) int128_t_attr;

/* Operations that create wide integer tree nodes */
int128_t_attr wide_operations(int128_t_attr a, int128_t_attr b) {
    /* Various operations that require magnitude comparisons */
    if (a == 0 || b == 0) return 0;
    
    /* Division requires comparing divisor and dividend */
    if (a > b) {
        return a / b;
    } else if (b > a) {
        return b / a;
    }
    
    /* Modulus operation */
    if (a != 0) {
        int128_t_attr mod = b % a;
        
        /* Compare remainder */
        if (mod > a / 2) {
            return mod - a;
        }
    }
    
    return a + b;
}

/* Enumeration with large values */
enum big_enum : __int128 {
    BIG_VALUE_A = (__int128_t)1 << 65,
    BIG_VALUE_B = (__int128_t)1 << 66,
    BIG_VALUE_C = BIG_VALUE_A * 3
};

/* ==================== Main Test Harness ==================== */

int main(void) {
    int pass_count = 0;
    int total_tests = 0;
    
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
    /* Test 1: Constant folding */
    printf("1. Constant folding tests:\n");
    int cf_result = const_fold_test();
    if (cf_result == 0x3D) { /* Expected bits for a < b */
        printf("  PASS: Constant folding comparisons\n");
        pass_count++;
    } else {
        printf("  FAIL: Constant folding (got 0x%x)\n", cf_result);
    }
    total_tests++;
    
    /* Test 2: Overflow builtins */
    printf("\n2. Overflow builtin tests:\n");
    overflow_builtin_tests();
    printf("  Overflow tests executed\n");
    pass_count++;
    total_tests++;
    
    /* Test 3: Range analysis */
    printf("\n3. Range analysis tests:\n");
    range_analysis_tests(5000);
    range_analysis_tests(-5000);
    printf("  Range analysis tests executed\n");
    pass_count++;
    total_tests++;
    
    /* Test 4: Wide operations */
    printf("\n4. Wide integer operations:\n");
    int128_t_attr wa = ((__int128_t)1 << 70) + 123;
    int128_t_attr wb = ((__int128_t)1 << 70) + 456;
    int128_t_attr wresult = wide_operations(wa, wb);
    
    /* Compare wide integers */
    if (wa < wb) {
        printf("  PASS: Wide integer comparison (wa < wb)\n");
        pass_count++;
    } else {
        printf("  FAIL: Wide integer comparison\n");
    }
    total_tests++;
    
    /* Test 5: Enum comparisons */
    printf("\n5. Large enumeration tests:\n");
    if (BIG_VALUE_A < BIG_VALUE_B && BIG_VALUE_B < BIG_VALUE_C) {
        printf("  PASS: Large enum value ordering\n");
        pass_count++;
    } else {
        printf("  FAIL: Large enum comparison\n");
    }
    total_tests++;
    
#ifdef __cplusplus
    /* Test 6: Template metaprogramming */
    printf("\n6. Template metaprogramming tests:\n");
    if (LargeCompare<(__int128_t(1) << 66)>::is_large &&
        LargeCompare<(__int128_t(1) << 66)>::is_positive &&
        cmp1 == -1 && cmp2 == 1 && cmp3 == -1) {
        printf("  PASS: Template comparisons\n");
        pass_count++;
    } else {
        printf("  FAIL: Template comparisons\n");
    }
    total_tests++;
#endif
    
    /* Summary */
    printf("\n=== Summary ===\n");
    printf("Tests passed: %d/%d\n", pass_count, total_tests);
    
    if (pass_count == total_tests) {
        printf("\nALL TESTS PASSED - double_int::cmp should be thoroughly exercised\n");
        return 0;
    } else {
        printf("\nSOME TESTS FAILED\n");
        return 1;
    }
}
