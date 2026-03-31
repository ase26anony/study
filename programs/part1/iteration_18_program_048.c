/* double-int-test.c - Test program to trigger double_int::cmp in GCC */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large 128-bit constants with varying high/low word patterns */
#define HIGH_WORD_DIFF_LOW_EQUAL \
    (((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL)
#define HIGH_WORD_DIFF_LOW_SMALLER \
    (((__int128)0x123456789ABCDEF0ULL << 64) | 0x0000000000000000ULL)
#define HIGH_WORD_EQUAL_LOW_DIFF1 \
    (((__int128)0x123456789ABCDEF0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL)
#define HIGH_WORD_EQUAL_LOW_DIFF2 \
    (((__int128)0x123456789ABCDEF0ULL << 64) | 0x7FFFFFFFFFFFFFFFULL)
#define NEGATIVE_LARGE \
    (((__int128)(-1) << 64) | 0x8000000000000000ULL)

/* Global arrays with 128-bit constants - forces compile-time evaluation */
static const __int128 global_consts[] = {
    HIGH_WORD_DIFF_LOW_EQUAL,
    HIGH_WORD_DIFF_LOW_SMALLER,
    HIGH_WORD_EQUAL_LOW_DIFF1,
    HIGH_WORD_EQUAL_LOW_DIFF2,
    NEGATIVE_LARGE,
    0,
    ~((__int128)0),  /* All bits set */
    ((__int128)1 << 127) - 1,  /* Max positive 128-bit */
    (__int128)1 << 127,        /* Most negative 128-bit */
};

/* Static assertions forcing compile-time comparisons */
_Static_assert(HIGH_WORD_DIFF_LOW_EQUAL > HIGH_WORD_DIFF_LOW_SMALLER, 
               "High word comparison should trigger");
_Static_assert(HIGH_WORD_EQUAL_LOW_DIFF1 > HIGH_WORD_EQUAL_LOW_DIFF2,
               "Low word comparison should trigger");
_Static_assert(NEGATIVE_LARGE < 0, "Negative comparison");

/* Function 1: Range analysis with wide integer comparisons */
__int128 range_analysis_test(__int128 x, __int128 y) {
    /* Comparisons that should trigger double_int::cmp in VRP */
    if (x > HIGH_WORD_DIFF_LOW_EQUAL) {
        return x + y;
    }
    if (y < HIGH_WORD_EQUAL_LOW_DIFF2) {
        return x - y;
    }
    
    /* Ternary with mixed types */
    return (x > 0xFFFFFFFFULL) ? (x | y) : (x & y);
}

/* Function 2: Loop with 128-bit induction variable */
__int128 loop_with_128bit_iv(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop that should cause VRP to analyze 128-bit ranges */
    for (__int128 i = start; i < end; i += ((__int128)1 << 60)) {
        /* Cross-word boundary operations */
        sum += i & (((__int128)0xFFFFFFFFULL << 64) | 0xFFFFFFFFULL);
        sum ^= i >> 32;
    }
    
    return sum;
}

/* Function 3: Overflow checking with 128-bit values */
int overflow_checks(__int128 a, __int128 b) {
    __int128 result;
    int overflow;
    
    /* These may trigger double_int comparisons internally */
    overflow = __builtin_add_overflow(a, b, &result);
    if (overflow) return -1;
    
    overflow = __builtin_mul_overflow(a, b, &result);
    if (overflow) return -2;
    
    /* Comparison with narrower type */
    if (a > (unsigned long long)0xFFFFFFFFFFFFFFFFULL) {
        return 1;
    }
    
    return 0;
}

/* Function 4: Bitwise operations crossing word boundaries */
__int128 cross_word_operations(__int128 x) {
    /* Shift operations that move bits between words */
    __int128 shifted = x << 65;  /* Moves bits from low to high word */
    __int128 masked = shifted & (((__int128)0x5555555555555555ULL << 64) | 
                                 0xAAAAAAAAAAAAAAAAULL);
    
    /* Arithmetic right shift on negative values */
    if (x < 0) {
        return masked >> 96;  /* Preserves sign in high word */
    }
    
    return masked;
}

/* Function 5: Dead code with constant comparisons */
/* This code won't execute but may be evaluated at compile time */
__int128 dead_code_comparisons(void) {
    __int128 result = 0;
    
    if (0) {  /* Dead code, but constants may be compared during compilation */
        /* Comparisons designed to hit all branches of double_int::cmp */
        if (HIGH_WORD_DIFF_LOW_EQUAL < HIGH_WORD_DIFF_LOW_SMALLER) result |= 1;
        if (HIGH_WORD_DIFF_LOW_EQUAL > HIGH_WORD_DIFF_LOW_SMALLER) result |= 2;
        if (HIGH_WORD_EQUAL_LOW_DIFF1 < HIGH_WORD_EQUAL_LOW_DIFF2) result |= 4;
        if (HIGH_WORD_EQUAL_LOW_DIFF1 > HIGH_WORD_EQUAL_LOW_DIFF2) result |= 8;
        
        /* Equal comparison */
        if (HIGH_WORD_DIFF_LOW_EQUAL == HIGH_WORD_DIFF_LOW_EQUAL) result |= 16;
    }
    
    return result;
}

/* Function 6: Mixed-type comparisons and conversions */
int mixed_type_comparisons(__int128 a, unsigned long long b) {
    /* Implicit conversions and comparisons */
    if (a == b) return 1;
    if (a < b) return -1;
    if (a > b) return 1;
    
    /* Ternary with different types */
    __int128 c = (b > 1000) ? ((__int128)b << 64) : a;
    
    /* Switch statement (simulated with if-else for 128-bit) */
    if (c == HIGH_WORD_DIFF_LOW_EQUAL) return 100;
    else if (c == HIGH_WORD_EQUAL_LOW_DIFF1) return 101;
    else if (c == NEGATIVE_LARGE) return 102;
    
    return 0;
}

/* Function to compute checksum of global constants */
__int128 compute_global_checksum(void) {
    __int128 sum = 0;
    for (size_t i = 0; i < sizeof(global_consts)/sizeof(global_consts[0]); i++) {
        /* Operations that require full 128-bit arithmetic */
        sum += global_consts[i];
        sum ^= global_consts[i] >> (i % 64);
    }
    return sum;
}

/* Main test function */
int main(void) {
    /* Test values designed to trigger various comparison scenarios */
    __int128 test_val1 = HIGH_WORD_DIFF_LOW_EQUAL;
    __int128 test_val2 = HIGH_WORD_EQUAL_LOW_DIFF2;
    __int128 test_val3 = NEGATIVE_LARGE;
    
    /* Call test functions */
    __int128 result1 = range_analysis_test(test_val1, test_val2);
    __int128 result2 = loop_with_128bit_iv(test_val3, test_val1);
    int result3 = overflow_checks(test_val1, test_val2);
    __int128 result4 = cross_word_operations(test_val1);
    __int128 result5 = dead_code_comparisons();
    int result6 = mixed_type_comparisons(test_val2, 0xFFFFFFFFFFFFFFFFULL);
    __int128 checksum = compute_global_checksum();
    
    /* Use results to prevent optimization removal */
    printf("Results: %lld (high) %lld (low)\n", 
           (long long)(result1 >> 64), (long long)result1);
    printf("Checksum: %lld (high) %lld (low)\n",
           (long long)(checksum >> 64), (long long)checksum);
    
    /* Additional compile-time checks */
    #if __builtin_constant_p(HIGH_WORD_DIFF_LOW_EQUAL > HIGH_WORD_DIFF_LOW_SMALLER)
    printf("Constant comparison evaluated at compile time\n");
    #endif
    
    return 0;
}
