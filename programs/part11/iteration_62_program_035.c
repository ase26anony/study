/* test_double_int_comparison.c
 * Tests GCC's double_int comparison logic (lines 1285-1293)
 * Compile with: gcc -O2 -std=c11 -fdump-rtl-expand test.c -o test
 */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Force runtime evaluation */
static volatile unsigned __int128 runtime_var;

/* Large constants that exercise high/low part comparisons */
#define HIGH_PART_DIFFERS_LOW_EQUAL_A  ((unsigned __int128)0x10000000000000000ULL) /* high=1, low=0 */
#define HIGH_PART_DIFFERS_LOW_EQUAL_B  ((unsigned __int128)0x20000000000000000ULL) /* high=2, low=0 */

#define HIGH_EQUAL_LOW_DIFFERS_A       ((unsigned __int128)0x10000000000000001ULL) /* high=1, low=1 */
#define HIGH_EQUAL_LOW_DIFFERS_B       ((unsigned __int128)0x10000000000000002ULL) /* high=1, low=2 */

#define BOTH_PARTS_DIFFER_A            ((unsigned __int128)0x10000000000000001ULL) /* high=1, low=1 */
#define BOTH_PARTS_DIFFER_B            ((unsigned __int128)0x20000000000000002ULL) /* high=2, low=2 */

/* Signed constants with sign bit implications */
#define SIGNED_NEGATIVE_ONE            ((__int128)-1)  /* All bits set: 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF */
#define SIGNED_ZERO                    ((__int128)0)
#define SIGNED_LARGE_POSITIVE          ((__int128)0x7FFFFFFFFFFFFFFFFFFFFFFFULL)
#define SIGNED_LARGE_NEGATIVE          ((__int128)0x800000000000000000000000ULL)

/* Compile-time comparisons using static assertions */
_Static_assert(HIGH_PART_DIFFERS_LOW_EQUAL_A < HIGH_PART_DIFFERS_LOW_EQUAL_B, 
               "High part less comparison failed");
_Static_assert(HIGH_EQUAL_LOW_DIFFERS_A < HIGH_EQUAL_LOW_DIFFERS_B,
               "Low part less comparison failed");
_Static_assert(BOTH_PARTS_DIFFER_A < BOTH_PARTS_DIFFER_B,
               "Both parts less comparison failed");
_Static_assert(SIGNED_NEGATIVE_ONE < SIGNED_ZERO,
               "Signed negative comparison failed");

/* Test unsigned comparisons at compile time */
_Static_assert((unsigned __int128)SIGNED_NEGATIVE_ONE > (unsigned __int128)SIGNED_ZERO,
               "Unsigned comparison of signed values failed");

/* Array size based on comparison result */
char array_high_less[(HIGH_PART_DIFFERS_LOW_EQUAL_A < HIGH_PART_DIFFERS_LOW_EQUAL_B) ? 10 : 20];
char array_low_less[(HIGH_EQUAL_LOW_DIFFERS_A < HIGH_EQUAL_LOW_DIFFERS_B) ? 20 : 30];
char array_both_less[(BOTH_PARTS_DIFFER_A < BOTH_PARTS_DIFFER_B) ? 30 : 40];

/* Constant expressions using comparisons */
const int result_high_greater = (HIGH_PART_DIFFERS_LOW_EQUAL_B > HIGH_PART_DIFFERS_LOW_EQUAL_A) ? 1 : 0;
const int result_low_greater = (HIGH_EQUAL_LOW_DIFFERS_B > HIGH_EQUAL_LOW_DIFFERS_A) ? 2 : 0;
const int result_both_greater = (BOTH_PARTS_DIFFER_B > BOTH_PARTS_DIFFER_A) ? 3 : 0;

/* Test built-in overflow functions that may use comparisons */
int test_builtin_overflow(void) {
    __int128 a = SIGNED_LARGE_POSITIVE;
    __int128 b = SIGNED_LARGE_POSITIVE;
    __int128 result;
    int overflow = __builtin_mul_overflow(a, b, &result);
    return overflow;
}

/* Runtime comparison tests */
unsigned int runtime_comparisons(void) {
    unsigned int checksum = 0;
    
    /* Test 1: High part differs, low part equal */
    unsigned __int128 a1 = HIGH_PART_DIFFERS_LOW_EQUAL_A;
    unsigned __int128 b1 = HIGH_PART_DIFFERS_LOW_EQUAL_B;
    if (a1 < b1) checksum |= 0x1;      /* Should take */
    if (a1 > b1) checksum |= 0x2;      /* Should NOT take */
    if (a1 <= b1) checksum |= 0x4;     /* Should take */
    if (a1 >= b1) checksum |= 0x8;     /* Should NOT take */
    
    /* Test 2: High part equal, low part differs */
    unsigned __int128 a2 = HIGH_EQUAL_LOW_DIFFERS_A;
    unsigned __int128 b2 = HIGH_EQUAL_LOW_DIFFERS_B;
    if (a2 < b2) checksum |= 0x10;     /* Should take */
    if (a2 > b2) checksum |= 0x20;     /* Should NOT take */
    if (a2 <= b2) checksum |= 0x40;    /* Should take */
    if (a2 >= b2) checksum |= 0x80;    /* Should NOT take */
    
    /* Test 3: Both parts differ */
    unsigned __int128 a3 = BOTH_PARTS_DIFFER_A;
    unsigned __int128 b3 = BOTH_PARTS_DIFFER_B;
    if (a3 < b3) checksum |= 0x100;    /* Should take */
    if (a3 > b3) checksum |= 0x200;    /* Should NOT take */
    if (a3 <= b3) checksum |= 0x400;   /* Should take */
    if (a3 >= b3) checksum |= 0x800;   /* Should NOT take */
    
    /* Test 4: Equal values */
    unsigned __int128 a4 = HIGH_EQUAL_LOW_DIFFERS_A;
    unsigned __int128 b4 = HIGH_EQUAL_LOW_DIFFERS_A;
    if (a4 < b4) checksum |= 0x1000;   /* Should NOT take */
    if (a4 > b4) checksum |= 0x2000;   /* Should NOT take */
    if (a4 <= b4) checksum |= 0x4000;  /* Should take */
    if (a4 >= b4) checksum |= 0x8000;  /* Should take */
    
    return checksum;
}

/* Signed comparison tests */
unsigned int signed_comparisons(void) {
    unsigned int checksum = 0;
    
    /* Test signed comparisons that require unsigned high part comparison */
    __int128 neg_one = SIGNED_NEGATIVE_ONE;
    __int128 zero = SIGNED_ZERO;
    
    /* These should use unsigned comparison for high parts */
    if (neg_one < zero) checksum |= 0x10000;    /* Should take (signed) */
    if (neg_one > zero) checksum |= 0x20000;    /* Should NOT take */
    if (neg_one <= zero) checksum |= 0x40000;   /* Should take */
    if (neg_one >= zero) checksum |= 0x80000;   /* Should NOT take */
    
    /* Cast to unsigned for explicit unsigned comparison */
    if ((unsigned __int128)neg_one < (unsigned __int128)zero) checksum |= 0x100000;  /* Should NOT take */
    if ((unsigned __int128)neg_one > (unsigned __int128)zero) checksum |= 0x200000;  /* Should take */
    
    /* Test large positive vs large negative */
    __int128 large_pos = SIGNED_LARGE_POSITIVE;
    __int128 large_neg = SIGNED_LARGE_NEGATIVE;
    
    if (large_pos < large_neg) checksum |= 0x400000;   /* Should NOT take */
    if (large_pos > large_neg) checksum |= 0x800000;   /* Should take */
    
    return checksum;
}

/* Mixed signed/unsigned comparisons */
unsigned int mixed_comparisons(void) {
    unsigned int checksum = 0;
    
    /* Mix signed and unsigned with same bit pattern */
    __int128 signed_val = -1;
    unsigned __int128 unsigned_val = (unsigned __int128)-1;
    
    /* These should trigger different comparison paths */
    if (signed_val < (__int128)unsigned_val) checksum |= 0x1000000;
    if ((unsigned __int128)signed_val < unsigned_val) checksum |= 0x2000000;
    
    return checksum;
}

/* Force comparisons in loop for runtime coverage */
void loop_comparisons(void) {
    unsigned __int128 base = HIGH_EQUAL_LOW_DIFFERS_A;
    volatile unsigned int counter = 0;
    
    for (unsigned __int128 i = 0; i < 10; i++) {
        unsigned __int128 val = base + i;
        
        /* Multiple comparison types in loop */
        if (val < base + 5) counter++;
        if (val > base + 3) counter++;
        if (val <= base + 7) counter++;
        if (val >= base + 2) counter++;
    }
    
    runtime_var = counter;  /* Use volatile to prevent optimization */
}

int main(void) {
    unsigned int total_checksum = 0;
    
    /* Compile-time constant results */
    total_checksum += result_high_greater;
    total_checksum += result_low_greater;
    total_checksum += result_both_greater;
    
    /* Runtime comparisons */
    total_checksum += runtime_comparisons();
    total_checksum += signed_comparisons();
    total_checksum += mixed_comparisons();
    
    /* Built-in overflow test */
    total_checksum += test_builtin_overflow();
    
    /* Loop comparisons */
    loop_comparisons();
    
    /* Use array sizes (prevents dead code elimination) */
    total_checksum += sizeof(array_high_less);
    total_checksum += sizeof(array_low_less);
    total_checksum += sizeof(array_both_less);
    
    printf("Total checksum: %u\n", total_checksum);
    
    /* Final assertion using 128-bit comparison */
    _Static_assert((unsigned __int128)0xFFFFFFFFFFFFFFFFULL < 
                   (unsigned __int128)0x10000000000000000ULL,
                   "Final boundary check failed");
    
    return 0;
}
