/* double-int-test.c - Test program to trigger double_int::cmp coverage */
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large 128-bit constants with varying high/low word patterns */
#define HIGH_WORD_DIFFS     ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL
#define HIGH_WORD_SAME_HIGH ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL
#define HIGH_WORD_SAME_LOW  ((__int128)0x123456789ABCDEF0ULL << 64) | 0x0000000000000000ULL
#define LOW_WORD_DIFFS      ((__int128)0x0000000000000000ULL << 64) | 0xFFFFFFFF00000000ULL
#define NEGATIVE_VALUE      ((__int128)(-1) << 64) | 0xFFFFFFFFFFFFFFFFULL

/* Global arrays with 128-bit constants - forces compile-time evaluation */
static const __int128 global_consts[] = {
    HIGH_WORD_DIFFS,
    HIGH_WORD_SAME_HIGH,
    HIGH_WORD_SAME_LOW,
    LOW_WORD_DIFFS,
    NEGATIVE_VALUE,
    ((__int128)0xAAAAAAAAAAAAAAAAULL << 64) | 0x5555555555555555ULL,
    ((__int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAAULL,
    ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0000000000000000ULL,
    ((__int128)0x0000000000000000ULL << 64) | 0xFFFFFFFFFFFFFFFFULL,
};

/* Static assertions to force compile-time comparisons */
_Static_assert(HIGH_WORD_DIFFS > HIGH_WORD_SAME_LOW, 
               "High word comparison should be true");
_Static_assert(HIGH_WORD_SAME_HIGH > HIGH_WORD_SAME_LOW, 
               "Low word comparison when high words equal");
_Static_assert(NEGATIVE_VALUE < 0, 
               "Negative value comparison");

/* Test 1: Function with __int128 comparisons for VRP */
__int128 test_range_analysis(__int128 x, __int128 y) {
    /* Comparisons that may trigger double_int::cmp in VRP */
    if (x > HIGH_WORD_DIFFS) {
        return x + y;
    }
    if (y < NEGATIVE_VALUE) {
        return x - y;
    }
    
    /* Ternary with mixed types */
    __int128 result = (x > 0) ? x : (__int128)((unsigned long long)y);
    
    /* Compare with constant where high words differ */
    if (result == HIGH_WORD_SAME_HIGH) {
        return result >> 1;
    }
    
    return result;
}

/* Test 2: Loop with __int128 induction variable */
__int128 test_loop_comparisons(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop with bounds that may differ in high/low words */
    for (__int128 i = start; i < end; i = i + 1) {
        /* Force comparison in loop condition */
        if (i > (HIGH_WORD_DIFFS >> 2)) {
            sum += i;
        } else {
            sum -= i;
        }
        
        /* Early exit based on comparison */
        if (i == (end - 100)) {
            break;
        }
    }
    
    return sum;
}

/* Test 3: Bitwise operations crossing word boundaries */
__int128 test_bitwise_ops(__int128 a, __int128 b) {
    /* Masks targeting specific words */
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    
    /* Operations that require reasoning about both words */
    __int128 shifted = a << 65;  /* Moves bits from low to high word */
    __int128 masked = b & mask_high;
    
    /* Comparison after shift - high word affected */
    if (shifted > masked) {
        return shifted | mask_low;
    }
    
    /* Arithmetic right shift on negative value */
    __int128 neg_shift = NEGATIVE_VALUE >> 3;
    
    return (a ^ b) + neg_shift;
}

/* Test 4: Overflow operations */
int test_overflow_ops(__int128 a, __int128 b, __int128 *result) {
    __int128 sum, product;
    int overflow_sum, overflow_mul;
    
    /* Builtins that may use double_int comparisons internally */
    overflow_sum = __builtin_add_overflow(a, b, &sum);
    overflow_mul = __builtin_mul_overflow(a, b, &product);
    
    /* Comparisons with results */
    if (sum > product) {
        *result = sum;
    } else {
        *result = product;
    }
    
    return overflow_sum || overflow_mul;
}

/* Test 5: Mixed-type comparisons and conversions */
__int128 test_mixed_comparisons(__int128 wide_val, unsigned long long narrow_val) {
    /* Implicit conversion and comparison */
    if (wide_val > narrow_val) {
        /* Shift that crosses word boundary */
        return wide_val << 72;
    }
    
    /* Switch-like logic with comparisons */
    __int128 ret = 0;
    if (wide_val == HIGH_WORD_DIFFS) {
        ret = 1;
    } else if (wide_val == HIGH_WORD_SAME_HIGH) {
        ret = 2;
    } else if (wide_val == HIGH_WORD_SAME_LOW) {
        ret = 3;
    } else if (wide_val < 0) {
        ret = 4;
    }
    
    /* Compare with converted narrow value */
    return ret + (__int128)narrow_val;
}

/* Test 6: Dead code with constant comparisons (still processed by compiler) */
void dead_code_paths(void) {
    if (0) {  /* Dead code, but constants still processed */
        /* These comparisons should still be evaluated during compilation */
        if (HIGH_WORD_DIFFS < HIGH_WORD_SAME_HIGH) {
            /* High words equal, compare low words */
            volatile int dummy = 1;
        }
        
        if (LOW_WORD_DIFFS > 0) {
            volatile int dummy = 2;
        }
        
        /* Preprocessor condition with constant check */
        #if __builtin_constant_p(HIGH_WORD_DIFFS)
            volatile int dummy = 3;
        #endif
    }
}

/* Test 7: Array operations with global constants */
__int128 process_global_constants(void) {
    __int128 sum = 0;
    
    /* Process array - forces compiler to handle constants */
    for (int i = 0; i < sizeof(global_consts)/sizeof(global_consts[0]); i++) {
        /* Compare array elements with each other */
        if (i > 0 && global_consts[i] > global_consts[i-1]) {
            sum += global_consts[i];
        } else {
            sum -= global_consts[i];
        }
    }
    
    return sum;
}

/* Main function that exercises all tests */
int main(void) {
    __int128 result = 0;
    __int128 temp;
    
    /* Initialize with values that exercise different comparison paths */
    __int128 test_val1 = HIGH_WORD_DIFFS;
    __int128 test_val2 = HIGH_WORD_SAME_LOW;
    __int128 test_val3 = NEGATIVE_VALUE;
    
    printf("Testing __int128 operations to trigger double_int::cmp coverage\n");
    
    /* Test 1: Range analysis */
    result += test_range_analysis(test_val1, test_val2);
    
    /* Test 2: Loop comparisons */
    result += test_loop_comparisons(test_val3, test_val1);
    
    /* Test 3: Bitwise operations */
    result ^= test_bitwise_ops(test_val1, test_val2);
    
    /* Test 4: Overflow operations */
    test_overflow_ops(test_val1, test_val2, &temp);
    result += temp;
    
    /* Test 5: Mixed comparisons */
    result += test_mixed_comparisons(test_val1, 0xFFFFFFFFULL);
    
    /* Test 6: Dead code (no effect on runtime) */
    dead_code_paths();
    
    /* Test 7: Global constants */
    result += process_global_constants();
    
    /* Print a simple checksum */
    unsigned long long low = (unsigned long long)result;
    unsigned long long high = (unsigned long long)(result >> 64);
    
    printf("Result checksum: high=0x%016llx low=0x%016llx\n", high, low);
    printf("Sum of global constants array: ");
    
    __int128 array_sum = 0;
    for (int i = 0; i < sizeof(global_consts)/sizeof(global_consts[0]); i++) {
        array_sum += global_consts[i];
    }
    
    low = (unsigned long long)array_sum;
    high = (unsigned long long)(array_sum >> 64);
    printf("high=0x%016llx low=0x%016llx\n", high, low);
    
    return 0;
}
