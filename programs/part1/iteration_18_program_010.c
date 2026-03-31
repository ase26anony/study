/* double_int_cmp_test.c - Test program to trigger double_int::cmp in GCC */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large 128-bit constants with varying high/low word patterns */
#define HIGH_WORD_DIFFERS_A ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL
#define HIGH_WORD_DIFFERS_B ((__int128)0x123456789ABCDEF1ULL << 64) | 0xFEDCBA9876543210ULL
#define LOW_WORD_DIFFERS_A  ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL
#define LOW_WORD_DIFFERS_B  ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543211ULL
#define EQUAL_VALUES        ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL
#define NEGATIVE_VALUE      ((__int128)(-1) << 64) | 0xFFFFFFFFFFFFFFFFULL

/* Global arrays with __int128 constants - forces compile-time evaluation */
static const __int128 global_consts[] = {
    HIGH_WORD_DIFFERS_A,
    HIGH_WORD_DIFFERS_B,
    LOW_WORD_DIFFERS_A,
    LOW_WORD_DIFFERS_B,
    EQUAL_VALUES,
    NEGATIVE_VALUE,
    ((__int128)0x0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL,  /* Only low word set */
    ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0ULL,  /* Only high word set */
};

/* Static assertions to force compile-time comparisons */
_Static_assert(HIGH_WORD_DIFFERS_A < HIGH_WORD_DIFFERS_B, 
               "High word comparison should pass");
_Static_assert(LOW_WORD_DIFFERS_A < LOW_WORD_DIFFERS_B,
               "Low word comparison should pass");
_Static_assert(EQUAL_VALUES == EQUAL_VALUES,
               "Equality comparison should pass");

/* Function 1: Range analysis with wide integer comparisons */
__int128 range_analysis_test(__int128 x, __int128 y) {
    /* Comparisons that should trigger double_int::cmp in VRP */
    if (x < HIGH_WORD_DIFFERS_A) {
        return x + 1;
    }
    if (y > LOW_WORD_DIFFERS_B) {
        return y - 1;
    }
    
    /* Ternary with mixed comparisons */
    __int128 result = (x < y) ? x : y;
    
    /* Compare high words only through shifting */
    if ((x >> 64) < (y >> 64)) {
        result = result | 0x1;
    }
    
    return result;
}

/* Function 2: Arithmetic operations crossing word boundaries */
__int128 boundary_crossing_ops(__int128 a) {
    /* Left shift moving bits from low to high word */
    __int128 shifted = a << 72;  /* >64 bits */
    
    /* Right shift on negative value (arithmetic shift) */
    __int128 neg_shifted = NEGATIVE_VALUE >> 68;
    
    /* Bitwise operations targeting specific words */
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    
    __int128 high_only = a & mask_high;
    __int128 low_only = a & mask_low;
    
    /* Comparisons after bitwise ops */
    if (high_only < mask_high) {
        shifted = shifted | 0x1;
    }
    
    return shifted + neg_shifted + (high_only >> 64) + low_only;
}

/* Function 3: Mixed-type comparisons and conversions */
__int128 mixed_type_comparisons(__int128 big, unsigned long long small) {
    /* Implicit conversion and comparison */
    if (big < small) {
        return small;
    }
    
    /* Explicit conversion in ternary */
    __int128 result = (big > 1000) ? (__int128)small << 64 : big;
    
    /* Compare with 64-bit boundary */
    if (big < ((__int128)1 << 63)) {
        result = result & 0xFFFFFFFFULL;  /* Narrow result */
    }
    
    return result;
}

/* Function 4: Loop with __int128 induction variable */
__int128 loop_with_wide_bounds(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop designed to be analyzed by VRP */
    for (__int128 i = start; i < end; i = i + 1) {
        /* Use overflow builtins - may trigger double_int comparisons */
        __int128 add_result;
        if (__builtin_add_overflow(sum, i, &add_result)) {
            sum = sum | 0x1;  /* Mark overflow */
        } else {
            sum = add_result;
        }
        
        /* Compare against constant mid-point */
        if (i < ((start + end) / 2)) {
            sum = sum ^ 0x1;  /* Flip LSB */
        }
    }
    
    return sum;
}

/* Function 5: Dead code with constant comparisons (still processed by compiler) */
__int128 dead_code_comparisons(__int128 x) {
    __int128 result = x;
    
    /* Dead code block - compiler may still evaluate constants */
    if (0) {  /* Always false */
        /* Multiple comparisons between large constants */
        if (HIGH_WORD_DIFFERS_A < HIGH_WORD_DIFFERS_B) {
            result = result + 1;
        }
        if (LOW_WORD_DIFFERS_A < LOW_WORD_DIFFERS_B) {
            result = result + 2;
        }
        if (EQUAL_VALUES == EQUAL_VALUES) {
            result = result + 4;
        }
        
        /* Complex expression with shifts and comparisons */
        __int128 temp = HIGH_WORD_DIFFERS_A << 5;
        if (temp > HIGH_WORD_DIFFERS_B) {
            result = result | 0x8;
        }
    }
    
    return result;
}

/* Function 6: Overflow checking with wide integers */
__int128 overflow_checks(__int128 a, __int128 b) {
    __int128 sum, product;
    int overflow;
    
    /* Add overflow check */
    overflow = __builtin_add_overflow(a, b, &sum);
    if (overflow) {
        sum = a | b;  /* Fallback */
    }
    
    /* Mul overflow check - may trigger double_int comparisons */
    overflow = __builtin_mul_overflow(a, b, &product);
    if (overflow) {
        product = a ^ b;  /* Fallback */
    }
    
    /* Compare results */
    if (sum < product) {
        return sum;
    } else {
        return product;
    }
}

/* Main test driver */
int main() {
    __int128 test_results = 0;
    
    /* Test 1: Range analysis */
    __int128 val1 = HIGH_WORD_DIFFERS_A;
    __int128 val2 = LOW_WORD_DIFFERS_B;
    test_results = test_results + range_analysis_test(val1, val2);
    
    /* Test 2: Boundary crossing operations */
    test_results = test_results + boundary_crossing_ops(val1);
    
    /* Test 3: Mixed type comparisons */
    test_results = test_results + mixed_type_comparisons(val1, 0x123456789ABCDEFULL);
    
    /* Test 4: Loop with wide bounds (small range for runtime) */
    test_results = test_results + loop_with_wide_bounds(1000, 2000);
    
    /* Test 5: Dead code comparisons */
    test_results = test_results + dead_code_comparisons(val2);
    
    /* Test 6: Overflow checks */
    test_results = test_results + overflow_checks(100, 200);
    
    /* Also process global constants to ensure they're used */
    __int128 const_sum = 0;
    for (int i = 0; i < sizeof(global_consts)/sizeof(global_consts[0]); i++) {
        const_sum = const_sum + global_consts[i];
    }
    test_results = test_results + const_sum;
    
    /* Print a simple checksum (lower 64 bits) */
    unsigned long long checksum = (unsigned long long)test_results;
    printf("Test checksum: %llu\n", checksum);
    
    /* Additional static assert to force more compile-time comparisons */
    #ifdef __OPTIMIZE__
    _Static_assert(sizeof(__int128) == 16, "__int128 must be 16 bytes");
    #endif
    
    return 0;
}
