/* test-double-int-comparison.c */
#include <stdio.h>
#include <stdint.h>

/* Force runtime evaluation */
static volatile __int128 volatile_sint128;
static volatile unsigned __int128 volatile_uint128;

/* Large constants that exercise different comparison paths */
#define HIGH_PART_DIFFERS_LOW_EQUAL_A ((unsigned __int128)0x10000000000000000ULL) /* high:1, low:0 */
#define HIGH_PART_DIFFERS_LOW_EQUAL_B ((unsigned __int128)0x20000000000000000ULL) /* high:2, low:0 */

#define HIGH_EQUAL_LOW_DIFFERS_A ((unsigned __int128)0x10000000000000001ULL) /* high:1, low:1 */
#define HIGH_EQUAL_LOW_DIFFERS_B ((unsigned __int128)0x10000000000000002ULL) /* high:1, low:2 */

#define BOTH_PARTS_DIFFER_A ((unsigned __int128)0x10000000000000001ULL) /* high:1, low:1 */
#define BOTH_PARTS_DIFFER_B ((unsigned __int128)0x20000000000000002ULL) /* high:2, low:2 */

/* Signed constants with sign bit implications */
#define SIGNED_NEG_ONE ((__int128)-1) /* 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF */
#define SIGNED_ZERO ((__int128)0)
#define SIGNED_LARGE_POS ((__int128)0x7FFFFFFFFFFFFFFFFFFFFFFFULL)
#define SIGNED_LARGE_NEG ((__int128)(-0x800000000000000000000000ULL))

/* Compile-time assertions to force constant folding */
_Static_assert(HIGH_PART_DIFFERS_LOW_EQUAL_A < HIGH_PART_DIFFERS_LOW_EQUAL_B, 
               "High part differs, low equal: A < B should be true");
_Static_assert(HIGH_EQUAL_LOW_DIFFERS_A < HIGH_EQUAL_LOW_DIFFERS_B,
               "High equal, low differs: A < B should be true");
_Static_assert(BOTH_PARTS_DIFFER_A < BOTH_PARTS_DIFFER_B,
               "Both parts differ: A < B should be true");
_Static_assert(SIGNED_NEG_ONE < SIGNED_ZERO,
               "Signed -1 < 0 should be true");

/* Test comparisons in constant expressions */
const int cmp_high_less = (HIGH_PART_DIFFERS_LOW_EQUAL_A < HIGH_PART_DIFFERS_LOW_EQUAL_B) ? 1 : 0;
const int cmp_high_greater = (HIGH_PART_DIFFERS_LOW_EQUAL_B > HIGH_PART_DIFFERS_LOW_EQUAL_A) ? 1 : 0;
const int cmp_low_less = (HIGH_EQUAL_LOW_DIFFERS_A < HIGH_EQUAL_LOW_DIFFERS_B) ? 1 : 0;
const int cmp_low_greater = (HIGH_EQUAL_LOW_DIFFERS_B > HIGH_EQUAL_LOW_DIFFERS_A) ? 1 : 0;
const int cmp_both_less = (BOTH_PARTS_DIFFER_A < BOTH_PARTS_DIFFER_B) ? 1 : 0;
const int cmp_both_greater = (BOTH_PARTS_DIFFER_B > BOTH_PARTS_DIFFER_A) ? 1 : 0;

/* Array size depending on comparison result */
char array_high_less[(HIGH_PART_DIFFERS_LOW_EQUAL_A < HIGH_PART_DIFFERS_LOW_EQUAL_B) ? 10 : 20];
char array_low_less[(HIGH_EQUAL_LOW_DIFFERS_A < HIGH_EQUAL_LOW_DIFFERS_B) ? 15 : 25];

/* Test built-in overflow functions with 128-bit comparisons */
static int test_builtin_overflow(void) {
    __int128 a = 0x7FFFFFFFFFFFFFFFULL; /* Max int64_t */
    __int128 b = 2;
    __int128 result;
    int overflow = __builtin_mul_overflow(a, b, &result);
    return overflow; /* Should be 0 (no overflow for 128-bit) */
}

/* Runtime comparison tests */
static int runtime_comparisons(void) {
    int checksum = 0;
    
    /* Test 1: High part differs, low part equal */
    unsigned __int128 r1_a = HIGH_PART_DIFFERS_LOW_EQUAL_A;
    unsigned __int128 r1_b = HIGH_PART_DIFFERS_LOW_EQUAL_B;
    if (r1_a < r1_b) checksum += 1;  /* Should take */
    if (r1_b > r1_a) checksum += 2;  /* Should take */
    if (r1_a > r1_b) checksum += 4;  /* Should NOT take */
    if (r1_b < r1_a) checksum += 8;  /* Should NOT take */
    
    /* Test 2: High part equal, low part differs */
    unsigned __int128 r2_a = HIGH_EQUAL_LOW_DIFFERS_A;
    unsigned __int128 r2_b = HIGH_EQUAL_LOW_DIFFERS_B;
    if (r2_a < r2_b) checksum += 16;  /* Should take */
    if (r2_b > r2_a) checksum += 32;  /* Should take */
    if (r2_a > r2_b) checksum += 64;  /* Should NOT take */
    if (r2_b < r2_a) checksum += 128; /* Should NOT take */
    
    /* Test 3: Both parts differ */
    unsigned __int128 r3_a = BOTH_PARTS_DIFFER_A;
    unsigned __int128 r3_b = BOTH_PARTS_DIFFER_B;
    if (r3_a < r3_b) checksum += 256;  /* Should take */
    if (r3_b > r3_a) checksum += 512;  /* Should take */
    
    /* Test 4: Signed comparisons with sign bit implications */
    __int128 s1 = SIGNED_NEG_ONE;
    __int128 s2 = SIGNED_ZERO;
    if (s1 < s2) checksum += 1024;   /* Should take: -1 < 0 */
    if (s2 > s1) checksum += 2048;   /* Should take: 0 > -1 */
    
    /* Test 5: Mixed signed/unsigned comparisons */
    unsigned __int128 u1 = 0xFFFFFFFFFFFFFFFFULL; /* Max uint64_t */
    __int128 s3 = 0x7FFFFFFFFFFFFFFFULL;          /* Max int64_t */
    if (u1 > (unsigned __int128)s3) checksum += 4096; /* Should take */
    
    /* Test 6: Equality comparisons */
    unsigned __int128 e1 = 0x123456789ABCDEF0ULL;
    unsigned __int128 e2 = e1;
    if (e1 == e2) checksum += 8192;   /* Should take */
    if (e1 != e2) checksum += 16384;  /* Should NOT take */
    
    return checksum;
}

/* Test with volatile variables to force runtime evaluation */
static int volatile_comparisons(void) {
    int checksum = 0;
    
    volatile_sint128 = SIGNED_NEG_ONE;
    volatile_uint128 = HIGH_PART_DIFFERS_LOW_EQUAL_A;
    
    /* These comparisons must be evaluated at runtime */
    if (volatile_sint128 < SIGNED_ZERO) checksum += 1;
    if (volatile_uint128 < HIGH_PART_DIFFERS_LOW_EQUAL_B) checksum += 2;
    
    /* Test <= and >= operators */
    if (HIGH_EQUAL_LOW_DIFFERS_A <= HIGH_EQUAL_LOW_DIFFERS_B) checksum += 4;
    if (HIGH_EQUAL_LOW_DIFFERS_B >= HIGH_EQUAL_LOW_DIFFERS_A) checksum += 8;
    if (BOTH_PARTS_DIFFER_A <= BOTH_PARTS_DIFFER_B) checksum += 16;
    if (BOTH_PARTS_DIFFER_B >= BOTH_PARTS_DIFFER_A) checksum += 32;
    
    return checksum;
}

int main(void) {
    int total_checksum = 0;
    
    /* Compile-time constant comparisons (already evaluated) */
    total_checksum += cmp_high_less;
    total_checksum += cmp_high_greater;
    total_checksum += cmp_low_less;
    total_checksum += cmp_low_greater;
    total_checksum += cmp_both_less;
    total_checksum += cmp_both_greater;
    
    /* Runtime comparisons */
    total_checksum += runtime_comparisons();
    
    /* Volatile comparisons */
    total_checksum += volatile_comparisons();
    
    /* Built-in overflow test */
    total_checksum += test_builtin_overflow();
    
    /* Use array sizes to prevent optimization */
    total_checksum += sizeof(array_high_less);
    total_checksum += sizeof(array_low_less);
    
    printf("Total checksum: %d\n", total_checksum);
    
    /* Additional exhaustive test cases */
    
    /* Test all comparison operators */
    _Static_assert((HIGH_PART_DIFFERS_LOW_EQUAL_A == HIGH_PART_DIFFERS_LOW_EQUAL_B) == 0, "==");
    _Static_assert((HIGH_PART_DIFFERS_LOW_EQUAL_A != HIGH_PART_DIFFERS_LOW_EQUAL_B) == 1, "!=");
    _Static_assert((HIGH_PART_DIFFERS_LOW_EQUAL_A < HIGH_PART_DIFFERS_LOW_EQUAL_B) == 1, "<");
    _Static_assert((HIGH_PART_DIFFERS_LOW_EQUAL_A <= HIGH_PART_DIFFERS_LOW_EQUAL_B) == 1, "<=");
    _Static_assert((HIGH_PART_DIFFERS_LOW_EQUAL_A > HIGH_PART_DIFFERS_LOW_EQUAL_B) == 0, ">");
    _Static_assert((HIGH_PART_DIFFERS_LOW_EQUAL_A >= HIGH_PART_DIFFERS_LOW_EQUAL_B) == 0, ">=");
    
    /* Test with extremely large values */
    const unsigned __int128 max_uint128 = ~((unsigned __int128)0);
    const unsigned __int128 almost_max = max_uint128 - 1;
    _Static_assert(almost_max < max_uint128, "almost_max < max");
    _Static_assert(max_uint128 > almost_max, "max > almost_max");
    
    /* Test signed comparisons near boundaries */
    const __int128 min_sint128 = ((__int128)1 << 127);
    const __int128 min_plus_one = min_sint128 + 1;
    _Static_assert(min_sint128 < min_plus_one, "min < min+1");
    
    return total_checksum != 0 ? 0 : 1;
}
