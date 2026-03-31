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
    CONST_A + 1,
    CONST_B - 1,
    CONST_C * 2,
    CONST_D >> 64,
    CONST_E << 1
};

/* Static assertions to force compile-time comparisons */
_Static_assert(CONST_A < CONST_B, "A should be less than B (same high, different low)");
_Static_assert(CONST_A < CONST_C, "A should be less than C (different high)");
_Static_assert(CONST_D < CONST_E, "D should be less than E");
_Static_assert(CONST_E > 0, "E should be positive");

/* Dead code with comparisons that GCC may evaluate during optimization */
static __int128 dead_code_comparisons(__int128 x) {
    if (0) {  /* Dead code, but compiler may still analyze constants */
        if (x < CONST_A) return CONST_B;
        if (x > CONST_C) return CONST_D;
        if (x == CONST_E) return 0;
    }
    return x;
}

/* Function to test range analysis with 128-bit values */
__int128 range_analysis_test(__int128 x, __int128 y) {
    /* Comparisons that should trigger double_int::cmp in VRP */
    if (x < CONST_A) {
        if (y > CONST_B) {
            return x + y;
        }
    }
    
    if (x > CONST_C && y < CONST_D) {
        return x - y;
    }
    
    /* Ternary operator with mixed types */
    unsigned long long ull = 0xFFFFFFFFFFFFFFFFULL;
    __int128 result = (x > 0) ? (__int128)ull : CONST_E;
    
    return result;
}

/* Loop with 128-bit induction variable */
__int128 loop_with_128bit_iv(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop that may trigger comparisons in loop optimization */
    for (__int128 i = start; i < end; i += (CONST_B - CONST_A)) {
        sum += i;
        
        /* Bitwise operations crossing word boundaries */
        __int128 masked = i & (((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x5555555555555555ULL);
        sum ^= masked;
    }
    
    return sum;
}

/* Function testing overflow operations */
__int128 overflow_test(__int128 a, __int128 b) {
    __int128 result;
    int overflow;
    
    /* These may trigger double_int comparisons internally */
    overflow = __builtin_add_overflow(a, b, &result);
    if (overflow) return CONST_A;
    
    overflow = __builtin_mul_overflow(a, CONST_B, &result);
    if (overflow) return CONST_C;
    
    return result;
}

/* Function with shift operations that cross word boundaries */
__int128 shift_operations(__int128 x) {
    /* Left shift moving bits from low to high word */
    __int128 left_shifted = x << 72;
    
    /* Right shift on negative value (arithmetic shift) */
    __int128 neg_val = -CONST_A;
    __int128 right_shifted = neg_val >> 80;
    
    /* Right shift on positive value */
    __int128 pos_shifted = CONST_E >> 96;
    
    return left_shifted + right_shifted + pos_shifted;
}

/* Mixed-type comparisons */
int mixed_type_comparisons(__int128 x, unsigned long long y) {
    /* Implicit conversion and comparison */
    if (x < y) return -1;
    if (x > (__int128)y) return 1;
    
    /* Comparison with different constant types */
    if (x == 0x7FFFFFFFFFFFFFFFLL) return 2;  /* 64-bit max signed */
    if (x == 0xFFFFFFFFFFFFFFFFULL) return 3; /* 64-bit max unsigned */
    
    return 0;
}

/* Switch statement with large constants (simulated with if-else) */
int switch_like_comparison(__int128 x) {
    if (x == CONST_A) return 1;
    else if (x == CONST_B) return 2;
    else if (x == CONST_C) return 3;
    else if (x == CONST_D) return 4;
    else if (x == CONST_E) return 5;
    else return 0;
}

/* Preprocessor condition with constant evaluation */
#if __builtin_constant_p(CONST_A < CONST_B)
/* This section will be compiled only if the compiler can evaluate
   CONST_A < CONST_B at compile time, which it should */
#define COMPILE_TIME_COMPARISON 1
#else
#define COMPILE_TIME_COMPARISON 0
#endif

/* Main function that exercises all test cases */
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
        (__int128)1 << 127,        /* Most negative 128-bit */
    };
    
    __int128 checksum = 0;
    
    /* Test range analysis */
    for (int i = 0; i < sizeof(test_values)/sizeof(test_values[0]); i++) {
        checksum += range_analysis_test(test_values[i], test_values[(i+1) % 8]);
    }
    
    /* Test loops with 128-bit bounds */
    checksum += loop_with_128bit_iv(CONST_D, CONST_A);
    
    /* Test overflow operations */
    checksum += overflow_test(CONST_A, CONST_B);
    checksum += overflow_test(CONST_E, 1);  /* Should overflow */
    
    /* Test shift operations */
    checksum += shift_operations(CONST_C);
    
    /* Test mixed-type comparisons */
    for (int i = 0; i < 4; i++) {
        checksum += mixed_type_comparisons(test_values[i], 0xFFFFFFFFFFFFFFFFULL);
    }
    
    /* Test switch-like comparisons */
    for (int i = 0; i < sizeof(test_values)/sizeof(test_values[0]); i++) {
        checksum += switch_like_comparison(test_values[i]);
    }
    
    /* Use global array to ensure constants are processed */
    for (int i = 0; i < sizeof(global_array)/sizeof(global_array[0]); i++) {
        checksum += global_array[i];
    }
    
    /* Print a simple result to verify the program runs */
    /* We'll print the low 64 bits of the checksum */
    unsigned long long low_part = (unsigned long long)checksum;
    printf("Checksum (low 64 bits): 0x%016llx\n", low_part);
    
    #if COMPILE_TIME_COMPARISON
    printf("Compile-time comparison was performed\n");
    #endif
    
    return 0;
}
