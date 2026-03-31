#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large 128-bit constants with different high/low word patterns */
#define CONST_A_HIGH 0x123456789ABCDEF0ULL
#define CONST_A_LOW  0xFEDCBA9876543210ULL
#define CONST_A (((__int128)CONST_A_HIGH << 64) | CONST_A_LOW)

#define CONST_B_HIGH 0x123456789ABCDEF0ULL  /* Same high as A */
#define CONST_B_LOW  0xFEDCBA9876543211ULL  /* Different low */
#define CONST_B (((__int128)CONST_B_HIGH << 64) | CONST_B_LOW)

#define CONST_C_HIGH 0x123456789ABCDEF1ULL  /* Different high */
#define CONST_C_LOW  0x0000000000000000ULL
#define CONST_C (((__int128)CONST_C_HIGH << 64) | CONST_C_LOW)

#define CONST_D_HIGH 0x0000000000000000ULL  /* Zero high */
#define CONST_D_LOW  0xFFFFFFFFFFFFFFFFULL  /* Max low */
#define CONST_D (((__int128)CONST_D_HIGH << 64) | CONST_D_LOW)

#define CONST_E_HIGH 0xFFFFFFFFFFFFFFFFULL  /* Max high */
#define CONST_E_LOW  0x0000000000000000ULL
#define CONST_E (((__int128)CONST_E_HIGH << 64) | CONST_E_LOW)

/* Global arrays with 128-bit constants - forces compile-time evaluation */
static const __int128 global_array[] = {
    CONST_A,
    CONST_B,
    CONST_C,
    CONST_D,
    CONST_E,
    CONST_A + 1,
    CONST_B - 1,
    CONST_C * 2,
    CONST_D >> 1,
    CONST_E << 1
};

/* Static assertions that force compile-time comparisons */
_Static_assert(CONST_A < CONST_B, "A should be less than B (low word diff)");
_Static_assert(CONST_B < CONST_C, "B should be less than C (high word diff)");
_Static_assert(CONST_D < CONST_E, "D should be less than E");
_Static_assert(CONST_A != CONST_B, "A and B should be different");

/* Dead code with comparisons that compiler may evaluate during optimization */
static __int128 dead_code_comparisons(__int128 x) {
    if (0) {  /* Dead code, but compiler may still analyze constants */
        if (x < CONST_A) return CONST_A;
        if (x > CONST_B) return CONST_B;
        if (x == CONST_C) return CONST_D;
        if (x <= CONST_E) return CONST_E;
    }
    return x;
}

/* Function to test range analysis with 128-bit values */
__int128 range_analysis_test(__int128 x, __int128 y) {
    /* Comparisons that should trigger VRP analysis */
    if (x < CONST_A && y > CONST_B) {
        return x + y;
    }
    
    if (x >= CONST_C || y <= CONST_D) {
        return x - y;
    }
    
    /* Ternary with mixed types */
    unsigned long long ull = 0xFFFFFFFFFFFFFFFFULL;
    __int128 result = (x > 0) ? ((__int128)ull << 64) : CONST_E;
    
    return result;
}

/* Function with 128-bit loop induction variable */
__int128 loop_with_128bit_iv(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop with 128-bit comparison in condition */
    for (__int128 i = start; i < end; i += (end - start) / 100) {
        sum += i;
        
        /* Additional comparisons inside loop */
        if (i < CONST_A) sum += 1;
        if (i > CONST_B) sum += 2;
        if (i >= CONST_C && i <= CONST_E) sum += 3;
    }
    
    return sum;
}

/* Function testing overflow operations */
__int128 overflow_operations(__int128 a, __int128 b) {
    __int128 result;
    int overflow;
    
    /* Overflow checks that may use double_int comparisons */
    overflow = __builtin_add_overflow(a, b, &result);
    if (overflow) {
        /* Compare with boundary values */
        if (a > 0 && b > 0 && a > CONST_E - b) {
            return CONST_E;
        }
    }
    
    overflow = __builtin_mul_overflow(a, b, &result);
    if (overflow) {
        /* Complex comparison for overflow detection */
        if (a != 0 && b > CONST_E / a) {
            return (a > 0) ? CONST_E : -CONST_E;
        }
    }
    
    return result;
}

/* Function with bitwise operations crossing word boundaries */
__int128 bitwise_operations(__int128 x) {
    /* Masks targeting specific words */
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    
    /* Operations that cross word boundaries */
    __int128 shifted = x << 65;  /* Shift into high word */
    __int128 masked = x & mask_high;
    __int128 combined = (x & mask_low) | ((x & mask_high) >> 64);
    
    /* Comparisons after bit manipulation */
    if (shifted < CONST_A) {
        return masked;
    }
    
    if (combined > CONST_B) {
        return shifted;
    }
    
    /* Arithmetic right shift on negative value */
    __int128 neg = -x;
    __int128 arith_shifted = neg >> 96;  /* Large shift affecting high word */
    
    return arith_shifted;
}

/* Function with switch statement using 128-bit derived values */
int switch_with_128bit(__int128 x) {
    /* Convert to smaller range for switch */
    int selector = 0;
    
    if (x < CONST_A) selector = 1;
    else if (x >= CONST_A && x < CONST_B) selector = 2;
    else if (x >= CONST_B && x < CONST_C) selector = 3;
    else if (x >= CONST_C && x < CONST_D) selector = 4;
    else if (x >= CONST_D && x < CONST_E) selector = 5;
    else selector = 6;
    
    switch (selector) {
        case 1: return 100;
        case 2: return 200;
        case 3: return 300;
        case 4: return 400;
        case 5: return 500;
        case 6: return 600;
        default: return 0;
    }
}

/* Main test function */
int main() {
    __int128 test_values[] = {
        CONST_A,
        CONST_B,
        CONST_C,
        CONST_D,
        CONST_E,
        0,
        -1,
        ((__int128)1 << 127) - 1,  /* Max positive 128-bit */
        (__int128)1 << 127,        /* Min negative 128-bit */
    };
    
    __int128 total_sum = 0;
    
    /* Test various operations */
    for (size_t i = 0; i < sizeof(test_values)/sizeof(test_values[0]); i++) {
        __int128 x = test_values[i];
        
        /* Call all test functions */
        total_sum += range_analysis_test(x, CONST_B);
        total_sum += loop_with_128bit_iv(x, x + 1000);
        total_sum += overflow_operations(x, CONST_A);
        total_sum += bitwise_operations(x);
        total_sum += switch_with_128bit(x);
        
        /* Sum global array elements for compile-time constant usage */
        for (size_t j = 0; j < sizeof(global_array)/sizeof(global_array[0]); j++) {
            if (x < global_array[j]) {
                total_sum += 1;
            }
        }
    }
    
    /* Also test with computed values */
    __int128 computed = CONST_A + CONST_B;
    total_sum += computed;
    
    computed = CONST_C - CONST_D;
    total_sum += computed;
    
    computed = CONST_E >> 64;  /* Extract high word */
    total_sum += computed;
    
    computed = CONST_D << 32;  /* Partial cross-boundary shift */
    total_sum += computed;
    
    /* Print a simple checksum (using 64-bit parts for portability) */
    unsigned long long low = (unsigned long long)(total_sum & 0xFFFFFFFFFFFFFFFFULL);
    unsigned long long high = (unsigned long long)((total_sum >> 64) & 0xFFFFFFFFFFFFFFFFULL);
    
    printf("Checksum: high=0x%016llx low=0x%016llx\n", high, low);
    printf("Test completed successfully.\n");
    
    return 0;
}
