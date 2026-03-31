#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large 128-bit constants with different high/low word patterns */
#define CONST_A_HIGH 0x123456789ABCDEF0ULL
#define CONST_A_LOW  0xFEDCBA9876543210ULL
#define CONST_B_HIGH 0x123456789ABCDEF0ULL  /* Same high as A */
#define CONST_B_LOW  0xFEDCBA9876543211ULL  /* Different low */
#define CONST_C_HIGH 0x123456789ABCDEF1ULL  /* Different high */
#define CONST_C_LOW  0xFEDCBA9876543210ULL

/* Global constants that force compile-time evaluation */
static const __int128 g_const_a = ((__int128)CONST_A_HIGH << 64) | CONST_A_LOW;
static const __int128 g_const_b = ((__int128)CONST_B_HIGH << 64) | CONST_B_LOW;
static const __int128 g_const_c = ((__int128)CONST_C_HIGH << 64) | CONST_C_LOW;

/* Global array with mixed 128-bit constants */
static const __int128 g_const_array[] = {
    ((__int128)0x0ULL << 64) | 0x0ULL,
    ((__int128)0x0ULL << 64) | 0x1ULL,
    ((__int128)0x1ULL << 64) | 0x0ULL,
    ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL,
    g_const_a,
    g_const_b,
    g_const_c,
};

/* Test 1: Range analysis with comparisons */
__int128 test_range_analysis(__int128 x, __int128 y) {
    /* Comparisons that should trigger double_int::cmp in VRP */
    if (x < g_const_a) {
        if (y > g_const_b) {
            return x + y;
        }
    }
    
    /* Mixed comparisons */
    if ((unsigned __int128)x < (unsigned __int128)g_const_c) {
        return x - y;
    }
    
    /* Ternary with 128-bit constants */
    return (x > 0) ? g_const_a : g_const_b;
}

/* Test 2: Loop with 128-bit induction variable */
__int128 test_loop_comparison(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop that compares 128-bit values */
    for (__int128 i = start; i < end; i += ((__int128)1 << 32)) {
        sum += i;
        
        /* Nested comparison with constant */
        if (i > g_const_a) {
            sum += g_const_b;
        }
    }
    
    return sum;
}

/* Test 3: Bitwise operations crossing boundaries */
__int128 test_bitwise_ops(__int128 x) {
    /* Masks targeting specific words */
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    
    /* Operations that cross word boundaries */
    __int128 shifted = x << 65;  /* Moves bits from low to high word */
    __int128 masked = (x & mask_high) | ((x & mask_low) << 32);
    
    /* Comparisons after shifting */
    if (shifted > g_const_c) {
        return masked;
    }
    
    /* Arithmetic right shift on negative value */
    __int128 neg_val = -((__int128)1 << 120);
    __int128 arith_shifted = neg_val >> 70;
    
    if (arith_shifted < x) {
        return arith_shifted;
    }
    
    return masked;
}

/* Test 4: Overflow operations */
__int128 test_overflow_ops(__int128 a, __int128 b) {
    __int128 result;
    int overflow;
    
    /* Overflow checks that may use double_int comparisons */
    overflow = __builtin_add_overflow(a, b, &result);
    if (overflow) {
        return g_const_a;
    }
    
    overflow = __builtin_mul_overflow(a, b, &result);
    if (overflow) {
        return g_const_b;
    }
    
    /* Compare with max/min values */
    __int128 max_128 = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    __int128 min_128 = ((__int128)0x8000000000000000ULL << 64) | 0x0ULL;
    
    if (result > max_128) {
        return max_128;
    }
    
    if (result < min_128) {
        return min_128;
    }
    
    return result;
}

/* Test 5: Mixed-type comparisons */
__int128 test_mixed_comparisons(__int128 x, unsigned long long y) {
    /* Compare 128-bit with 64-bit */
    if (x < (__int128)y) {
        return x;
    }
    
    /* Ternary with mixed types */
    __int128 result = (y > 1000) ? g_const_a : (__int128)y;
    
    /* Compare with different signedness */
    if ((unsigned __int128)x > (unsigned __int128)result) {
        return result;
    }
    
    return x;
}

/* Test 6: Dead code with constant comparisons */
void test_dead_code_comparisons(void) {
    /* Dead code that still gets evaluated at compile time */
    if (0) {
        /* These comparisons should be evaluated during constant folding */
        if (g_const_a < g_const_b) {
            /* High words equal, low words differ */
            volatile int dummy = 1;
            (void)dummy;
        }
        
        if (g_const_a < g_const_c) {
            /* High words differ */
            volatile int dummy = 2;
            (void)dummy;
        }
        
        if (((__int128)0x0ULL << 64) | 0x0ULL < 
            ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL) {
            volatile int dummy = 3;
            (void)dummy;
        }
    }
}

/* Test 7: Static assertions with 128-bit comparisons */
void test_static_asserts(void) {
    /* Force compile-time evaluation of comparisons */
    static_assert(
        ((__int128)0x1ULL << 64) > 0xFFFFFFFFFFFFFFFFULL,
        "128-bit comparison should hold"
    );
    
    static_assert(
        g_const_a != g_const_b,
        "Constants with different low words should differ"
    );
    
    static_assert(
        g_const_a != g_const_c,
        "Constants with different high words should differ"
    );
}

/* Test 8: Array operations with 128-bit values */
__int128 test_array_operations(void) {
    __int128 sum = 0;
    
    /* Process global array - forces constant evaluation */
    for (size_t i = 0; i < sizeof(g_const_array)/sizeof(g_const_array[0]); i++) {
        sum += g_const_array[i];
        
        /* Compare array elements with constants */
        if (g_const_array[i] > g_const_a) {
            sum += 1;
        }
    }
    
    return sum;
}

/* Test 9: Switch-like logic with comparisons */
__int128 test_switch_like(__int128 x) {
    /* Series of comparisons that act like a switch */
    if (x < ((__int128)0x1000000000000000ULL << 64)) {
        return x + 1;
    } else if (x < ((__int128)0x2000000000000000ULL << 64)) {
        return x - 1;
    } else if (x < ((__int128)0x3000000000000000ULL << 64)) {
        return x * 2;
    } else {
        return x / 2;
    }
}

/* Main function that exercises all tests */
int main(void) {
    /* Initialize test values with different high/low patterns */
    __int128 test_val1 = ((__int128)0x123456789ABCDEF0ULL << 64) | 0x1111111111111111ULL;
    __int128 test_val2 = ((__int128)0x123456789ABCDEF0ULL << 64) | 0x2222222222222222ULL;
    __int128 test_val3 = ((__int128)0x223456789ABCDEF0ULL << 64) | 0x1111111111111111ULL;
    
    /* Run all tests */
    __int128 result = 0;
    
    result += test_range_analysis(test_val1, test_val2);
    result += test_loop_comparison(test_val1, test_val3);
    result += test_bitwise_ops(test_val2);
    result += test_overflow_ops(test_val1, test_val2);
    result += test_mixed_comparisons(test_val3, 0xFFFFFFFFFFFFFFFFULL);
    
    test_dead_code_comparisons();
    test_static_asserts();
    
    result += test_array_operations();
    result += test_switch_like(test_val1);
    
    /* Print a simple checksum to verify execution */
    unsigned long long low = (unsigned long long)(result & 0xFFFFFFFFFFFFFFFFULL);
    unsigned long long high = (unsigned long long)((result >> 64) & 0xFFFFFFFFFFFFFFFFULL);
    
    printf("Result checksum: high=0x%016llx low=0x%016llx\n", high, low);
    printf("Test completed successfully.\n");
    
    return 0;
}
