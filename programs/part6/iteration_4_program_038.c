/* test_double_int_cmp.c - Comprehensive test for double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

/* ========== 1. Trigger Constant Folding with Large Integers ========== */

/* Use __int128 for wide integer operations */
#define LARGE_CONST_1 (((__int128_t)1) << 70)      /* 2^70 */
#define LARGE_CONST_2 (((__int128_t)1) << 65)      /* 2^65 */
#define LARGE_CONST_3 (((__int128_t)0x123456789ABCDEF) << 32)
#define LARGE_NEGATIVE ((__int128_t)-1 << 80)

/* Static assertions force compile-time comparisons */
static_assert(LARGE_CONST_1 > LARGE_CONST_2, "Large constant comparison 1");
static_assert(LARGE_CONST_2 < LARGE_CONST_1, "Large constant comparison 2");
static_assert(LARGE_CONST_3 > 0, "Positive large constant");
static_assert(LARGE_NEGATIVE < 0, "Negative large constant");

/* Template-like macro for compile-time comparisons */
#define COMPILE_TIME_CMP(a, b, op) \
    do { \
        if (__builtin_constant_p((a) op (b))) { \
            static_assert((a) op (b), #a " " #op " " #b); \
        } \
    } while(0)

/* Force evaluation at compile time */
const __int128_t product = ((__int128_t)0x7FFFFFFFFFFFFFFF) * 2;
static_assert(product > 0x7FFFFFFFFFFFFFFF, "Multiplication overflow comparison");

/* ========== 2. GCC Builtins That Return or Manipulate double_int ========== */

/* Test overflow builtins with large values */
void test_overflow_builtins(void) {
    long long a, b;
    long long res;
    int overflow;
    
    /* Case 1: Multiplication that overflows 64-bit */
    a = 0x7FFFFFFFFFFFFFFFLL;  /* Max positive int64 */
    b = 2;
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("Mul overflow test 1: overflow=%d, res=%lld\n", overflow, res);
    
    /* Case 2: Large multiplication with overflow check */
    a = 0x123456789ABCDEFLL;
    b = 0xFEDCBA987654321LL;
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("Mul overflow test 2: overflow=%d\n", overflow);
    
    /* Case 3: Addition overflow */
    a = 0x7FFFFFFFFFFFFFFFLL;
    b = 1;
    overflow = __builtin_add_overflow(a, b, &res);
    printf("Add overflow test: overflow=%d\n", overflow);
    
    /* Case 4: Subtraction underflow */
    a = -0x7FFFFFFFFFFFFFFFLL - 1;
    b = 1;
    overflow = __builtin_sub_overflow(a, b, &res);
    printf("Sub overflow test: overflow=%d\n", overflow);
    
    /* __builtin_constant_p with overflow checks */
    if (__builtin_constant_p(__builtin_mul_overflow_p(0x7FFFFFFFFFFFFFFFLL, 2, 0LL))) {
        printf("Constant overflow check triggered\n");
    }
}

/* ========== 3. Range Calculations That Compare Bounds ========== */

void test_range_calculations(int x) {
    /* Create complex range analysis scenarios */
    
    /* Scenario 1: Known bounds with multiplication */
    if (x > 1000 && x < 2000) {
        /* This multiplication's range calculation uses double_int::cmp */
        long long y = (long long)x * x;
        
        /* Further comparisons with the result */
        if (y > 1000000 && y < 4000000) {
            printf("Range test 1 passed: y=%lld\n", y);
        }
    }
    
    /* Scenario 2: Large range with overflow potential */
    if (x > 0x100000000 && x < 0x200000000) {
        /* 64-bit multiplication of values > 2^32 */
        unsigned long long ux = (unsigned long long)x;
        unsigned long long prod = ux * ux;
        
        /* Comparisons that require wide integer logic */
        if (prod > ((unsigned long long)1 << 63)) {
            printf("Range test 2: prod > 2^63\n");
        }
    }
    
    /* Scenario 3: Loop with induction variable analysis */
    for (long long i = 0x7000000000000000LL; i < 0x7800000000000000LL; i += 0x100000000LL) {
        /* Loop analysis may use double_int for wrap-around checks */
        if (i > 0x7500000000000000LL) {
            printf("Loop analysis test: i=%lld\n", i);
            break;
        }
    }
}

/* ========== 4. Template Metaprogramming (C++ version available) ========== */

#ifdef __cplusplus
/* C++ specific tests with templates */
template <__int128_t N>
struct LargeCompare {
    static const bool greater_than_2_65 = N > (((__int128_t)1) << 65);
    static const bool less_than_neg_2_70 = N < (((__int128_t)-1) << 70);
    static const bool equals_self = N == N;
};

/* Instantiate templates with various large values */
template struct LargeCompare<((__int128_t)1) << 66>;
template struct LargeCompare<((__int128_t)-1) << 75>;
template struct LargeCompare<0x123456789ABCDEF0123456789ABCDEF>;
#endif

/* ========== 5. Force Tree Node Construction for Wide Constants ========== */

/* Use __int128 with attributes */
typedef __int128_t __attribute__((mode(TI))) wide_int;

/* Operations that require magnitude comparisons */
wide_int wide_division(wide_int a, wide_int b) {
    /* Division requires comparing magnitudes */
    return a / b;
}

wide_int wide_modulus(wide_int a, wide_int b) {
    /* Modulus requires comparing magnitudes */
    return a % b;
}

/* Enumeration with large values */
enum big_enum : __int128 {
    BIG_VAL1 = ((__int128_t)1) << 70,
    BIG_VAL2 = ((__int128_t)1) << 80,
    BIG_VAL3 = ((__int128_t)-1) << 60
};

/* ========== Runtime Validation ========== */

int main(void) {
    int all_passed = 1;
    
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
    /* Test 1: Basic large constant comparisons */
    printf("Test 1: Large constant comparisons\n");
    const __int128_t very_large = ((__int128_t)1 << 70);
    const __int128_t another_large = ((__int128_t)1 << 65);
    
    /* These comparisons should trigger double_int::cmp */
    if (very_large > another_large) {
        printf("  ✓ very_large > another_large\n");
    } else {
        printf("  ✗ Comparison failed\n");
        all_passed = 0;
    }
    
    if (very_large != another_large) {
        printf("  ✓ very_large != another_large\n");
    }
    
    /* Test 2: Overflow builtins */
    printf("\nTest 2: Overflow builtins\n");
    test_overflow_builtins();
    
    /* Test 3: Range calculations */
    printf("\nTest 3: Range calculations\n");
    test_range_calculations(1500);
    test_range_calculations(0x180000000);
    
    /* Test 4: Wide integer operations */
    printf("\nTest 4: Wide integer operations\n");
    wide_int w1 = ((wide_int)0x7FFFFFFFFFFFFFFF) << 10;
    wide_int w2 = ((wide_int)0x3FFFFFFFFFFFFFFF) << 11;
    
    if (w1 > w2) {
        printf("  ✓ w1 > w2\n");
    } else {
        printf("  ✗ w1 <= w2\n");
        all_passed = 0;
    }
    
    /* Test division and modulus (require magnitude comparisons) */
    wide_int w3 = wide_division(w1, w2);
    wide_int w4 = wide_modulus(w1, w2);
    printf("  w1 / w2 = ... (truncated)\n");
    printf("  w1 %% w2 = ... (truncated)\n");
    
    /* Test 5: Mixed signedness comparisons */
    printf("\nTest 5: Mixed signedness comparisons\n");
    __int128_t signed_large = -((__int128_t)1 << 70);
    unsigned __int128 unsigned_large = (unsigned __int128)1 << 71;
    
    /* This should compare high parts as unsigned */
    if ((unsigned __int128)signed_large < unsigned_large) {
        printf("  ✓ (unsigned)signed_large < unsigned_large\n");
    } else {
        printf("  ✗ Unexpected comparison result\n");
        all_passed = 0;
    }
    
    /* Test 6: Edge cases for lexicographic comparison */
    printf("\nTest 6: Edge cases\n");
    __int128_t a = ((__int128_t)0x1) << 64;
    __int128_t b = a + 1;
    
    if (a < b) {
        printf("  ✓ a < b (same high, different low)\n");
    } else {
        printf("  ✗ a >= b\n");
        all_passed = 0;
    }
    
    /* Same high part, different low part */
    __int128_t c = ((__int128_t)0xFFFFFFFFFFFFFFFF) << 64;
    __int128_t d = c | 0x1;
    
    if (c < d) {
        printf("  ✓ c < d (same high, different low, negative)\n");
    }
    
    /* Final result */
    printf("\n=== Test %s ===\n", all_passed ? "PASSED" : "FAILED");
    
    return all_passed ? 0 : 1;
}
