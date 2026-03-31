/* double_int_cmp_test.c - Test program to trigger double_int::cmp in GCC */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large 128-bit constants with varying high/low word patterns */
#define HIGH_WORD_DIFF_LOW_EQUAL \
    (((__int128)0x123456789ABCDEF0ULL << 64) | 0x1111111111111111ULL)
#define HIGH_WORD_DIFF_LOW_SMALLER \
    (((__int128)0x123456789ABCDEF0ULL << 64) | 0x1111111111111110ULL)
#define HIGH_WORD_DIFF_LOW_LARGER \
    (((__int128)0x123456789ABCDEF0ULL << 64) | 0x1111111111111112ULL)
#define HIGH_WORD_EQUAL_LOW_DIFF1 \
    (((__int128)0x123456789ABCDEF0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL)
#define HIGH_WORD_EQUAL_LOW_DIFF2 \
    (((__int128)0x123456789ABCDEF0ULL << 64) | 0xFFFFFFFFFFFFFFFEULL)

/* Negative 128-bit constants */
#define NEG_HIGH_WORD \
    (((__int128)(int64_t)0xFEDCBA9876543210ULL << 64) | 0x0123456789ABCDEFULL)

/* Global arrays with 128-bit constants - forces compile-time evaluation */
static const __int128 global_consts[] = {
    HIGH_WORD_DIFF_LOW_EQUAL,
    HIGH_WORD_DIFF_LOW_SMALLER,
    HIGH_WORD_DIFF_LOW_LARGER,
    HIGH_WORD_EQUAL_LOW_DIFF1,
    HIGH_WORD_EQUAL_LOW_DIFF2,
    NEG_HIGH_WORD,
    0,
    ~((__int128)0)  /* All ones */
};

/* Static assertions to force compile-time comparisons */
_Static_assert(HIGH_WORD_DIFF_LOW_EQUAL > HIGH_WORD_DIFF_LOW_SMALLER, 
               "Compile-time comparison 1");
_Static_assert(HIGH_WORD_EQUAL_LOW_DIFF1 > HIGH_WORD_EQUAL_LOW_DIFF2,
               "Compile-time comparison 2");

/* Function to test range analysis with 128-bit values */
__int128 range_analysis_test(__int128 x, __int128 y) {
    /* Comparisons that should trigger double_int::cmp in VRP */
    if (x > HIGH_WORD_DIFF_LOW_EQUAL) {
        /* High word differs case */
        return x + y;
    } else if (x < HIGH_WORD_EQUAL_LOW_DIFF1 && x > HIGH_WORD_EQUAL_LOW_DIFF2) {
        /* High words equal, low words differ case */
        return x - y;
    } else if ((unsigned __int128)x < (unsigned __int128)y) {
        /* Unsigned comparison requiring high word analysis */
        return x * y;
    }
    return x;
}

/* Function with 128-bit loop induction variable */
__int128 loop_with_128bit_induction(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop with 128-bit bounds - compiler must analyze range */
    for (__int128 i = start; i < end; i += (((__int128)1) << 64)) {
        /* Shift operation that crosses word boundaries */
        sum += i >> 32;
    }
    return sum;
}

/* Function using overflow builtins with 128-bit values */
int overflow_checks(__int128 a, __int128 b) {
    __int128 result;
    int overflow;
    
    /* These may trigger double_int comparisons internally */
    overflow = __builtin_add_overflow(a, b, &result);
    if (overflow) return -1;
    
    overflow = __builtin_mul_overflow(a, b, &result);
    if (overflow) return -2;
    
    return 0;
}

/* Function with bitwise operations crossing word boundaries */
__int128 bitwise_operations(__int128 x) {
    /* Masks targeting specific words */
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    
    /* Operations that require reasoning about both words */
    __int128 high_part = x & mask_high;
    __int128 low_part = x & mask_low;
    
    /* Shifts that move bits between words */
    __int128 shifted = (x << 65) | (x >> 63);
    
    return high_part ^ low_part ^ shifted;
}

/* Function with mixed-type comparisons */
__int128 mixed_type_comparisons(__int128 a, unsigned long long b) {
    /* Implicit conversions and comparisons */
    if (a == (__int128)b) {
        return a;
    }
    
    /* Ternary with mixed types */
    __int128 result = (a > HIGH_WORD_DIFF_LOW_EQUAL) ? 
                      a : (__int128)b;
    
    /* Comparison with sign extension */
    if ((int64_t)a < (int64_t)b) {
        result = -result;
    }
    
    return result;
}

/* Dead code with comparisons that compiler may still evaluate */
void dead_code_paths(void) {
    if (0) {  /* Dead code, but compiler may still analyze constants */
        /* Comparisons covering all uncovered cases */
        if (HIGH_WORD_DIFF_LOW_EQUAL < HIGH_WORD_DIFF_LOW_LARGER) {
            /* High words differ */
            volatile int dummy = 1;
        }
        
        if (HIGH_WORD_EQUAL_LOW_DIFF1 > HIGH_WORD_EQUAL_LOW_DIFF2) {
            /* High words equal, low words differ */
            volatile int dummy = 2;
        }
        
        /* Negative comparisons */
        if (NEG_HIGH_WORD < 0) {
            volatile int dummy = 3;
        }
    }
}

/* Main test function */
int main(void) {
    __int128 sum = 0;
    
    /* Process global constants */
    for (size_t i = 0; i < sizeof(global_consts)/sizeof(global_consts[0]); i++) {
        sum += global_consts[i];
    }
    
    /* Test range analysis */
    __int128 test_val = HIGH_WORD_DIFF_LOW_SMALLER;
    sum += range_analysis_test(test_val, test_val + 1);
    
    /* Test loop with 128-bit induction */
    __int128 loop_start = HIGH_WORD_EQUAL_LOW_DIFF2;
    __int128 loop_end = HIGH_WORD_EQUAL_LOW_DIFF1 + 100;
    sum += loop_with_128bit_induction(loop_start, loop_end);
    
    /* Test overflow checks */
    sum += overflow_checks(HIGH_WORD_DIFF_LOW_EQUAL, 1000);
    
    /* Test bitwise operations */
    sum += bitwise_operations(HIGH_WORD_DIFF_LOW_EQUAL);
    
    /* Test mixed-type comparisons */
    sum += mixed_type_comparisons(HIGH_WORD_DIFF_LOW_SMALLER, 0xFFFFFFFFULL);
    
    /* Call dead code (won't execute but may be analyzed) */
    dead_code_paths();
    
    /* Print a verifiable result */
    printf("Checksum high word: 0x%016llX\n", 
           (unsigned long long)(sum >> 64));
    printf("Checksum low word: 0x%016llX\n", 
           (unsigned long long)sum);
    
    return 0;
}
