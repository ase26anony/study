/* test-double-int.c - Designed to trigger GCC's internal double_int comparison logic */
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Define 128-bit constants that cross 64-bit boundaries */
#define HIGH_BIT_64   0x8000000000000000ULL
#define MAX_UINT64    0xFFFFFFFFFFFFFFFFULL
#define HIGH_WORD_DIFF 0x1000000000000000ULL

/* Force compile-time evaluation with static assertions */
_Static_assert(((__int128)HIGH_BIT_64 << 64) > ((__int128)HIGH_BIT_64 << 63), 
               "High word comparison should be triggered");

/* Test 1: High word differs (signed) */
static int test_high_word_signed(void) {
    __int128 a = ((__int128)HIGH_BIT_64 << 64) | 0x123456789ABCDEF0ULL;
    __int128 b = ((__int128)(HIGH_BIT_64 >> 1) << 64) | 0x123456789ABCDEF0ULL;
    __int128 c = -((__int128)HIGH_BIT_64 << 64) | 0x123456789ABCDEF0ULL;
    
    int result = 0;
    if (a > b) result |= 1;      /* High word: 0x8000... > 0x4000... */
    if (c < b) result |= 2;      /* High word: negative < positive */
    if (a != b) result |= 4;
    
    return result;
}

/* Test 2: High word differs (unsigned) */
static int test_high_word_unsigned(void) {
    unsigned __int128 a = ((unsigned __int128)MAX_UINT64 << 64) | 0xFFFFFFFFFFFFFFFFULL;
    unsigned __int128 b = ((unsigned __int128)MAX_UINT64 << 63) | 0xFFFFFFFFFFFFFFFFULL;
    unsigned __int128 c = ((unsigned __int128)HIGH_WORD_DIFF << 64) | 0x0ULL;
    
    int result = 0;
    if (a > b) result |= 1;      /* High word: 0xFFFF... > 0x7FFF... */
    if (c < a) result |= 2;      /* High word: 0x1000... < 0xFFFF... */
    if (b >= c) result |= 4;
    
    return result;
}

/* Test 3: High words equal, low words differ */
static int test_low_word_diff(void) {
    __int128 base_high = ((__int128)0x123456789ABCDEF0ULL << 64);
    __int128 a = base_high | 0xFFFFFFFFFFFFFFFFULL;
    __int128 b = base_high | 0x7FFFFFFFFFFFFFFFULL;
    __int128 c = base_high | 0x0ULL;
    
    int result = 0;
    if (a > b) result |= 1;      /* Same high, low: 0xFFF... > 0x7FF... */
    if (b > c) result |= 2;      /* Same high, low: 0x7FF... > 0 */
    if (c < a) result |= 4;
    
    return result;
}

/* Test 4: Boundary values */
static int test_boundaries(void) {
    __int128 max_signed = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | MAX_UINT64;
    __int128 min_signed = ((__int128)HIGH_BIT_64 << 64) | 0x0ULL;
    unsigned __int128 max_unsigned = ((unsigned __int128)MAX_UINT64 << 64) | MAX_UINT64;
    
    int result = 0;
    if (max_signed > min_signed) result |= 1;
    if (min_signed < 0) result |= 2;
    if ((unsigned __int128)max_unsigned > (unsigned __int128)max_signed) result |= 4;
    
    /* Force high-word comparison at boundary */
    __int128 near_max = max_signed - 1;
    __int128 near_min = min_signed + 1;
    if (near_max > near_min) result |= 8;
    
    return result;
}

/* Test 5: Range analysis with loops */
static int test_range_analysis(void) {
    int result = 0;
    
    /* Loop with __int128 induction variable crossing 64-bit boundary */
    for (__int128 i = ((__int128)0x7FFFFFFFFFFFFFF0ULL << 64); 
         i < ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) + 100; 
         i++) {
        if (i > ((__int128)0x7FFFFFFFFFFFFFFFULL << 64)) {
            result += (int)(i & 0xFF);
        }
    }
    
    /* Value range propagation test */
    __int128 x = ((__int128)0x12345678ULL << 64);
    __int128 y = x + 0xFFFFFFFFULL;
    
    if (x < y) result |= 0x10;
    if (y > x) result |= 0x20;
    
    return result & 0xFF;
}

/* Test 6: Bitwise operations crossing 64-bit boundary */
static int test_bitwise_ops(void) {
    unsigned __int128 a = ((unsigned __int128)0xF0F0F0F0F0F0F0F0ULL << 64) | 0x0F0F0F0F0F0F0F0FULL;
    unsigned __int128 b = ((unsigned __int128)0x0F0F0F0F0F0F0F0FULL << 64) | 0xF0F0F0F0F0F0F0F0ULL;
    
    int result = 0;
    unsigned __int128 c = a & b;
    unsigned __int128 d = a | b;
    unsigned __int128 e = a ^ b;
    unsigned __int128 f = a << 65;  /* Shift across boundary */
    unsigned __int128 g = b >> 65;
    
    if (c == 0) result |= 1;
    if (d == ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL) result |= 2;
    if (e == d) result |= 4;
    if (f > a) result |= 8;
    if (g < b) result |= 16;
    
    return result;
}

/* Test 7: Overflow checking with builtins */
static int test_overflow_checks(void) {
    int result = 0;
    __int128 x = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFEULL;
    __int128 y = 3;
    __int128 overflow;
    
    /* These should trigger overflow checks with wide comparisons */
    if (__builtin_add_overflow(x, y, &overflow)) {
        result |= 1;
    }
    
    __int128 z = ((__int128)0x4000000000000000ULL << 64);
    if (__builtin_mul_overflow(z, 2, &overflow)) {
        result |= 2;
    }
    
    return result;
}

/* Test 8: Mixed-precision operations */
static int test_mixed_precision(void) {
    int result = 0;
    
    __int128 a = ((__int128)0x123456789ABCDEF0ULL << 64);
    unsigned long long b = 0xFFFFFFFFFFFFFFFFULL;
    size_t c = SIZE_MAX;
    
    /* Mixed comparisons */
    if (a > b) result |= 1;
    if ((unsigned __int128)a < c) result |= 2;
    
    /* Ternary with mixed types */
    __int128 d = (b > 1000) ? a : (__int128)b;
    if (d == a) result |= 4;
    
    /* Array indexing with wide integers (simulated) */
    __int128 values[4] = {
        ((__int128)0x1ULL << 64),
        ((__int128)0x2ULL << 64),
        ((__int128)0x3ULL << 64),
        ((__int128)0x4ULL << 64)
    };
    
    for (int i = 0; i < 4; i++) {
        if (values[i] > ((__int128)0x2ULL << 64)) {
            result += 8;
        }
    }
    
    return result;
}

/* Test 9: Compiler builtins with wide integers */
static int test_builtins(void) {
    int result = 0;
    
    unsigned __int128 x = ((unsigned __int128)0x1ULL << 127) | 0x1;
    
    /* Count leading zeros - may trigger comparisons */
    int clz_high = __builtin_clzll((unsigned long long)(x >> 64));
    int clz_low = __builtin_clzll((unsigned long long)x);
    
    if (clz_high >= 0) result |= 1;
    if (clz_low >= 0) result |= 2;
    
    /* Population count */
    unsigned __int128 y = x - 1;
    int popcnt = __builtin_popcountll((unsigned long long)(y >> 64)) +
                 __builtin_popcountll((unsigned long long)y);
    
    if (popcnt > 0) result |= 4;
    
    /* Branch prediction with wide comparison */
    if (__builtin_expect(x > ((unsigned __int128)0x1ULL << 126), 1)) {
        result |= 8;
    }
    
    return result;
}

/* Test 10: Switch statement with __int128 cases */
static int test_switch_cases(__int128 value) {
    int result = 0;
    
    /* Switch on lower bits to avoid massive jump tables */
    switch ((unsigned long long)(value & 0xF)) {
        case 0:
            if (value > ((__int128)0x1ULL << 64)) result = 1;
            break;
        case 1:
            if (value < ((__int128)0x2ULL << 64)) result = 2;
            break;
        case 2:
            if (value == ((__int128)0x3ULL << 64)) result = 3;
            break;
        default:
            if (value >= 0) result = 4;
            break;
    }
    
    return result;
}

/* Main function with array operations to prevent optimization */
int main(void) {
    int checksum = 0;
    
    /* Array of __int128 to give optimizer work */
    __int128 array[8] = {
        ((__int128)0x1ULL << 64) | 0x1,
        ((__int128)0x2ULL << 64) | 0x2,
        ((__int128)0x3ULL << 64) | 0x3,
        ((__int128)0x4ULL << 64) | 0x4,
        ((__int128)0x5ULL << 64) | 0x5,
        ((__int128)0x6ULL << 64) | 0x6,
        ((__int128)0x7ULL << 64) | 0x7,
        ((__int128)0x8ULL << 64) | 0x8
    };
    
    /* Process array with comparisons */
    for (int i = 0; i < 7; i++) {
        if (array[i] < array[i + 1]) {
            checksum += (int)(array[i] & 0xFF);
        }
    }
    
    /* Run all tests */
    checksum += test_high_word_signed();
    checksum += test_high_word_unsigned();
    checksum += test_low_word_diff();
    checksum += test_boundaries();
    checksum += test_range_analysis();
    checksum += test_bitwise_ops();
    checksum += test_overflow_checks();
    checksum += test_mixed_precision();
    checksum += test_builtins();
    checksum += test_switch_cases(array[0]);
    
    printf("Checksum: %d\n", checksum);
    
    /* Force use of variadic function with __int128 */
    printf("Sample values: %lld, %lld\n", 
           (long long)(array[0] >> 64), 
           (long long)(array[0] & 0xFFFFFFFFFFFFFFFFULL));
    
    return checksum != 0 ? 0 : 1;
}
