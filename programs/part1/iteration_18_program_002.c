/* double_int_test.c - Test program to trigger GCC's double_int::cmp method */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large 128-bit constants with varying high/low word patterns */
#define HIGH_WORD_DIFFERS_A ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL
#define HIGH_WORD_DIFFERS_B ((__int128)0x123456789ABCDEF1ULL << 64) | 0xFEDCBA9876543210ULL
#define LOW_WORD_DIFFERS_A  ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL
#define LOW_WORD_DIFFERS_B  ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543211ULL
#define MAX_INT128          ((__int128)((~(__int128)0) >> 1))
#define MIN_INT128          ((__int128)1 << 127)
#define HIGH_BIT_SET        ((__int128)0x8000000000000000ULL << 64) | 0x0000000000000000ULL

/* Global arrays with 128-bit constants - forces compile-time evaluation */
static const __int128 global_consts[] = {
    HIGH_WORD_DIFFERS_A,
    HIGH_WORD_DIFFERS_B,
    LOW_WORD_DIFFERS_A,
    LOW_WORD_DIFFERS_B,
    MAX_INT128,
    MIN_INT128,
    HIGH_BIT_SET,
    0,
    -1,
    ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL  /* -1 in two's complement */
};

/* Static assertions forcing compile-time comparisons */
_Static_assert(HIGH_WORD_DIFFERS_A < HIGH_WORD_DIFFERS_B, 
               "High word comparison test 1");
_Static_assert(LOW_WORD_DIFFERS_A < LOW_WORD_DIFFERS_B, 
               "Low word comparison test");
_Static_assert(MAX_INT128 > MIN_INT128, 
               "Signed comparison test");

/* Dead code with comparisons that will be evaluated during early passes */
#ifdef __OPTIMIZE__
static const int dead_code_check = 1;
#else
static const int dead_code_check = 0;
#endif

/* Function 1: Range analysis with wide integer comparisons */
__int128 range_analysis_test(__int128 x, __int128 y) {
    /* Comparisons that should trigger double_int::cmp in VRP */
    if (x < HIGH_WORD_DIFFERS_A) {
        if (y > LOW_WORD_DIFFERS_B) {
            return x + y;
        }
    }
    
    /* Ternary with mixed comparisons */
    __int128 result = (x < y) ? x : y;
    
    /* Compare with constant where high words differ */
    if (result < HIGH_WORD_DIFFERS_B) {
        result += 1;
    }
    
    return result;
}

/* Function 2: Bitwise operations crossing word boundaries */
__int128 bitwise_operations(__int128 a, __int128 b) {
    /* Masks targeting specific words */
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    
    /* Operations that require reasoning about both words */
    __int128 shifted = a << 65;  /* Moves bits from low to high word */
    __int128 masked = b & mask_high;
    
    /* Comparison after shift - high word will be affected */
    if (shifted > masked) {
        return shifted | mask_low;
    }
    
    /* Arithmetic right shift on negative value */
    __int128 neg_val = -HIGH_WORD_DIFFERS_A;
    __int128 arith_shifted = neg_val >> 72;
    
    /* Compare shifted negative value */
    if (arith_shifted < b) {
        return arith_shifted;
    }
    
    return a ^ b;
}

/* Function 3: Overflow checking with wide integers */
__int128 overflow_checks(__int128 x, __int128 y) {
    __int128 sum, product;
    int overflow;
    
    /* Builtin overflow checks - may use double_int internally */
    overflow = __builtin_add_overflow(x, y, &sum);
    if (overflow) {
        /* Compare with boundary values */
        if (x > 0 && y > 0 && sum < x) {
            return MAX_INT128;
        }
        if (x < 0 && y < 0 && sum > x) {
            return MIN_INT128;
        }
    }
    
    overflow = __builtin_mul_overflow(x, y, &product);
    if (overflow) {
        /* Comparisons during overflow analysis */
        __int128 abs_x = (x < 0) ? -x : x;
        __int128 abs_y = (y < 0) ? -y : y;
        
        if (abs_x > MAX_INT128 / abs_y) {
            return (x < 0) ^ (y < 0) ? MIN_INT128 : MAX_INT128;
        }
    }
    
    return overflow ? (x > 0 ? MAX_INT128 : MIN_INT128) : product;
}

/* Function 4: Loop with 128-bit induction variable */
__int128 loop_with_128bit_iv(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop with 128-bit comparison in condition */
    for (__int128 i = start; i < end; i += (end - start) / 100) {
        /* Mix in comparisons with global constants */
        if (i < HIGH_WORD_DIFFERS_A) {
            sum += i & 0xFF;
        } else if (i > LOW_WORD_DIFFERS_B) {
            sum -= i & 0xFF;
        } else {
            sum ^= i & 0xFF;
        }
        
        /* Early exit based on comparison */
        if (sum > ((__int128)0x7FFFFFFFFFFFFFFFULL)) {
            break;
        }
    }
    
    return sum;
}

/* Function 5: Mixed-type comparisons and conversions */
__int128 mixed_type_comparisons(__int128 a, unsigned long long b) {
    /* Implicit conversion and comparison */
    if (a < b) {
        return a + b;
    }
    
    /* Ternary with mixed types */
    __int128 result = (a > 0) ? (__int128)b : a;
    
    /* Compare with constant after conversion */
    __int128 large_const = HIGH_WORD_DIFFERS_A;
    if (result < large_const) {
        result = large_const;
    }
    
    /* Switch-like logic using comparisons */
    if (a < 100) {
        return b;
    } else if (a < 10000) {
        return a;
    } else if (a < HIGH_WORD_DIFFERS_A) {
        return a + b;
    } else {
        return a - b;
    }
}

/* Function to process global array - ensures constants are used */
__int128 process_global_consts(void) {
    __int128 sum = 0;
    const int size = sizeof(global_consts) / sizeof(global_consts[0]);
    
    /* Simple computation using all constants */
    for (int i = 0; i < size; i++) {
        /* Compare each element with different thresholds */
        if (global_consts[i] < HIGH_WORD_DIFFERS_A) {
            sum += global_consts[i] & 0xFFFF;
        } else if (global_consts[i] > LOW_WORD_DIFFERS_B) {
            sum -= global_consts[i] & 0xFFFF;
        } else {
            sum ^= global_consts[i] & 0xFFFF;
        }
    }
    
    return sum;
}

/* Dead code path with comparisons - will be evaluated during early passes */
static void dead_code_path(void) {
    if (dead_code_check) {
        /* These comparisons should still be evaluated by the compiler */
        volatile __int128 cmp1 = HIGH_WORD_DIFFERS_A;
        volatile __int128 cmp2 = HIGH_WORD_DIFFERS_B;
        volatile __int128 cmp3 = LOW_WORD_DIFFERS_A;
        volatile __int128 cmp4 = LOW_WORD_DIFFERS_B;
        
        /* Force comparisons in dead code */
        if (cmp1 < cmp2 && cmp3 < cmp4) {
            /* Never executed, but comparisons are parsed */
            __builtin_unreachable();
        }
    }
}

/* Main test function */
int main(void) {
    /* Test values designed to exercise different comparison paths */
    __int128 test_val1 = HIGH_WORD_DIFFERS_A - 1;
    __int128 test_val2 = LOW_WORD_DIFFERS_B + 1;
    __int128 test_val3 = ((__int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAALL;
    __int128 test_val4 = -test_val3;
    
    /* Call test functions */
    __int128 result1 = range_analysis_test(test_val1, test_val2);
    __int128 result2 = bitwise_operations(test_val3, test_val4);
    __int128 result3 = overflow_checks(test_val1, test_val2);
    __int128 result4 = loop_with_128bit_iv(test_val4, test_val3);
    __int128 result5 = mixed_type_comparisons(test_val1, 0xFFFFFFFFFFFFFFFFULL);
    __int128 result6 = process_global_consts();
    
    /* Call dead code path (does nothing at runtime) */
    dead_code_path();
    
    /* Combine results into a simple checksum */
    __int128 final_result = result1 + result2 + result3 + result4 + result5 + result6;
    
    /* Print lower 64 bits as verification */
    printf("Test checksum (lower 64 bits): %llu\n", 
           (unsigned long long)(final_result & 0xFFFFFFFFFFFFFFFFULL));
    
    /* Additional compile-time checks */
#if defined(__GNUC__) && __GNUC__ >= 5
    /* Use __builtin_constant_p to force more compile-time evaluation */
    if (__builtin_constant_p(HIGH_WORD_DIFFERS_A < HIGH_WORD_DIFFERS_B)) {
        printf("Compile-time comparison verified\n");
    }
#endif
    
    return 0;
}
