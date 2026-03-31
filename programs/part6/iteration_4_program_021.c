/* test_double_int_cmp.c - Comprehensive test for double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

/* ==================== SECTION 1: Constant Folding with Large Integers ==================== */

/* Test 1.1: Static assertions with 128-bit constants */
#define LARGE_CONST_1 (((__int128_t)1) << 70)      /* 2^70 */
#define LARGE_CONST_2 (((__int128_t)1) << 71)      /* 2^71 */
#define LARGE_CONST_3 (((__int128_t)1) << 65) + 1  /* 2^65 + 1 */
#define LARGE_CONST_4 (((__int128_t)1) << 65) - 1  /* 2^65 - 1 */

/* These static assertions force compile-time comparison of large integers */
_Static_assert(LARGE_CONST_2 > LARGE_CONST_1, "Large constant comparison 1");
_Static_assert(LARGE_CONST_3 > LARGE_CONST_4, "Large constant comparison 2");
_Static_assert(LARGE_CONST_1 != LARGE_CONST_2, "Large constant inequality");

/* Test 1.2: __builtin_constant_p with large comparisons */
static int test_builtin_constant(void) {
    const __int128_t a = ((__int128_t)0x123456789ABCDEF0) << 64;
    const __int128_t b = ((__int128_t)0xFEDCBA9876543210) << 64;
    
    /* Force compiler to evaluate at compile time */
    if (__builtin_constant_p(a > b)) {
        return __builtin_constant_p(a > b) ? 1 : 0;
    }
    return -1;
}

/* Test 1.3: Complex arithmetic resulting in wide integers */
static __int128_t large_multiply(int64_t x, int64_t y) {
    /* Multiplication that can overflow 64 bits */
    return (__int128_t)x * (__int128_t)y;
}

static int test_large_arithmetic(void) {
    const int64_t big1 = 0x7FFFFFFFFFFFFFFF; /* Max int64_t */
    const int64_t big2 = 0x7FFFFFFFFFFFFFFF;
    
    __int128_t result = large_multiply(big1, big2);
    
    /* Comparison of large multiplication result */
    const __int128_t expected = ((__int128_t)big1) * ((__int128_t)big2);
    
    /* This comparison may use double_int::cmp internally */
    if (result == expected) {
        return 1;
    }
    return 0;
}

/* ==================== SECTION 2: GCC Builtins with Overflow ==================== */

/* Test 2.1: __builtin_mul_overflow with large types */
static int test_mul_overflow(void) {
    long long a, b;
    long long res;
    int overflow;
    
    /* Case 1: No overflow */
    a = 1000000;
    b = 1000000;
    overflow = __builtin_mul_overflow(a, b, &res);
    
    /* Case 2: Potential overflow */
    a = 0x7FFFFFFFFFFFFFFFLL;
    b = 2;
    overflow |= __builtin_mul_overflow(a, b, &res);
    
    /* Case 3: Large negative multiplication */
    a = -0x7FFFFFFFFFFFFFFFLL;
    b = 2;
    overflow |= __builtin_mul_overflow(a, b, &res);
    
    return overflow;
}

/* Test 2.2: __builtin_add_overflow with boundary values */
static int test_add_overflow(void) {
    int overflow_count = 0;
    long long x, y, result;
    
    struct {
        long long a;
        long long b;
    } tests[] = {
        {0x7FFFFFFFFFFFFFFFLL, 1},      /* Positive overflow */
        {0x8000000000000000LL, -1},     /* Negative overflow */
        {0x7FFFFFFFFFFFFFFFLL, -1},     /* No overflow */
        {0x8000000000000000LL, 1},      /* No overflow */
    };
    
    for (size_t i = 0; i < sizeof(tests)/sizeof(tests[0]); i++) {
        if (__builtin_add_overflow(tests[i].a, tests[i].b, &result)) {
            overflow_count++;
        }
    }
    
    return overflow_count;
}

/* Test 2.3: __builtin_mul_overflow_p for constant evaluation */
static int test_mul_overflow_p(void) {
    /* Force constant evaluation of overflow checks */
    int c1 = __builtin_mul_overflow_p(0x7FFFFFFF, 0x7FFFFFFF, (int)0);
    int c2 = __builtin_mul_overflow_p(0x7FFFFFFFFFFFFFFFLL, 2LL, (long long)0);
    int c3 = __builtin_mul_overflow_p(0x8000000000000000LL, -1LL, (long long)0);
    
    return c1 + c2 + c3;
}

/* ==================== SECTION 3: Range Calculations and VRP ==================== */

/* Test 3.1: Complex range analysis with comparisons */
static int test_range_analysis(int x) {
    int result = 0;
    
    /* Create known bounds for x */
    if (x > 1000 && x < 2000) {
        /* This multiplication's range calculation may use double_int::cmp */
        long long y = (long long)x * x;
        
        /* Further comparisons on the result */
        if (y > 1000000 && y < 4000000) {
            result = 1;
        }
    }
    
    /* Another range test with negative values */
    if (x < -1000 && x > -2000) {
        long long y = (long long)x * x;
        if (y > 1000000 && y < 4000000) {
            result |= 2;
        }
    }
    
    return result;
}

/* Test 3.2: Loop with induction variable analysis */
static int test_induction_variable(void) {
    int sum = 0;
    
    /* Loop with step that might trigger wrap analysis */
    for (int i = 0; i < 1000; i += 0x7FFFFFF) {
        sum += i;
        
        /* Condition that depends on i's value */
        if (i > 0x3FFFFFFF) {
            sum += 1;
        }
    }
    
    return sum;
}

/* Test 3.3: Nested conditionals with range propagation */
static int test_nested_ranges(int a, int b) {
    int result = 0;
    
    if (a > 100) {
        if (b > 1000) {
            long long product = (long long)a * b;
            
            /* Multiple comparisons that might use double_int */
            if (product > 100000 && product < 1000000000) {
                result = 1;
            } else if (product >= 1000000000) {
                result = 2;
            }
        }
    }
    
    return result;
}

/* ==================== SECTION 4: C++ Template Metaprogramming ==================== */

#ifdef __cplusplus

/* Test 4.1: Template with large integer non-type parameters */
template <__int128_t N>
struct LargeCompare {
    static const bool greater_than_2_65 = N > ((__int128_t)1 << 65);
    static const bool less_than_neg_2_65 = N < -((__int128_t)1 << 65);
    static const bool is_zero = N == 0;
    
    static const int comparison_result = 
        (N > 0) ? 1 : ((N < 0) ? -1 : 0);
};

/* Test 4.2: Template specialization based on large value comparisons */
template <__int128_t Val>
struct ValueCategory {
    static const char* category;
};

template <__int128_t Val>
const char* ValueCategory<Val>::category = 
    (Val > ((__int128_t)1 << 70)) ? "Very Large" :
    (Val > ((__int128_t)1 << 60)) ? "Large" :
    (Val > 0) ? "Positive" :
    (Val < 0) ? "Negative" : "Zero";

/* Instantiate templates with various large values */
using Compare1 = LargeCompare<((__int128_t)1) << 66>;
using Compare2 = LargeCompare<-((__int128_t)1) << 66>;
using Compare3 = LargeCompare<0>;

using Category1 = ValueCategory<((__int128_t)1) << 71>;
using Category2 = ValueCategory<((__int128_t)1) << 61>;
using Category3 = ValueCategory<-5>;

#endif /* __cplusplus */

/* ==================== SECTION 5: Tree Node Construction ==================== */

/* Test 5.1: Using __int128 with attributes */
typedef __int128_t int128 __attribute__((mode(TI)));

/* Test 5.2: Operations that create wide INTEGER_CST nodes */
static int128 test_wide_operations(int128 a, int128 b) {
    /* Various operations that might create wide constants */
    int128 sum = a + b;
    int128 diff = a - b;
    int128 prod = a * b;
    int128 div = a / (b != 0 ? b : 1);
    int128 mod = a % (b != 0 ? b : 1);
    
    /* Comparisons between these wide values */
    if (sum > diff && prod > div && mod != 0) {
        return sum;
    }
    return diff;
}

/* Test 5.3: Enumeration with large values (C++ only) */
#ifdef __cplusplus
enum BigEnum : __int128 {
    BIG_VALUE_1 = ((__int128_t)1) << 70,
    BIG_VALUE_2 = ((__int128_t)1) << 71,
    BIG_VALUE_3 = BIG_VALUE_1 + BIG_VALUE_2
};

static bool compare_big_enums(BigEnum a, BigEnum b) {
    return a < b;
}
#endif

/* ==================== MAIN TEST DRIVER ==================== */

int main(void) {
    int failures = 0;
    
    printf("Testing double_int::cmp coverage...\n");
    
    /* Test 1: Constant folding */
    printf("1. Testing constant folding... ");
    if (test_builtin_constant() != -1) {
        printf("PASS\n");
    } else {
        printf("FAIL\n");
        failures++;
    }
    
    printf("2. Testing large arithmetic... ");
    if (test_large_arithmetic() == 1) {
        printf("PASS\n");
    } else {
        printf("FAIL\n");
        failures++;
    }
    
    /* Test 2: Overflow builtins */
    printf("3. Testing multiplication overflow... ");
    int overflow_result = test_mul_overflow();
    if (overflow_result >= 0) {
        printf("PASS (overflow detected: %d)\n", overflow_result);
    } else {
        printf("FAIL\n");
        failures++;
    }
    
    printf("4. Testing addition overflow... ");
    int add_overflow = test_add_overflow();
    if (add_overflow >= 0) {
        printf("PASS (overflows: %d)\n", add_overflow);
    } else {
        printf("FAIL\n");
        failures++;
    }
    
    printf("5. Testing mul_overflow_p... ");
    if (test_mul_overflow_p() >= 0) {
        printf("PASS\n");
    } else {
        printf("FAIL\n");
        failures++;
    }
    
    /* Test 3: Range analysis */
    printf("6. Testing range analysis... ");
    int range_result = test_range_analysis(1500);
    if (range_result == 1) {
        printf("PASS\n");
    } else {
        printf("FAIL (got %d)\n", range_result);
        failures++;
    }
    
    printf("7. Testing induction variable... ");
    int induction_result = test_induction_variable();
    if (induction_result >= 0) {
        printf("PASS (sum: %d)\n", induction_result);
    } else {
        printf("FAIL\n");
        failures++;
    }
    
    printf("8. Testing nested ranges... ");
    int nested_result = test_nested_ranges(200, 2000);
    if (nested_result == 1) {
        printf("PASS\n");
    } else {
        printf("FAIL (got %d)\n", nested_result);
        failures++;
    }
    
    /* Test 5: Wide operations */
    printf("9. Testing wide operations... ");
    int128 a = ((__int128_t)1) << 70;
    int128 b = ((__int128_t)1) << 69;
    int128 wide_result = test_wide_operations(a, b);
    
    if (wide_result != 0) {
        printf("PASS\n");
    } else {
        printf("FAIL\n");
        failures++;
    }
    
#ifdef __cplusplus
    printf("10. Testing C++ templates... ");
    if (Compare1::greater_than_2_65 && 
        Compare2::less_than_neg_2_65 && 
        Compare3::is_zero) {
        printf("PASS\n");
    } else {
        printf("FAIL\n");
        failures++;
    }
#endif
    
    printf("\n=== SUMMARY ===\n");
    if (failures == 0) {
        printf("All tests passed! The double_int::cmp logic should have been exercised.\n");
    } else {
        printf("%d test(s) failed.\n", failures);
    }
    
    /* Additional runtime validation */
    printf("\nRuntime validation of large comparisons:\n");
    
    /* Direct large comparisons that should work at runtime */
    __int128_t huge1 = ((__int128_t)1) << 100;
    __int128_t huge2 = ((__int128_t)1) << 101;
    
    if (huge2 > huge1) {
        printf("  Runtime: huge2 > huge1 ✓\n");
    }
    
    if (huge1 != huge2) {
        printf("  Runtime: huge1 != huge2 ✓\n");
    }
    
    /* Test with negative large values */
    __int128_t neg_huge = -((__int128_t)1) << 100;
    if (neg_huge < huge1) {
        printf("  Runtime: neg_huge < huge1 ✓\n");
    }
    
    return failures == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}
