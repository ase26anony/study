/* test_double_int_comparison.c
 * Designed to trigger GCC's internal double_int comparison logic
 * for 128-bit integer operations during constant folding and optimization.
 */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Define 128-bit constants that cross 64-bit boundaries */
#define HIGH_BIT_64   0x8000000000000000ULL
#define MAX_64        0xFFFFFFFFFFFFFFFFULL
#define MID_128       0x123456789ABCDEF0ULL

/* Force compile-time evaluation with static assertions */
_Static_assert(((__int128)HIGH_BIT_64 << 64) > 0, 
               "High-bit shift should be positive");
_Static_assert(((__int128)MAX_64) < ((__int128)MAX_64 << 64), 
               "128-bit comparison with high word difference");

/* Function to prevent dead code elimination */
volatile __int128 global_accumulator = 0;

/* Range analysis with __int128 induction variables */
void range_analysis_test(void) {
    /* Loop with __int128 induction variable crossing 64-bit boundary */
    for (__int128 i = HIGH_BIT_64 - 100; i < HIGH_BIT_64 + 100; i++) {
        __int128 j = i * 3;
        if (j < 0) {
            global_accumulator += j;
        } else {
            global_accumulator -= j;
        }
    }
    
    /* Value range propagation with overflow checks */
    __int128 x = HIGH_BIT_64;
    for (int k = 0; k < 1000; k++) {
        __int128 old_x = x;
        int overflow = __builtin_add_overflow(x, HIGH_BIT_64, &x);
        
        /* Force comparison of values with different high words */
        if (overflow || x < old_x) {
            global_accumulator ^= x;
        }
    }
}

/* Mixed-precision operations triggering conversions */
__int128 mixed_precision_ops(unsigned long long a, long long b) {
    __int128 wide_a = a;
    __int128 wide_b = b;
    
    /* Comparisons between different types */
    if (wide_a > (__int128)b) {
        wide_a = wide_a * 2;
    }
    
    /* Ternary operator with mixed types */
    __int128 result = (a > b) ? (__int128)a : (__int128)(-b);
    
    /* Bitwise operations crossing 64-bit boundary */
    result = (result << 65) | (result >> 63);
    
    return result;
}

/* Constant folding boundaries with switch statement */
int constant_folding_switch(__int128 value) {
    /* Switch with __int128 case labels (compile-time constants) */
    switch (value) {
        case ((__int128)0x7FFFFFFFFFFFFFFFULL << 64):
            return 1;
        case ((__int128)HIGH_BIT_64 << 64):
            return 2;
        case ((__int128)MAX_64 << 64) | MAX_64:
            return 3;
        case -((__int128)HIGH_BIT_64 << 64):
            return 4;
        default:
            return 0;
    }
}

/* Array operations for optimizer to work on */
void array_operations(void) {
    __int128 arr[8];
    unsigned __int128 uarr[8];
    
    /* Initialize with values that exercise high/low word comparisons */
    for (int i = 0; i < 8; i++) {
        arr[i] = ((__int128)i << 64) | (MID_128 + i);
        uarr[i] = ((unsigned __int128)i << 64) | (MAX_64 - i);
    }
    
    /* Comparisons that will exercise the uncovered lines */
    for (int i = 0; i < 7; i++) {
        /* Compare values where high words differ */
        if (arr[i] < arr[i + 1]) {
            global_accumulator += arr[i];
        }
        
        /* Compare values where high words are equal but low words differ */
        __int128 same_high = ((__int128)i << 64) | (MID_128 + 100);
        __int128 same_high2 = ((__int128)i << 64) | (MID_128 + 200);
        if (same_high < same_high2) {
            global_accumulator ^= same_high;
        }
        
        /* Unsigned comparisons */
        if (uarr[i] > uarr[i + 1]) {
            global_accumulator -= (__int128)uarr[i];
        }
    }
    
    /* Boundary value comparisons */
    __int128 max_signed = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | MAX_64;
    __int128 min_signed = ((__int128)HIGH_BIT_64 << 64);
    unsigned __int128 max_unsigned = ((unsigned __int128)MAX_64 << 64) | MAX_64;
    
    if (max_signed > min_signed) {
        global_accumulator += max_signed;
    }
    
    if ((unsigned __int128)max_unsigned > (unsigned __int128)max_signed) {
        global_accumulator -= (__int128)max_unsigned;
    }
}

/* Built-in function usage */
void builtin_operations(void) {
    __int128 x = ((__int128)MID_128 << 64) | MID_128;
    
    /* Use builtins that may trigger wide integer operations */
    for (int i = 0; i < 64; i++) {
        __int128 shifted = x << i;
        
        /* Count leading zeros on high and low parts */
        int clz_high = __builtin_clzll((unsigned long long)(shifted >> 64));
        int clz_low = __builtin_clzll((unsigned long long)shifted);
        
        if (clz_high != clz_low) {
            global_accumulator ^= shifted;
        }
        
        /* Byte swap simulation */
        __int128 swapped = 0;
        for (int j = 0; j < 16; j++) {
            unsigned char byte = ((unsigned char *)&shifted)[j];
            ((unsigned char *)&swapped)[15 - j] = byte;
        }
        
        if (swapped != shifted) {
            global_accumulator += swapped;
        }
    }
}

/* Variadic function to trigger conversions */
void variadic_conversions(__int128 a, unsigned __int128 b) {
    /* Force conversions through printf-like usage */
    printf("Testing 128-bit values:\n");
    printf("  Signed high: %llx, low: %llx\n", 
           (unsigned long long)(a >> 64), (unsigned long long)a);
    printf("  Unsigned high: %llx, low: %llx\n", 
           (unsigned long long)(b >> 64), (unsigned long long)b);
    
    /* Comparisons in variadic context */
    if (__builtin_expect(a < b, 0)) {
        printf("  a < b (unexpected)\n");
    } else {
        printf("  a >= b (expected)\n");
    }
}

int main(void) {
    printf("Starting 128-bit comparison tests...\n");
    
    /* Test 1: Range analysis */
    range_analysis_test();
    
    /* Test 2: Mixed precision operations */
    __int128 mixed_result = mixed_precision_ops(MAX_64, -HIGH_BIT_64);
    global_accumulator += mixed_result;
    
    /* Test 3: Constant folding boundaries */
    __int128 test_values[] = {
        ((__int128)0x7FFFFFFFFFFFFFFFULL << 64),
        ((__int128)HIGH_BIT_64 << 64),
        ((__int128)MAX_64 << 64) | MAX_64,
        -((__int128)HIGH_BIT_64 << 64),
        0
    };
    
    for (int i = 0; i < 4; i++) {
        int switch_result = constant_folding_switch(test_values[i]);
        global_accumulator += switch_result;
    }
    
    /* Test 4: Array operations */
    array_operations();
    
    /* Test 5: Built-in operations */
    builtin_operations();
    
    /* Test 6: Variadic conversions */
    variadic_conversions(
        ((__int128)HIGH_BIT_64 << 64) | MID_128,
        ((unsigned __int128)MAX_64 << 64) | MAX_64
    );
    
    /* Final checksum to prevent optimization */
    printf("Accumulator checksum: %llx%llx\n",
           (unsigned long long)(global_accumulator >> 64),
           (unsigned long long)global_accumulator);
    
    return 0;
}
