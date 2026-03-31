/* test_double_int_comparison.c
 * Designed to trigger GCC's internal double_int comparison logic
 * Specifically targeting lines 1285-1293 of double-int.cc
 */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Define 128-bit constants that cross 64-bit boundaries */
#define HIGH_BIT_64 0x8000000000000000ULL
#define MAX_64 0xFFFFFFFFFFFFFFFFULL
#define LARGE_CONSTANT_128 ((__int128)0x123456789ABCDEF0ULL << 64 | 0xFEDCBA9876543210ULL)

/* Force compile-time evaluation with static assertions */
_Static_assert(((__int128)1 << 64) > 0, "128-bit shift should work");
_Static_assert(((__int128)HIGH_BIT_64 << 64) < 0, "Negative 128-bit constant");

/* Function to create value ranges for VRP analysis */
__int128 create_range(int selector) {
    switch(selector) {
        case 0: return (__int128)INT64_MAX + 1;  /* Crosses 64-bit boundary */
        case 1: return (__int128)INT64_MIN - 1;
        case 2: return (__int128)UINT64_MAX << 32;
        case 3: return ~((__int128)0) >> 1;  /* INT128_MAX */
        case 4: return ((__int128)1) << 127;  /* INT128_MIN */
        default: return 0;
    }
}

/* Test high-word comparisons (both positive and negative) */
int test_high_word_comparisons(void) {
    int checksum = 0;
    
    /* Comparisons where high words differ (positive case) */
    __int128 a1 = ((__int128)0x1ULL << 64) | 0x123456789ABCDEF0ULL;
    __int128 b1 = ((__int128)0x2ULL << 64) | 0x123456789ABCDEF0ULL;
    checksum += (a1 < b1) ? 1 : 0;  /* Should be true */
    checksum += (a1 > b1) ? 2 : 0;  /* Should be false */
    
    /* Comparisons where high words differ (negative case) */
    __int128 a2 = -((__int128)0x2ULL << 64) | 0x123456789ABCDEF0ULL;
    __int128 b2 = -((__int128)0x1ULL << 64) | 0x123456789ABCDEF0ULL;
    checksum += (a2 < b2) ? 4 : 0;  /* Should be true */
    checksum += (a2 > b2) ? 8 : 0;  /* Should be false */
    
    /* Comparisons where high words are equal but low words differ */
    __int128 a3 = ((__int128)0x1ULL << 64) | 0x0ULL;
    __int128 b3 = ((__int128)0x1ULL << 64) | 0x1ULL;
    checksum += (a3 < b3) ? 16 : 0;  /* Should be true */
    checksum += (a3 > b3) ? 32 : 0;  /* Should be false */
    
    return checksum;
}

/* Test boundary values */
int test_boundary_comparisons(void) {
    int checksum = 0;
    
    /* Get boundary values */
    __int128 int128_max = ~((__int128)0) >> 1;
    __int128 int128_min = ((__int128)1) << 127;
    unsigned __int128 uint128_max = ~((unsigned __int128)0);
    
    /* Signed boundary comparisons */
    checksum += (int128_min < int128_max) ? 1 : 0;
    checksum += (int128_max > int128_min) ? 2 : 0;
    
    /* Unsigned comparisons that cross signed boundary */
    checksum += ((unsigned __int128)int128_min < uint128_max) ? 4 : 0;
    checksum += ((unsigned __int128)int128_max < uint128_max) ? 8 : 0;
    
    /* Zero comparisons */
    checksum += (0 < int128_max) ? 16 : 0;
    checksum += (0 > int128_min) ? 32 : 0;
    
    return checksum;
}

/* Test overflow operations that require wide comparisons */
int test_overflow_operations(void) {
    int checksum = 0;
    
    /* Use builtin overflow checks */
    __int128 x = ((__int128)INT64_MAX) * 2;
    __int128 y = ((__int128)INT64_MAX) * 3;
    
    __int128 sum, diff, prod;
    int overflow;
    
    /* Addition overflow check */
    overflow = __builtin_add_overflow(x, y, &sum);
    checksum += overflow ? 1 : 0;
    
    /* Multiplication overflow check */
    overflow = __builtin_mul_overflow(x, (__int128)10, &prod);
    checksum += overflow ? 2 : 0;
    
    /* Comparisons after potential overflow */
    checksum += (sum > x) ? 4 : 0;
    checksum += (prod > y) ? 8 : 0;
    
    return checksum;
}

/* Test mixed-precision operations */
int test_mixed_precision(void) {
    int checksum = 0;
    
    /* Compare __int128 with narrower types */
    __int128 wide_val = ((__int128)1) << 66;
    long long narrow_val = LLONG_MAX;
    
    checksum += (wide_val > narrow_val) ? 1 : 0;
    checksum += (wide_val < (__int128)narrow_val * 2) ? 2 : 0;
    
    /* Ternary operator with mixed types */
    __int128 result = (narrow_val > 0) ? wide_val : (__int128)narrow_val;
    checksum += (result == wide_val) ? 4 : 0;
    
    /* Compare with size_t */
    size_t size_val = SIZE_MAX;
    checksum += ((unsigned __int128)wide_val > size_val) ? 8 : 0;
    
    return checksum;
}

/* Test bitwise operations crossing 64-bit boundary */
int test_bitwise_operations(void) {
    int checksum = 0;
    
    __int128 val = LARGE_CONSTANT_128;
    
    /* Shift operations that cross word boundary */
    __int128 left_shift = val << 32;
    __int128 right_shift = val >> 32;
    
    checksum += (left_shift > val) ? 1 : 0;
    checksum += (right_shift < val) ? 2 : 0;
    
    /* Bitwise AND/OR crossing boundary */
    __int128 mask = ((__int128)0xFFFFULL << 64) | 0xFFFFULL;
    __int128 masked = val & mask;
    
    checksum += (masked < val) ? 4 : 0;
    
    /* Test builtin bit operations */
    unsigned __int128 uval = (unsigned __int128)val;
    int popcount = __builtin_popcountll((unsigned long long)(uval >> 64)) +
                   __builtin_popcountll((unsigned long long)uval);
    checksum += (popcount > 0) ? 8 : 0;
    
    return checksum;
}

/* Test with arrays to give optimizer substantial work */
int test_array_operations(void) {
    __int128 arr[8] = {
        ((__int128)0) - 1,                    /* -1 */
        ((__int128)1) << 63,                  /* Just crossing 64-bit */
        ((__int128)1) << 64,                  /* Definitely in high word */
        ((__int128)0x7FFFFFFFFFFFFFFFULL) << 64 | 0xFFFFFFFFFFFFFFFFULL,
        ((__int128)0x8000000000000000ULL) << 64,
        ((__int128)0x123456789ABCDEF0ULL) << 64 | 0xFEDCBA9876543210ULL,
        0,
        ~((__int128)0) >> 1                   /* INT128_MAX */
    };
    
    int checksum = 0;
    
    /* Compare all pairs to exercise comparison logic */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (i != j) {
                checksum += (arr[i] < arr[j]) ? 1 : 0;
                checksum += (arr[i] > arr[j]) ? 2 : 0;
                checksum += (arr[i] == arr[j]) ? 4 : 0;
            }
        }
    }
    
    /* Loop with __int128 induction variable */
    for (__int128 i = ((__int128)1) << 62; i < ((__int128)1) << 63; i += ((__int128)1) << 60) {
        checksum += (i > 0) ? 1 : 0;
    }
    
    return checksum;
}

/* Test switch statement with __int128 case labels */
int test_switch_statement(__int128 value) {
    /* Force compiler to generate comparison tree */
    switch (value) {
        case ((__int128)0ULL):
            return 1;
        case ((__int128)1ULL << 64):
            return 2;
        case ((__int128)0x8000000000000000ULL << 64):
            return 3;
        case LARGE_CONSTANT_128:
            return 4;
        default:
            return 0;
    }
}

/* Main function that runs all tests */
int main(void) {
    int total_checksum = 0;
    
    printf("Testing GCC double_int comparison logic...\n");
    
    /* Run all test suites */
    total_checksum += test_high_word_comparisons();
    total_checksum += test_boundary_comparisons();
    total_checksum += test_overflow_operations();
    total_checksum += test_mixed_precision();
    total_checksum += test_bitwise_operations();
    total_checksum += test_array_operations();
    
    /* Test switch with constant folding */
    total_checksum += test_switch_statement(0);
    total_checksum += test_switch_statement((__int128)1ULL << 64);
    total_checksum += test_switch_statement(LARGE_CONSTANT_128);
    
    /* Use __builtin_expect with wide comparisons */
    __int128 x = ((__int128)1) << 65;
    __int128 y = ((__int128)1) << 66;
    
    if (__builtin_expect(x < y, 1)) {
        total_checksum += 1;
    }
    
    /* Force printf conversion sequence */
    printf("Intermediate checksum: %d\n", total_checksum);
    
    /* Final computation to prevent dead code elimination */
    __int128 final_val = ((__int128)total_checksum) << 32;
    printf("Final 128-bit value high word: 0x%016llx\n", 
           (unsigned long long)(final_val >> 64));
    printf("Final 128-bit value low word: 0x%016llx\n",
           (unsigned long long)final_val);
    
    return total_checksum & 0xFF;
}
