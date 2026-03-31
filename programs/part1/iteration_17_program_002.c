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
#define LARGE_CONST_128 ((__int128)0x123456789ABCDEF0ULL << 64 | 0xFEDCBA9876543210ULL)

/* Force compile-time evaluation with static assertions */
_Static_assert(((__int128)1 << 64) > 0, "128-bit shift should work");
_Static_assert(((__int128)HIGH_BIT_64 << 64) < 0, "Negative 128-bit constant");

/* Function to exercise range analysis with __int128 */
static __int128 process_range(__int128 start, __int128 end, int iterations) {
    __int128 sum = 0;
    __int128 step = (end - start) / iterations;
    
    for (__int128 i = start; i < end; i += step) {
        /* Force comparisons that might use double_int logic */
        if (i < 0) {
            sum -= i;
        } else if (i > ((__int128)HIGH_BIT_64 << 64)) {
            sum += i >> 2;
        } else {
            sum += i;
        }
        
        /* Cross 64-bit boundary with bitwise operations */
        sum = (sum << 1) | (sum >> 127);
    }
    return sum;
}

/* Test high-word comparisons (both positive and negative) */
static unsigned test_high_word_comparisons(void) {
    unsigned checksum = 0;
    
    /* Array of __int128 values where high words differ */
    __int128 values[] = {
        /* High word positive comparisons */
        ((__int128)0x1ULL << 64) | 0xFFFFFFFFULL,      /* high=1, low large */
        ((__int128)0x2ULL << 64) | 0x00000001ULL,      /* high=2, low small */
        
        /* High word negative comparisons */
        ((__int128)HIGH_BIT_64 << 64) | 0x0ULL,        /* Most negative */
        ((__int128)(HIGH_BIT_64 | 0x1ULL) << 64) | 0x0ULL, /* Slightly less negative */
        
        /* Equal high word, different low word */
        ((__int128)0x5ULL << 64) | 0x100000000ULL,
        ((__int128)0x5ULL << 64) | 0x200000000ULL,
        
        /* Boundary values */
        ((__int128)MAX_64 << 64) | MAX_64,            /* Near max positive */
        ((__int128)HIGH_BIT_64 << 64) | 0x0ULL,        /* Min negative */
    };
    
    for (size_t i = 0; i < sizeof(values)/sizeof(values[0]) - 1; i++) {
        if (values[i] < values[i + 1]) checksum += 1;
        if (values[i] > values[i + 1]) checksum += 2;
        if (values[i] == values[i + 1]) checksum += 4;
        
        /* Force unsigned comparison through cast */
        unsigned __int128 u1 = (unsigned __int128)values[i];
        unsigned __int128 u2 = (unsigned __int128)values[i + 1];
        if (u1 < u2) checksum += 8;
        if (u1 > u2) checksum += 16;
    }
    
    return checksum;
}

/* Test overflow operations that require wide comparisons */
static unsigned test_overflow_checks(void) {
    unsigned checksum = 0;
    __int128 a, b, result;
    int overflow;
    
    /* Test near boundary additions */
    a = ((__int128)MAX_64 << 64) | (MAX_64 - 100);
    b = 200;
    overflow = __builtin_add_overflow(a, b, &result);
    checksum += overflow ? 1 : 0;
    
    /* Test multiplication that crosses word boundaries */
    a = ((__int128)0x100000000ULL << 32);
    b = 0x100000000ULL;
    overflow = __builtin_mul_overflow(a, b, &result);
    checksum += overflow ? 2 : 0;
    
    /* Test signed overflow with negative numbers */
    a = ((__int128)HIGH_BIT_64 << 64) | 0x0ULL;  /* INT128_MIN */
    b = -1;
    overflow = __builtin_add_overflow(a, b, &result);
    checksum += overflow ? 4 : 0;
    
    return checksum;
}

/* Test mixed-precision operations and conversions */
static unsigned test_mixed_precision(void) {
    unsigned checksum = 0;
    
    /* Compare __int128 with 64-bit types */
    __int128 large = ((__int128)0x1ULL << 64) | 0xFFFFFFFFULL;
    long long medium = 0x7FFFFFFFFFFFFFFFLL;
    unsigned long long ularge = 0xFFFFFFFFFFFFFFFFULL;
    
    checksum += (large > medium) ? 1 : 0;
    checksum += ((unsigned __int128)large > ularge) ? 2 : 0;
    
    /* Ternary operator with mixed types */
    __int128 ternary_result = (medium > 0) ? large : (__int128)medium;
    checksum += (ternary_result == large) ? 4 : 0;
    
    /* Array indexing with __int128 calculations */
    int array[10] = {0};
    __int128 index = ((__int128)0x0ULL << 64) | 0x5ULL;
    array[(int)index] = 42;  /* Force conversion */
    checksum += array[5] == 42 ? 8 : 0;
    
    return checksum;
}

/* Test switch statement with __int128 case labels (compile-time constants) */
static unsigned test_switch_constants(int selector) {
    unsigned result = 0;
    
    /* Convert to __int128 for switch comparison */
    __int128 value = (__int128)selector;
    
    switch (value) {
        case ((__int128)0x0ULL << 64) | 0x0ULL:
            result = 1;
            break;
        case ((__int128)0x0ULL << 64) | 0x1ULL:
            result = 2;
            break;
        case ((__int128)0x1ULL << 64) | 0x0ULL:
            result = 3;  /* High word differs */
            break;
        case ((__int128)HIGH_BIT_64 << 64) | 0x0ULL:
            result = 4;  /* Negative case */
            break;
        default:
            result = 5;
            break;
    }
    
    return result;
}

/* Test bitwise operations crossing 64-bit boundaries */
static unsigned test_bitwise_operations(void) {
    unsigned checksum = 0;
    
    __int128 mask = ((__int128)0x5555555555555555ULL << 64) | 0x5555555555555555ULL;
    __int128 value = LARGE_CONST_128;
    
    /* Operations that need to handle both high and low words */
    __int128 and_result = value & mask;
    __int128 or_result = value | mask;
    __int128 shift_left = value << 65;  /* Cross word boundary */
    __int128 shift_right = value >> 65;
    
    /* Comparisons after bitwise operations */
    checksum += (and_result < or_result) ? 1 : 0;
    checksum += (shift_left > shift_right) ? 2 : 0;
    
    /* Use builtins that might trigger comparisons */
    int popcount = __builtin_popcountll((unsigned long long)(value >> 64)) +
                   __builtin_popcountll((unsigned long long)value);
    checksum += (popcount > 64) ? 4 : 0;
    
    return checksum;
}

/* Test loops with __int128 induction variables */
static unsigned test_loop_boundaries(void) {
    unsigned checksum = 0;
    
    /* Loop that crosses 64-bit boundary */
    for (__int128 i = ((__int128)0xFFFFFFFFFFFFFF00ULL << 64); 
         i < ((__int128)0xFFFFFFFFFFFFFF00ULL << 64) + 300; 
         i++) {
        checksum += (unsigned)(i & 0xFF);
    }
    
    /* Reverse loop with negative values */
    for (__int128 j = -100; j < 100; j++) {
        checksum += (unsigned)(j & 0xFF);
    }
    
    return checksum & 0xFF;
}

int main(void) {
    unsigned total_checksum = 0;
    
    printf("Testing GCC double_int comparison logic...\n");
    
    /* Exercise all test functions */
    total_checksum += test_high_word_comparisons();
    printf("High word comparisons checksum: %u\n", test_high_word_comparisons());
    
    total_checksum += test_overflow_checks();
    printf("Overflow checks checksum: %u\n", test_overflow_checks());
    
    total_checksum += test_mixed_precision();
    printf("Mixed precision checksum: %u\n", test_mixed_precision());
    
    total_checksum += test_switch_constants(3);
    printf("Switch constant test: %u\n", test_switch_constants(3));
    
    total_checksum += test_bitwise_operations();
    printf("Bitwise operations checksum: %u\n", test_bitwise_operations());
    
    total_checksum += test_loop_boundaries();
    printf("Loop boundaries checksum: %u\n", test_loop_boundaries());
    
    /* Force range analysis with large values */
    __int128 range_result = process_range(
        ((__int128)0x0ULL << 64) | 0x0ULL,
        ((__int128)0x10ULL << 64) | 0x0ULL,
        1000
    );
    total_checksum += (unsigned)(range_result & 0xFFFFFFFF);
    
    /* Final printf with __int128 argument (forces conversions) */
    __int128 final_value = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    printf("Final 128-bit value: 0x%016llx%016llx\n", 
           (unsigned long long)(final_value >> 64),
           (unsigned long long)(final_value & 0xFFFFFFFFFFFFFFFFULL));
    
    printf("Total checksum: %u\n", total_checksum);
    
    return (int)(total_checksum & 0xFF);
}
