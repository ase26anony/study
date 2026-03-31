/* double-int-test.c - Test program to trigger double_int::cmp coverage */
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large 128-bit constants with varying high/low word patterns */
#define HIGH_A 0x123456789ABCDEF0ULL
#define LOW_A  0xFEDCBA9876543210ULL
#define HIGH_B 0x123456789ABCDEF0ULL  /* Same high as A */
#define LOW_B  0xFEDCBA9876543211ULL  /* Different low */
#define HIGH_C 0x123456789ABCDEEFULL  /* Different high */
#define LOW_C  0xFEDCBA9876543210ULL

/* Construct 128-bit constants */
static const __int128 CONST_A = ((__int128)HIGH_A << 64) | LOW_A;
static const __int128 CONST_B = ((__int128)HIGH_B << 64) | LOW_B;
static const __int128 CONST_C = ((__int128)HIGH_C << 64) | LOW_C;
static const __int128 CONST_MAX = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
static const __int128 CONST_MIN = ((__int128)0x8000000000000000ULL << 64) | 0x0000000000000000ULL;

/* Global arrays with 128-bit constants - forces compile-time evaluation */
static const __int128 global_array[] = {
    CONST_A,
    CONST_B,
    CONST_C,
    CONST_MAX,
    CONST_MIN,
    ((__int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAAULL,
    ((__int128)0xAAAAAAAAAAAAAAAAULL << 64) | 0x5555555555555555ULL,
    0,  /* Zero */
    ~(__int128)0,  /* -1 (all bits set) */
};

/* Test 1: Function with comparisons that differ in low word only */
__int128 test_low_word_comparison(__int128 x) {
    /* These comparisons should trigger cmp with equal high words */
    if (x < CONST_A) {
        return x + 1;
    }
    if (x > CONST_B) {  /* Same high as A, different low */
        return x - 1;
    }
    return x;
}

/* Test 2: Function with comparisons that differ in high word */
__int128 test_high_word_comparison(__int128 x) {
    /* These should trigger cmp with different high words */
    if (x < CONST_C) {  /* Different high than A/B */
        return x << 2;  /* Shift to potentially cross word boundary */
    }
    if (x > CONST_MAX) {
        return CONST_MAX;
    }
    if (x < CONST_MIN) {
        return CONST_MIN;
    }
    return x >> 1;  /* Arithmetic right shift */
}

/* Test 3: Range analysis with 128-bit values */
__int128 test_range_analysis(__int128 a, __int128 b) {
    /* Complex comparisons that VRP should analyze */
    __int128 sum = a + b;
    
    /* Comparisons that span both high and low words */
    if (sum > (((__int128)0x4000000000000000ULL << 64) | 0x0000000000000000ULL)) {
        /* Force evaluation of both high and low parts */
        return sum & (((__int128)0xFFFFFFFFULL << 96) | 0xFFFFFFFFULL);
    }
    
    if (sum < (((__int128)0xC000000000000000ULL << 64) | 0x0000000000000000ULL)) {
        return sum | (((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL);
    }
    
    return sum;
}

/* Test 4: Bitwise operations crossing word boundaries */
__int128 test_bitwise_ops(__int128 x) {
    /* Operations that require reasoning about both words */
    __int128 result = x;
    
    /* Left shift moving bits from low to high word */
    result = result << 65;
    
    /* Mask operations targeting specific words */
    __int128 mask_high = ((__int128)0xFFFF0000FFFF0000ULL << 64) | 0x0000FFFF0000FFFFULL;
    result = result & mask_high;
    
    /* Arithmetic right shift of negative values */
    if (x < 0) {
        result = result >> 72;  /* Crosses word boundary */
    }
    
    return result;
}

/* Test 5: Mixed-type comparisons and conversions */
__int128 test_mixed_comparisons(__int128 x, unsigned long long y) {
    /* Force type promotions and comparisons */
    __int128 y128 = y;  /* Zero-extend */
    
    /* Compare 128-bit with 64-bit (promotion required) */
    if (x < y128) {
        return y128;
    }
    
    /* Ternary with mixed types */
    __int128 z = (x > 1000) ? ((__int128)0x12345678ULL << 64) : y;
    
    /* Compare with constant that has specific high/low pattern */
    if (z == (((__int128)0x00000000FFFFFFFFULL << 64) | 0xFFFFFFFF00000000ULL)) {
        return x ^ y128;
    }
    
    return z;
}

/* Test 6: Loop with 128-bit induction variable */
__int128 test_loop_comparisons(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop with 128-bit bounds - compiler analyzes comparisons */
    for (__int128 i = start; i < end; i += (((__int128)1ULL << 63) | 1ULL)) {
        /* Complex comparison in loop condition */
        if (i > (((__int128)0x1000000000000000ULL << 64) | 0x0000000000000000ULL)) {
            sum += i & 0xFF;
        } else {
            sum += i & 0xF;
        }
        
        /* Early exit based on comparison */
        if (i > (((__int128)0x2000000000000000ULL << 64) | 0x0000000000000000ULL)) {
            break;
        }
    }
    
    return sum;
}

/* Test 7: Overflow operations */
__int128 test_overflow_ops(__int128 a, __int128 b) {
    __int128 result;
    int overflow;
    
    /* Builtin overflow checks may use double_int comparisons */
    overflow = __builtin_add_overflow(a, b, &result);
    if (overflow) {
        return a > 0 ? CONST_MAX : CONST_MIN;
    }
    
    overflow = __builtin_mul_overflow(result, a, &result);
    if (overflow) {
        /* Compare against power of two boundaries */
        __int128 limit = ((__int128)1ULL << 126);
        if (a > limit || a < -limit) {
            return 0;
        }
    }
    
    return result;
}

/* Test 8: Dead code with constant comparisons (still evaluated by compiler) */
void dead_code_comparisons(void) {
    /* These comparisons are in dead code but may be evaluated during compilation */
    if (0) {  /* Always false */
        /* Compare constants with different high words */
        if (CONST_A < CONST_C) {
            volatile int dummy = 1;
            (void)dummy;
        }
        
        /* Compare constants with same high, different low */
        if (CONST_A < CONST_B) {
            volatile int dummy = 2;
            (void)dummy;
        }
        
        /* Complex constant expression */
        __int128 dead_val = ((__int128)0x3333333333333333ULL << 64) | 0xCCCCCCCCCCCCCCCCULL;
        if (dead_val > (((__int128)0x2222222222222222ULL << 64) | 0xDDDDDDDDDDDDDDDDULL)) {
            volatile int dummy = 3;
            (void)dummy;
        }
    }
}

/* Test 9: Static assertions with 128-bit constants */
/* These force compile-time evaluation of comparisons */
#define STATIC_ASSERT(cond) _Static_assert(cond, #cond)

STATIC_ASSERT(CONST_A != CONST_B);  /* Same high, different low */
STATIC_ASSERT(CONST_A != CONST_C);  /* Different high */
STATIC_ASSERT(CONST_MAX > CONST_MIN);
STATIC_ASSERT((((__int128)0x1ULL << 64) | 0x0ULL) > 0xFFFFFFFFFFFFFFFFULL);

/* Test 10: Global initializer with constant expressions */
static const __int128 computed_global = 
    (CONST_A + CONST_B) / 2;  /* Compile-time arithmetic */

/* Main function that exercises all tests */
int main(void) {
    /* Initialize with values that will trigger various comparison paths */
    __int128 test_val1 = CONST_A - 1;  /* Just below A */
    __int128 test_val2 = CONST_B + 1;  /* Just above B */
    __int128 test_val3 = CONST_C;      /* Equal to C */
    __int128 test_val4 = ((__int128)-1 << 64) | 0x8000000000000000ULL; /* Negative */
    
    /* Run tests */
    __int128 result1 = test_low_word_comparison(test_val1);
    __int128 result2 = test_high_word_comparison(test_val2);
    __int128 result3 = test_range_analysis(test_val3, test_val4);
    __int128 result4 = test_bitwise_ops(test_val1);
    __int128 result5 = test_mixed_comparisons(test_val2, 0xFFFFFFFFFFFFFFFFULL);
    __int128 result6 = test_loop_comparisons(CONST_MIN >> 2, CONST_MAX >> 2);
    __int128 result7 = test_overflow_ops(test_val3, test_val4);
    
    /* Call dead code function (no effect at runtime) */
    dead_code_comparisons();
    
    /* Compute checksum from global array */
    __int128 checksum = 0;
    for (size_t i = 0; i < sizeof(global_array)/sizeof(global_array[0]); i++) {
        checksum ^= global_array[i];
    }
    
    /* Add all results to checksum */
    checksum += result1 + result2 + result3 + result4 + result5 + result6 + result7;
    checksum += computed_global;
    
    /* Print lower 64 bits of checksum for verification */
    printf("Checksum (low 64 bits): %llu\n", (unsigned long long)(checksum & 0xFFFFFFFFFFFFFFFFULL));
    
    return 0;
}
