#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large 128-bit constants with varying high/low word patterns */
#define CONST_A_HIGH 0x123456789ABCDEF0ULL
#define CONST_A_LOW  0xFEDCBA9876543210ULL
#define CONST_A (((__int128)CONST_A_HIGH << 64) | CONST_A_LOW)

#define CONST_B_HIGH 0x123456789ABCDEF0ULL  /* Same high as A */
#define CONST_B_LOW  0xFEDCBA9876543211ULL  /* Different low */
#define CONST_B (((__int128)CONST_B_HIGH << 64) | CONST_B_LOW)

#define CONST_C_HIGH 0x123456789ABCDEF1ULL  /* Different high */
#define CONST_C_LOW  0xFEDCBA9876543210ULL
#define CONST_C (((__int128)CONST_C_HIGH << 64) | CONST_C_LOW)

#define CONST_D_HIGH 0x0000000000000000ULL  /* Zero high word */
#define CONST_D_LOW  0xFFFFFFFFFFFFFFFFULL  /* Max low word */
#define CONST_D (((__int128)CONST_D_HIGH << 64) | CONST_D_LOW)

#define CONST_E_HIGH 0xFFFFFFFFFFFFFFFFULL  /* Max high word */
#define CONST_E_LOW  0x0000000000000000ULL  /* Zero low word */
#define CONST_E (((__int128)CONST_E_HIGH << 64) | CONST_E_LOW)

/* Global arrays with 128-bit constants - forces compile-time evaluation */
static __int128 global_array[] = {
    CONST_A,
    CONST_B,
    CONST_C,
    CONST_D,
    CONST_E,
    -CONST_A,
    -CONST_B,
    ((__int128)0x8000000000000000ULL << 64) | 0x0000000000000000ULL, /* Min signed */
    ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL, /* Max signed */
};

/* Static assertions forcing compile-time comparisons */
_Static_assert(CONST_A < CONST_B, "A < B comparison");  /* High equal, low differs */
_Static_assert(CONST_A < CONST_C, "A < C comparison");  /* High differs */
_Static_assert(CONST_D < CONST_E, "D < E comparison");  /* Both words differ */
_Static_assert(CONST_A != CONST_B, "A != B comparison");

/* Function to test range analysis with 128-bit comparisons */
__int128 range_analysis_test(__int128 x, __int128 y) {
    /* Comparisons that should trigger double_int::cmp in VRP */
    if (x < CONST_A) {
        if (y > CONST_B) {
            return x + y;
        }
    }
    
    /* Ternary with mixed-type comparisons */
    __int128 result = (x < 1000) ? (__int128)1000 : CONST_C;
    
    /* Nested comparisons */
    if (x > CONST_D && x < CONST_E) {
        result = result | y;
    }
    
    return result;
}

/* Function with 128-bit loop induction variable */
void loop_with_128bit_iv(__int128 start, __int128 end) {
    /* Loop that may be analyzed by VRP */
    for (__int128 i = start; i < end; i += ((__int128)1 << 60)) {
        /* Empty loop body - just for analysis */
        (void)i;
    }
}

/* Function using overflow builtins with 128-bit values */
int overflow_test(__int128 a, __int128 b) {
    __int128 sum, product;
    int overflow_add, overflow_mul;
    
    overflow_add = __builtin_add_overflow(a, b, &sum);
    overflow_mul = __builtin_mul_overflow(a, b, &product);
    
    /* Comparisons after overflow checks */
    if (sum > CONST_A && product < CONST_B) {
        return 1;
    }
    return 0;
}

/* Function with bitwise operations crossing word boundaries */
__int128 bitwise_operations(__int128 x) {
    /* Shift operations that move bits between high/low words */
    __int128 shifted = x << 65;  /* Moves bits from low to high word */
    
    /* Mask operations targeting specific words */
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    
    __int128 high_part = x & mask_high;
    __int128 low_part = x & mask_low;
    
    /* Arithmetic right shift on negative value */
    __int128 neg_x = -x;
    __int128 arith_shifted = neg_x >> 72;
    
    return (shifted ^ high_part) | (low_part & arith_shifted);
}

/* Function with mixed-type comparisons */
int mixed_type_comparisons(unsigned long long ull_val, __int128 s128_val) {
    /* Implicit conversion and comparison */
    if (s128_val > ull_val) {
        return 1;
    }
    
    /* Ternary with different types */
    __int128 result = (ull_val > 1000) ? CONST_A : (__int128)ull_val;
    
    /* Comparison with constant that has zero high word */
    if (result < CONST_D) {
        return 2;
    }
    
    return 0;
}

/* Dead code with 128-bit constant comparisons (may still be evaluated) */
void dead_code_paths(void) {
    if (0) {  /* Always false, but constants may be compared during compilation */
        if (CONST_A < CONST_B) {
            /* This should trigger high equal, low different comparison */
            __builtin_unreachable();
        }
        
        if (CONST_C > CONST_A) {
            /* This should trigger high different comparison */
            __builtin_unreachable();
        }
        
        /* Test all comparison operators */
        _Bool cmp1 = CONST_A == CONST_B;
        _Bool cmp2 = CONST_A != CONST_C;
        _Bool cmp3 = CONST_D <= CONST_E;
        _Bool cmp4 = CONST_E >= CONST_A;
        (void)cmp1; (void)cmp2; (void)cmp3; (void)cmp4;
    }
}

/* Main function that exercises all test cases */
int main(void) {
    /* Initialize with values that will exercise different comparison paths */
    __int128 test_val1 = CONST_A;
    __int128 test_val2 = CONST_B;
    __int128 test_val3 = CONST_C;
    
    /* Test range analysis */
    __int128 range_result = range_analysis_test(test_val1, test_val2);
    
    /* Test overflow operations */
    int overflow_result = overflow_test(test_val1, test_val2);
    
    /* Test bitwise operations */
    __int128 bitwise_result = bitwise_operations(test_val3);
    
    /* Test mixed-type comparisons */
    int mixed_result = mixed_type_comparisons(0xFFFFFFFFFFFFFFFFULL, test_val1);
    
    /* Call loop function */
    loop_with_128bit_iv(CONST_D, CONST_E);
    
    /* Call dead code paths */
    dead_code_paths();
    
    /* Compute checksum using global array */
    __int128 checksum = 0;
    for (size_t i = 0; i < sizeof(global_array)/sizeof(global_array[0]); i++) {
        checksum ^= global_array[i];
    }
    
    /* Print some results to prevent optimization removal */
    printf("Range result high: 0x%016llx, low: 0x%016llx\n",
           (unsigned long long)(range_result >> 64),
           (unsigned long long)range_result);
    printf("Overflow test: %d\n", overflow_result);
    printf("Mixed type test: %d\n", mixed_result);
    printf("Checksum high: 0x%016llx, low: 0x%016llx\n",
           (unsigned long long)(checksum >> 64),
           (unsigned long long)checksum);
    
    /* Additional static assertions for compile-time evaluation */
    #if __has_builtin(__builtin_constant_p)
    #define FORCE_COMPILE_TIME(expr) \
        (__builtin_constant_p(expr) ? (expr) : (expr))
    
    /* Force evaluation of comparisons at compile time */
    _Static_assert(FORCE_COMPILE_TIME(CONST_A < CONST_B), "CT A<B");
    _Static_assert(FORCE_COMPILE_TIME(CONST_C > CONST_A), "CT C>A");
    _Static_assert(FORCE_COMPILE_TIME(CONST_D != CONST_E), "CT D!=E");
    #endif
    
    return 0;
}
