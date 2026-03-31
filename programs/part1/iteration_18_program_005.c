/* double-int-test.c - Test program to trigger double_int::cmp coverage */
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large 128-bit constants with varying high/low word patterns */
#define HIGH_WORD_DIFF_LOW_EQUAL ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL
#define HIGH_WORD_DIFF_LOW_SMALLER ((__int128)0x123456789ABCDEF0ULL << 64) | 0x0ULL
#define HIGH_WORD_EQUAL_LOW_DIFF1 ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL
#define HIGH_WORD_EQUAL_LOW_DIFF2 ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFFFFFFFFFFFFFFFEULL
#define NEGATIVE_LARGE ((__int128)(-1) << 64) | 0x0ULL  /* High word all 1s when cast to unsigned */

/* Global arrays with 128-bit constants - forces constant folding */
static const __int128 global_consts[] = {
    HIGH_WORD_DIFF_LOW_EQUAL,
    HIGH_WORD_DIFF_LOW_SMALLER,
    HIGH_WORD_EQUAL_LOW_DIFF1,
    HIGH_WORD_EQUAL_LOW_DIFF2,
    NEGATIVE_LARGE,
    ((__int128)0x0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL,  /* Only low word set */
    ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0ULL,  /* Only high word set */
};

/* Static assertions forcing compile-time comparisons */
static_assert(HIGH_WORD_DIFF_LOW_EQUAL > HIGH_WORD_DIFF_LOW_SMALLER, 
              "High word difference comparison");
static_assert(HIGH_WORD_EQUAL_LOW_DIFF1 > HIGH_WORD_EQUAL_LOW_DIFF2, 
              "Low word difference comparison");
static_assert(NEGATIVE_LARGE < 0, "Negative value check");

/* Function 1: Range analysis with comparisons in both high and low words */
__int128 range_compare(__int128 a, __int128 b) {
    /* These comparisons should trigger double_int::cmp in VRP */
    if (a < HIGH_WORD_DIFF_LOW_EQUAL) {
        if (b > HIGH_WORD_EQUAL_LOW_DIFF2) {
            return a + b;
        }
    }
    
    /* Cross-type comparison */
    if (a < (unsigned long long)0xFFFFFFFFFFFFFFFFULL) {
        return b;
    }
    
    /* Ternary with type conversion */
    return (a > 0) ? a : (__int128)b;
}

/* Function 2: Bitwise operations crossing word boundaries */
__int128 bitwise_operations(__int128 x) {
    /* Shift moving bits from low to high word */
    __int128 shifted = x << 72;
    
    /* Mask targeting specific words */
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    
    /* Operations requiring high/low word reasoning */
    __int128 high_part = x & mask_high;
    __int128 low_part = x & mask_low;
    
    /* Arithmetic right shift on negative values */
    __int128 neg_shift = (x < 0) ? (x >> 68) : (x >> 4);
    
    return (high_part | (low_part << 8)) ^ neg_shift;
}

/* Function 3: Loop with 128-bit induction variable */
__int128 loop_with_128bit_iv(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop bound comparison should trigger double_int::cmp */
    for (__int128 i = start; i < end; i += ((__int128)1 << 32)) {
        /* Overflow check - may use double_int comparisons internally */
        __int128 temp;
        if (!__builtin_add_overflow(sum, i, &temp)) {
            sum = temp;
        }
        
        /* Nested comparison with constant */
        if (i > HIGH_WORD_EQUAL_LOW_DIFF1) {
            sum += 1;
        }
    }
    
    return sum;
}

/* Function 4: Overflow operations with 128-bit values */
int overflow_checks(__int128 a, __int128 b) {
    __int128 result;
    int overflow;
    
    /* These may trigger double_int comparisons during overflow analysis */
    overflow = __builtin_add_overflow(a, b, &result);
    if (!overflow) {
        overflow = __builtin_mul_overflow(a, b, &result);
    }
    
    /* Compare result against large constant */
    if (result > HIGH_WORD_DIFF_LOW_SMALLER) {
        return 1;
    }
    
    return overflow;
}

/* Function 5: Dead code with constant comparisons (still processed by compiler) */
__int128 dead_code_comparisons(__int128 x) {
    __int128 result = x;
    
    /* Dead code - but compiler may still evaluate the comparisons */
    if (0) {
        /* Series of comparisons covering all uncovered cases */
        if (HIGH_WORD_DIFF_LOW_EQUAL < HIGH_WORD_DIFF_LOW_SMALLER) result += 1;
        if (HIGH_WORD_EQUAL_LOW_DIFF1 < HIGH_WORD_EQUAL_LOW_DIFF2) result += 2;
        if (NEGATIVE_LARGE > 0) result += 4;
        
        /* Mixed-type in ternary */
        result = (HIGH_WORD_DIFF_LOW_EQUAL > 100) ? 
                 HIGH_WORD_DIFF_LOW_EQUAL : (__int128)100;
    }
    
    return result;
}

/* Function to process global constants */
__int128 process_globals(void) {
    __int128 sum = 0;
    
    /* Compare each element with neighbors */
    for (int i = 0; i < (int)(sizeof(global_consts)/sizeof(global_consts[0]) - 1); i++) {
        if (global_consts[i] < global_consts[i + 1]) {
            sum += global_consts[i];
        } else {
            sum -= global_consts[i + 1];
        }
    }
    
    return sum;
}

/* Main test driver */
int main(void) {
    /* Test values designed to exercise different comparison paths */
    __int128 test_values[] = {
        HIGH_WORD_DIFF_LOW_EQUAL,
        HIGH_WORD_DIFF_LOW_SMALLER,
        HIGH_WORD_EQUAL_LOW_DIFF1,
        HIGH_WORD_EQUAL_LOW_DIFF2,
        NEGATIVE_LARGE,
        0,
        ((__int128)1 << 127) - 1,  /* Max positive */
        (__int128)(-1) << 127,     /* Min negative */
    };
    
    __int128 total = 0;
    
    /* Execute various test functions */
    for (int i = 0; i < (int)(sizeof(test_values)/sizeof(test_values[0])); i++) {
        for (int j = 0; j < (int)(sizeof(test_values)/sizeof(test_values[0])); j++) {
            total += range_compare(test_values[i], test_values[j]);
            total += bitwise_operations(test_values[i] ^ test_values[j]);
            
            /* Only run loops for small ranges to avoid long execution */
            if (i < 3 && j < 3) {
                total += loop_with_128bit_iv(test_values[i], test_values[j]);
            }
        }
        
        overflow_checks(test_values[i], test_values[(i + 1) % 8]);
        total += dead_code_comparisons(test_values[i]);
    }
    
    total += process_globals();
    
    /* Print a checksum to verify execution */
    /* Split 128-bit result into two 64-bit parts for printing */
    unsigned long long low = (unsigned long long)(total & 0xFFFFFFFFFFFFFFFFULL);
    unsigned long long high = (unsigned long long)((total >> 64) & 0xFFFFFFFFFFFFFFFFULL);
    
    printf("Checksum: 0x%016llx%016llx\n", high, low);
    printf("Test completed successfully.\n");
    
    return 0;
}
