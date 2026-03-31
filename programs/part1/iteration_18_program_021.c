/* double_int_cmp_test.c - Test program to trigger double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large 128-bit constants with varying high/low word patterns */
#define HIGH_A  0x123456789ABCDEF0ULL
#define LOW_A   0xFEDCBA9876543210ULL
#define HIGH_B  0x123456789ABCDEF0ULL  /* Same high as A */
#define LOW_B   0xFEDCBA9876543211ULL  /* Different low */
#define HIGH_C  0x123456789ABCDEF1ULL  /* Different high */
#define LOW_C   0xFEDCBA9876543210ULL

/* Construct 128-bit constants */
static const __int128 CONST_A = ((__int128)HIGH_A << 64) | LOW_A;
static const __int128 CONST_B = ((__int128)HIGH_B << 64) | LOW_B;
static const __int128 CONST_C = ((__int128)HIGH_C << 64) | LOW_C;
static const __int128 CONST_D = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL; /* -1 */
static const __int128 CONST_E = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL; /* Max positive */

/* Global arrays with 128-bit constants - forces compile-time evaluation */
static const __int128 global_array[] = {
    CONST_A,
    CONST_B,
    CONST_C,
    CONST_D,
    CONST_E,
    ((__int128)0x0ULL << 64) | 0x0ULL,  /* Zero */
    ((__int128)0x8000000000000000ULL << 64) | 0x0ULL,  /* Min negative */
};

/* Static assertions to force compile-time comparisons */
_Static_assert(CONST_A < CONST_B, "A < B comparison");  /* Same high, different low */
_Static_assert(CONST_A < CONST_C, "A < C comparison");  /* Different high */
_Static_assert(CONST_D < CONST_A, "Negative < positive");
_Static_assert(CONST_E > CONST_A, "Max > A");

/* Test 1: Function with 128-bit comparisons for VRP */
__int128 test_range_analysis(__int128 x, __int128 y) {
    /* These comparisons should trigger double_int::cmp in VRP */
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

/* Test 2: Loop with 128-bit induction variable */
__int128 test_loop_comparisons(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop with 128-bit bounds - compiler must compare induction variable */
    for (__int128 i = start; i < end; i = i + 1) {
        sum += i;
        
        /* Additional comparison inside loop */
        if (i < CONST_A) {
            sum += 1;
        }
    }
    
    return sum;
}

/* Test 3: Bitwise operations crossing word boundaries */
__int128 test_bitwise_ops(__int128 x) {
    /* Masks targeting specific words */
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    
    /* Operations that affect both words */
    __int128 shifted = x << 65;  /* Moves bits from low to high word */
    __int128 masked = x & mask_high;  /* Isolate high word */
    
    /* Comparisons after bitwise ops */
    if (shifted < CONST_B) {
        return masked;
    }
    
    /* Arithmetic shift on negative value */
    __int128 neg_val = -x;
    __int128 arith_shifted = neg_val >> 67;
    
    if (arith_shifted > CONST_D) {
        return arith_shifted;
    }
    
    return x;
}

/* Test 4: Mixed-type comparisons and conversions */
__int128 test_mixed_comparisons(__int128 x, unsigned long long y) {
    /* Compare 128-bit with 64-bit - requires promotion */
    if (x < y) {
        return y;  /* Implicit conversion to 128-bit */
    }
    
    /* Ternary operator with mixed types */
    __int128 result = (x > 1000) ? CONST_A : y;
    
    /* Compare with different constant types */
    if (result == (__int128)y) {
        return x;
    }
    
    return result;
}

/* Test 5: Overflow operations */
__int128 test_overflow_ops(__int128 a, __int128 b) {
    __int128 sum, product;
    int overflow;
    
    /* Overflow addition - may trigger internal comparisons */
    overflow = __builtin_add_overflow(a, b, &sum);
    if (overflow) {
        return CONST_D;  /* Return -1 on overflow */
    }
    
    /* Overflow multiplication */
    overflow = __builtin_mul_overflow(a, b, &product);
    if (overflow) {
        return CONST_E;  /* Return max on overflow */
    }
    
    /* Compare results with constants */
    if (sum < CONST_A && product > CONST_B) {
        return sum + product;
    }
    
    return sum;
}

/* Test 6: Dead code with constant comparisons (still processed by compiler) */
void dead_code_paths(void) {
    if (0) {  /* Dead code, but compiler may still analyze constants */
        /* Multiple comparisons that should trigger double_int::cmp */
        if (CONST_A < CONST_B) {
            /* This block intentionally left empty */
        }
        if (CONST_C > CONST_A) {
            /* This block intentionally left empty */
        }
        if (((__int128)0x1ULL << 120) < CONST_E) {
            /* This block intentionally left empty */
        }
    }
}

/* Test 7: Switch statement with 128-bit values (via conversion) */
int test_switch_conversion(__int128 x) {
    /* Convert to 64-bit for switch (triggers range analysis) */
    long long val = (long long)(x >> 64);  /* Use high word */
    
    switch (val) {
        case 0x123456789ABCDEF0LL:  /* Matches HIGH_A */
            return 1;
        case 0x123456789ABCDEF1LL:  /* Matches HIGH_C */
            return 2;
        case 0x7FFFFFFFFFFFFFFFLL:  /* Matches high of CONST_E */
            return 3;
        default:
            return 0;
    }
}

/* Main function that exercises all tests */
int main(void) {
    __int128 result = 0;
    
    /* Initialize with values that will trigger different comparison paths */
    __int128 test_val1 = CONST_A;
    __int128 test_val2 = CONST_B;
    __int128 test_val3 = CONST_C;
    
    /* Test 1: Range analysis */
    result += test_range_analysis(test_val1, test_val2);
    
    /* Test 2: Loop comparisons (small range to avoid long execution) */
    result += test_loop_comparisons(CONST_A, CONST_A + 10);
    
    /* Test 3: Bitwise operations */
    result += test_bitwise_ops(test_val3);
    
    /* Test 4: Mixed comparisons */
    result += test_mixed_comparisons(test_val1, 0x12345678ULL);
    
    /* Test 5: Overflow operations */
    result += test_overflow_ops(1000, 2000);
    
    /* Test 6: Dead code (no effect on result) */
    dead_code_paths();
    
    /* Test 7: Switch conversion */
    int switch_result = test_switch_conversion(test_val1);
    result += switch_result;
    
    /* Also process global array */
    for (size_t i = 0; i < sizeof(global_array)/sizeof(global_array[0]); i++) {
        result += global_array[i];
    }
    
    /* Print a simple checksum (using 64-bit parts for portability) */
    unsigned long long high = (unsigned long long)(result >> 64);
    unsigned long long low = (unsigned long long)result;
    
    printf("Result checksum: 0x%016llx%016llx\n", high, low);
    printf("Test completed successfully.\n");
    
    return 0;
}
