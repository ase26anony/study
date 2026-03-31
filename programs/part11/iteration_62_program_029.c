/* test_double_int_comparison.c */
#include <stdio.h>
#include <stdint.h>

/* Force runtime evaluation */
static volatile __int128 volatile_sint128;
static volatile unsigned __int128 volatile_uint128;

/* Large constants that exercise different comparison paths */
#define HIGH_DIFF_LOW_EQUAL_A ((unsigned __int128)0x10000000000000000ULL)  /* high:1, low:0 */
#define HIGH_DIFF_LOW_EQUAL_B ((unsigned __int128)0x20000000000000000ULL)  /* high:2, low:0 */

#define HIGH_EQUAL_LOW_DIFF_A ((unsigned __int128)0x10000000000000001ULL)  /* high:1, low:1 */
#define HIGH_EQUAL_LOW_DIFF_B ((unsigned __int128)0x10000000000000002ULL)  /* high:1, low:2 */

#define BOTH_DIFF_A ((unsigned __int128)0x10000000000000001ULL)  /* high:1, low:1 */
#define BOTH_DIFF_B ((unsigned __int128)0x20000000000000002ULL)  /* high:2, low:2 */

/* Signed constants with sign bit implications */
#define SIGNED_NEG_ONE ((__int128)-1)  /* All bits set: 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF */
#define SIGNED_ZERO    ((__int128)0)
#define SIGNED_LARGE_POS ((__int128)0x7FFFFFFFFFFFFFFFFFFFFFFFULL)  /* Large positive */
#define SIGNED_LARGE_NEG ((__int128)(-0x800000000000000000000000ULL)) /* Large negative */

/* Compile-time comparisons using static assertions */
/* Test 1: High part differs, low part equal (unsigned) */
_Static_assert(HIGH_DIFF_LOW_EQUAL_A < HIGH_DIFF_LOW_EQUAL_B, 
               "High diff, low equal: A should be less than B");
_Static_assert(HIGH_DIFF_LOW_EQUAL_B > HIGH_DIFF_LOW_EQUAL_A,
               "High diff, low equal: B should be greater than A");

/* Test 2: High part equal, low part differs (unsigned) */
_Static_assert(HIGH_EQUAL_LOW_DIFF_A < HIGH_EQUAL_LOW_DIFF_B,
               "High equal, low diff: A should be less than B");
_Static_assert(HIGH_EQUAL_LOW_DIFF_B > HIGH_EQUAL_LOW_DIFF_A,
               "High equal, low diff: B should be greater than A");

/* Test 3: Both parts differ (unsigned) */
_Static_assert(BOTH_DIFF_A < BOTH_DIFF_B,
               "Both diff: A should be less than B");
_Static_assert(BOTH_DIFF_B > BOTH_DIFF_A,
               "Both diff: B should be greater than A");

/* Test 4: Signed comparisons with sign bit handling */
_Static_assert(SIGNED_NEG_ONE < SIGNED_ZERO,
               "Signed: -1 should be less than 0");
_Static_assert(SIGNED_LARGE_NEG < SIGNED_LARGE_POS,
               "Signed: large negative should be less than large positive");

/* Constant expressions that force compile-time evaluation */
const int cmp_high_diff = (HIGH_DIFF_LOW_EQUAL_A <= HIGH_DIFF_LOW_EQUAL_B) ? 1 : 0;
const int cmp_high_equal = (HIGH_EQUAL_LOW_DIFF_A >= HIGH_EQUAL_LOW_DIFF_A) ? 1 : 0;
const int cmp_both_diff = (BOTH_DIFF_A != BOTH_DIFF_B) ? 1 : 0;
const int cmp_signed = (SIGNED_NEG_ONE == SIGNED_NEG_ONE) ? 1 : 0;

/* Array size depending on comparison result */
char array_high_diff[(HIGH_DIFF_LOW_EQUAL_A < HIGH_DIFF_LOW_EQUAL_B) ? 10 : 20];
char array_low_diff[(HIGH_EQUAL_LOW_DIFF_A > HIGH_EQUAL_LOW_DIFF_B) ? 10 : 20];

/* Runtime comparison function */
static int runtime_comparisons(void) {
    int checksum = 0;
    
    /* Runtime comparisons with volatile variables */
    volatile_sint128 = SIGNED_NEG_ONE;
    volatile_uint128 = HIGH_DIFF_LOW_EQUAL_A;
    
    /* Test various comparison operators */
    if (volatile_sint128 < SIGNED_ZERO) checksum += 1;      /* Should be true */
    if (volatile_sint128 <= SIGNED_NEG_ONE) checksum += 2;  /* Should be true */
    if (volatile_sint128 > SIGNED_LARGE_NEG) checksum += 4; /* Should be true */
    
    if (volatile_uint128 < HIGH_DIFF_LOW_EQUAL_B) checksum += 8;   /* Should be true */
    if (volatile_uint128 <= HIGH_DIFF_LOW_EQUAL_A) checksum += 16; /* Should be true */
    if (volatile_uint128 > 0) checksum += 32;                     /* Should be true */
    
    /* Force evaluation of both high and low parts */
    volatile_uint128 = HIGH_EQUAL_LOW_DIFF_A;
    if (volatile_uint128 < HIGH_EQUAL_LOW_DIFF_B) checksum += 64;  /* Should be true */
    if (volatile_uint128 > 0) checksum += 128;                     /* Should be true */
    
    volatile_uint128 = BOTH_DIFF_A;
    if (volatile_uint128 < BOTH_DIFF_B) checksum += 256;           /* Should be true */
    if (volatile_uint128 != BOTH_DIFF_B) checksum += 512;          /* Should be true */
    
    /* Mix signed and unsigned comparisons */
    volatile_sint128 = SIGNED_LARGE_POS;
    if (volatile_sint128 > SIGNED_LARGE_NEG) checksum += 1024;     /* Should be true */
    
    return checksum;
}

/* Test GCC built-in functions that may use double_int comparisons */
static int test_builtins(void) {
    int checksum = 0;
    __int128 a, b, result;
    int overflow;
    
    /* Test overflow builtins with large values */
    a = ((__int128)0x7FFFFFFFFFFFFFFFULL) << 32;
    b = 2;
    overflow = __builtin_mul_overflow(a, b, &result);
    if (overflow) checksum += 1;  /* May overflow */
    
    /* Test with unsigned __int128 */
    unsigned __int128 ua = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL) << 64;
    unsigned __int128 ub = 1;
    overflow = __builtin_add_overflow_p(ua, ub, (unsigned __int128)0);
    if (!overflow) checksum += 2;  /* Should not overflow */
    
    return checksum;
}

/* C++ version with constexpr (compile as C++ with g++) */
#ifdef __cplusplus
#include <cstdint>

constexpr bool constexpr_compare_high_diff() {
    const unsigned __int128 a = HIGH_DIFF_LOW_EQUAL_A;
    const unsigned __int128 b = HIGH_DIFF_LOW_EQUAL_B;
    return a < b;  /* Should return true */
}

constexpr bool constexpr_compare_low_diff() {
    const unsigned __int128 a = HIGH_EQUAL_LOW_DIFF_A;
    const unsigned __int128 b = HIGH_EQUAL_LOW_DIFF_B;
    return a > b;  /* Should return false */
}

template<unsigned __int128 A, unsigned __int128 B>
struct CompareTemplate {
    static constexpr bool less = A < B;
    static constexpr bool greater = A > B;
    static constexpr bool equal = A == B;
};

/* Force template instantiation with different comparison scenarios */
constexpr bool template_test1 = CompareTemplate<HIGH_DIFF_LOW_EQUAL_A, 
                                                HIGH_DIFF_LOW_EQUAL_B>::less;
constexpr bool template_test2 = CompareTemplate<HIGH_EQUAL_LOW_DIFF_A,
                                                HIGH_EQUAL_LOW_DIFF_B>::greater;
#endif

int main(void) {
    int total_checksum = 0;
    
    /* Add compile-time comparison results */
    total_checksum += cmp_high_diff;
    total_checksum += cmp_high_equal;
    total_checksum += cmp_both_diff;
    total_checksum += cmp_signed;
    
    /* Runtime comparisons */
    total_checksum += runtime_comparisons();
    
    /* Built-in function tests */
    total_checksum += test_builtins();
    
#ifdef __cplusplus
    /* C++ constexpr tests */
    static_assert(constexpr_compare_high_diff(), 
                  "C++ constexpr: high diff comparison failed");
    static_assert(!constexpr_compare_low_diff(),
                  "C++ constexpr: low diff comparison failed");
    static_assert(template_test1, "Template comparison 1 failed");
    static_assert(!template_test2, "Template comparison 2 failed");
    
    total_checksum += constexpr_compare_high_diff() ? 2048 : 0;
    total_checksum += constexpr_compare_low_diff() ? 4096 : 0;
#endif
    
    printf("Comparison checksum: %d\n", total_checksum);
    printf("Array sizes: %zu, %zu\n", 
           sizeof(array_high_diff), sizeof(array_low_diff));
    
    return 0;
}
