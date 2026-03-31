/* double_int_cmp_test.c - Test program to trigger double_int::cmp in GCC */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large 128-bit constants with varying high/low word patterns */
static const __int128 C1 = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
static const __int128 C2 = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543211ULL;
static const __int128 C3 = ((__int128)0x123456789ABCDEF1ULL << 64) | 0xFEDCBA9876543210ULL;
static const __int128 C4 = ((__int128)0x0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
static const __int128 C5 = ((__int128)0x1ULL << 64) | 0x0ULL;
static const __int128 C6 = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL; /* -1 */

/* Global arrays with 128-bit constants - forces constant folding */
static const __int128 global_array[] = {
    C1, C2, C3, C4, C5, C6,
    ((__int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAAULL,
    ((__int128)0xAAAAAAAAAAAAAAAAULL << 64) | 0x5555555555555555ULL,
    ((__int128)0x8000000000000000ULL << 64) | 0x0000000000000000ULL, /* Min signed */
    ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL, /* Max signed */
};

/* Static assertions to force compile-time comparisons */
_Static_assert(C1 < C2, "C1 should be less than C2 (low word diff)");
_Static_assert(C2 < C3, "C2 should be less than C3 (high word diff)");
_Static_assert(C4 < C5, "C4 should be less than C5 (high word zero vs one)");

/* Function 1: Range analysis with comparisons in both high and low words */
__int128 range_compare(__int128 a, __int128 b) {
    /* Comparisons that will be analyzed by VRP */
    if (a < C1) {
        return a + C4;
    } else if (a > C3) {
        return a - C4;
    } else if (a >= C1 && a <= C2) {
        /* High words equal, low words differ */
        return a | C4;
    }
    return a ^ b;
}

/* Function 2: Loop with 128-bit induction variable */
__int128 loop_with_128bit_iv(__int128 start, __int128 end) {
    __int128 sum = 0;
    /* Loop that may trigger range analysis */
    for (__int128 i = start; i < end; i += (C5 >> 64)) { /* Add 1 */
        sum += i;
        if (i == C1) {
            sum |= C2;
        }
    }
    return sum;
}

/* Function 3: Mixed-type comparisons and conversions */
__int128 mixed_type_comparisons(unsigned long long x, __int128 y) {
    __int128 result = 0;
    
    /* Compare __int128 with unsigned long long */
    if (y > (__int128)x) {
        result = y;
    } else {
        result = (__int128)x;
    }
    
    /* Ternary with mixed types */
    result = (x > 1000) ? C1 : C2;
    
    /* Compare with constant where high word matters */
    if (result < ((__int128)0x100000000ULL << 64)) {
        result &= C4; /* Clear high word */
    }
    
    return result;
}

/* Function 4: Bitwise operations crossing word boundaries */
__int128 cross_word_operations(__int128 a) {
    __int128 result = a;
    
    /* Left shift moving bits from low to high word */
    result = result << 65;
    
    /* Right shift on negative value (arithmetic shift) */
    __int128 neg = C6; /* -1 */
    result |= (neg >> 32);
    
    /* Mask operations targeting specific words */
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    
    result = (result & mask_high) | (a & mask_low);
    
    return result;
}

/* Function 5: Overflow checking with 128-bit values */
__int128 overflow_checks(__int128 a, __int128 b) {
    __int128 sum, product;
    int overflow;
    
    /* Addition overflow check */
    overflow = __builtin_add_overflow(a, b, &sum);
    if (overflow) {
        return C6; /* Return -1 on overflow */
    }
    
    /* Multiplication overflow check */
    overflow = __builtin_mul_overflow(a, b, &product);
    if (overflow) {
        return C6;
    }
    
    /* Compare results against large constants */
    if (sum < C1 && product > C2) {
        return sum ^ product;
    }
    
    return sum + product;
}

/* Function 6: Dead code with constant comparisons */
void dead_code_with_comparisons(void) {
    /* Dead code that still gets parsed and constant-folded */
    if (0) { /* Always false */
        /* These comparisons should still be evaluated during compilation */
        if (C1 < C2) {
            /* High words equal, low words differ */
            volatile __int128 dummy = C1;
            (void)dummy;
        }
        if (C2 < C3) {
            /* High words differ */
            volatile __int128 dummy = C2;
            (void)dummy;
        }
        if (((__int128)0x0ULL << 64) < ((__int128)0x1ULL << 64)) {
            /* Only high words differ, low words both 0 */
            volatile __int128 dummy = C4;
            (void)dummy;
        }
    }
}

/* Function 7: Switch statement with 128-bit cases (simulated) */
int switch_with_large_values(__int128 val) {
    /* GCC doesn't directly support __int128 in switch, but we can simulate */
    if (val == C1) return 1;
    if (val == C2) return 2;
    if (val == C3) return 3;
    if (val == C4) return 4;
    if (val == C5) return 5;
    return 0;
}

/* Main test driver */
int main(void) {
    __int128 result = 0;
    __int128 test_val;
    
    /* Initialize with a value that has specific high/low pattern */
    test_val = ((__int128)0x12345678ULL << 96) | 0x9ABCDEF012345678ULL;
    
    /* Test 1: Range comparison */
    result += range_compare(test_val, C1);
    
    /* Test 2: Loop with bounds that differ in high word */
    result += loop_with_128bit_iv(C4, C5); /* 0xFFFFFFFFFFFFFFFF to 0x10000000000000000 */
    
    /* Test 3: Mixed type operations */
    result += mixed_type_comparisons(0x123456789ABCDEF0ULL, test_val);
    
    /* Test 4: Cross-word operations */
    result += cross_word_operations(test_val);
    
    /* Test 5: Overflow checks */
    result += overflow_checks(C4, C5);
    
    /* Call dead code function (compiler still processes it) */
    dead_code_with_comparisons();
    
    /* Test 6: Switch-like comparisons */
    int switch_result = switch_with_large_values(C2);
    result += switch_result;
    
    /* Process global array to ensure constants are used */
    for (size_t i = 0; i < sizeof(global_array)/sizeof(global_array[0]); i++) {
        result ^= global_array[i];
    }
    
    /* Print a simple checksum to verify execution */
    /* Split 128-bit result into two 64-bit parts for printing */
    unsigned long long low = (unsigned long long)result;
    unsigned long long high = (unsigned long long)(result >> 64);
    
    printf("Result checksum: high=0x%016llx low=0x%016llx\n", high, low);
    printf("Test completed successfully.\n");
    
    return 0;
}
