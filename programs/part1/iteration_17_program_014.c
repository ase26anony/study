/* test-double-int.c - Target GCC's double_int comparison logic */
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Define 128-bit constants that cross 64-bit boundaries */
#define HIGH_BIT_64   0x8000000000000000ULL
#define MAX_64        0xFFFFFFFFFFFFFFFFULL
#define MID_128       0x123456789ABCDEF0ULL

/* Force compile-time evaluation with static assertions */
_Static_assert(((__int128)HIGH_BIT_64 << 64) > 0, 
               "High-bit shift should be positive");
_Static_assert(((__int128)0x7FFFFFFFFFFFFFFFULL << 64) < 
               ((__int128)HIGH_BIT_64 << 64),
               "Comparison with different high words");

/* Test 1: High word comparisons (signed) */
__attribute__((noinline))
int test_high_word_comparisons(void) {
    volatile __int128 a, b;
    int checksum = 0;
    
    /* Case 1: High words differ, both positive */
    a = ((__int128)0x1ULL << 64) | 0x123456789ABCDEF0ULL;
    b = ((__int128)0x2ULL << 64) | 0x123456789ABCDEF0ULL;
    checksum += (a < b) ? 1 : 0;  /* Should be true */
    checksum += (a > b) ? 2 : 0;  /* Should be false */
    
    /* Case 2: High words differ, negative vs positive */
    a = ((__int128)HIGH_BIT_64 << 64) | 0x0ULL;  /* Negative */
    b = ((__int128)0x0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL;  /* Positive */
    checksum += (a < b) ? 4 : 0;  /* Should be true (negative < positive) */
    
    /* Case 3: High words equal, low words differ */
    a = ((__int128)0x1ULL << 64) | 0x0ULL;
    b = ((__int128)0x1ULL << 64) | 0x1ULL;
    checksum += (a < b) ? 8 : 0;  /* Should be true */
    
    return checksum;
}

/* Test 2: Boundary value comparisons */
__attribute__((noinline))
int test_boundary_comparisons(void) {
    volatile __int128 max_pos = ~((__int128)0) >> 1;
    volatile __int128 min_neg = ((__int128)HIGH_BIT_64 << 64);
    volatile __int128 zero = 0;
    int checksum = 0;
    
    /* INT128_MAX comparisons */
    checksum += (max_pos > zero) ? 1 : 0;
    checksum += (max_pos < zero) ? 2 : 0;
    checksum += (max_pos == max_pos) ? 4 : 0;
    
    /* INT128_MIN comparisons */
    checksum += (min_neg < zero) ? 8 : 0;
    checksum += (min_neg > zero) ? 16 : 0;
    checksum += (min_neg == min_neg) ? 32 : 0;
    
    /* Max vs Min */
    checksum += (max_pos > min_neg) ? 64 : 0;
    
    return checksum;
}

/* Test 3: Mixed unsigned/signed comparisons */
__attribute__((noinline))
int test_mixed_comparisons(void) {
    volatile unsigned __int128 ua, ub;
    volatile __int128 sa, sb;
    int checksum = 0;
    
    /* Unsigned comparisons that exercise high word logic */
    ua = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFEULL;
    ub = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    checksum += (ua < ub) ? 1 : 0;  /* High equal, low differs */
    
    /* Different high words */
    ua = ((unsigned __int128)0x1ULL << 64);
    ub = ((unsigned __int128)0x2ULL << 64);
    checksum += (ua < ub) ? 2 : 0;
    
    /* Mixed signed/unsigned via ternary */
    sa = -((__int128)1 << 120);
    ua = (unsigned __int128)sa;
    checksum += (sa < 0) ? 4 : 0;
    checksum += (ua > 0) ? 8 : 0;  /* Unsigned view of negative */
    
    return checksum;
}

/* Test 4: Range analysis triggers with loops */
__attribute__((noinline))
int test_range_analysis(void) {
    __int128 accum = 0;
    int checksum = 0;
    
    /* Loop with __int128 induction variable near 64-bit boundary */
    for (__int128 i = ((__int128)0x7FFFFFFFFFFFFFFFULL - 10); 
         i < ((__int128)0x7FFFFFFFFFFFFFFFULL + 10); 
         i++) {
        accum += i;
        /* Force comparison in loop condition */
        if (i < 0) checksum += 1;
        if (i > ((__int128)0x7FFFFFFFFFFFFFFFULL << 1)) checksum += 2;
    }
    
    /* Overflow checking with builtins */
    __int128 x = ((__int128)0x7FFFFFFFFFFFFFFFULL << 60);
    __int128 y = ((__int128)0x7FFFFFFFFFFFFFFFULL << 60);
    __int128 result;
    int overflow = __builtin_add_overflow(x, y, &result);
    checksum += overflow ? 16 : 0;
    
    /* Multiplication overflow check */
    x = ((__int128)0x1ULL << 62);
    overflow = __builtin_mul_overflow(x, x, &result);
    checksum += overflow ? 32 : 0;
    
    return checksum + (int)(accum & 0xFF);
}

/* Test 5: Bitwise operations crossing 64-bit boundary */
__attribute__((noinline))
int test_bitwise_operations(void) {
    volatile __int128 a, b, c;
    int checksum = 0;
    
    /* Shifts crossing 64-bit boundary */
    a = 0x1ULL;
    b = a << 65;  /* Now in high word */
    c = a << 64;  /* Exactly at boundary */
    
    checksum += (b > c) ? 1 : 0;
    checksum += (b > a) ? 2 : 0;
    
    /* Bitwise AND/OR across boundary */
    a = ((__int128)0xFFFF0000FFFF0000ULL << 64) | 0x0000FFFF0000FFFFULL;
    b = ((__int128)0x0000FFFF0000FFFFULL << 64) | 0xFFFF0000FFFF0000ULL;
    c = a & b;
    checksum += (c == 0) ? 4 : 0;
    
    /* Right shift of negative number */
    a = ((__int128)HIGH_BIT_64 << 64) | HIGH_BIT_64;  /* Negative */
    b = a >> 1;
    checksum += (b < 0) ? 8 : 0;  /* Should remain negative */
    
    return checksum;
}

/* Test 6: Switch statement with __int128 case labels */
__attribute__((noinline))
int test_switch_statement(__int128 value) {
    int result = 0;
    
    /* Force compiler to generate comparison tree */
    switch (value) {
        case ((__int128)0x1ULL << 64):
            result = 1;
            break;
        case ((__int128)0x2ULL << 64):
            result = 2;
            break;
        case ((__int128)0x3ULL << 64):
            result = 3;
            break;
        case ((__int128)0x0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL:
            result = 4;
            break;
        default:
            result = (value < 0) ? -1 : 0;
            break;
    }
    
    return result;
}

/* Test 7: Array operations for optimizer */
__attribute__((noinline))
int test_array_operations(void) {
    __int128 arr[8] = {
        ((__int128)0x0ULL << 64) | 0x0000000000000000ULL,
        ((__int128)0x0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL,
        ((__int128)0x1ULL << 64) | 0x0000000000000000ULL,
        ((__int128)0x1ULL << 64) | 0xFFFFFFFFFFFFFFFFULL,
        ((__int128)HIGH_BIT_64 << 64) | 0x0000000000000000ULL,
        ((__int128)HIGH_BIT_64 << 64) | 0xFFFFFFFFFFFFFFFFULL,
        ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL,
        ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x7FFFFFFFFFFFFFFFULL,
    };
    
    int checksum = 0;
    
    /* Compare each element with its neighbors */
    for (int i = 0; i < 7; i++) {
        checksum += (arr[i] < arr[i + 1]) ? (1 << i) : 0;
        checksum += (arr[i] > arr[i + 1]) ? (1 << (i + 8)) : 0;
    }
    
    /* Find min/max in array */
    __int128 min_val = arr[0];
    __int128 max_val = arr[0];
    for (int i = 1; i < 8; i++) {
        if (arr[i] < min_val) min_val = arr[i];
        if (arr[i] > max_val) max_val = arr[i];
    }
    
    checksum += (min_val == arr[4]) ? 256 : 0;  /* Should be most negative */
    checksum += (max_val == arr[6]) ? 512 : 0;  /* Should be most positive */
    
    return checksum;
}

/* Test 8: Builtin functions with __int128 */
__attribute__((noinline))
int test_builtin_functions(void) {
    unsigned __int128 x = ((unsigned __int128)0x1ULL << 127) | 0x1ULL;
    int checksum = 0;
    
    /* Count leading zeros - may trigger internal comparisons */
    checksum += __builtin_clzll((unsigned long long)(x >> 64));
    checksum += __builtin_clzll((unsigned long long)x);
    
    /* Population count */
    checksum += __builtin_popcountll((unsigned long long)(x >> 64));
    checksum += __builtin_popcountll((unsigned long long)x);
    
    /* Use __builtin_expect with wide comparisons */
    __int128 a = ((__int128)0x1ULL << 120);
    __int128 b = ((__int128)0x2ULL << 120);
    if (__builtin_expect(a < b, 1)) {
        checksum += 64;
    }
    
    return checksum;
}

/* Test 9: Mixed precision and conversions */
__attribute__((noinline))
int test_mixed_precision(void) {
    int checksum = 0;
    
    /* Compare __int128 with narrower types */
    __int128 big = ((__int128)0x1ULL << 64);
    long long small = 0x1ULL;
    
    checksum += (big > small) ? 1 : 0;
    checksum += (big == (__int128)small) ? 2 : 0;
    
    /* Ternary with mixed types */
    long long result = (big > 0) ? (long long)big : small;
    checksum += (result == (long long)big) ? 4 : 0;
    
    /* Conversion to size_t */
    size_t s = (size_t)(big >> 64);
    checksum += (s == 1) ? 8 : 0;
    
    /* Variadic function argument (simulated) */
    __int128 var = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    /* Can't actually printf __int128 portably, but force conversion */
    volatile long long parts[2] = {(long long)(var >> 64), (long long)var};
    checksum += (parts[0] == 0x123456789ABCDEF0LL) ? 16 : 0;
    
    return checksum;
}

int main(void) {
    int total_checksum = 0;
    
    printf("Testing GCC double_int comparison logic...\n");
    
    total_checksum += test_high_word_comparisons();
    printf("Test 1 checksum: %d\n", test_high_word_comparisons());
    
    total_checksum += test_boundary_comparisons();
    printf("Test 2 checksum: %d\n", test_boundary_comparisons());
    
    total_checksum += test_mixed_comparisons();
    printf("Test 3 checksum: %d\n", test_mixed_comparisons());
    
    total_checksum += test_range_analysis();
    printf("Test 4 checksum: %d\n", test_range_analysis());
    
    total_checksum += test_bitwise_operations();
    printf("Test 5 checksum: %d\n", test_bitwise_operations());
    
    total_checksum += test_switch_statement((__int128)0x2ULL << 64);
    printf("Test 6 result: %d\n", test_switch_statement((__int128)0x2ULL << 64));
    
    total_checksum += test_array_operations();
    printf("Test 7 checksum: %d\n", test_array_operations());
    
    total_checksum += test_builtin_functions();
    printf("Test 8 checksum: %d\n", test_builtin_functions());
    
    total_checksum += test_mixed_precision();
    printf("Test 9 checksum: %d\n", test_mixed_precision());
    
    printf("Total checksum: 0x%08x\n", total_checksum);
    
    /* Force use of all results to prevent dead code elimination */
    volatile int sink = total_checksum;
    
    return (total_checksum == 0) ? 1 : 0;
}
