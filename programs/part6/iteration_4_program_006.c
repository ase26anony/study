/* test_double_int_cmp.c - Comprehensive test to trigger double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Test 1: Large integer constants and static assertions */
static const __int128_t VERY_LARGE_POS = ((__int128_t)1 << 70);
static const __int128_t VERY_LARGE_NEG = -((__int128_t)1 << 70);
static const __int128_t LARGE_MULT = ((__int128_t)0x7FFFFFFFFFFFFFFF) * 2;

/* Static assertions force compile-time comparison */
_Static_assert(VERY_LARGE_POS > 0, "Large positive constant");
_Static_assert(VERY_LARGE_NEG < 0, "Large negative constant");
_Static_assert(LARGE_MULT > INT64_MAX, "Multiplication overflow check");

/* Test 2: Template metaprogramming (C++ compatible) */
#ifdef __cplusplus
template <__int128_t N>
struct LargeCompare {
    static const bool greater_than_zero = N > 0;
    static const bool less_than_max = N < ((__int128_t)1 << 120);
};

template struct LargeCompare<((__int128_t)1 << 75)>;
template struct LargeCompare<-((__int128_t)1 << 75)>;
#endif

/* Test 3: Overflow builtins with comparison */
int test_overflow_builtins(void) {
    int64_t a, b;
    int64_t res;
    int overflow;
    
    /* Test cases that should trigger overflow detection */
    a = INT64_MAX;
    b = 2;
    overflow = __builtin_mul_overflow(a, b, &res);
    if (overflow) {
        printf("Overflow detected: %lld * 2\n", (long long)a);
    }
    
    a = INT64_MIN;
    b = -1;
    overflow = __builtin_mul_overflow(a, b, &res);
    if (overflow) {
        printf("Overflow detected: %lld * -1\n", (long long)a);
    }
    
    /* Constant overflow checks */
    if (__builtin_constant_p(VERY_LARGE_POS > VERY_LARGE_NEG)) {
        printf("Constant comparison evaluated\n");
    }
    
    return 0;
}

/* Test 4: Range analysis with complex conditions */
void test_range_analysis(int x) {
    /* Create known bounds for range analysis */
    if (x > 1000 && x < 2000) {
        /* This multiplication's range calculation uses double_int::cmp */
        long long y = (long long)x * x;
        
        /* Further comparisons with large constants */
        if (y > ((int64_t)1 << 40)) {
            printf("y is very large: %lld\n", y);
        }
        
        /* Nested range checks */
        if (x > 1500) {
            long long z = y * 2;
            if (z < 0) {
                printf("Potential overflow in range\n");
            }
        }
    }
    
    /* Test with negative ranges */
    if (x < -1000 && x > -2000) {
        long long y = (long long)x * x;
        if (y > ((int64_t)1 << 40)) {
            printf("Negative x gives large y: %lld\n", y);
        }
    }
}

/* Test 5: Loop induction variables with large steps */
void test_loop_induction(void) {
    for (int64_t i = INT64_MAX - 100; i < INT64_MAX; i += 1000000) {
        /* Loop analysis may use double_int for wrap-around checks */
        if (i > INT64_MAX - 50) {
            printf("Near overflow: %lld\n", (long long)i);
            break;
        }
    }
    
    /* Large step that could overflow */
    for (int64_t j = 0; j < INT64_MAX; j += ((int64_t)1 << 60)) {
        /* Empty loop - compiler analyzes bounds */
        (void)j;
    }
}

/* Test 6: Enumeration with large values */
enum big_enum : __int128 {
    BIG_VALUE = ((__int128_t)1 << 65),
    BIGGER_VALUE = ((__int128_t)1 << 66),
    BIG_NEGATIVE = -((__int128_t)1 << 65)
};

/* Test 7: Compile-time comparisons in switch statements */
void test_switch_large(__int128_t val) {
    switch (val) {
        case ((__int128_t)1 << 63):
            printf("Case 2^63\n");
            break;
        case ((__int128_t)1 << 64):
            printf("Case 2^64\n");
            break;
        default:
            printf("Other large value\n");
    }
}

/* Test 8: Bitfield operations (non-portable but may trigger paths) */
struct large_bitfield {
    __int128_t field1 : 70;
    __int128_t field2 : 70;
};

/* Test 9: Array indexing with large constants */
void test_array_index(void) {
    char buffer[100];
    
    /* Comparisons with large indices */
    __int128_t idx = ((__int128_t)1 << 65);
    if (idx > sizeof(buffer)) {
        printf("Index out of bounds (large constant)\n");
    }
}

/* Test 10: Mixed signed/unsigned comparisons */
void test_mixed_comparisons(void) {
    unsigned __int128 ubig = ((unsigned __int128)1 << 70);
    __int128_t sbig = ((__int128_t)1 << 70);
    
    /* These comparisons may use different code paths */
    if (__builtin_constant_p(ubig > sbig)) {
        printf("Mixed signed/unsigned constant comparison\n");
    }
    
    if (__builtin_constant_p(ubig < 0)) {
        printf("Unsigned compared to zero\n");
    }
}

/* Test 11: Division and modulus with large numbers */
void test_division_large(void) {
    __int128_t dividend = ((__int128_t)1 << 100);
    __int128_t divisor = ((__int128_t)1 << 30);
    
    /* Division requires magnitude comparison */
    __int128_t quotient = dividend / divisor;
    printf("Large division result: %lld (high bits)\n", 
           (long long)(quotient >> 64));
    
    /* Modulus operation */
    __int128_t remainder = dividend % divisor;
    printf("Remainder: %lld\n", (long long)remainder);
}

/* Test 12: Shift operations beyond word size */
void test_large_shifts(void) {
    __int128_t val = 1;
    
    /* Shifts that require double_int representation */
    __int128_t shifted = val << 80;
    if (shifted > 0) {
        printf("Large shift produced positive result\n");
    }
    
    /* Right shift of large value */
    __int128_t large_val = ((__int128_t)1 << 100);
    __int128_t right_shifted = large_val >> 50;
    if (right_shifted < large_val) {
        printf("Right shift reduced value\n");
    }
}

/* Main test driver */
int main(void) {
    int test_result = 0;
    
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
    /* Test 1: Verify static assertions passed */
    printf("Test 1: Static assertions - PASS\n");
    
    /* Test 2: Overflow builtins */
    printf("\nTest 2: Overflow builtins\n");
    test_result |= test_overflow_builtins();
    
    /* Test 3: Range analysis */
    printf("\nTest 3: Range analysis\n");
    test_range_analysis(1500);
    test_range_analysis(-1500);
    
    /* Test 4: Loop induction */
    printf("\nTest 4: Loop induction variables\n");
    test_loop_induction();
    
    /* Test 5: Enumeration */
    printf("\nTest 5: Large enumeration values\n");
    printf("BIG_VALUE = %lld (high), BIGGER_VALUE = %lld (high)\n",
           (long long)(BIG_VALUE >> 64),
           (long long)(BIGGER_VALUE >> 64));
    
    /* Test 6: Switch with large values */
    printf("\nTest 6: Switch with large constants\n");
    test_switch_large(((__int128_t)1 << 63));
    
    /* Test 7: Array indexing */
    printf("\nTest 7: Array index bounds checking\n");
    test_array_index();
    
    /* Test 8: Mixed comparisons */
    printf("\nTest 8: Mixed signed/unsigned comparisons\n");
    test_mixed_comparisons();
    
    /* Test 9: Division */
    printf("\nTest 9: Division with large numbers\n");
    test_division_large();
    
    /* Test 10: Large shifts */
    printf("\nTest 10: Shift operations\n");
    test_large_shifts();
    
    /* Additional runtime validation */
    printf("\n=== Runtime validation ===\n");
    
    /* Verify some comparisons work correctly at runtime */
    __int128_t a = ((__int128_t)1 << 70);
    __int128_t b = ((__int128_t)1 << 69);
    
    if (a > b) {
        printf("Runtime comparison 1: 2^70 > 2^69 - PASS\n");
    } else {
        printf("Runtime comparison 1: FAIL\n");
        test_result = 1;
    }
    
    if (b < a) {
        printf("Runtime comparison 2: 2^69 < 2^70 - PASS\n");
    } else {
        printf("Runtime comparison 2: FAIL\n");
        test_result = 1;
    }
    
    /* Test equality */
    __int128_t c = ((__int128_t)1 << 70);
    if (a == c) {
        printf("Runtime equality: 2^70 == 2^70 - PASS\n");
    } else {
        printf("Runtime equality: FAIL\n");
        test_result = 1;
    }
    
    /* Test negative comparisons */
    __int128_t neg_a = -((__int128_t)1 << 70);
    __int128_t neg_b = -((__int128_t)1 << 69);
    
    if (neg_a < neg_b) {
        printf("Runtime comparison 3: -2^70 < -2^69 - PASS\n");
    } else {
        printf("Runtime comparison 3: FAIL\n");
        test_result = 1;
    }
    
    printf("\n=== All tests completed ===\n");
    
    return test_result;
}
