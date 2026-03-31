/* double_int_cmp_test.c
 * Designed to trigger GCC's internal double_int::cmp comparisons
 * during constant folding, range analysis, and optimization passes.
 */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large __int128 constants with varying high/low word patterns */
#define HIGH_A  0x123456789ABCDEF0ULL
#define LOW_A   0xFEDCBA9876543210ULL
#define HIGH_B  0x123456789ABCDEF0ULL  /* Same high as A */
#define LOW_B   0xFEDCBA9876543211ULL  /* Different low */
#define HIGH_C  0x123456789ABCDEF1ULL  /* Different high */
#define LOW_C   0xFEDCBA9876543210ULL

/* Full 128-bit constants */
static const __int128 CONST_A = ((__int128)HIGH_A << 64) | LOW_A;
static const __int128 CONST_B = ((__int128)HIGH_B << 64) | LOW_B;
static const __int128 CONST_C = ((__int128)HIGH_C << 64) | LOW_C;
static const __int128 CONST_MAX = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
static const __int128 CONST_MIN = ((__int128)0x8000000000000000ULL << 64) | 0x0000000000000000ULL;
static const __int128 CONST_NEG = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFEULL; /* -2 */

/* Global array with __int128 constants - forces compile-time evaluation */
static const __int128 global_array[] = {
    CONST_A,
    CONST_B,
    CONST_C,
    CONST_MAX,
    CONST_MIN,
    CONST_NEG,
    ((__int128)0x0ULL << 64) | 0x0ULL,
    ((__int128)0x1ULL << 64) | 0x0ULL,
    ((__int128)0x0ULL << 64) | 0x1ULL,
    ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0ULL,
};

/* Static assertions to force compile-time comparisons */
_Static_assert(CONST_A < CONST_C, "CONST_A should be less than CONST_C (different high words)");
_Static_assert(CONST_A < CONST_B, "CONST_A should be less than CONST_B (same high, different low)");
_Static_assert(CONST_MIN < CONST_MAX, "MIN should be less than MAX");
_Static_assert(CONST_NEG < 0, "CONST_NEG should be negative");

/* Dead code with comparisons that compiler may still evaluate */
#ifdef __OPTIMIZE__
static const int dead_code_check = 
    (CONST_A == CONST_B) ? 0 : 1;  /* Compiler may evaluate this */
#endif

/* Function 1: Range analysis with __int128 comparisons */
__int128 range_compare(__int128 x, __int128 y) {
    /* These comparisons should trigger double_int::cmp in VRP */
    if (x < CONST_A) {
        return x + 1;
    }
    if (x > CONST_B && x < CONST_C) {
        return x - 1;
    }
    if (y >= CONST_MIN && y <= CONST_MAX) {
        return y;
    }
    
    /* Cross-word boundary comparison */
    if ((x >> 64) == HIGH_A) {
        /* High words equal, compare low words */
        if ((x & 0xFFFFFFFFFFFFFFFFULL) < LOW_A) {
            return CONST_A;
        }
    }
    
    return x + y;
}

/* Function 2: Loop with __int128 induction variable */
__int128 loop_with_wide_bounds(__int128 start, __int128 end) {
    __int128 sum = 0;
    __int128 i;
    
    /* Loop where bounds differ in both high and low words */
    for (i = start; i < end; i = i + 1) {
        sum = sum + i;
        
        /* Additional comparison inside loop */
        if (i == CONST_A) {
            sum = sum + 100;
        }
    }
    
    return sum;
}

/* Function 3: Bitwise operations crossing word boundaries */
__int128 bitwise_operations(__int128 x) {
    /* Shift operations that move bits between words */
    __int128 shifted = x << 65;  /* Moves bits from low to high word */
    __int128 arith_shifted = x >> 95;  /* Arithmetic shift with sign extension */
    
    /* Bit masks targeting specific words */
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    
    __int128 high_part = x & mask_high;
    __int128 low_part = x & mask_low;
    
    /* Comparisons that may trigger double_int::cmp */
    if (high_part > (CONST_A & mask_high)) {
        return shifted;
    }
    if (low_part < (CONST_B & mask_low)) {
        return arith_shifted;
    }
    
    return high_part | (low_part << 32);
}

/* Function 4: Overflow checking with __int128 */
int overflow_checks(__int128 a, __int128 b, __int128 *result) {
    __int128 sum, product;
    
    /* Builtin overflow checks may use double_int comparisons internally */
    if (__builtin_add_overflow(a, b, &sum)) {
        return -1;
    }
    
    if (__builtin_mul_overflow(a, b, &product)) {
        return -2;
    }
    
    /* Compare results with constants */
    if (sum > CONST_MAX) {
        *result = CONST_MAX;
    } else if (sum < CONST_MIN) {
        *result = CONST_MIN;
    } else {
        *result = sum;
    }
    
    return 0;
}

/* Function 5: Mixed-type comparisons and conversions */
__int128 mixed_type_comparisons(unsigned long long ull_val, __int128 wide_val) {
    /* Implicit conversion and comparison */
    if (wide_val < ull_val) {
        return wide_val + ull_val;
    }
    
    /* Ternary operator with different types */
    __int128 result = (ull_val > 1000) ? CONST_A : wide_val;
    
    /* Compare with different constant types */
    if (result == (__int128)ull_val) {
        return result >> 4;
    }
    
    return result;
}

/* Function 6: Switch statement with __int128 (simulated) */
int switch_like_comparison(__int128 val) {
    /* GCC may convert switch to comparison chain */
    if (val == CONST_A) return 1;
    if (val == CONST_B) return 2;
    if (val == CONST_C) return 3;
    if (val == CONST_MAX) return 4;
    if (val == CONST_MIN) return 5;
    
    /* Range comparisons */
    if (val > CONST_A && val < CONST_C) return 6;
    if (val < 0) return 7;
    
    return 0;
}

/* Main test function that exercises all patterns */
int main(void) {
    __int128 test_val1 = CONST_A;
    __int128 test_val2 = CONST_B;
    __int128 test_val3 = CONST_C;
    
    printf("Testing double_int::cmp triggering patterns...\n");
    
    /* Test 1: Range comparisons */
    __int128 r1 = range_compare(test_val1, test_val2);
    __int128 r2 = range_compare(test_val3, test_val1);
    
    /* Test 2: Loop with wide bounds */
    __int128 loop_sum = loop_with_wide_bounds(
        CONST_A - 10,
        CONST_A + 10
    );
    
    /* Test 3: Bitwise operations */
    __int128 bitwise_result = bitwise_operations(test_val1);
    
    /* Test 4: Overflow checks */
    __int128 overflow_result;
    int overflow_ret = overflow_checks(test_val1, test_val2, &overflow_result);
    
    /* Test 5: Mixed type */
    __int128 mixed_result = mixed_type_comparisons(0xFFFFFFFFFFFFFFFFULL, test_val1);
    
    /* Test 6: Switch-like */
    int switch_result = switch_like_comparison(test_val2);
    
    /* Compute checksum from global array (forces constant processing) */
    __int128 checksum = 0;
    for (size_t i = 0; i < sizeof(global_array)/sizeof(global_array[0]); i++) {
        checksum = checksum + global_array[i];
    }
    
    /* Additional constant expressions that may be evaluated at compile time */
    const __int128 compile_time_sum = CONST_A + CONST_B + CONST_C;
    const int compile_time_cmp = (CONST_A < CONST_B) ? 1 : 0;
    
    /* Dead code that might still be processed */
    if (0) {
        /* These comparisons should never execute, but may be evaluated at compile time */
        volatile __int128 dead1 = (CONST_A == CONST_B) ? 1 : 0;
        volatile __int128 dead2 = (CONST_MAX > CONST_MIN) ? 1 : 0;
        volatile __int128 dead3 = (CONST_NEG < 0) ? 1 : 0;
        
        /* Cross-word comparisons */
        volatile __int128 dead4 = ((CONST_A >> 64) == (CONST_B >> 64)) ? 
                                 ((CONST_A & 0xFFFFFFFFFFFFFFFFULL) < 
                                  (CONST_B & 0xFFFFFFFFFFFFFFFFULL)) ? 1 : 0 : 0;
    }
    
    /* Print simple verification results */
    printf("Checksum: 0x%016llx%016llx\n", 
           (unsigned long long)(checksum >> 64),
           (unsigned long long)(checksum & 0xFFFFFFFFFFFFFFFFULL));
    
    printf("Range compare results: 0x%016llx%016llx, 0x%016llx%016llx\n",
           (unsigned long long)(r1 >> 64), (unsigned long long)(r1 & 0xFFFFFFFFFFFFFFFFULL),
           (unsigned long long)(r2 >> 64), (unsigned long long)(r2 & 0xFFFFFFFFFFFFFFFFULL));
    
    printf("Loop sum: 0x%016llx%016llx\n",
           (unsigned long long)(loop_sum >> 64),
           (unsigned long long)(loop_sum & 0xFFFFFFFFFFFFFFFFULL));
    
    printf("All tests completed.\n");
    
    return 0;
}
