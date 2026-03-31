/* test-double-int-comparison.c */
#include <stdio.h>
#include <stdint.h>

/* Force runtime evaluation */
static volatile unsigned __int128 volatile_uint128;
static volatile __int128 volatile_sint128;

/* Large constants that exercise high/low parts */
#define HIGH_PART_DIFFERS_LOW_EQUAL_A ((unsigned __int128)0x10000000000000000ULL) /* high=1, low=0 */
#define HIGH_PART_DIFFERS_LOW_EQUAL_B ((unsigned __int128)0x20000000000000000ULL) /* high=2, low=0 */

#define HIGH_EQUAL_LOW_DIFFERS_A ((unsigned __int128)0x10000000000000001ULL) /* high=1, low=1 */
#define HIGH_EQUAL_LOW_DIFFERS_B ((unsigned __int128)0x10000000000000002ULL) /* high=1, low=2 */

#define BOTH_PARTS_DIFFER_A ((unsigned __int128)0x10000000000000001ULL) /* high=1, low=1 */
#define BOTH_PARTS_DIFFER_B ((unsigned __int128)0x20000000000000002ULL) /* high=2, low=2 */

/* Signed constants with sign bit implications */
#define SIGNED_NEG_ONE ((__int128)-1) /* high=0xFFFFFFFFFFFFFFFF, low=0xFFFFFFFFFFFFFFFF */
#define SIGNED_ZERO ((__int128)0)
#define SIGNED_LARGE_POS ((__int128)0x7FFFFFFFFFFFFFFFULL << 64 | 0xFFFFFFFFFFFFFFFFULL)
#define SIGNED_LARGE_NEG ((__int128)0x8000000000000000ULL << 64)

/* Compile-time comparisons using static assertions */
_Static_assert(HIGH_PART_DIFFERS_LOW_EQUAL_A < HIGH_PART_DIFFERS_LOW_EQUAL_B, 
               "High part differs, low equal: A < B");
_Static_assert(HIGH_PART_DIFFERS_LOW_EQUAL_B > HIGH_PART_DIFFERS_LOW_EQUAL_A,
               "High part differs, low equal: B > A");

_Static_assert(HIGH_EQUAL_LOW_DIFFERS_A < HIGH_EQUAL_LOW_DIFFERS_B,
               "High equal, low differs: A < B");
_Static_assert(HIGH_EQUAL_LOW_DIFFERS_B > HIGH_EQUAL_LOW_DIFFERS_A,
               "High equal, low differs: B > A");

_Static_assert(BOTH_PARTS_DIFFER_A < BOTH_PARTS_DIFFER_B,
               "Both parts differ: A < B");
_Static_assert(BOTH_PARTS_DIFFER_B > BOTH_PARTS_DIFFER_A,
               "Both parts differ: B > A");

/* Signed comparisons that use unsigned high-part comparison */
_Static_assert(SIGNED_NEG_ONE < SIGNED_ZERO,
               "Signed: -1 < 0 (tests unsigned high comparison)");
_Static_assert(SIGNED_ZERO > SIGNED_NEG_ONE,
               "Signed: 0 > -1");
_Static_assert(SIGNED_LARGE_NEG < SIGNED_LARGE_POS,
               "Signed: large negative < large positive");

/* Constant expressions that force comparison evaluation */
const int cmp_high_diff = (HIGH_PART_DIFFERS_LOW_EQUAL_A < HIGH_PART_DIFFERS_LOW_EQUAL_B) ? 1 : 0;
const int cmp_high_equal = (HIGH_EQUAL_LOW_DIFFERS_A <= HIGH_EQUAL_LOW_DIFFERS_B) ? 1 : 0;
const int cmp_both_diff = (BOTH_PARTS_DIFFER_A != BOTH_PARTS_DIFFER_B) ? 1 : 0;
const int cmp_signed = (SIGNED_NEG_ONE >= SIGNED_ZERO) ? 0 : 1;

/* Array size depending on comparison result */
char array_high_diff[(HIGH_PART_DIFFERS_LOW_EQUAL_A < HIGH_PART_DIFFERS_LOW_EQUAL_B) ? 10 : 20];
char array_low_diff[(HIGH_EQUAL_LOW_DIFFERS_A > HIGH_EQUAL_LOW_DIFFERS_B) ? 5 : 15];

/* Runtime comparison function */
static int runtime_comparisons(void) {
    int checksum = 0;
    
    /* Runtime unsigned comparisons */
    unsigned __int128 ru_a = HIGH_PART_DIFFERS_LOW_EQUAL_A;
    unsigned __int128 ru_b = HIGH_PART_DIFFERS_LOW_EQUAL_B;
    
    if (ru_a < ru_b) checksum += 1;  /* Should take: high less */
    if (ru_b > ru_a) checksum += 2;  /* Should take: high greater */
    
    ru_a = HIGH_EQUAL_LOW_DIFFERS_A;
    ru_b = HIGH_EQUAL_LOW_DIFFERS_B;
    
    if (ru_a < ru_b) checksum += 4;  /* Should take: low less */
    if (ru_b > ru_a) checksum += 8;  /* Should take: low greater */
    
    /* Runtime signed comparisons */
    __int128 rs_a = SIGNED_NEG_ONE;
    __int128 rs_b = SIGNED_ZERO;
    
    if (rs_a < rs_b) checksum += 16;  /* Should take: signed negative < 0 */
    if (rs_b > rs_a) checksum += 32;  /* Should take: 0 > signed negative */
    
    /* Equal comparisons */
    ru_a = HIGH_EQUAL_LOW_DIFFERS_A;
    ru_b = HIGH_EQUAL_LOW_DIFFERS_A;  /* Same value */
    
    if (ru_a == ru_b) checksum += 64;
    if (ru_a <= ru_b) checksum += 128;
    if (ru_a >= ru_b) checksum += 256;
    
    /* Mix signed and unsigned via casts */
    if ((unsigned __int128)rs_a < (unsigned __int128)rs_b) checksum += 512;
    
    return checksum;
}

/* Use GCC built-ins that may trigger internal comparisons */
static int builtin_comparisons(void) {
    int checksum = 0;
    __int128 a = SIGNED_LARGE_POS;
    __int128 b = SIGNED_LARGE_POS / 2;
    __int128 result;
    int overflow;
    
    /* __builtin_mul_overflow may internally compare */
    overflow = __builtin_mul_overflow(a, b, &result);
    checksum += overflow ? 1 : 2;
    
    /* __builtin_add_overflow_p for constant folding */
    checksum += __builtin_add_overflow_p(a, b, (__int128)0) ? 4 : 8;
    
    return checksum;
}

/* Force volatile comparisons at runtime */
static int volatile_comparisons(void) {
    int checksum = 0;
    
    volatile_uint128 = HIGH_PART_DIFFERS_LOW_EQUAL_A;
    unsigned __int128 local_b = HIGH_PART_DIFFERS_LOW_EQUAL_B;
    
    if (volatile_uint128 < local_b) checksum += 1;
    if (local_b > volatile_uint128) checksum += 2;
    
    volatile_sint128 = SIGNED_NEG_ONE;
    __int128 local_zero = 0;
    
    if (volatile_sint128 < local_zero) checksum += 4;
    if (local_zero > volatile_sint128) checksum += 8;
    
    return checksum;
}

int main(void) {
    int total_checksum = 0;
    
    /* Compile-time constant checksum components */
    total_checksum += cmp_high_diff;
    total_checksum += cmp_high_equal;
    total_checksum += cmp_both_diff;
    total_checksum += cmp_signed;
    
    /* Runtime comparisons */
    total_checksum += runtime_comparisons();
    
    /* Built-in function comparisons */
    total_checksum += builtin_comparisons();
    
    /* Volatile comparisons */
    total_checksum += volatile_comparisons();
    
    /* Print result to prevent optimization */
    printf("Comparison checksum: %d\n", total_checksum);
    printf("Array sizes: %zu, %zu\n", 
           sizeof(array_high_diff), sizeof(array_low_diff));
    
    return 0;
}
