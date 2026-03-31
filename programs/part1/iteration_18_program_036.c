/* double-int-test.c - Designed to trigger GCC's double_int::cmp comparisons */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large 128-bit constants with varying high/low word patterns */
#define HIGH_DIFF_LOW_EQUAL_A ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL
#define HIGH_DIFF_LOW_EQUAL_B ((__int128)0x223456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL

#define HIGH_EQUAL_LOW_DIFF_A ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0000000000000000ULL
#define HIGH_EQUAL_LOW_DIFF_B ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0000000000000001ULL

#define NEGATIVE_LARGE_A ((__int128)(-1) * (((__int128)0x8000000000000000ULL << 64) | 0x0000000000000000ULL))
#define NEGATIVE_LARGE_B ((__int128)(-1) * (((__int128)0x8000000000000000ULL << 64) | 0x0000000000000001ULL))

/* Global arrays with 128-bit constants - forces compile-time evaluation */
static const __int128 global_consts[] = {
    HIGH_DIFF_LOW_EQUAL_A,
    HIGH_DIFF_LOW_EQUAL_B,
    HIGH_EQUAL_LOW_DIFF_A,
    HIGH_EQUAL_LOW_DIFF_B,
    NEGATIVE_LARGE_A,
    NEGATIVE_LARGE_B,
    ((__int128)0x0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL,  /* Only low word set */
    ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0ULL,  /* Only high word set */
};

/* Static assertions forcing compile-time comparisons */
_Static_assert(HIGH_DIFF_LOW_EQUAL_A < HIGH_DIFF_LOW_EQUAL_B, 
               "High word difference comparison should hold");
_Static_assert(HIGH_EQUAL_LOW_DIFF_A < HIGH_EQUAL_LOW_DIFF_B,
               "Low word difference comparison should hold");
_Static_assert(NEGATIVE_LARGE_A < NEGATIVE_LARGE_B,
               "Negative value comparison should hold");

/* Dead code with comparisons that GCC may evaluate during optimization */
static __int128 dead_code_comparisons(__int128 x) {
    __int128 result = 0;
    
    /* These comparisons are dead but may be evaluated during constant folding */
    if (0) {  /* Always false, but compiler may still analyze the expressions */
        if (HIGH_DIFF_LOW_EQUAL_A < HIGH_DIFF_LOW_EQUAL_B) {
            result += 1;
        }
        if (HIGH_EQUAL_LOW_DIFF_A < HIGH_EQUAL_LOW_DIFF_B) {
            result += 2;
        }
        if (NEGATIVE_LARGE_A < NEGATIVE_LARGE_B) {
            result += 4;
        }
        
        /* Cross-type comparisons */
        unsigned long long ull = 0xFFFFFFFFFFFFFFFFULL;
        if (HIGH_EQUAL_LOW_DIFF_A > (__int128)ull) {
            result += 8;
        }
    }
    
    return result;
}

/* Function using __int128 comparisons for range analysis */
__int128 range_analysis_function(__int128 a, __int128 b) {
    __int128 result = 0;
    
    /* Comparisons that should trigger double_int::cmp */
    if (a < HIGH_DIFF_LOW_EQUAL_A) {
        result = a + 1;
    } else if (a > HIGH_DIFF_LOW_EQUAL_B) {
        result = a - 1;
    } else {
        result = a;
    }
    
    /* Nested comparisons */
    if (b < HIGH_EQUAL_LOW_DIFF_A) {
        result += b;
    } else if (b > HIGH_EQUAL_LOW_DIFF_B) {
        result -= b;
    }
    
    return result;
}

/* Function with bitwise operations crossing word boundaries */
__int128 bitwise_operations(__int128 x) {
    /* Masks targeting specific words */
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    
    /* Operations that require reasoning about both words */
    __int128 shifted = x << 65;  /* Shift into high word */
    __int128 masked = (x & mask_high) | (~x & mask_low);
    __int128 xor_result = x ^ (((__int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAAULL);
    
    /* Comparisons after bitwise ops */
    if (shifted > mask_high) {
        return masked;
    } else if (xor_result < x) {
        return xor_result;
    }
    
    return shifted;
}

/* Function using overflow builtins with __int128 */
__int128 overflow_operations(__int128 a, __int128 b) {
    __int128 sum, product;
    int overflow;
    
    /* Addition with overflow check */
    overflow = __builtin_add_overflow(a, b, &sum);
    if (overflow) {
        /* Compare against extremes */
        if (a > 0 && b > 0 && sum < a) {
            return HIGH_DIFF_LOW_EQUAL_A;
        } else if (a < 0 && b < 0 && sum > a) {
            return NEGATIVE_LARGE_A;
        }
    }
    
    /* Multiplication with overflow check */
    overflow = __builtin_mul_overflow(a, b, &product);
    if (overflow) {
        /* Comparisons in overflow handling */
        if ((a > 0 && b > 0 && product < a) || 
            (a < 0 && b < 0 && product > a)) {
            return HIGH_EQUAL_LOW_DIFF_B;
        }
    }
    
    return sum + product;
}

/* Loop with __int128 induction variable */
__int128 loop_with_wide_induction(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop bound comparisons should trigger double_int::cmp */
    for (__int128 i = start; i < end; i += ((__int128)1 << 32)) {
        /* Mix in some comparisons inside the loop */
        if (i < HIGH_DIFF_LOW_EQUAL_A) {
            sum += 1;
        } else if (i > HIGH_DIFF_LOW_EQUAL_B) {
            sum -= 1;
        } else {
            sum += i & 0xFF;
        }
        
        /* Additional comparison for VRP */
        if (i > 0 && i < ((__int128)0x100000000ULL << 32)) {
            sum += 2;
        }
    }
    
    return sum;
}

/* Function with mixed-type comparisons and conversions */
__int128 mixed_type_comparisons(__int128 a, unsigned long long b) {
    __int128 result = 0;
    
    /* Implicit conversion and comparison */
    if (a < (__int128)b) {
        result = b;
    } else if (a > (__int128)(b * 2ULL)) {
        result = a - b;
    }
    
    /* Ternary with mixed types */
    result = (a < 100) ? (__int128)b : a;
    
    /* Compare with different constant types */
    if (a < 0x7FFFFFFFFFFFFFFFLL) {  /* 64-bit signed max */
        result += 1;
    }
    if (a > 0xFFFFFFFFFFFFFFFFULL) {  /* 64-bit unsigned max */
        result += 2;
    }
    
    return result;
}

/* Main test function that exercises all patterns */
int main(void) {
    __int128 checksum = 0;
    
    /* Test 1: Range analysis with comparisons */
    checksum += range_analysis_function(
        HIGH_DIFF_LOW_EQUAL_A - 1,
        HIGH_EQUAL_LOW_DIFF_B + 1
    );
    
    /* Test 2: Bitwise operations */
    checksum += bitwise_operations(
        ((__int128)0x1234567812345678ULL << 64) | 0x8765432187654321ULL
    );
    
    /* Test 3: Overflow operations */
    checksum += overflow_operations(
        ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL,  /* Large positive */
        0x1000000000000000ULL
    );
    
    /* Test 4: Loop with wide bounds */
    checksum += loop_with_wide_induction(
        0,
        ((__int128)0x10ULL << 64)  /* Moderate upper bound */
    );
    
    /* Test 5: Mixed type comparisons */
    checksum += mixed_type_comparisons(
        HIGH_EQUAL_LOW_DIFF_A,
        0xFFFFFFFFFFFFFFFFULL
    );
    
    /* Process global constants array */
    for (size_t i = 0; i < sizeof(global_consts)/sizeof(global_consts[0]); i++) {
        checksum += global_consts[i] & 0xFF;  /* Use just low byte for simplicity */
    }
    
    /* Call dead code function (should be optimized away but may trigger comparisons) */
    checksum += dead_code_comparisons(checksum);
    
    /* Print result (simplified for 128-bit output) */
    unsigned long long low = (unsigned long long)checksum;
    unsigned long long high = (unsigned long long)(checksum >> 64);
    printf("Checksum: 0x%016llx%016llx\n", high, low);
    
    return 0;
}
