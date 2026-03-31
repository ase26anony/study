/* test_double_int_comparison.c */
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Define large 128-bit constants that exercise all comparison paths */

/* High part differs, low part equal */
#define LARGE_A_HIGH_DIFF_LOW_EQ ((unsigned __int128)0x10000000000000000ULL)  /* high: 1, low: 0 */
#define LARGE_B_HIGH_DIFF_LOW_EQ ((unsigned __int128)0x20000000000000000ULL)  /* high: 2, low: 0 */

/* High part equal, low part differs */
#define LARGE_A_HIGH_EQ_LOW_DIFF ((unsigned __int128)0x10000000000000001ULL)  /* high: 1, low: 1 */
#define LARGE_B_HIGH_EQ_LOW_DIFF ((unsigned __int128)0x10000000000000002ULL)  /* high: 1, low: 2 */

/* Both parts differ */
#define LARGE_A_BOTH_DIFF ((unsigned __int128)0x10000000000000001ULL)  /* high: 1, low: 1 */
#define LARGE_B_BOTH_DIFF ((unsigned __int128)0x20000000000000002ULL)  /* high: 2, low: 2 */

/* Edge cases with sign bits for signed comparisons */
#define SIGNED_NEG_ONE ((__int128)-1)  /* 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF */
#define SIGNED_ZERO ((__int128)0)
#define SIGNED_LARGE_POS ((__int128)0x7FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFULL)
#define SIGNED_LARGE_NEG ((__int128)0x80000000000000000000000000000000ULL)

/* Compile-time comparisons using static assertions */
_Static_assert(LARGE_A_HIGH_DIFF_LOW_EQ < LARGE_B_HIGH_DIFF_LOW_EQ, 
               "High part less comparison should pass");
_Static_assert(LARGE_B_HIGH_DIFF_LOW_EQ > LARGE_A_HIGH_DIFF_LOW_EQ, 
               "High part greater comparison should pass");
_Static_assert(LARGE_A_HIGH_EQ_LOW_DIFF < LARGE_B_HIGH_EQ_LOW_DIFF, 
               "Low part less comparison should pass");
_Static_assert(LARGE_B_HIGH_EQ_LOW_DIFF > LARGE_A_HIGH_EQ_LOW_DIFF, 
               "Low part greater comparison should pass");
_Static_assert(LARGE_A_BOTH_DIFF < LARGE_B_BOTH_DIFF, 
               "Both parts less comparison should pass");
_Static_assert(LARGE_B_BOTH_DIFF > LARGE_A_BOTH_DIFF, 
               "Both parts greater comparison should pass");

/* Signed comparisons that use unsigned semantics for high part */
_Static_assert(SIGNED_NEG_ONE < SIGNED_ZERO, 
               "Signed negative < zero should pass (uses unsigned high comparison)");
_Static_assert(SIGNED_ZERO > SIGNED_NEG_ONE, 
               "Signed zero > negative should pass");
_Static_assert(SIGNED_LARGE_NEG < SIGNED_LARGE_POS, 
               "Signed min < max should pass");

/* Constant expressions for array sizes */
char arr_high_less[(LARGE_A_HIGH_DIFF_LOW_EQ < LARGE_B_HIGH_DIFF_LOW_EQ) ? 10 : 20];
char arr_low_less[(LARGE_A_HIGH_EQ_LOW_DIFF < LARGE_B_HIGH_EQ_LOW_DIFF) ? 15 : 25];
char arr_both_less[(LARGE_A_BOTH_DIFF < LARGE_B_BOTH_DIFF) ? 20 : 30];

/* Runtime comparison function */
static int runtime_comparisons(void) {
    int checksum = 0;
    
    /* Volatile variables to force runtime evaluation */
    volatile unsigned __int128 v1 = LARGE_A_HIGH_DIFF_LOW_EQ;
    volatile unsigned __int128 v2 = LARGE_B_HIGH_DIFF_LOW_EQ;
    volatile unsigned __int128 v3 = LARGE_A_HIGH_EQ_LOW_DIFF;
    volatile unsigned __int128 v4 = LARGE_B_HIGH_EQ_LOW_DIFF;
    volatile unsigned __int128 v5 = LARGE_A_BOTH_DIFF;
    volatile unsigned __int128 v6 = LARGE_B_BOTH_DIFF;
    
    volatile __int128 sv1 = SIGNED_NEG_ONE;
    volatile __int128 sv2 = SIGNED_ZERO;
    volatile __int128 sv3 = SIGNED_LARGE_POS;
    volatile __int128 sv4 = SIGNED_LARGE_NEG;
    
    /* High part less */
    if (v1 < v2) checksum += 1;  /* Should take */
    if (v2 > v1) checksum += 2;  /* Should take */
    
    /* Low part less */
    if (v3 < v4) checksum += 4;  /* Should take */
    if (v4 > v3) checksum += 8;  /* Should take */
    
    /* Both parts differ */
    if (v5 < v6) checksum += 16; /* Should take */
    if (v6 > v5) checksum += 32; /* Should take */
    
    /* Equal comparisons */
    if (v1 == v1) checksum += 64;
    if (v2 == v2) checksum += 128;
    
    /* Signed comparisons with unsigned high part semantics */
    if (sv1 < sv2) checksum += 256;   /* -1 < 0 */
    if (sv2 > sv1) checksum += 512;   /* 0 > -1 */
    if (sv4 < sv3) checksum += 1024;  /* min < max */
    if (sv3 > sv4) checksum += 2048;  /* max > min */
    
    /* Less than or equal */
    if (v1 <= v2) checksum += 4096;
    if (v3 <= v4) checksum += 8192;
    if (sv1 <= sv2) checksum += 16384;
    
    /* Greater than or equal */
    if (v2 >= v1) checksum += 32768;
    if (v4 >= v3) checksum += 65536;
    if (sv2 >= sv1) checksum += 131072;
    
    return checksum;
}

/* Use GCC built-ins that may trigger double_int comparisons */
static int builtin_overflow_checks(void) {
    int checksum = 0;
    __int128 a = SIGNED_LARGE_POS;
    __int128 b = 1;
    __int128 result;
    int overflow;
    
    /* Multiplication overflow check */
    overflow = __builtin_mul_overflow(a, b, &result);
    checksum += overflow ? 1 : 2;
    
    /* Addition overflow check with large constants */
    unsigned __int128 ua = LARGE_A_BOTH_DIFF;
    unsigned __int128 ub = LARGE_B_BOTH_DIFF;
    unsigned __int128 uresult;
    
    overflow = __builtin_add_overflow(ua, ub, &uresult);
    checksum += overflow ? 4 : 8;
    
    /* Overflow check in constant expressions */
    const int const_overflow = __builtin_add_overflow_p(
        (unsigned __int128)0xFFFFFFFFFFFFFFFFULL, 
        (unsigned __int128)1, 
        (unsigned __int128)0);
    checksum += const_overflow ? 16 : 32;
    
    return checksum;
}

/* C++ version with constexpr (compile as .cpp to use) */
#ifdef __cplusplus
constexpr bool constexpr_compare_high_less() {
    const unsigned __int128 a = LARGE_A_HIGH_DIFF_LOW_EQ;
    const unsigned __int128 b = LARGE_B_HIGH_DIFF_LOW_EQ;
    return a < b;
}

constexpr bool constexpr_compare_low_less() {
    const unsigned __int128 a = LARGE_A_HIGH_EQ_LOW_DIFF;
    const unsigned __int128 b = LARGE_B_HIGH_EQ_LOW_DIFF;
    return a < b;
}

constexpr bool constexpr_compare_signed() {
    const __int128 a = SIGNED_NEG_ONE;
    const __int128 b = SIGNED_ZERO;
    return a < b;
}

template<unsigned __int128 A, unsigned __int128 B>
struct CompareTemplate {
    static const bool less = A < B;
    static const bool greater = A > B;
    static const bool equal = A == B;
};
#endif

int main(void) {
    int total_checksum = 0;
    
    /* Compile-time constant expressions */
    const int const_result = (LARGE_A_HIGH_DIFF_LOW_EQ < LARGE_B_HIGH_DIFF_LOW_EQ) ? 1 : 0;
    total_checksum += const_result;
    
    const int const_result2 = (LARGE_A_HIGH_EQ_LOW_DIFF <= LARGE_B_HIGH_EQ_LOW_DIFF) ? 2 : 0;
    total_checksum += const_result2;
    
    const int const_result3 = (SIGNED_NEG_ONE < SIGNED_ZERO) ? 4 : 0;
    total_checksum += const_result3;
    
    /* Runtime comparisons */
    total_checksum += runtime_comparisons();
    
    /* Built-in overflow checks */
    total_checksum += builtin_overflow_checks();
    
#ifdef __cplusplus
    /* C++ constexpr comparisons */
    static_assert(constexpr_compare_high_less(), "C++ constexpr high less");
    static_assert(constexpr_compare_low_less(), "C++ constexpr low less");
    static_assert(constexpr_compare_signed(), "C++ constexpr signed");
    
    /* Template comparisons */
    const bool t1 = CompareTemplate<LARGE_A_HIGH_DIFF_LOW_EQ, 
                                    LARGE_B_HIGH_DIFF_LOW_EQ>::less;
    const bool t2 = CompareTemplate<LARGE_B_HIGH_DIFF_LOW_EQ, 
                                    LARGE_A_HIGH_DIFF_LOW_EQ>::greater;
    total_checksum += t1 ? 1 : 0;
    total_checksum += t2 ? 2 : 0;
#endif
    
    printf("Total checksum: %d\n", total_checksum);
    
    /* Verify expected comparisons at runtime */
    assert(LARGE_A_HIGH_DIFF_LOW_EQ < LARGE_B_HIGH_DIFF_LOW_EQ);
    assert(LARGE_A_HIGH_EQ_LOW_DIFF < LARGE_B_HIGH_EQ_LOW_DIFF);
    assert(LARGE_A_BOTH_DIFF < LARGE_B_BOTH_DIFF);
    assert(SIGNED_NEG_ONE < SIGNED_ZERO);
    assert(SIGNED_LARGE_NEG < SIGNED_LARGE_POS);
    
    return 0;
}
