/* double_int_test.c - Test program to trigger GCC's double_int::cmp comparisons */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large __int128 constants with varying high/low word patterns */
#define CONST_A_HIGH_LOW   (((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL)
#define CONST_B_HIGH_LOW   (((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543211ULL)
#define CONST_C_HIGH_HIGH  (((__int128)0x123456789ABCDEF1ULL << 64) | 0xFEDCBA9876543210ULL)
#define CONST_D_LOW_HIGH   (((__int128)0x0000000000000000ULL << 64) | 0xFFFFFFFFFFFFFFFFULL)
#define CONST_E_NEGATIVE   (((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFEULL) /* -2 */
#define CONST_F_MAX        (((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL) /* INT128_MAX */

/* Global arrays with __int128 constants - forces compile-time evaluation */
static const __int128 global_consts[] = {
    CONST_A_HIGH_LOW,
    CONST_B_HIGH_LOW,
    CONST_C_HIGH_HIGH,
    CONST_D_LOW_HIGH,
    CONST_E_NEGATIVE,
    CONST_F_MAX,
    0,
    -1,
    ((__int128)1 << 127) - 1,
    (__int128)-1 << 127
};

/* Static assertions to force compile-time comparisons */
_Static_assert(CONST_A_HIGH_LOW < CONST_B_HIGH_LOW, 
               "High equal, low differs - should trigger low word comparison");
_Static_assert(CONST_A_HIGH_LOW < CONST_C_HIGH_HIGH, 
               "High differs - should trigger high word comparison");

/* Function 1: Range analysis with comparisons in both high and low words */
__int128 range_compare(__int128 x, __int128 y) {
    /* These comparisons may trigger double_int::cmp during VRP */
    if (x < CONST_A_HIGH_LOW) {
        return x + 1;
    }
    if (x > CONST_C_HIGH_HIGH) {
        return x - 1;
    }
    if (y < CONST_D_LOW_HIGH) {
        return y * 2;
    }
    return x + y;
}

/* Function 2: Loop with __int128 induction variable */
__int128 loop_with_wide_bounds(__int128 start, __int128 end) {
    __int128 sum = 0;
    /* Loop bound comparison may trigger double_int::cmp */
    for (__int128 i = start; i < end; i += ((__int128)1 << 60)) {
        sum += i;
        /* Additional comparison inside loop */
        if (i > CONST_D_LOW_HIGH) {
            sum += 1;
        }
    }
    return sum;
}

/* Function 3: Bitwise operations crossing word boundaries */
__int128 cross_boundary_ops(__int128 a, __int128 b) {
    /* Shifts that move bits between high and low words */
    __int128 shift1 = a << 65;  /* Crosses 64-bit boundary */
    __int128 shift2 = b >> 96;  /* Large right shift */
    
    /* Bitwise operations with masks targeting specific words */
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    
    __int128 high_part = a & mask_high;
    __int128 low_part = b & mask_low;
    
    /* Comparisons after bitwise operations */
    if ((high_part >> 64) > 0x7FFFFFFFULL) {
        return shift1 | shift2;
    }
    
    return (high_part | low_part) ^ CONST_A_HIGH_LOW;
}

/* Function 4: Mixed-type comparisons and conversions */
__int128 mixed_type_comparisons(unsigned long long ull_val, __int128 wide_val) {
    /* Compare __int128 with narrower types */
    if (wide_val > ull_val) {
        /* Ternary with mixed types */
        __int128 result = (ull_val > 1000) ? 
                         CONST_B_HIGH_LOW : 
                         (__int128)ull_val;
        
        /* Additional comparison in return path */
        return (result < CONST_C_HIGH_HIGH) ? result : wide_val;
    }
    
    /* Overflow checking with __int128 */
    __int128 overflow_test;
    if (__builtin_add_overflow(wide_val, CONST_D_LOW_HIGH, &overflow_test)) {
        return -1;
    }
    
    return wide_val + ull_val;
}

/* Function 5: Dead code with constant comparisons (may still be evaluated) */
__int128 dead_code_comparisons(__int128 x) {
    __int128 result = x;
    
    /* Dead code block - but constants may still be compared during compilation */
    if (0) {  /* Always false */
        /* These comparisons should still be evaluated by the compiler */
        if (CONST_A_HIGH_LOW < CONST_B_HIGH_LOW) {
            result = CONST_C_HIGH_HIGH;
        }
        
        /* More complex dead comparisons */
        __int128 dead_var = CONST_D_LOW_HIGH;
        while (dead_var < CONST_F_MAX) {
            dead_var += CONST_A_HIGH_LOW;
        }
    }
    
    return result;
}

/* Function 6: Switch-like logic with wide comparisons */
int wide_comparison_chain(__int128 a, __int128 b) {
    /* Chain of comparisons that may be optimized */
    if (a < b) return -1;
    if (a > b) return 1;
    
    /* Additional comparisons with constants */
    if (a < CONST_D_LOW_HIGH) return -2;
    if (a > CONST_F_MAX) return 2;
    
    /* Nested comparisons */
    if ((a > CONST_A_HIGH_LOW) && (b < CONST_C_HIGH_HIGH)) {
        return 3;
    }
    
    return 0;
}

/* Helper: Print __int128 as hex */
void print_int128(__int128 val) {
    unsigned long long high = (unsigned long long)(val >> 64);
    unsigned long long low = (unsigned long long)val;
    printf("0x%016llx%016llx", high, low);
}

/* Main function that exercises all test cases */
int main() {
    __int128 test_sum = 0;
    
    /* Test 1: Range analysis function */
    __int128 val1 = CONST_A_HIGH_LOW;
    __int128 val2 = CONST_B_HIGH_LOW;
    __int128 result1 = range_compare(val1, val2);
    test_sum += result1;
    
    printf("Test 1 - range_compare: ");
    print_int128(result1);
    printf("\n");
    
    /* Test 2: Loop with wide bounds */
    __int128 loop_result = loop_with_wide_bounds(CONST_D_LOW_HIGH, CONST_A_HIGH_LOW);
    test_sum += loop_result;
    
    printf("Test 2 - loop_with_wide_bounds: ");
    print_int128(loop_result);
    printf("\n");
    
    /* Test 3: Cross-boundary operations */
    __int128 cross_result = cross_boundary_ops(CONST_A_HIGH_LOW, CONST_C_HIGH_HIGH);
    test_sum += cross_result;
    
    printf("Test 3 - cross_boundary_ops: ");
    print_int128(cross_result);
    printf("\n");
    
    /* Test 4: Mixed-type comparisons */
    unsigned long long ull_test = 0xFFFFFFFFFFFFFFFFULL;
    __int128 mixed_result = mixed_type_comparisons(ull_test, CONST_B_HIGH_LOW);
    test_sum += mixed_result;
    
    printf("Test 4 - mixed_type_comparisons: ");
    print_int128(mixed_result);
    printf("\n");
    
    /* Test 5: Dead code (still may trigger compile-time comparisons) */
    __int128 dead_result = dead_code_comparisons(CONST_F_MAX);
    test_sum += dead_result;
    
    printf("Test 5 - dead_code_comparisons: ");
    print_int128(dead_result);
    printf("\n");
    
    /* Test 6: Comparison chain */
    int cmp_result = wide_comparison_chain(CONST_A_HIGH_LOW, CONST_B_HIGH_LOW);
    test_sum += cmp_result;
    
    printf("Test 6 - wide_comparison_chain result: %d\n", cmp_result);
    
    /* Process global array (ensures constants are used) */
    __int128 array_sum = 0;
    for (size_t i = 0; i < sizeof(global_consts)/sizeof(global_consts[0]); i++) {
        array_sum += global_consts[i];
    }
    test_sum += array_sum;
    
    printf("Global array sum: ");
    print_int128(array_sum);
    printf("\n");
    
    /* Final checksum */
    printf("\nFinal test checksum: ");
    print_int128(test_sum);
    printf("\n");
    
    return 0;
}
