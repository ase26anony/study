/* test_double_int_comparison.c
 * Designed to trigger GCC's internal double_int comparison logic
 * Specifically targeting lines 1285-1293 of double-int.cc
 */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Define 128-bit constants that cross 64-bit boundaries */
#define HIGH_BIT_64   0x8000000000000000ULL
#define MAX_64        0xFFFFFFFFFFFFFFFFULL
#define HIGH_WORD_DIFF 0x1000000000000000ULL

/* Force compile-time evaluation with static assertions */
_Static_assert(((__int128)HIGH_BIT_64 << 64) > (__int128)0, 
               "High-bit shift should be positive");
_Static_assert(((__int128)0 - ((__int128)HIGH_BIT_64 << 64)) < (__int128)0,
               "Negative 128-bit value check");

/* Test function that exercises __int128 comparisons */
static __int128 test_signed_comparisons(void) {
    __int128 a, b, result = 0;
    
    /* Test 1: High words differ, low words equal (both positive) */
    a = ((__int128)0x1ULL << 64) | 0x123456789ABCDEF0ULL;
    b = ((__int128)0x2ULL << 64) | 0x123456789ABCDEF0ULL;
    if (a < b) result += 1;  /* Should take this path */
    if (b > a) result += 2;  /* Should take this path */
    
    /* Test 2: High words differ, low words equal (both negative) */
    a = -(((__int128)0x2ULL << 64) | 0x123456789ABCDEF0ULL);
    b = -(((__int128)0x1ULL << 64) | 0x123456789ABCDEF0ULL);
    if (a < b) result += 4;  /* Should take this path */
    if (b > a) result += 8;  /* Should take this path */
    
    /* Test 3: High words equal, low words differ */
    a = ((__int128)0x1ULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    b = ((__int128)0x1ULL << 64) | 0x0ULL;
    if (a > b) result += 16; /* Should take low-word comparison path */
    if (b < a) result += 32; /* Should take low-word comparison path */
    
    /* Test 4: Boundary values */
    __int128 max_pos = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | MAX_64;
    __int128 min_neg = ((__int128)0x8000000000000000ULL << 64) | 0x0ULL;
    if (max_pos > min_neg) result += 64;
    if (min_neg < max_pos) result += 128;
    
    return result;
}

/* Test unsigned __int128 comparisons */
static unsigned __int128 test_unsigned_comparisons(void) {
    unsigned __int128 a, b, result = 0;
    
    /* Test unsigned high-word comparisons */
    a = ((unsigned __int128)0x1ULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    b = ((unsigned __int128)0x2ULL << 64) | 0x0ULL;
    if (a < b) result += 1;
    if (b > a) result += 2;
    
    /* Test with MAX values */
    unsigned __int128 max_u128 = ~((unsigned __int128)0);
    unsigned __int128 almost_max = max_u128 - 1;
    if (almost_max < max_u128) result += 4;
    if (max_u128 > almost_max) result += 8;
    
    return result;
}

/* Force range analysis with loops */
static __int128 test_range_analysis(void) {
    __int128 sum = 0;
    
    /* Loop with __int128 induction variable */
    for (__int128 i = -((__int128)10 << 64); 
         i < ((__int128)10 << 64); 
         i += ((__int128)1 << 62)) {
        if (i > 0) sum += i;
        else sum -= i;
    }
    
    /* Another loop crossing 64-bit boundary */
    for (unsigned __int128 j = 0xFFFFFFFFFFFFFF00ULL;
         j < 0x10000000000000100ULL;
         j += 0x100) {
        if (j > 0xFFFFFFFFFFFFFFFFULL) sum += 1;
    }
    
    return sum;
}

/* Test overflow operations that require comparisons */
static __int128 test_overflow_checks(void) {
    __int128 result = 0;
    __int128 a, b;
    int overflow;
    
    /* Test __builtin_add_overflow */
    a = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFEULL;
    b = 1;
    if (__builtin_add_overflow(a, b, &a)) {
        result += 1;  /* Should overflow */
    }
    
    /* Test __builtin_mul_overflow */
    a = ((__int128)0x2ULL << 62);
    b = ((__int128)0x2ULL << 62);
    if (__builtin_mul_overflow(a, b, &a)) {
        result += 2;  /* Check overflow detection */
    }
    
    return result;
}

/* Test mixed-precision operations */
static __int128 test_mixed_precision(void) {
    __int128 result = 0;
    
    /* Compare __int128 with narrower types */
    __int128 large = ((__int128)0x1ULL << 64) | 0x123456789ABCDEF0ULL;
    long long medium = 0x123456789ABCDEF0LL;
    unsigned long long ularge = 0xFEDCBA9876543210ULL;
    
    if (large > medium) result += 1;
    if ((unsigned __int128)large < ularge) result += 2;
    
    /* Ternary operator with mixed types */
    result = (large > 0) ? large : (__int128)medium;
    
    /* Array operations with __int128 */
    __int128 arr[8] = {
        ((__int128)0x0ULL << 64) | 0x0ULL,
        ((__int128)0x0ULL << 64) | 0x1ULL,
        ((__int128)0x1ULL << 64) | 0x0ULL,
        ((__int128)0x1ULL << 64) | 0x1ULL,
        -(((__int128)0x1ULL << 64) | 0x0ULL),
        -(((__int128)0x1ULL << 64) | 0x1ULL),
        ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | MAX_64,
        ((__int128)0x8000000000000000ULL << 64) | 0x0ULL
    };
    
    for (int i = 0; i < 7; i++) {
        if (arr[i] < arr[i + 1]) result += arr[i];
        else result -= arr[i + 1];
    }
    
    return result;
}

/* Test bitwise operations crossing 64-bit boundary */
static __int128 test_bitwise_ops(void) {
    __int128 a = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    __int128 b = ((__int128)0xFEDCBA9876543210ULL << 64) | 0x123456789ABCDEF0ULL;
    __int128 result = 0;
    
    /* Bitwise operations */
    __int128 c = a & b;
    __int128 d = a | b;
    __int128 e = a ^ b;
    
    /* Shifts crossing 64-bit boundary */
    __int128 left_shift = a << 65;  /* Crosses word boundary */
    __int128 right_shift = b >> 65;
    
    /* Comparisons after bitwise ops */
    if (c < d) result += 1;
    if (e > c) result += 2;
    if (left_shift > right_shift) result += 4;
    
    /* Use __builtin_clzll on high and low parts */
    unsigned long long high_part = (unsigned long long)(a >> 64);
    unsigned long long low_part = (unsigned long long)a;
    result += __builtin_clzll(high_part);
    result += __builtin_ctzll(low_part);
    
    return result;
}

/* Test switch statement with __int128 cases (compile-time constants) */
static __int128 test_switch_case(void) {
    __int128 value = ((__int128)0x1ULL << 64) | 0x1234ULL;
    __int128 result = 0;
    
    /* Switch forces compiler to generate comparison trees */
    switch ((unsigned __int128)value) {
        case ((unsigned __int128)0x0ULL << 64) | 0x0ULL:
            result = 1;
            break;
        case ((unsigned __int128)0x1ULL << 64) | 0x1234ULL:
            result = 2;  /* Should match */
            break;
        case ((unsigned __int128)0x1ULL << 64) | 0x1235ULL:
            result = 3;
            break;
        case ((unsigned __int128)0x2ULL << 64) | 0x0ULL:
            result = 4;
            break;
        default:
            result = 5;
            break;
    }
    
    return result;
}

/* Main function that runs all tests */
int main(void) {
    __int128 checksum = 0;
    
    printf("Testing GCC double_int comparison logic...\n");
    
    /* Run all test functions and accumulate results */
    checksum += test_signed_comparisons();
    checksum += (__int128)test_unsigned_comparisons();
    checksum += test_range_analysis();
    checksum += test_overflow_checks();
    checksum += test_mixed_precision();
    checksum += test_bitwise_ops();
    checksum += test_switch_case();
    
    /* Print checksum to prevent dead code elimination */
    /* Split 128-bit checksum into two 64-bit parts for printing */
    unsigned long long high = (unsigned long long)(checksum >> 64);
    unsigned long long low = (unsigned long long)checksum;
    printf("Checksum: 0x%016llx%016llx\n", high, low);
    
    /* Additional forced comparisons at boundaries */
    volatile __int128 v1 = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | MAX_64;
    volatile __int128 v2 = ((__int128)0x8000000000000000ULL << 64) | 0x0ULL;
    if (v1 > v2) {
        printf("Boundary comparison passed\n");
    }
    
    /* Test __builtin_expect with wide comparisons */
    __int128 x = ((__int128)0x1ULL << 64) | 0x1ULL;
    __int128 y = ((__int128)0x1ULL << 64) | 0x2ULL;
    if (__builtin_expect(x < y, 1)) {
        printf("Builtin expect comparison passed\n");
    }
    
    return 0;
}
