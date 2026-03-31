/* test_double_int_comparison.c
 * Designed to trigger GCC's internal double_int comparison logic
 * Specifically targeting lines 1285-1293 of double-int.cc
 */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Define 128-bit constants that cross 64-bit boundaries */
#define HIGH_BIT_64    0x8000000000000000ULL
#define MAX_64         0xFFFFFFFFFFFFFFFFULL
#define MID_128        0x123456789ABCDEF0ULL

/* Force compile-time evaluation with static assertions */
_Static_assert(((__int128)HIGH_BIT_64 << 64) > 0, 
               "High bit shift should be positive");
_Static_assert(((__int128)MAX_64) < ((__int128)MAX_64 << 64),
               "128-bit comparison with high word difference");

/* Test function that exercises __int128 range analysis */
static __int128 process_range(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop with 128-bit induction variable - forces VRP analysis */
    for (__int128 i = start; i < end; i += (HIGH_BIT_64 >> 61)) {
        /* Mix operations that require high/low word comparisons */
        if (i > (HIGH_BIT_64 << 64)) {
            sum += i * 2;
        } else if (i < -((__int128)HIGH_BIT_64 << 64)) {
            sum -= i;
        } else {
            sum += i;
        }
        
        /* Force overflow checks with builtins */
        __int128 overflow_check;
        if (__builtin_add_overflow(sum, i, &overflow_check)) {
            sum = overflow_check / 2;
        }
    }
    return sum;
}

/* Function to test comparisons where high words differ */
static int test_high_word_comparisons(void) {
    int checksum = 0;
    
    /* Create 128-bit values with different high words */
    __int128 values[] = {
        ((__int128)HIGH_BIT_64 << 64) | 0x1,          /* High word has MSB set */
        ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | MAX_64, /* Max positive */
        ((__int128)0x1ULL << 64),                     /* Just crosses 64-bit */
        0xFFFFFFFF12345678ULL,                        /* Fits in 64-bit */
        -((__int128)HIGH_BIT_64 << 64) | 0x1,         /* Large negative */
        ((__int128)-1LL << 64) | 0xFFFFFFFFULL,       /* Negative high word */
    };
    
    /* Exercise all comparison operators */
    for (int i = 0; i < 6; i++) {
        for (int j = 0; j < 6; j++) {
            if (values[i] < values[j]) checksum += 1;
            if (values[i] > values[j]) checksum += 2;
            if (values[i] <= values[j]) checksum += 4;
            if (values[i] >= values[j]) checksum += 8;
            if (values[i] == values[j]) checksum += 16;
            if (values[i] != values[j]) checksum += 32;
        }
    }
    
    return checksum;
}

/* Function to test comparisons where high words are equal */
static int test_low_word_comparisons(void) {
    int checksum = 0;
    
    /* Same high word, different low words */
    const __int128 high_part = ((__int128)0x12345678ABCDEF00ULL << 64);
    __int128 values[] = {
        high_part | 0x0,
        high_part | 0x1,
        high_part | 0xFFFFFFFFFFFFFFFEULL,
        high_part | 0xFFFFFFFFFFFFFFFFULL,
        high_part | HIGH_BIT_64,
        high_part | (HIGH_BIT_64 >> 1),
    };
    
    /* Mixed comparisons with different operators */
    for (int i = 0; i < 6; i++) {
        for (int j = i + 1; j < 6; j++) {
            /* These should trigger low-word comparison path */
            checksum += (values[i] < values[j]) ? i : j;
            checksum += (values[i] > values[j]) ? j : i;
            
            /* Force conversion to narrower type for mixed comparisons */
            unsigned long long low_i = (unsigned long long)values[i];
            unsigned long long low_j = (unsigned long long)values[j];
            checksum += (low_i < low_j) ? 1 : 0;
        }
    }
    
    return checksum;
}

/* Test bitwise operations that cross 64-bit boundaries */
static int test_bitwise_operations(void) {
    unsigned __int128 mask = ((unsigned __int128)MAX_64 << 64) | MAX_64;
    unsigned __int128 values[8];
    int checksum = 0;
    
    /* Initialize array with patterns that exercise high/low words */
    for (int i = 0; i < 8; i++) {
        values[i] = ((unsigned __int128)i << (i * 8)) | 
                   ((unsigned __int128)(i + 1) << (64 - i * 4));
    }
    
    /* Perform bitwise operations requiring 128-bit comparisons */
    for (int i = 0; i < 8; i++) {
        unsigned __int128 shifted = values[i] << 32;
        unsigned __int128 masked = values[i] & mask;
        unsigned __int128 orred = values[i] | (mask >> 32);
        
        /* Comparisons after bitwise ops */
        checksum += (shifted > masked) ? 1 : 0;
        checksum += (orred < mask) ? 2 : 0;
        checksum += ((shifted & mask) == masked) ? 4 : 0;
        
        /* Use builtins that may trigger double_int comparisons */
        if (__builtin_expect(values[i] > (mask >> 1), 0)) {
            checksum += i;
        }
    }
    
    return checksum;
}

/* Test overflow operations with __int128 */
static int test_overflow_checks(void) {
    int checksum = 0;
    __int128 max_128 = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | MAX_64;
    __int128 min_128 = -max_128 - 1;
    
    /* Test overflow in addition */
    __int128 a = max_128 - 100;
    __int128 b = 200;
    __int128 result;
    
    if (__builtin_add_overflow(a, b, &result)) {
        checksum += 1;  /* Should trigger */
    }
    
    /* Test overflow in multiplication */
    __int128 c = max_128 / 2;
    __int128 d = 3;
    
    if (__builtin_mul_overflow(c, d, &result)) {
        checksum += 2;  /* Should trigger */
    }
    
    /* Test boundary comparisons */
    checksum += (max_128 > (max_128 - 1)) ? 4 : 0;
    checksum += (min_128 < (min_128 + 1)) ? 8 : 0;
    checksum += ((max_128 >> 1) > (max_128 >> 2)) ? 16 : 0;
    
    return checksum;
}

/* Test switch statement with __int128 case labels */
static int test_switch_comparisons(__int128 value) {
    int result = 0;
    
    /* Switch on 128-bit value - forces compiler to generate comparison tree */
    switch ((unsigned __int128)value) {
        case ((unsigned __int128)0x1ULL << 127):
            result = 1;
            break;
        case ((unsigned __int128)MAX_64 << 64):
            result = 2;
            break;
        case ((unsigned __int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL:
            result = 3;
            break;
        case 0:
            result = 4;
            break;
        case ((unsigned __int128)MAX_64):
            result = 5;
            break;
        default:
            result = 6;
            break;
    }
    
    return result;
}

/* Main function that runs all tests */
int main(void) {
    int total_checksum = 0;
    
    printf("Testing GCC double_int comparison logic...\n");
    
    /* Test 1: High word comparisons */
    total_checksum += test_high_word_comparisons();
    printf("Test 1 complete, checksum so far: %d\n", total_checksum);
    
    /* Test 2: Low word comparisons */
    total_checksum += test_low_word_comparisons();
    printf("Test 2 complete, checksum so far: %d\n", total_checksum);
    
    /* Test 3: Bitwise operations */
    total_checksum += test_bitwise_operations();
    printf("Test 3 complete, checksum so far: %d\n", total_checksum);
    
    /* Test 4: Overflow checks */
    total_checksum += test_overflow_checks();
    printf("Test 4 complete, checksum so far: %d\n", total_checksum);
    
    /* Test 5: Range analysis with loops */
    __int128 range_result = process_range(-1000, 1000);
    total_checksum += (int)range_result;
    printf("Test 5 complete, range result: %lld (low part)\n", 
           (long long)range_result);
    
    /* Test 6: Switch statement comparisons */
    total_checksum += test_switch_comparisons(
        ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL);
    printf("Test 6 complete, checksum so far: %d\n", total_checksum);
    
    /* Test 7: Mixed precision comparisons */
    {
        __int128 large = ((__int128)HIGH_BIT_64 << 64);
        long long medium = HIGH_BIT_64;
        size_t small = 100;
        
        /* Mixed type comparisons */
        total_checksum += (large > (__int128)medium) ? 1 : 0;
        total_checksum += ((unsigned __int128)large < (unsigned __int128)medium) ? 2 : 0;
        total_checksum += (large > (__int128)small) ? 4 : 0;
        
        /* Ternary with mixed types */
        __int128 ternary_result = (medium > 0) ? large : (__int128)small;
        total_checksum += (int)(ternary_result >> 64);
    }
    
    printf("Final checksum: %d\n", total_checksum);
    
    /* Use result to prevent dead code elimination */
    if (total_checksum == 0) {
        printf("Unexpected zero checksum\n");
        return 1;
    }
    
    return 0;
}
