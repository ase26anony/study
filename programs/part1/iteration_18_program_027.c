#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large 128-bit constants with varying high/low word patterns */
#define HIGH_A  0x123456789ABCDEF0ULL
#define LOW_A   0xFEDCBA9876543210ULL
#define HIGH_B  0x123456789ABCDEF0ULL  /* Same high as A */
#define LOW_B   0xFEDCBA9876543211ULL  /* Different low */
#define HIGH_C  0x123456789ABCDEEFULL  /* Different high */
#define LOW_C   0xFEDCBA9876543210ULL

/* Full 128-bit constants */
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
    ((__int128)0x8000000000000000ULL << 64) | 0x0ULL,  /* Min negative */
};

/* Test 1: Static assertions with 128-bit comparisons */
/* These force compile-time evaluation of cmp operations */
#ifdef __cplusplus
#define STATIC_ASSERT _Static_assert
#else
#define STATIC_ASSERT _Static_assert
#endif

/* Compare high words equal, low words different */
STATIC_ASSERT(CONST_A < CONST_B, "A should be less than B");
STATIC_ASSERT(CONST_B > CONST_A, "B should be greater than A");

/* Compare high words different */
STATIC_ASSERT(CONST_C < CONST_A, "C should be less than A");
STATIC_ASSERT(CONST_A > CONST_C, "A should be greater than C");

/* Test 2: Range analysis with 128-bit parameters */
__int128 range_test_compare(__int128 x, __int128 y) {
    /* These comparisons should trigger double_int::cmp in VRP */
    if (x < CONST_A) {
        if (y > CONST_B) {
            return x + y;
        }
    }
    
    /* Compare with different high words */
    if (x > CONST_C && y < CONST_A) {
        return x - y;
    }
    
    /* Nested comparisons */
    if ((x < CONST_B) && (y > CONST_C) && (x != y)) {
        return x * y;
    }
    
    return x;
}

/* Test 3: Loop with 128-bit induction variable */
void loop_with_128bit_iv(__int128 start, __int128 end) {
    /* Loop that should cause VRP to analyze 128-bit ranges */
    for (__int128 i = start; i < end; i = i + 1) {
        /* Force comparison in loop condition */
        if (i == CONST_A) {
            printf("Found A in loop\n");
        }
    }
}

/* Test 4: Arithmetic operations crossing word boundaries */
__int128 cross_boundary_ops(__int128 x) {
    /* Left shift moving bits from low to high word */
    __int128 shifted = x << 72;
    
    /* Right shift with sign extension */
    __int128 arith_shifted = x >> 96;
    
    /* Bitwise operations targeting specific words */
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    
    __int128 high_only = x & mask_high;
    __int128 low_only = x & mask_low;
    
    /* Mix operations that require full 128-bit comparison */
    if ((high_only > CONST_C) && (low_only < 0x100000000ULL)) {
        return shifted;
    }
    
    return arith_shifted;
}

/* Test 5: Mixed-type comparisons and conversions */
int mixed_type_comparisons(__int128 x, unsigned long long y) {
    /* Compare 128-bit with 64-bit - requires promotion */
    if (x > y) {
        return 1;
    }
    
    /* Ternary operator with mixed types */
    __int128 result = (x < CONST_A) ? y : x;
    
    /* Compare with overflow builtins */
    __int128 sum;
    if (__builtin_add_overflow(x, result, &sum)) {
        return -1;
    }
    
    __int128 prod;
    if (__builtin_mul_overflow(x, y, &prod)) {
        return -2;
    }
    
    return 0;
}

/* Test 6: Dead code with 128-bit comparisons (still processed by early opts) */
void dead_code_paths(void) {
    if (0) {  /* Dead code, but constants still evaluated */
        /* These comparisons should still be processed during constant folding */
        if (CONST_A < CONST_B) {
            printf("Never printed A < B\n");
        }
        
        if (CONST_C > CONST_D) {
            printf("Never printed C > D\n");
        }
        
        /* Complex dead expression */
        __int128 dead_var = CONST_A + CONST_B;
        if (dead_var > CONST_C && dead_var < CONST_E) {
            printf("Never printed range check\n");
        }
    }
}

/* Test 7: Switch-like logic with 128-bit comparisons */
int switch_like_logic(__int128 x) {
    /* GCC may convert this to decision tree with comparisons */
    if (x == CONST_A) return 1;
    if (x == CONST_B) return 2;
    if (x == CONST_C) return 3;
    if (x == CONST_D) return 4;
    if (x == CONST_E) return 5;
    
    /* Range checks */
    if (x < CONST_A) return -1;
    if (x > CONST_E) return -2;
    
    return 0;
}

/* Test 8: Array operations with 128-bit values */
__int128 process_global_array(void) {
    __int128 sum = 0;
    for (size_t i = 0; i < sizeof(global_array)/sizeof(global_array[0]); i++) {
        /* Compare array elements with constants */
        if (global_array[i] > CONST_A) {
            sum += global_array[i];
        } else if (global_array[i] < CONST_C) {
            sum -= global_array[i];
        }
    }
    return sum;
}

/* Test 9: Function with multiple comparison patterns */
__int128 complex_comparison_chain(__int128 a, __int128 b, __int128 c) {
    /* Chain of comparisons that should test all cmp paths */
    if (a < b) {
        if (b < c) {
            if (a < c) {
                return a;
            }
        }
    }
    
    if (a > b) {
        if (b > c) {
            if (a > c) {
                return b;
            }
        }
    }
    
    /* Equal high word, different low word cases */
    if ((a & ~0xFFFFFFFFULL) == (b & ~0xFFFFFFFFULL)) {
        if ((a & 0xFFFFFFFFULL) < (b & 0xFFFFFFFFULL)) {
            return c;
        }
    }
    
    return a + b + c;
}

/* Main function that exercises all tests */
int main(void) {
    /* Initialize with values that will trigger different comparison paths */
    __int128 test_val1 = CONST_A;
    __int128 test_val2 = CONST_B;  /* Same high, different low */
    __int128 test_val3 = CONST_C;  /* Different high */
    __int128 test_val4 = CONST_D;  /* -1 */
    
    printf("Starting 128-bit comparison tests...\n");
    
    /* Test 1: Range analysis */
    __int128 range_result = range_test_compare(test_val1, test_val2);
    printf("Range test result: high=%016llx low=%016llx\n",
           (unsigned long long)(range_result >> 64),
           (unsigned long long)(range_result & 0xFFFFFFFFFFFFFFFFULL));
    
    /* Test 2: Loop with bounds that differ in high word */
    loop_with_128bit_iv(test_val3, test_val1);
    
    /* Test 3: Cross-boundary operations */
    __int128 cross_result = cross_boundary_ops(test_val1);
    printf("Cross-boundary result: high=%016llx low=%016llx\n",
           (unsigned long long)(cross_result >> 64),
           (unsigned long long)(cross_result & 0xFFFFFFFFFFFFFFFFULL));
    
    /* Test 4: Mixed type comparisons */
    int mixed_result = mixed_type_comparisons(test_val1, 0x123456789ABCDEF0ULL);
    printf("Mixed type result: %d\n", mixed_result);
    
    /* Test 5: Dead code paths (should still process constants) */
    dead_code_paths();
    
    /* Test 6: Switch-like logic */
    int switch_result = switch_like_logic(test_val2);
    printf("Switch-like result for B: %d\n", switch_result);
    
    /* Test 7: Array processing */
    __int128 array_sum = process_global_array();
    printf("Array sum: high=%016llx low=%016llx\n",
           (unsigned long long)(array_sum >> 64),
           (unsigned long long)(array_sum & 0xFFFFFFFFFFFFFFFFULL));
    
    /* Test 8: Complex comparison chain */
    __int128 chain_result = complex_comparison_chain(test_val1, test_val2, test_val3);
    printf("Chain result: high=%016llx low=%016llx\n",
           (unsigned long long)(chain_result >> 64),
           (unsigned long long)(chain_result & 0xFFFFFFFFFFFFFFFFULL));
    
    /* Final verification sum */
    __int128 final_sum = range_result + cross_result + array_sum + chain_result;
    printf("Final checksum: high=%016llx low=%016llx\n",
           (unsigned long long)(final_sum >> 64),
           (unsigned long long)(final_sum & 0xFFFFFFFFFFFFFFFFULL));
    
    return 0;
}
