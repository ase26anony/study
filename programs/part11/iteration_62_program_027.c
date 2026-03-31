/* test_double_int_comparison.c */
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Define large 128-bit constants that exercise different comparison paths */

/* High part differs, low part equal */
#define LARGE_A_HIGH_DIFF_LOW_EQ ((unsigned __int128)0x10000000000000000ULL)  /* high=1, low=0 */
#define LARGE_B_HIGH_DIFF_LOW_EQ ((unsigned __int128)0x20000000000000000ULL)  /* high=2, low=0 */

/* High part equal, low part differs */
#define LARGE_A_HIGH_EQ_LOW_DIFF ((unsigned __int128)0x10000000000000001ULL)  /* high=1, low=1 */
#define LARGE_B_HIGH_EQ_LOW_DIFF ((unsigned __int128)0x10000000000000002ULL)  /* high=1, low=2 */

/* Both parts differ */
#define LARGE_A_BOTH_DIFF ((unsigned __int128)0x10000000000000001ULL)  /* high=1, low=1 */
#define LARGE_B_BOTH_DIFF ((unsigned __int128)0x20000000000000002ULL)  /* high=2, low=2 */

/* Edge cases with sign bits for signed comparisons */
#define SIGNED_NEG_ONE ((__int128)-1)  /* 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF */
#define SIGNED_ZERO ((__int128)0)
#define SIGNED_LARGE_POS ((__int128)0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFULL)
#define SIGNED_LARGE_NEG ((__int128)0x80000000000000000000000000000000ULL)

/* Compile-time comparisons using static assertions */
_Static_assert(LARGE_A_HIGH_DIFF_LOW_EQ < LARGE_B_HIGH_DIFF_LOW_EQ, 
               "High part less comparison should be true");
_Static_assert(LARGE_A_HIGH_EQ_LOW_DIFF < LARGE_B_HIGH_EQ_LOW_DIFF, 
               "Low part less comparison should be true");
_Static_assert(LARGE_A_BOTH_DIFF < LARGE_B_BOTH_DIFF, 
               "Both parts less comparison should be true");
_Static_assert(SIGNED_NEG_ONE < SIGNED_ZERO, 
               "Signed negative < zero should be true");

/* Constant expressions that force compile-time evaluation */
const int cmp_high_less = (LARGE_A_HIGH_DIFF_LOW_EQ < LARGE_B_HIGH_DIFF_LOW_EQ) ? 1 : 0;
const int cmp_low_less = (LARGE_A_HIGH_EQ_LOW_DIFF < LARGE_B_HIGH_EQ_LOW_DIFF) ? 1 : 0;
const int cmp_both_less = (LARGE_A_BOTH_DIFF < LARGE_B_BOTH_DIFF) ? 1 : 0;
const int cmp_signed_neg = (SIGNED_NEG_ONE < SIGNED_ZERO) ? 1 : 0;

/* Array size depending on comparison result */
char arr_high_greater[(LARGE_B_HIGH_DIFF_LOW_EQ > LARGE_A_HIGH_DIFF_LOW_EQ) ? 10 : 20];
char arr_low_greater[(LARGE_B_HIGH_EQ_LOW_DIFF > LARGE_A_HIGH_EQ_LOW_DIFF) ? 15 : 25];

/* Runtime comparison function that mixes signed and unsigned */
static int runtime_comparisons(void) {
    int checksum = 0;
    
    /* Use volatile to force runtime evaluation */
    volatile unsigned __int128 v1 = LARGE_A_HIGH_DIFF_LOW_EQ;
    volatile unsigned __int128 v2 = LARGE_B_HIGH_DIFF_LOW_EQ;
    volatile unsigned __int128 v3 = LARGE_A_HIGH_EQ_LOW_DIFF;
    volatile unsigned __int128 v4 = LARGE_B_HIGH_EQ_LOW_DIFF;
    volatile __int128 v5 = SIGNED_NEG_ONE;
    volatile __int128 v6 = SIGNED_ZERO;
    volatile __int128 v7 = SIGNED_LARGE_POS;
    volatile __int128 v8 = SIGNED_LARGE_NEG;
    
    /* Exercise all comparison operators */
    if (v1 < v2) checksum += 1;    /* high less */
    if (v2 > v1) checksum += 2;    /* high greater */
    if (v3 < v4) checksum += 4;    /* low less */
    if (v4 > v3) checksum += 8;    /* low greater */
    if (v5 < v6) checksum += 16;   /* signed negative less */
    if (v6 > v5) checksum += 32;   /* signed zero greater than negative */
    if (v7 > v8) checksum += 64;   /* signed positive greater than most negative */
    if (v8 < v7) checksum += 128;  /* signed most negative less than positive */
    
    /* Equality comparisons */
    if (v1 == v1) checksum += 256;
    if (v2 != v1) checksum += 512;
    
    /* Less than or equal, greater than or equal */
    if (v1 <= v2) checksum += 1024;
    if (v2 >= v1) checksum += 2048;
    if (v3 <= v4) checksum += 4096;
    if (v4 >= v3) checksum += 8192;
    
    return checksum;
}

/* Test built-in overflow functions that may use comparisons */
static int test_builtin_overflow(void) {
    int checksum = 0;
    __int128 a = SIGNED_LARGE_POS;
    __int128 b = 1;
    __int128 result;
    
    /* These built-ins may internally compare values */
    if (__builtin_mul_overflow(a, b, &result)) {
        checksum += 1;
    }
    
    if (__builtin_add_overflow_p(a, b, (__int128)0)) {
        checksum += 2;
    }
    
    unsigned __int128 ua = LARGE_B_BOTH_DIFF;
    unsigned __int128 ub = 1;
    unsigned __int128 uresult;
    
    if (__builtin_add_overflow(ua, ub, &uresult)) {
        checksum += 4;
    }
    
    return checksum;
}

/* C++ constexpr version (compile as C++ to use) */
#ifdef __cplusplus
constexpr bool constexpr_compare_high_less() {
    const unsigned __int128 x = LARGE_A_HIGH_DIFF_LOW_EQ;
    const unsigned __int128 y = LARGE_B_HIGH_DIFF_LOW_EQ;
    return x < y;
}

constexpr bool constexpr_compare_low_less() {
    const unsigned __int128 x = LARGE_A_HIGH_EQ_LOW_DIFF;
    const unsigned __int128 y = LARGE_B_HIGH_EQ_LOW_DIFF;
    return x < y;
}

constexpr bool constexpr_compare_signed() {
    const __int128 x = SIGNED_NEG_ONE;
    const __int128 y = SIGNED_ZERO;
    return x < y;
}

static_assert(constexpr_compare_high_less(), "C++ constexpr high less");
static_assert(constexpr_compare_low_less(), "C++ constexpr low less");
static_assert(constexpr_compare_signed(), "C++ constexpr signed");
#endif

int main(void) {
    int total_checksum = 0;
    
    /* Add compile-time comparison results */
    total_checksum += cmp_high_less;
    total_checksum += cmp_low_less;
    total_checksum += cmp_both_less;
    total_checksum += cmp_signed_neg;
    
    /* Runtime comparisons */
    total_checksum += runtime_comparisons();
    
    /* Built-in overflow tests */
    total_checksum += test_builtin_overflow();
    
    /* Verify array sizes were correctly determined */
    total_checksum += sizeof(arr_high_greater);
    total_checksum += sizeof(arr_low_greater);
    
    printf("Comparison checksum: %d\n", total_checksum);
    
    /* Additional assertions to ensure comparisons were evaluated */
    assert(cmp_high_less == 1);
    assert(cmp_low_less == 1);
    assert(cmp_both_less == 1);
    assert(cmp_signed_neg == 1);
    
    return 0;
}
