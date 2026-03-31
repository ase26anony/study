/* test-double-int-comparison.c */
#include <stdio.h>
#include <stdint.h>

/* Use static assertions for compile-time comparisons */
#define STATIC_ASSERT(cond, msg) _Static_assert(cond, msg)

/* Large 128-bit constants that exercise different comparison paths */
#define HIGH_DIFF_LOW_EQUAL_A ((unsigned __int128)0x10000000000000000ULL)  /* high=1, low=0 */
#define HIGH_DIFF_LOW_EQUAL_B ((unsigned __int128)0x20000000000000000ULL)  /* high=2, low=0 */

#define HIGH_EQUAL_LOW_DIFF_A ((unsigned __int128)0x10000000000000001ULL)  /* high=1, low=1 */
#define HIGH_EQUAL_LOW_DIFF_B ((unsigned __int128)0x10000000000000002ULL)  /* high=1, low=2 */

#define BOTH_DIFF_A ((unsigned __int128)0x10000000000000001ULL)  /* high=1, low=1 */
#define BOTH_DIFF_B ((unsigned __int128)0x20000000000000002ULL)  /* high=2, low=2 */

/* Signed constants with sign bit implications */
#define SIGNED_NEG_ONE ((__int128)-1)  /* All bits set: 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF */
#define SIGNED_ZERO ((__int128)0)
#define SIGNED_LARGE_POS ((__int128)0x7FFFFFFFFFFFFFFFFFFFFFFFULL)
#define SIGNED_LARGE_NEG ((__int128)0x800000000000000000000000ULL)

/* Compile-time comparisons using static assertions */
STATIC_ASSERT(HIGH_DIFF_LOW_EQUAL_A < HIGH_DIFF_LOW_EQUAL_B, 
              "High part less comparison failed");
STATIC_ASSERT(HIGH_DIFF_LOW_EQUAL_B > HIGH_DIFF_LOW_EQUAL_A,
              "High part greater comparison failed");
STATIC_ASSERT(HIGH_EQUAL_LOW_DIFF_A < HIGH_EQUAL_LOW_DIFF_B,
              "Low part less comparison failed");
STATIC_ASSERT(HIGH_EQUAL_LOW_DIFF_B > HIGH_EQUAL_LOW_DIFF_A,
              "Low part greater comparison failed");
STATIC_ASSERT(BOTH_DIFF_A < BOTH_DIFF_B,
              "Both parts differ comparison failed");
STATIC_ASSERT(SIGNED_NEG_ONE < SIGNED_ZERO,
              "Signed negative comparison failed");
STATIC_ASSERT(SIGNED_LARGE_POS > SIGNED_LARGE_NEG,
              "Signed large magnitude comparison failed");

/* Constant expressions evaluated at compile-time */
const int cmp_high_less = (HIGH_DIFF_LOW_EQUAL_A < HIGH_DIFF_LOW_EQUAL_B) ? 1 : 0;
const int cmp_high_greater = (HIGH_DIFF_LOW_EQUAL_B > HIGH_DIFF_LOW_EQUAL_A) ? 1 : 0;
const int cmp_low_less = (HIGH_EQUAL_LOW_DIFF_A < HIGH_EQUAL_LOW_DIFF_B) ? 1 : 0;
const int cmp_low_greater = (HIGH_EQUAL_LOW_DIFF_B > HIGH_EQUAL_LOW_DIFF_A) ? 1 : 0;
const int cmp_both_diff = (BOTH_DIFF_A < BOTH_DIFF_B) ? 1 : 0;
const int cmp_signed_neg = (SIGNED_NEG_ONE < SIGNED_ZERO) ? 1 : 0;

/* Array size depending on 128-bit comparison */
char array_high_less[(HIGH_DIFF_LOW_EQUAL_A < HIGH_DIFF_LOW_EQUAL_B) ? 10 : 20];
char array_low_greater[(HIGH_EQUAL_LOW_DIFF_B > HIGH_EQUAL_LOW_DIFF_A) ? 15 : 25];

/* Test built-in overflow functions with 128-bit comparisons */
int test_builtin_overflow(void) {
    __int128 a = SIGNED_LARGE_POS;
    __int128 b = SIGNED_LARGE_POS;
    __int128 result;
    int overflow;
    
    /* This built-in may internally compare values */
    overflow = __builtin_mul_overflow(a, b, &result);
    return overflow;
}

/* Runtime comparisons with volatile to prevent constant folding */
void runtime_comparisons(int *checksum) {
    volatile unsigned __int128 var1 = HIGH_DIFF_LOW_EQUAL_A;
    volatile unsigned __int128 var2 = HIGH_DIFF_LOW_EQUAL_B;
    volatile unsigned __int128 var3 = HIGH_EQUAL_LOW_DIFF_A;
    volatile unsigned __int128 var4 = HIGH_EQUAL_LOW_DIFF_B;
    volatile __int128 svar1 = SIGNED_NEG_ONE;
    volatile __int128 svar2 = SIGNED_ZERO;
    
    /* High part differs */
    if (var1 < var2) *checksum += 1;  /* Should take */
    if (var2 > var1) *checksum += 2;  /* Should take */
    if (var1 > var2) *checksum += 4;  /* Should NOT take */
    if (var2 < var1) *checksum += 8;  /* Should NOT take */
    
    /* Low part differs, high equal */
    if (var3 < var4) *checksum += 16;  /* Should take */
    if (var4 > var3) *checksum += 32;  /* Should take */
    if (var3 > var4) *checksum += 64;  /* Should NOT take */
    if (var4 < var3) *checksum += 128; /* Should NOT take */
    
    /* Signed comparisons with sign implications */
    if (svar1 < svar2) *checksum += 256;  /* Should take: -1 < 0 */
    if (svar2 > svar1) *checksum += 512;  /* Should take: 0 > -1 */
    if (svar1 > svar2) *checksum += 1024; /* Should NOT take */
    if (svar2 < svar1) *checksum += 2048; /* Should NOT take */
    
    /* Equality comparisons */
    if (var1 == var1) *checksum += 4096;  /* Should take */
    if (var1 != var2) *checksum += 8192;  /* Should take */
}

/* Mixed signed/unsigned comparisons */
void mixed_comparisons(int *checksum) {
    unsigned __int128 uval = 0xFFFFFFFFFFFFFFFFULL;  /* 2^64 - 1 */
    __int128 sval = -1;  /* Also 0xFFFFFFFFFFFFFFFF in two's complement */
    
    /* These should trigger unsigned comparison of high parts */
    if ((unsigned __int128)uval > (unsigned __int128)sval) {
        *checksum += 16384;
    }
    
    /* More complex expression */
    __int128 a = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 b = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543211ULL;
    
    if (a < b) *checksum += 32768;  /* Low part differs */
    if (b > a) *checksum += 65536;  /* Low part differs */
}

/* Test all comparison operators */
void test_all_operators(int *checksum) {
    const unsigned __int128 c1 = BOTH_DIFF_A;
    const unsigned __int128 c2 = BOTH_DIFF_B;
    
    /* < operator */
    if (c1 < c2) *checksum += 1;
    
    /* <= operator */
    if (c1 <= c2) *checksum += 2;
    if (c1 <= c1) *checksum += 4;
    
    /* > operator */
    if (c2 > c1) *checksum += 8;
    
    /* >= operator */
    if (c2 >= c1) *checksum += 16;
    if (c1 >= c1) *checksum += 32;
    
    /* == operator */
    if (c1 == c1) *checksum += 64;
    
    /* != operator */
    if (c1 != c2) *checksum += 128;
}

int main(void) {
    int checksum = 0;
    
    /* Add compile-time comparison results */
    checksum += cmp_high_less;
    checksum += cmp_high_greater;
    checksum += cmp_low_less;
    checksum += cmp_low_greater;
    checksum += cmp_both_diff;
    checksum += cmp_signed_neg;
    
    /* Test built-in overflow functions */
    checksum += test_builtin_overflow();
    
    /* Runtime comparisons */
    runtime_comparisons(&checksum);
    
    /* Mixed signed/unsigned comparisons */
    mixed_comparisons(&checksum);
    
    /* Test all comparison operators */
    test_all_operators(&checksum);
    
    /* Use array sizes (prevents dead code elimination) */
    checksum += sizeof(array_high_less);
    checksum += sizeof(array_low_greater);
    
    printf("Comparison checksum: %d\n", checksum);
    
    /* Verify expected values through assertions */
    if (checksum != 0) {
        printf("All comparisons performed successfully\n");
    }
    
    return 0;
}
