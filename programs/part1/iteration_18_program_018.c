/* double_int_cmp_test.c
 * Designed to trigger GCC's internal double_int::cmp comparisons
 * during constant folding, range analysis, and optimization passes.
 */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large 128-bit constants with varying high/low word patterns */
#define HIGH_WORD_DIFFERS_A ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL
#define HIGH_WORD_DIFFERS_B ((__int128)0x123456789ABCDEF1ULL << 64) | 0xFEDCBA9876543210ULL
#define LOW_WORD_DIFFERS_A  ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL
#define LOW_WORD_DIFFERS_B  ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543211ULL
#define NEGATIVE_LARGE      ((__int128)(-1) << 120)  /* High bit set */
#define POSITIVE_LARGE      ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL

/* Global arrays with 128-bit constants - forces compile-time initialization */
static const __int128 global_consts[] = {
    HIGH_WORD_DIFFERS_A,
    HIGH_WORD_DIFFERS_B,
    LOW_WORD_DIFFERS_A,
    LOW_WORD_DIFFERS_B,
    NEGATIVE_LARGE,
    POSITIVE_LARGE,
    ((__int128)0x0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL,  /* Only low word set */
    ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0ULL,  /* Only high word set */
};

/* Static assertions forcing compile-time comparisons */
_Static_assert(HIGH_WORD_DIFFERS_A < HIGH_WORD_DIFFERS_B, 
               "High word comparison should be evaluated at compile time");
_Static_assert(LOW_WORD_DIFFERS_A < LOW_WORD_DIFFERS_B,
               "Low word comparison should be evaluated at compile time");

/* Dead code with comparisons that GCC may evaluate during early passes */
#ifdef __OPTIMIZE__
static const int dead_code_check = 
    (HIGH_WORD_DIFFERS_A == HIGH_WORD_DIFFERS_B) ? 0 : 1;
#endif

/* Function 1: Range analysis with high word differences */
__int128 range_analysis_high_diff(__int128 x, __int128 y) {
    /* Comparisons that may trigger double_int::cmp in VRP */
    if (x < HIGH_WORD_DIFFERS_A) {
        if (y > HIGH_WORD_DIFFERS_B) {
            return x + y;
        }
    }
    
    /* Ternary with mixed-type comparison */
    return (x < ((__int128)0x10000000000000000ULL)) ? x : y;
}

/* Function 2: Range analysis with low word differences */
__int128 range_analysis_low_diff(__int128 x, __int128 y) {
    __int128 result = 0;
    
    /* Chain of comparisons where high words equal but low words differ */
    if (x >= LOW_WORD_DIFFERS_A && x <= LOW_WORD_DIFFERS_B) {
        result = x;
    } else if (y >= LOW_WORD_DIFFERS_A && y <= LOW_WORD_DIFFERS_B) {
        result = y;
    } else {
        result = LOW_WORD_DIFFERS_A;
    }
    
    return result;
}

/* Function 3: Bitwise operations crossing word boundaries */
__int128 cross_word_operations(__int128 a, __int128 b) {
    /* Masks targeting specific words */
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    __int128 mask_low  = 0xFFFFFFFFFFFFFFFFULL;
    
    /* Operations that require reasoning about both words */
    __int128 shifted = a << 65;  /* Moves bits from low to high word */
    __int128 masked = b & mask_high;
    
    /* Comparison after shift */
    if (shifted > masked) {
        return shifted | (b & mask_low);
    }
    
    return masked | (a & mask_low);
}

/* Function 4: Arithmetic with overflow checking */
__int128 arithmetic_with_overflow(__int128 a, __int128 b) {
    __int128 sum, product;
    int overflow;
    
    /* Overflow checks may use double_int comparisons internally */
    overflow = __builtin_add_overflow(a, b, &sum);
    if (overflow) {
        /* Handle overflow - compare against limits */
        if (a > 0 && b > 0 && sum < 0) {
            return POSITIVE_LARGE;
        }
        if (a < 0 && b < 0 && sum > 0) {
            return NEGATIVE_LARGE;
        }
    }
    
    overflow = __builtin_mul_overflow(a, b, &product);
    if (overflow) {
        /* Compare product against a for range analysis */
        return (product > a) ? product : a;
    }
    
    return sum + product;
}

/* Function 5: Loop with 128-bit induction variable */
__int128 loop_with_128bit_iv(__int128 start, __int128 end) {
    __int128 sum = 0;
    __int128 i;
    
    /* Loop where bounds differ in both high and low words */
    for (i = start; i < end; i = i + 1) {
        /* Conditional that depends on both words */
        if ((i & ((__int128)0x8000000000000000ULL << 64)) == 0) {
            /* Positive values only */
            sum = sum + (i & 0xFF);
        }
        
        /* Early exit based on comparison */
        if (i > (end - 100)) {
            break;
        }
    }
    
    return sum;
}

/* Function 6: Mixed-type comparisons and conversions */
__int128 mixed_type_comparisons(__int128 a, unsigned long long b) {
    /* Implicit conversion and comparison */
    if (a == (__int128)b) {
        return a;
    }
    
    /* Ternary with different types */
    __int128 result = (a < 100) ? (__int128)b : a;
    
    /* Compare with 64-bit constant promoted to 128-bit */
    if (result > 0xFFFFFFFFFFFFFFFFULL) {
        return result >> 1;
    }
    
    return result;
}

/* Function 7: Switch statement with large cases (simulated with if-else) */
int switch_like_comparisons(__int128 value) {
    /* Simulate switch with large constants */
    if (value == HIGH_WORD_DIFFERS_A) return 1;
    if (value == HIGH_WORD_DIFFERS_B) return 2;
    if (value == LOW_WORD_DIFFERS_A) return 3;
    if (value == LOW_WORD_DIFFERS_B) return 4;
    if (value == NEGATIVE_LARGE) return 5;
    if (value == POSITIVE_LARGE) return 6;
    
    /* Range check */
    if (value >= 0 && value <= 100) return 7;
    
    return 0;
}

/* Function to compute checksum of global constants */
__int128 compute_global_checksum(void) {
    __int128 checksum = 0;
    size_t i;
    
    for (i = 0; i < sizeof(global_consts) / sizeof(global_consts[0]); i++) {
        /* Force compiler to process each constant */
        checksum = checksum ^ global_consts[i];
        
        /* Add conditional that uses comparison */
        if (global_consts[i] > 0) {
            checksum = checksum + 1;
        } else {
            checksum = checksum - 1;
        }
    }
    
    return checksum;
}

/* Main test driver */
int main(void) {
    __int128 test_a = HIGH_WORD_DIFFERS_A;
    __int128 test_b = LOW_WORD_DIFFERS_B;
    __int128 test_c = NEGATIVE_LARGE;
    unsigned long long test_ull = 0xFFFFFFFFFFFFFFFFULL;
    
    printf("Testing 128-bit comparison paths in GCC...\n");
    
    /* Test 1: Range analysis with high word differences */
    __int128 result1 = range_analysis_high_diff(test_a, test_b);
    printf("Range analysis (high diff) result: 0x%016llx%016llx\n",
           (unsigned long long)(result1 >> 64),
           (unsigned long long)result1);
    
    /* Test 2: Range analysis with low word differences */
    __int128 result2 = range_analysis_low_diff(test_a, test_b);
    printf("Range analysis (low diff) result: 0x%016llx%016llx\n",
           (unsigned long long)(result2 >> 64),
           (unsigned long long)result2);
    
    /* Test 3: Cross-word operations */
    __int128 result3 = cross_word_operations(test_a, test_b);
    printf("Cross-word operations result: 0x%016llx%016llx\n",
           (unsigned long long)(result3 >> 64),
           (unsigned long long)result3);
    
    /* Test 4: Arithmetic with overflow */
    __int128 result4 = arithmetic_with_overflow(test_a, test_c);
    printf("Arithmetic with overflow result: 0x%016llx%016llx\n",
           (unsigned long long)(result4 >> 64),
           (unsigned long long)result4);
    
    /* Test 5: Loop with 128-bit induction variable */
    __int128 loop_start = HIGH_WORD_DIFFERS_A;
    __int128 loop_end = HIGH_WORD_DIFFERS_A + 1000;
    __int128 result5 = loop_with_128bit_iv(loop_start, loop_end);
    printf("Loop with 128-bit IV result: 0x%016llx%016llx\n",
           (unsigned long long)(result5 >> 64),
           (unsigned long long)result5);
    
    /* Test 6: Mixed-type comparisons */
    __int128 result6 = mixed_type_comparisons(test_b, test_ull);
    printf("Mixed-type comparisons result: 0x%016llx%016llx\n",
           (unsigned long long)(result6 >> 64),
           (unsigned long long)result6);
    
    /* Test 7: Switch-like comparisons */
    int result7 = switch_like_comparisons(test_a);
    printf("Switch-like comparisons result: %d\n", result7);
    
    /* Final checksum of global constants */
    __int128 final_checksum = compute_global_checksum();
    printf("Global constants checksum: 0x%016llx%016llx\n",
           (unsigned long long)(final_checksum >> 64),
           (unsigned long long)final_checksum);
    
    /* Verify with a simple assertion */
    assert(result1 != 0 || result2 != 0 || result3 != 0);
    
    printf("All tests completed (compilation should have triggered double_int::cmp)\n");
    
    return 0;
}
