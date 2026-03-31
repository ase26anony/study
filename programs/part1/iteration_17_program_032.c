/* test-double-int.c
 * Designed to trigger GCC's internal double_int comparison logic
 * Specifically targeting lines 1285-1293 of double-int.cc
 */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>

/* Define 128-bit constants that cross 64-bit boundaries */
#define HIGH_BIT_64   0x8000000000000000ULL
#define MAX_UINT64    0xFFFFFFFFFFFFFFFFULL
#define MID_RANGE_64  0x7FFFFFFFFFFFFFFFULL

/* Large 128-bit constants for comparison */
static const __int128 NEGATIVE_BIG = ((__int128)HIGH_BIT_64 << 64) | HIGH_BIT_64;
static const __int128 POSITIVE_BIG = ((__int128)MID_RANGE_64 << 64) | MAX_UINT64;
static const __int128 ZERO_HIGH_POS_LOW = (__int128)0 << 64 | HIGH_BIT_64;
static const __int128 ZERO_HIGH_NEG_LOW = (__int128)0 << 64 | (HIGH_BIT_64 >> 1);

/* Unsigned versions */
static const unsigned __int128 UMAX_128 = ~((unsigned __int128)0);
static const unsigned __int128 UHIGH_ONLY = ((unsigned __int128)MAX_UINT64 << 64);
static const unsigned __int128 ULOW_ONLY = (unsigned __int128)MAX_UINT64;

/* Force compile-time comparisons with static assertions */
_Static_assert(((__int128)1 << 120) > (((__int128)1 << 120) - 1), 
               "128-bit constant folding comparison 1");
_Static_assert((unsigned __int128)UHIGH_ONLY > ULOW_ONLY,
               "128-bit constant folding comparison 2");

/* Function to create value ranges that span both words */
static __int128 create_range_based_value(int selector) {
    __int128 result = 0;
    
    /* Different ranges based on selector */
    switch (selector & 7) {
        case 0:
            /* High word negative, low word positive */
            result = NEGATIVE_BIG + (selector * 1000);
            break;
        case 1:
            /* High word zero, low word near boundary */
            result = ZERO_HIGH_POS_LOW - (selector * 500);
            break;
        case 2:
            /* Both words positive */
            result = POSITIVE_BIG >> (selector * 3);
            break;
        case 3:
            /* High word positive, low word negative */
            result = ((__int128)MID_RANGE_64 << 64) | (__int128)(-selector * 1000);
            break;
        case 4:
            /* Near INT128_MIN */
            result = ((__int128)HIGH_BIT_64 << 64) + selector;
            break;
        case 5:
            /* Near INT128_MAX */
            result = ~((__int128)HIGH_BIT_64 << 64) - selector;
            break;
        case 6:
            /* Cross 64-bit boundary with bit operations */
            result = ((__int128)selector << 96) | ((__int128)selector << 32) | selector;
            break;
        case 7:
            /* Alternating bit pattern */
            result = ((__int128)0xAAAAAAAAAAAAAAAAULL << 64) | 0x5555555555555555ULL;
            result += selector;
            break;
    }
    
    return result;
}

/* Force VRP to analyze __int128 ranges */
static int analyze_128bit_range(__int128 start, __int128 end) {
    int count = 0;
    
    /* Loop with 128-bit induction variable */
    for (__int128 i = start; i < end && i < start + 100; ++i) {
        /* Comparisons that exercise high/low word logic */
        if (i < 0) {
            if ((unsigned __int128)i > UHIGH_ONLY) {
                count += 1;
            }
        } else {
            if (i > ZERO_HIGH_POS_LOW) {
                count += 2;
            }
        }
        
        /* Mixed-type comparisons */
        if (i > (long long)MID_RANGE_64) {
            count += 4;
        }
        
        /* Bitwise operations crossing 64-bit boundary */
        __int128 shifted = i << 3;
        if ((shifted & (((__int128)1 << 100))) != 0) {
            count += 8;
        }
    }
    
    return count;
}

/* Test overflow operations requiring wide comparisons */
static int test_overflow_operations(void) {
    int checksum = 0;
    __int128 a, b, result;
    unsigned __int128 ua, ub, uresult;
    int overflow;
    
    /* Test signed overflow */
    a = ((__int128)MID_RANGE_64 << 64) | MAX_UINT64;
    b = 1;
    overflow = __builtin_add_overflow(a, b, &result);
    checksum += overflow ? 1 : 0;
    
    if (overflow) {
        /* This comparison should exercise the high word check */
        checksum += (result < a) ? 2 : 0;
    }
    
    /* Test unsigned overflow */
    ua = UMAX_128 - 100;
    ub = 200;
    overflow = __builtin_add_overflow(ua, ub, &uresult);
    checksum += overflow ? 4 : 0;
    
    /* Test multiplication overflow */
    a = ((__int128)1 << 62);
    overflow = __builtin_mul_overflow(a, a, &result);
    checksum += overflow ? 8 : 0;
    
    return checksum;
}

/* Switch statement with __int128 case labels (compile-time constants) */
static int switch_128bit(__int128 value) {
    /* GCC must generate comparison trees for these cases */
    switch ((unsigned __int128)value & 0xFF) {
        case ((unsigned __int128)0x01ULL << 64):
            return 1;
        case ((unsigned __int128)0x80ULL << 64):
            return 2;
        case ((unsigned __int128)MAX_UINT64 << 63):
            return 3;
        default:
            if (value == NEGATIVE_BIG) return 4;
            if (value == POSITIVE_BIG) return 5;
            if (value == ZERO_HIGH_POS_LOW) return 6;
            return 0;
    }
}

/* Array operations with __int128 */
static int process_128bit_array(void) {
    __int128 arr[8];
    unsigned __int128 uarr[8];
    int checksum = 0;
    
    /* Initialize arrays with values that exercise different comparison paths */
    for (int i = 0; i < 8; i++) {
        arr[i] = create_range_based_value(i);
        uarr[i] = (unsigned __int128)arr[i];
        
        /* Force comparisons during initialization */
        if (i > 0) {
            /* Compare with previous element - exercises high/low word logic */
            if (arr[i] > arr[i-1]) checksum += 1 << (i*2);
            if (uarr[i] < uarr[i-1]) checksum += 1 << (i*2 + 1);
        }
    }
    
    /* Additional comparisons between array elements */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (i != j) {
                /* These comparisons should trigger the target code */
                if (arr[i] < arr[j]) checksum += 1;
                if (uarr[i] > uarr[j]) checksum += 2;
                
                /* Mixed signed/unsigned comparisons */
                if ((unsigned __int128)arr[i] < uarr[j]) checksum += 4;
            }
        }
    }
    
    /* Boundary value comparisons */
    if (arr[0] == NEGATIVE_BIG) checksum += 0x100;
    if (arr[7] < POSITIVE_BIG) checksum += 0x200;
    if (uarr[4] >= UHIGH_ONLY) checksum += 0x400;
    
    return checksum;
}

/* Use __builtin_expect with 128-bit comparisons */
static int builtin_expect_test(__int128 a, __int128 b) {
    int result = 0;
    
    /* Force GCC to generate branch prediction data */
    if (__builtin_expect(a < b, 1)) {
        result |= 0x01;
    }
    
    if (__builtin_expect((unsigned __int128)a > (unsigned __int128)b, 0)) {
        result |= 0x02;
    }
    
    /* Compare high words only through shifting */
    if (__builtin_expect((a >> 64) == (b >> 64), 0)) {
        result |= 0x04;
        /* Now compare low words */
        if (__builtin_expect((a & MAX_UINT64) < (b & MAX_UINT64), 1)) {
            result |= 0x08;
        }
    }
    
    return result;
}

/* Variadic function to force conversions */
static void variadic_conversion_test(__int128 value) {
    /* Force conversions to narrower types */
    long long ll_part = (long long)(value & MAX_UINT64);
    unsigned long long ull_part = (unsigned long long)((value >> 64) & MAX_UINT64);
    
    /* These comparisons may trigger internal conversions */
    if (value > ll_part) {
        printf("Value > ll_part\n");
    }
    
    if ((unsigned __int128)value < ull_part) {
        printf("Value < ull_part\n");
    }
}

/* Main test driver */
int main(void) {
    int total_checksum = 0;
    
    printf("Testing 128-bit comparison paths...\n");
    
    /* Test 1: Direct comparisons with boundary values */
    printf("Test 1: Boundary comparisons\n");
    total_checksum += (NEGATIVE_BIG < POSITIVE_BIG) ? 1 : 0;
    total_checksum += (ZERO_HIGH_POS_LOW > ZERO_HIGH_NEG_LOW) ? 2 : 0;
    total_checksum += ((unsigned __int128)UHIGH_ONLY > ULOW_ONLY) ? 4 : 0;
    
    /* Test 2: Range analysis */
    printf("Test 2: Range analysis\n");
    total_checksum += analyze_128bit_range(NEGATIVE_BIG, NEGATIVE_BIG + 50);
    total_checksum += analyze_128bit_range(0, 1000);
    
    /* Test 3: Overflow operations */
    printf("Test 3: Overflow checks\n");
    total_checksum += test_overflow_operations();
    
    /* Test 4: Array processing */
    printf("Test 4: Array operations\n");
    total_checksum += process_128bit_array();
    
    /* Test 5: Builtin expect with comparisons */
    printf("Test 5: Builtin expect\n");
    total_checksum += builtin_expect_test(NEGATIVE_BIG, POSITIVE_BIG);
    total_checksum += builtin_expect_test(ZERO_HIGH_POS_LOW, ZERO_HIGH_NEG_LOW);
    
    /* Test 6: Switch statement */
    printf("Test 6: Switch statement\n");
    total_checksum += switch_128bit(NEGATIVE_BIG);
    total_checksum += switch_128bit(POSITIVE_BIG);
    
    /* Test 7: Variadic conversions */
    printf("Test 7: Variadic conversions\n");
    variadic_conversion_test(NEGATIVE_BIG);
    variadic_conversion_test(POSITIVE_BIG);
    
    /* Test 8: Ternary operations with mixed types */
    printf("Test 8: Ternary operations\n");
    for (int i = 0; i < 4; i++) {
        __int128 test_val = create_range_based_value(i);
        long long narrow = (i % 2) ? MAX_UINT64 : -HIGH_BIT_64;
        
        /* Ternary with 128-bit and 64-bit types */
        __int128 ternary_result = (test_val > 0) ? test_val : narrow;
        total_checksum += (ternary_result > 0) ? (1 << i) : 0;
    }
    
    /* Test 9: Bitwise operations crossing boundaries */
    printf("Test 9: Bitwise operations\n");
    __int128 bitwise = ((__int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAAULL;
    __int128 shifted = bitwise << 33;  /* Crosses 64-bit boundary */
    __int128 masked = shifted & (((__int128)0xF0F0F0F0F0F0F0F0ULL << 64) | 0x0F0F0F0F0F0F0F0FULL);
    
    total_checksum += (shifted > bitwise) ? 0x10 : 0;
    total_checksum += (masked < shifted) ? 0x20 : 0;
    
    /* Test 10: Mixed-precision arithmetic */
    printf("Test 10: Mixed precision\n");
    __int128 mixed = 0;
    for (int i = 0; i < 8; i++) {
        mixed += (size_t)(1ULL << i);  /* Mix with size_t */
        mixed -= (unsigned long long)(i * 1000);
        
        /* Compare with different types */
        if (mixed > (long long)MID_RANGE_64) total_checksum += 0x40;
        if ((unsigned __int128)mixed < (size_t)(-1)) total_checksum += 0x80;
    }
    
    printf("Total checksum: %d\n", total_checksum);
    printf("All tests completed.\n");
    
    return total_checksum != 0 ? 0 : 1;
}
