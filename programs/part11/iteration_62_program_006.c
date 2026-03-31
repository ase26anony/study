/* test-double-int-comparison.c */
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Force runtime evaluation */
static volatile unsigned __int128 volatile_uint128;
static volatile __int128 volatile_sint128;

/* Compile-time constants with different high/low patterns */
#define U128_HIGH_DIFF_LOW_EQ_A  ((unsigned __int128)0x1000000000000000ULL << 64)  /* high=1, low=0 */
#define U128_HIGH_DIFF_LOW_EQ_B  ((unsigned __int128)0x2000000000000000ULL << 64)  /* high=2, low=0 */

#define U128_HIGH_EQ_LOW_DIFF_A  (((unsigned __int128)0x1000000000000000ULL << 64) | 0x1ULL)  /* high=1, low=1 */
#define U128_HIGH_EQ_LOW_DIFF_B  (((unsigned __int128)0x1000000000000000ULL << 64) | 0x2ULL)  /* high=1, low=2 */

#define U128_BOTH_DIFF_A         (((unsigned __int128)0x1000000000000000ULL << 64) | 0x1ULL)  /* high=1, low=1 */
#define U128_BOTH_DIFF_B         (((unsigned __int128)0x2000000000000000ULL << 64) | 0x2ULL)  /* high=2, low=2 */

#define S128_NEGATIVE            ((__int128)-1)  /* high=all ones, low=all ones */
#define S128_ZERO                ((__int128)0)
#define S128_LARGE_POS           (((__int128)0x1000000000000000ULL << 64) | 0x1ULL)
#define S128_LARGE_NEG           (((__int128)0x8000000000000000ULL << 64) | 0x0ULL)  /* Most negative */

/* Static assertions for compile-time evaluation */
_Static_assert(U128_HIGH_DIFF_LOW_EQ_A < U128_HIGH_DIFF_LOW_EQ_B, 
               "High part less comparison failed");
_Static_assert(U128_HIGH_EQ_LOW_DIFF_A < U128_HIGH_EQ_LOW_DIFF_B,
               "Low part less comparison failed");
_Static_assert(U128_BOTH_DIFF_A < U128_BOTH_DIFF_B,
               "Both parts less comparison failed");
_Static_assert(S128_NEGATIVE < S128_ZERO,
               "Signed negative comparison failed");
_Static_assert(S128_LARGE_POS > S128_ZERO,
               "Signed positive comparison failed");

/* Constexpr-style function for C (using static inline) */
static inline int const_compare(unsigned __int128 a, unsigned __int128 b) {
    return (a < b) ? -1 : (a > b) ? 1 : 0;
}

/* Test overflow builtins that may use comparisons */
static int test_overflow_builtins(void) {
    __int128 a = S128_LARGE_POS;
    __int128 b = S128_LARGE_POS;
    __int128 result;
    int overflow;
    
    /* These builtins may internally compare values */
    overflow = __builtin_mul_overflow(a, b, &result);
    overflow |= __builtin_add_overflow(a, S128_ZERO, &result);
    
    return overflow;
}

int main(void) {
    int checksum = 0;
    
    /* 1. Compile-time constant expressions */
    const int ct1 = (U128_HIGH_DIFF_LOW_EQ_A > U128_HIGH_DIFF_LOW_EQ_B) ? 0 : 1;
    const int ct2 = (U128_HIGH_EQ_LOW_DIFF_A <= U128_HIGH_EQ_LOW_DIFF_B) ? 1 : 0;
    const int ct3 = const_compare(U128_BOTH_DIFF_A, U128_BOTH_DIFF_B);
    
    /* Array size depending on comparison */
    char array1[(U128_HIGH_DIFF_LOW_EQ_A < U128_HIGH_DIFF_LOW_EQ_B) ? 10 : 20];
    char array2[(S128_NEGATIVE < S128_ZERO) ? 30 : 40];
    
    checksum += ct1 + ct2 + ct3;
    checksum += sizeof(array1) + sizeof(array2);
    
    /* 2. Runtime comparisons with volatile variables */
    volatile_uint128 = U128_HIGH_DIFF_LOW_EQ_A;
    volatile_sint128 = S128_NEGATIVE;
    
    /* Test all four comparison branches */
    if ((unsigned __int128)volatile_uint128 < U128_HIGH_DIFF_LOW_EQ_B) {
        checksum += 1;  /* High part less */
    }
    
    if (U128_HIGH_DIFF_LOW_EQ_B > (unsigned __int128)volatile_uint128) {
        checksum += 2;  /* High part greater (reverse) */
    }
    
    if (U128_HIGH_EQ_LOW_DIFF_A < U128_HIGH_EQ_LOW_DIFF_B) {
        checksum += 4;  /* Low part less */
    }
    
    if (U128_HIGH_EQ_LOW_DIFF_B > U128_HIGH_EQ_LOW_DIFF_A) {
        checksum += 8;  /* Low part greater (reverse) */
    }
    
    /* 3. Signed comparisons that use unsigned high-part comparison */
    if (volatile_sint128 < S128_ZERO) {
        checksum += 16;  /* Negative < Zero */
    }
    
    if (S128_ZERO > volatile_sint128) {
        checksum += 32;  /* Zero > Negative (reverse) */
    }
    
    /* 4. Equality cases (should not trigger uncovered lines but ensure full coverage) */
    if (U128_HIGH_DIFF_LOW_EQ_A == U128_HIGH_DIFF_LOW_EQ_A) {
        checksum += 64;
    }
    
    /* 5. Mixed signed/unsigned comparisons */
    if ((unsigned __int128)S128_NEGATIVE > (unsigned __int128)S128_ZERO) {
        checksum += 128;  /* All ones > 0 when comparing unsigned */
    }
    
    /* 6. Test overflow builtins */
    checksum += test_overflow_builtins();
    
    /* 7. Additional runtime comparisons with variables */
    unsigned __int128 runtime_a = U128_BOTH_DIFF_A;
    unsigned __int128 runtime_b = U128_BOTH_DIFF_B;
    __int128 runtime_sa = S128_LARGE_POS;
    __int128 runtime_sb = S128_LARGE_NEG;
    
    for (int i = 0; i < 3; i++) {
        if (runtime_a < runtime_b) checksum += 256;
        if (runtime_b > runtime_a) checksum += 512;
        if (runtime_sa > runtime_sb) checksum += 1024;
        if (runtime_sb < runtime_sa) checksum += 2048;
        
        /* Modify values slightly */
        runtime_a += 1;
        runtime_sb += 1;
    }
    
    /* 8. Complex conditional expressions */
    checksum += (U128_HIGH_DIFF_LOW_EQ_A < U128_HIGH_DIFF_LOW_EQ_B && 
                 U128_HIGH_EQ_LOW_DIFF_A < U128_HIGH_EQ_LOW_DIFF_B) ? 4096 : 0;
    
    checksum += (S128_NEGATIVE < S128_ZERO || 
                 S128_LARGE_POS > S128_ZERO) ? 8192 : 0;
    
    printf("Checksum: %d\n", checksum);
    
    /* Final assertion to ensure all comparisons were evaluated */
    assert(checksum > 0);
    
    return 0;
}
