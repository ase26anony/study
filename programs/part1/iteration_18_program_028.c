#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large 128-bit constants with varying high/low word patterns */
#define HIGH_A 0x123456789ABCDEF0ULL
#define LOW_A  0xFEDCBA9876543210ULL
#define HIGH_B 0x123456789ABCDEF0ULL  /* Same high word as A */
#define LOW_B  0xFEDCBA9876543211ULL  /* Different low word */
#define HIGH_C 0x123456789ABCDEF1ULL  /* Different high word */
#define LOW_C  0xFEDCBA9876543210ULL

/* Construct 128-bit constants */
static const __int128 CONST_A = ((__int128)HIGH_A << 64) | LOW_A;
static const __int128 CONST_B = ((__int128)HIGH_B << 64) | LOW_B;
static const __int128 CONST_C = ((__int128)HIGH_C << 64) | LOW_C;
static const __int128 CONST_D = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL; /* -1 */
static const __int128 CONST_E = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL; /* Max positive */

/* Global arrays with 128-bit constants - forces compile-time initialization */
static const __int128 global_array[] = {
    CONST_A,
    CONST_B,
    CONST_C,
    CONST_D,
    CONST_E,
    ((__int128)0x0ULL << 64) | 0x1ULL,  /* Small positive */
    ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0ULL,  /* High word all 1s, low 0 */
};

/* Test 1: Range analysis with comparisons */
__int128 test_range_analysis(__int128 x, __int128 y) {
    /* Comparisons that should trigger double_int::cmp in VRP */
    if (x < CONST_A) {
        if (y > CONST_B) {
            return x + y;
        }
    }
    
    /* Compare where high words are equal but low words differ */
    if (x >= CONST_A && x <= CONST_B) {
        return x | 0x1;
    }
    
    /* Compare where high words differ */
    if (x > CONST_C) {
        return x << 2;
    }
    
    return y;
}

/* Test 2: Loop with 128-bit induction variable */
__int128 test_loop_comparison(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop that compares 128-bit values - may trigger cmp in loop analysis */
    for (__int128 i = start; i < end; i = i + 1) {
        sum += i;
        
        /* Additional comparison inside loop */
        if (i > CONST_A) {
            sum |= 0x1;
        }
    }
    
    return sum;
}

/* Test 3: Bitwise operations crossing word boundaries */
__int128 test_bitwise_ops(__int128 a, __int128 b) {
    __int128 result = 0;
    
    /* Shifts that move bits between high/low words */
    result = a << 65;  /* Moves bits from low to high word */
    result |= b >> 67; /* Right shift with sign extension */
    
    /* Masks targeting specific words */
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    
    /* Comparisons after masking */
    if ((a & mask_high) < (CONST_C & mask_high)) {
        result &= mask_low;
    }
    
    return result;
}

/* Test 4: Overflow operations */
__int128 test_overflow_ops(__int128 a, __int128 b) {
    __int128 sum, product;
    int overflow;
    
    /* Overflow checks that may use double_int comparisons internally */
    overflow = __builtin_add_overflow(a, b, &sum);
    if (overflow) {
        return a;
    }
    
    overflow = __builtin_mul_overflow(a, 2, &product);
    if (!overflow && product > CONST_B) {
        return product;
    }
    
    return sum;
}

/* Test 5: Mixed-type comparisons and conversions */
__int128 test_mixed_comparisons(__int128 a, unsigned long long b) {
    /* Compare 128-bit with 64-bit - requires promotion */
    if (a < b) {
        return b;
    }
    
    /* Ternary with mixed types */
    __int128 result = (a > 1000) ? a : (__int128)b;
    
    /* Compare with negative values */
    if (a > CONST_D) {  /* CONST_D is -1 */
        result |= 0x1;
    }
    
    return result;
}

/* Test 6: Dead code with constant comparisons (still processed by early opts) */
void dead_code_paths(void) {
    if (0) {  /* Dead code, but constants still parsed */
        /* These comparisons should be evaluated during constant folding */
        if (CONST_A < CONST_B) {
            /* High words equal, low words differ */
            volatile int dummy = 1;
            (void)dummy;
        }
        
        if (CONST_B < CONST_C) {
            /* High words differ */
            volatile int dummy = 2;
            (void)dummy;
        }
        
        /* Edge cases */
        if (((__int128)0 < CONST_D)) {  /* 0 < -1 */
            volatile int dummy = 3;
            (void)dummy;
        }
    }
}

/* Test 7: Static assertions forcing compile-time evaluation */
#ifdef __cplusplus
#define STATIC_ASSERT _Static_assert
#else
#define STATIC_ASSERT _Static_assert
#endif

/* These static asserts force compile-time comparison of 128-bit constants */
STATIC_ASSERT(CONST_A != CONST_B, "Constants must differ");
STATIC_ASSERT(CONST_B < CONST_C, "B should be less than C");

/* Preprocessor-like constant evaluation */
#if defined(__GNUC__) && __GNUC__ >= 5
/* Use __builtin_constant_p to force constant evaluation */
#define FORCE_CONST_EVAL(expr) (__builtin_constant_p(expr) ? (expr) : (expr))
#else
#define FORCE_CONST_EVAL(expr) (expr)
#endif

/* Test 8: Array operations with 128-bit values */
__int128 process_global_array(void) {
    __int128 sum = 0;
    size_t i;
    
    for (i = 0; i < sizeof(global_array)/sizeof(global_array[0]); i++) {
        /* Compare array elements with constants */
        if (global_array[i] > CONST_A) {
            sum += global_array[i];
        } else {
            sum -= global_array[i];
        }
    }
    
    return sum;
}

/* Test 9: Switch-like logic with 128-bit comparisons */
__int128 test_switch_like(__int128 val) {
    __int128 result = 0;
    
    /* Chain of comparisons simulating switch on 128-bit */
    if (val == CONST_A) {
        result = 1;
    } else if (val == CONST_B) {
        result = 2;
    } else if (val == CONST_C) {
        result = 3;
    } else if (val < CONST_A) {
        result = 4;
    } else if (val > CONST_C) {
        result = 5;
    } else {
        /* Between CONST_B and CONST_C */
        result = 6;
    }
    
    return result;
}

int main(void) {
    __int128 result = 0;
    
    /* Initialize with values that exercise different comparison paths */
    __int128 test_val1 = CONST_A + 1;
    __int128 test_val2 = CONST_B - 1;
    __int128 test_val3 = CONST_C;
    
    /* Call various test functions */
    result += test_range_analysis(test_val1, test_val2);
    result += test_loop_comparison(CONST_A, CONST_B);
    result += test_bitwise_ops(test_val1, test_val2);
    result += test_overflow_ops(test_val1, test_val2);
    result += test_mixed_comparisons(test_val3, 0xFFFFFFFFULL);
    
    dead_code_paths();
    
    result += process_global_array();
    result += test_switch_like(test_val1);
    
    /* Print a simple checksum (just lower 64 bits for simplicity) */
    unsigned long long checksum = (unsigned long long)result + 
                                  (unsigned long long)(result >> 64);
    
    printf("Checksum: %llu\n", checksum);
    printf("Test completed successfully.\n");
    
    return 0;
}
