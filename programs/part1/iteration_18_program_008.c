/* double_int_test.c - Test program to trigger double_int::cmp in GCC */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large __int128 constants with varying high/low word patterns */
#define HIGH_A    0x123456789ABCDEF0ULL
#define LOW_A     0xFEDCBA9876543210ULL
#define HIGH_B    0x123456789ABCDEF0ULL  /* Same high as A */
#define LOW_B     0xFEDCBA9876543211ULL  /* Different low */
#define HIGH_C    0x123456789ABCDEF1ULL  /* Different high */
#define LOW_C     0xFEDCBA9876543210ULL

/* Construct 128-bit constants */
static const __int128 CONST_A = ((__int128)HIGH_A << 64) | LOW_A;
static const __int128 CONST_B = ((__int128)HIGH_B << 64) | LOW_B;
static const __int128 CONST_C = ((__int128)HIGH_C << 64) | LOW_C;
static const __int128 CONST_D = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL; /* -1 */
static const __int128 CONST_E = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL; /* Max positive */

/* Global arrays with __int128 constants - forces compile-time initialization */
static const __int128 global_array[] = {
    CONST_A,
    CONST_B,
    CONST_C,
    CONST_D,
    CONST_E,
    ((__int128)0x0ULL << 64) | 0x0ULL,
    ((__int128)0x1ULL << 64) | 0x0ULL,
    ((__int128)0x0ULL << 64) | 0x1ULL,
};

/* Static assertions forcing compile-time comparisons */
_Static_assert(CONST_A < CONST_C, "CONST_A should be less than CONST_C");
_Static_assert(CONST_B > CONST_A, "CONST_B should be greater than CONST_A");
_Static_assert(CONST_D < CONST_E, "-1 should be less than max positive");

/* Test 1: Function with __int128 comparisons for VRP */
__int128 test_range_analysis(__int128 x, __int128 y) {
    /* Comparisons that should trigger double_int::cmp in VRP */
    if (x < CONST_A) {
        return x + 1;
    }
    if (y > CONST_C) {
        return y - 1;
    }
    
    /* Compare parameters with each other */
    if (x < y) {
        return x;
    } else if (x > y) {
        return y;
    }
    
    return 0;
}

/* Test 2: Loop with __int128 induction variable */
__int128 test_loop_comparisons(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop that compares __int128 values */
    for (__int128 i = start; i < end; i = i + 1) {
        /* Additional comparisons inside loop */
        if (i < CONST_A) {
            sum += 1;
        } else if (i > CONST_B) {
            sum += 2;
        } else {
            sum += 3;
        }
        
        /* Break early to avoid long runtime */
        if (i > start + 100) {
            break;
        }
    }
    
    return sum;
}

/* Test 3: Bitwise operations crossing word boundaries */
__int128 test_bitwise_operations(__int128 x) {
    /* Masks targeting specific words */
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    
    /* Operations that require reasoning about both words */
    __int128 shifted = x << 65;  /* Moves bits from low to high word */
    __int128 masked = x & mask_high;
    
    /* Comparisons after bitwise operations */
    if (shifted > CONST_A) {
        return shifted | mask_low;
    }
    
    if (masked < CONST_B) {
        return masked ^ mask_low;
    }
    
    return x;
}

/* Test 4: Mixed-type comparisons and conversions */
__int128 test_mixed_type_comparisons(__int128 x, unsigned long long y) {
    /* Implicit conversion and comparison */
    if (x < y) {
        return y;  /* y promoted to __int128 */
    }
    
    /* Ternary operator with different types */
    __int128 result = (x > 1000) ? CONST_A : (__int128)y;
    
    /* Compare with negative values */
    if (result > CONST_D) {  /* CONST_D is -1 */
        return result;
    }
    
    return x;
}

/* Test 5: Overflow operations */
__int128 test_overflow_operations(__int128 a, __int128 b) {
    __int128 sum, product;
    int overflow;
    
    /* Overflow addition - may trigger internal comparisons */
    overflow = __builtin_add_overflow(a, b, &sum);
    if (overflow) {
        return CONST_A;
    }
    
    /* Overflow multiplication */
    overflow = __builtin_mul_overflow(a, b, &product);
    if (overflow) {
        return CONST_B;
    }
    
    /* Compare results with constants */
    if (sum > CONST_C && product < CONST_A) {
        return sum + product;
    }
    
    return sum;
}

/* Test 6: Dead code with __int128 comparisons (still processed by compiler) */
void test_dead_code_comparisons(void) {
    /* Dead code that still gets parsed and potentially constant-folded */
    if (0) {  /* Always false */
        /* These comparisons may still be evaluated during compilation */
        if (CONST_A < CONST_B) {
            __builtin_printf("Dead code path A\n");
        }
        
        if (((__int128)0x8000000000000000ULL << 64) > CONST_D) {
            __builtin_printf("Dead code path B\n");
        }
        
        /* Complex expression forcing high/low word comparison */
        __int128 dead_var = CONST_A + CONST_B;
        if (dead_var > CONST_C && dead_var < CONST_E) {
            __builtin_printf("Dead code path C\n");
        }
    }
}

/* Test 7: Switch statement with __int128 (via if-else chain) */
int test_switch_like(__int128 x) {
    /* GCC may convert this to switch-like logic */
    if (x == CONST_A) return 1;
    if (x == CONST_B) return 2;
    if (x == CONST_C) return 3;
    if (x == CONST_D) return 4;
    if (x == CONST_E) return 5;
    
    /* Range comparisons */
    if (x < CONST_A) return 6;
    if (x > CONST_E) return 7;
    
    return 0;
}

/* Main function that exercises all tests */
int main(void) {
    __int128 result = 0;
    
    /* Initialize with values that exercise different comparison paths */
    __int128 val1 = CONST_A;
    __int128 val2 = CONST_B;  /* Same high, different low */
    __int128 val3 = CONST_C;  /* Different high */
    __int128 val4 = CONST_D;  /* -1 */
    
    /* Test 1: Range analysis */
    result += test_range_analysis(val1, val2);
    
    /* Test 2: Loop comparisons */
    result += test_loop_comparisons(val4, val1);  /* From -1 to CONST_A */
    
    /* Test 3: Bitwise operations */
    result += test_bitwise_operations(val3);
    
    /* Test 4: Mixed-type comparisons */
    result += test_mixed_type_comparisons(val1, 0xFFFFFFFFFFFFFFFFULL);
    
    /* Test 5: Overflow operations */
    result += test_overflow_operations(val1, val2);
    
    /* Test 6: Dead code (no runtime effect, but compiler processes) */
    test_dead_code_comparisons();
    
    /* Test 7: Switch-like comparisons */
    int switch_result = test_switch_like(val2);
    result += switch_result;
    
    /* Also process global array */
    for (size_t i = 0; i < sizeof(global_array)/sizeof(global_array[0]); i++) {
        result += global_array[i];
    }
    
    /* Print a simple checksum to verify execution */
    /* Split 128-bit result into two 64-bit parts for printing */
    unsigned long long high = (unsigned long long)(result >> 64);
    unsigned long long low = (unsigned long long)result;
    
    printf("Result checksum: high=0x%016llx low=0x%016llx\n", high, low);
    printf("Test completed successfully.\n");
    
    return 0;
}
