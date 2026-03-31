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
#define HIGH_WORD_DIFF 0x1000000000000000ULL

/* Force compile-time evaluation with static assertions */
_Static_assert(((__int128)HIGH_BIT_64 << 64) > 0, 
               "High-bit shift should create positive __int128");
_Static_assert(((__int128)MAX_64) < ((__int128)MAX_64 << 64), 
               "Comparison across 64-bit boundary");

/* Test 1: High word comparisons (signed) */
static int test_high_word_comparisons(void) {
    volatile __int128 a, b;
    int result = 0;
    
    /* Case 1: High words differ, both positive */
    a = ((__int128)HIGH_BIT_64 << 64) | 0x123456789ABCDEF0ULL;
    b = ((__int128)(HIGH_BIT_64 >> 1) << 64) | 0xFEDCBA9876543210ULL;
    result += (a > b) ? 1 : 0;
    result += (a < b) ? -1 : 0;
    
    /* Case 2: High words differ, negative values */
    a = -(((__int128)HIGH_BIT_64 << 64) | 0x123456789ABCDEF0ULL);
    b = -(((__int128)(HIGH_BIT_64 >> 2) << 64) | 0xFEDCBA9876543210ULL);
    result += (a > b) ? 1 : 0;
    result += (a < b) ? -1 : 0;
    
    /* Case 3: High words equal, low words differ */
    a = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0x0000000000000001ULL;
    b = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    result += (a > b) ? 1 : 0;
    result += (a < b) ? -1 : 0;
    
    return result;
}

/* Test 2: Boundary value comparisons */
static int test_boundary_comparisons(void) {
    volatile __int128 max_pos = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | MAX_64;
    volatile __int128 min_neg = ((__int128)HIGH_BIT_64 << 64);
    volatile __int128 zero = 0;
    volatile __int128 one = 1;
    volatile __int128 neg_one = -1;
    
    int result = 0;
    
    /* Compare at signed boundaries */
    result += (max_pos > min_neg) ? 1 : 0;
    result += (zero > min_neg) ? 1 : 0;
    result += (max_pos < zero) ? -1 : 0;
    
    /* Compare with -1 (all bits set in two's complement) */
    result += (neg_one > min_neg) ? 1 : 0;
    result += (neg_one < max_pos) ? -1 : 0;
    
    return result;
}

/* Test 3: Mixed unsigned/signed comparisons */
static int test_mixed_comparisons(void) {
    volatile unsigned __int128 ua, ub;
    volatile __int128 sa, sb;
    int result = 0;
    
    /* Unsigned comparisons that exercise high word logic */
    ua = ((unsigned __int128)MAX_64 << 64) | 0x123456789ABCDEF0ULL;
    ub = ((unsigned __int128)MAX_64 << 64) | 0xFEDCBA9876543210ULL;
    result += (ua > ub) ? 1 : 0;
    result += (ua < ub) ? -1 : 0;
    
    /* Compare unsigned with signed (forcing conversions) */
    sa = ((__int128)HIGH_BIT_64 << 64) | 0x123456789ABCDEF0ULL; /* Negative */
    ua = ((unsigned __int128)HIGH_BIT_64 << 64) | 0x123456789ABCDEF0ULL; /* Positive */
    
    /* These comparisons require careful handling of signedness */
    result += ((unsigned __int128)sa > ua) ? 1 : 0;
    result += (sa < (__int128)ua) ? -1 : 0;
    
    return result;
}

/* Test 4: Range analysis triggers with loops */
static int test_range_analysis(void) {
    volatile __int128 start = ((__int128)HIGH_BIT_64 << 63); /* Middle of range */
    volatile __int128 end = start + 1000;
    volatile __int128 i;
    int result = 0;
    
    /* Loop with __int128 induction variable */
    for (i = start; i < end; i = i + 1) {
        /* Force comparisons in loop condition */
        result += (i > start) ? 1 : 0;
        result += (i < end) ? -1 : 0;
        
        /* Overflow checks */
        __int128 sum, product;
        int overflow_add, overflow_mul;
        
        overflow_add = __builtin_add_overflow(i, 1, &sum);
        overflow_mul = __builtin_mul_overflow(i, 2, &product);
        
        result += overflow_add ? 1 : 0;
        result += overflow_mul ? -1 : 0;
    }
    
    return result;
}

/* Test 5: Bitwise operations crossing 64-bit boundary */
static int test_bitwise_operations(void) {
    volatile __int128 a = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    volatile __int128 b = ((__int128)0xFEDCBA9876543210ULL << 64) | 0x123456789ABCDEF0ULL;
    int result = 0;
    
    /* Shifts that cross word boundary */
    volatile __int128 left_shift = a << 65;  /* Crosses both words */
    volatile __int128 right_shift = b >> 65;
    
    result += (left_shift > right_shift) ? 1 : 0;
    result += (left_shift < right_shift) ? -1 : 0;
    
    /* Bitwise operations */
    volatile __int128 and_result = a & b;
    volatile __int128 or_result = a | b;
    volatile __int128 xor_result = a ^ b;
    
    result += (and_result > or_result) ? -1 : 0;
    result += (or_result > xor_result) ? 1 : 0;
    result += (xor_result < and_result) ? -1 : 0;
    
    /* Population count across 128 bits */
    unsigned long long pop_low = __builtin_popcountll((unsigned long long)a);
    unsigned long long pop_high = __builtin_popcountll((unsigned long long)(a >> 64));
    result += (pop_low + pop_high) & 1;
    
    return result;
}

/* Test 6: Switch statement with __int128 case labels */
static int test_switch_statement(void) {
    volatile __int128 key = ((__int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAAULL;
    int result = 0;
    
    /* Switch forces compiler to generate comparison trees */
    switch ((unsigned long long)(key >> 64)) { /* Using high word as switch key */
        case 0x5555555555555555ULL:
            result += 1;
            if ((unsigned long long)key == 0xAAAAAAAAAAAAAAAAULL) {
                result += 2;
            }
            break;
        case 0xAAAAAAAAAAAAAAAAULL:
            result -= 1;
            break;
        default:
            result += 3;
    }
    
    return result;
}

/* Test 7: Array operations with __int128 */
static int test_array_operations(void) {
    /* Array of at least 8 elements as requested */
    volatile __int128 arr[8] = {
        0,
        ((__int128)1 << 64),
        ((__int128)HIGH_BIT_64 << 64),
        ((__int128)MAX_64 << 64) | MAX_64,
        -(((__int128)1 << 64)),
        -(((__int128)HIGH_BIT_64 << 64)),
        ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL,
        ((__int128)0xFEDCBA9876543210ULL << 64) | 0x123456789ABCDEF0ULL
    };
    
    int result = 0;
    
    /* Compare each element with others */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (i != j) {
                result += (arr[i] > arr[j]) ? 1 : 0;
                result += (arr[i] < arr[j]) ? -1 : 0;
                
                /* Also compare with narrower types */
                result += (arr[i] > (long long)arr[j]) ? 1 : 0;
                result += (arr[i] < (unsigned long long)arr[j]) ? -1 : 0;
            }
        }
    }
    
    return result;
}

/* Test 8: Built-in functions and branch prediction */
static int test_builtins_and_branches(void) {
    volatile __int128 a = ((__int128)0x123456789ABCDEF0ULL << 64) | 0x0;
    volatile __int128 b = ((__int128)0x0 << 64) | 0xFEDCBA9876543210ULL;
    int result = 0;
    
    /* Use __builtin_expect with __int128 comparisons */
    if (__builtin_expect(a > b, 1)) {
        result += 1;
    }
    
    if (__builtin_expect(a < b, 0)) {
        result -= 1;
    }
    
    /* Count leading zeros in high word */
    if (a != 0) {
        int clz_high = __builtin_clzll((unsigned long long)(a >> 64));
        int clz_low = __builtin_clzll((unsigned long long)a);
        result += (clz_high < clz_low) ? 1 : -1;
    }
    
    /* Byte swap simulation (manual 128-bit byte swap) */
    unsigned __int128 swapped = 0;
    for (int i = 0; i < 16; i++) {
        swapped |= ((unsigned __int128)((unsigned char*)&a)[i]) << (8 * (15 - i));
    }
    result += (swapped > (unsigned __int128)a) ? 1 : -1;
    
    return result;
}

/* Test 9: Ternary operator with mixed types */
static int test_ternary_operator(void) {
    volatile __int128 large = ((__int128)MAX_64 << 64) | MAX_64;
    volatile long long medium = 0x7FFFFFFFFFFFFFFFLL;
    volatile int small = 100;
    
    int result = 0;
    
    /* Ternary with __int128 and narrower types */
    volatile __int128 ternary1 = (small > 50) ? large : (__int128)medium;
    volatile __int128 ternary2 = (medium > 0) ? (__int128)medium : large;
    volatile long long ternary3 = (large > 0) ? (long long)large : medium;
    
    result += (ternary1 > ternary2) ? 1 : 0;
    result += (ternary2 < ternary3) ? -1 : 0;
    result += (ternary3 == medium) ? 1 : -1;
    
    return result;
}

/* Test 10: Variadic function arguments (conversion sequences) */
static int test_variadic_conversions(void) {
    volatile __int128 a = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    volatile __int128 b = -a;
    
    /* These will trigger conversion sequences when compiled */
    /* Note: printf with %lld for __int128 is not portable, 
       but will trigger internal conversions */
    int result = 0;
    
    /* Compare after conversion to narrower types */
    long long a_low = (long long)a;
    long long b_low = (long long)b;
    unsigned long long a_high = (unsigned long long)(a >> 64);
    unsigned long long b_high = (unsigned long long)(b >> 64);
    
    result += (a_low > b_low) ? 1 : -1;
    result += (a_high < b_high) ? -1 : 1;
    
    return result;
}

int main(void) {
    int checksum = 0;
    
    printf("Starting double_int comparison tests...\n");
    
    /* Run all tests to exercise the comparison logic */
    checksum += test_high_word_comparisons();
    checksum += test_boundary_comparisons();
    checksum += test_mixed_comparisons();
    checksum += test_range_analysis();
    checksum += test_bitwise_operations();
    checksum += test_switch_statement();
    checksum += test_array_operations();
    checksum += test_builtins_and_branches();
    checksum += test_ternary_operator();
    checksum += test_variadic_conversions();
    
    printf("Checksum: %d\n", checksum);
    printf("All tests completed.\n");
    
    /* Force use of results to prevent dead code elimination */
    volatile int result = checksum;
    
    /* Additional compile-time forced comparisons */
    _Static_assert((((__int128)HIGH_BIT_64 << 64) > 0) == 1, 
                   "Compile-time comparison 1");
    _Static_assert((((__int128)MAX_64 << 64) < ((__int128)MAX_64 << 64 | 1)) == 1,
                   "Compile-time comparison 2");
    
    return result != 0;
}
