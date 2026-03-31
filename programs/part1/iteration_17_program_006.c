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
_Static_assert(((__int128)HIGH_BIT_64 << 64) > 0, 
               "High-bit shift should be positive");
_Static_assert(((__int128)0x7FFFFFFFFFFFFFFFULL << 64) < 
               ((__int128)0x8000000000000000ULL << 64),
               "High-word comparison test");

/* Test 1: High word differs (signed) */
__attribute__((noinline))
int test_high_word_signed_comparison(void) {
    volatile __int128 a = ((__int128)HIGH_BIT_64 << 64) | 0x123456789ABCDEF0ULL;
    volatile __int128 b = ((__int128)(HIGH_BIT_64 >> 1) << 64) | 0x123456789ABCDEF0ULL;
    volatile __int128 c = -((__int128)HIGH_BIT_64 << 64) | 0x123456789ABCDEF0ULL;
    
    int result = 0;
    result += (a > b) ? 1 : 0;  /* High word: 0x8000... > 0x4000... */
    result += (c < b) ? 2 : 0;  /* Negative high word < positive */
    result += (a != b) ? 4 : 0;
    
    /* Force unsigned comparison of high words */
    volatile unsigned __int128 ua = (unsigned __int128)a;
    volatile unsigned __int128 ub = (unsigned __int128)b;
    result += (ua > ub) ? 8 : 0;
    
    return result;
}

/* Test 2: High words equal, low words differ */
__attribute__((noinline))
int test_low_word_comparison(void) {
    const __int128 base = ((__int128)0x123456789ABCDEF0ULL << 64);
    volatile __int128 vals[8] = {
        base | 0x0000000000000000ULL,
        base | 0x0000000000000001ULL,
        base | 0x7FFFFFFFFFFFFFFFULL,
        base | 0x8000000000000000ULL,
        base | 0xFFFFFFFFFFFFFFFEULL,
        base | 0xFFFFFFFFFFFFFFFFULL,
        -base | 0x0000000000000000ULL,
        -base | 0xFFFFFFFFFFFFFFFFULL
    };
    
    int result = 0;
    for (int i = 0; i < 7; i++) {
        result += (vals[i] < vals[i + 1]) ? (1 << i) : 0;
        result += (vals[i] != vals[i + 1]) ? (1 << (i + 8)) : 0;
    }
    
    return result;
}

/* Test 3: Boundary value comparisons */
__attribute__((noinline))
int test_boundary_comparisons(void) {
    volatile __int128 max_signed = ~((__int128)1 << 127);
    volatile __int128 min_signed = ((__int128)1 << 127);
    volatile unsigned __int128 max_unsigned = ~((unsigned __int128)0);
    
    int result = 0;
    
    /* Signed comparisons at boundaries */
    result += (max_signed > 0) ? 1 : 0;
    result += (min_signed < 0) ? 2 : 0;
    result += (max_signed > min_signed) ? 4 : 0;
    
    /* Mixed signed/unsigned */
    result += ((unsigned __int128)max_signed < max_unsigned) ? 8 : 0;
    result += ((__int128)max_unsigned < 0) ? 16 : 0;  /* Sign extension */
    
    /* Near overflow boundaries */
    volatile __int128 near_overflow = max_signed - 1;
    result += (near_overflow + 1 > near_overflow) ? 32 : 0;
    
    return result;
}

/* Test 4: Range analysis with loops */
__attribute__((noinline))
int test_range_analysis(void) {
    int result = 0;
    
    /* Loop with __int128 induction variable */
    for (__int128 i = ((__int128)HIGH_BIT_64 >> 2); 
         i < ((__int128)HIGH_BIT_64 >> 2) + 100; 
         i++) {
        if (i > ((__int128)HIGH_BIT_64 >> 2) + 50) {
            result += (int)(i & 0xFF);
        }
    }
    
    /* Overflow checking with builtins */
    volatile __int128 x = ((__int128)0x7FFFFFFFFFFFFFFFULL << 32);
    volatile __int128 y = ((__int128)0x7FFFFFFFFFFFFFFFULL << 32);
    __int128 sum;
    
    if (__builtin_add_overflow(x, y, &sum)) {
        result |= 0x100;
    }
    
    /* Multiplication overflow check */
    volatile __int128 m1 = ((__int128)0x1000000000000000ULL);
    volatile __int128 m2 = ((__int128)0x1000000000000000ULL);
    __int128 prod;
    
    if (__builtin_mul_overflow(m1, m2, &prod)) {
        result |= 0x200;
    }
    
    return result;
}

/* Test 5: Bitwise operations crossing 64-bit boundary */
__attribute__((noinline))
int test_bitwise_operations(void) {
    volatile __int128 a = ((__int128)0xF0F0F0F0F0F0F0F0ULL << 64) | 
                          0x0F0F0F0F0F0F0F0FULL;
    volatile __int128 b = ((__int128)0x0F0F0F0F0F0F0F0FULL << 64) | 
                          0xF0F0F0F0F0F0F0F0ULL;
    
    int result = 0;
    
    /* Bitwise operations */
    volatile __int128 and_result = a & b;
    volatile __int128 or_result = a | b;
    volatile __int128 xor_result = a ^ b;
    volatile __int128 shift_left = a << 65;  /* Cross word boundary */
    volatile __int128 shift_right = a >> 65;
    
    /* Comparisons after bitwise ops */
    result += (and_result < or_result) ? 1 : 0;
    result += (xor_result > and_result) ? 2 : 0;
    result += (shift_left > a) ? 4 : 0;
    result += (shift_right < a) ? 8 : 0;
    
    /* Builtin bit operations */
    result += __builtin_popcountll((unsigned long long)(a >> 64)) << 4;
    result += __builtin_clzll((unsigned long long)(b & MAX_64)) << 8;
    
    return result;
}

/* Test 6: Mixed precision and conversions */
__attribute__((noinline))
int test_mixed_precision(void) {
    int result = 0;
    
    /* Compare __int128 with narrower types */
    volatile __int128 wide = ((__int128)0x123456789ABCDEF0ULL << 32);
    volatile long long narrow = 0x123456789ABCDEF0LL;
    volatile unsigned long long unarrow = 0xFEDCBA9876543210ULL;
    volatile size_t sizet = SIZE_MAX;
    
    result += (wide > narrow) ? 1 : 0;
    result += ((unsigned __int128)wide > unarrow) ? 2 : 0;
    result += (wide < (__int128)sizet) ? 4 : 0;
    
    /* Ternary operator with mixed types */
    volatile int condition = 1;
    volatile __int128 ternary_result = condition ? 
        ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) : 
        (__int128)narrow;
    
    result += (ternary_result > 0) ? 8 : 0;
    
    /* Array of __int128 with comparisons */
    volatile __int128 arr[8];
    for (int i = 0; i < 8; i++) {
        arr[i] = ((__int128)i << 66) | (i * 0x1000000000000000ULL);
    }
    
    for (int i = 0; i < 7; i++) {
        if (arr[i] < arr[i + 1]) {
            result += (1 << (i + 4));
        }
    }
    
    return result;
}

/* Test 7: Switch statement with __int128 cases */
__attribute__((noinline))
int test_switch_statement(volatile __int128 val) {
    int result = 0;
    
    /* Switch forces compiler to generate comparison tree */
    switch (val) {
        case ((__int128)0x1000000000000000ULL << 64):
            result = 1;
            break;
        case ((__int128)0x2000000000000000ULL << 64):
            result = 2;
            break;
        case ((__int128)0x3000000000000000ULL << 64):
            result = 3;
            break;
        case ((__int128)0x4000000000000000ULL << 64):
            result = 4;
            break;
        case 0:
            result = 5;
            break;
        case -((__int128)0x1000000000000000ULL << 64):
            result = 6;
            break;
        default:
            result = 7;
            break;
    }
    
    return result;
}

/* Test 8: Builtin expect with __int128 comparisons */
__attribute__((noinline))
int test_builtin_expect(void) {
    volatile __int128 likely_small = 100;
    volatile __int128 unlikely_large = ((__int128)HIGH_BIT_64 << 64);
    
    int result = 0;
    
    if (__builtin_expect(likely_small < unlikely_large, 1)) {
        result |= 1;
    }
    
    if (__builtin_expect(unlikely_large > likely_small, 0)) {
        result |= 2;
    }
    
    /* Chain of comparisons */
    volatile __int128 mid = ((__int128)HIGH_BIT_64 >> 1) << 64;
    if (__builtin_expect(likely_small < mid && mid < unlikely_large, 1)) {
        result |= 4;
    }
    
    return result;
}

int main(void) {
    int checksum = 0;
    
    printf("Testing double_int comparison logic...\n");
    
    /* Run all tests to exercise different comparison paths */
    checksum ^= test_high_word_signed_comparison();
    printf("Test 1 result: %d\n", checksum);
    
    checksum ^= test_low_word_comparison();
    printf("Test 2 result: %d\n", checksum);
    
    checksum ^= test_boundary_comparisons();
    printf("Test 3 result: %d\n", checksum);
    
    checksum ^= test_range_analysis();
    printf("Test 4 result: %d\n", checksum);
    
    checksum ^= test_bitwise_operations();
    printf("Test 5 result: %d\n", checksum);
    
    checksum ^= test_mixed_precision();
    printf("Test 6 result: %d\n", checksum);
    
    /* Test switch with different values */
    checksum ^= test_switch_statement(((__int128)0x1000000000000000ULL << 64));
    checksum ^= test_switch_statement(0);
    checksum ^= test_switch_statement(-((__int128)0x1000000000000000ULL << 64));
    printf("Test 7 result: %d\n", checksum);
    
    checksum ^= test_builtin_expect();
    printf("Test 8 result: %d\n", checksum);
    
    printf("Final checksum: %d\n", checksum);
    
    /* Force use of variadic function with __int128 */
    volatile __int128 var1 = ((__int128)0x123456789ABCDEF0ULL << 64);
    volatile __int128 var2 = 0xFEDCBA9876543210ULL;
    
    /* This may trigger conversion sequences */
    printf("Variadic test: %lld %lld\n", 
           (long long)(var1 >> 64), 
           (long long)(var2 & 0xFFFFFFFFFFFFFFFFULL));
    
    return checksum != 0 ? 0 : 1;
}
