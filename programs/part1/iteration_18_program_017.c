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
#define CONST_C_LOW  0xFEDCBA9876543210ULL
#define CONST_C (((__int128)CONST_C_HIGH << 64) | CONST_C_LOW)

#define CONST_D_HIGH 0x0000000000000000ULL  /* Small value */
#define CONST_D_LOW  0xFFFFFFFFFFFFFFFFULL
#define CONST_D (((__int128)CONST_D_HIGH << 64) | CONST_D_LOW)

#define CONST_E_HIGH 0xFFFFFFFFFFFFFFFFULL  /* Large negative */
#define CONST_E_LOW  0x0000000000000000ULL
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
    CONST_D >> 4,
    CONST_E << 2
};

/* Static assertions forcing compile-time comparisons */
_Static_assert(CONST_A < CONST_B, "A < B comparison");  /* High equal, low differs */
_Static_assert(CONST_B < CONST_C, "B < C comparison");  /* High differs */
_Static_assert(CONST_D > 0, "D > 0 comparison");        /* Mixed-type comparison */
_Static_assert(CONST_E < 0, "E < 0 comparison");        /* Negative comparison */

/* Function to test range analysis with 128-bit values */
__int128 range_analysis_test(__int128 x, __int128 y) {
    /* Comparisons that should trigger VRP analysis */
    if (x < CONST_A) {
        if (y > CONST_B) {
            return x + y;
        } else if (y < CONST_C) {
            return x - y;
        }
    } else if (x > CONST_C) {
        if (y < CONST_D) {
            return x * y;
        }
    }
    
    /* Ternary with mixed types */
    return (x < 1000ULL) ? (__int128)1000ULL : x;
}

/* Function with loop using 128-bit induction variable */
__int128 loop_with_128bit_iv(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop that may be analyzed by VRP */
    for (__int128 i = start; i < end; i += (end - start) / 100) {
        sum += i;
        
        /* Comparison inside loop */
        if (i > CONST_D && i < CONST_A) {
            sum += 1;
        }
    }
    
    return sum;
}

/* Function testing bitwise operations crossing word boundaries */
__int128 bitwise_operations(__int128 x) {
    /* Masks targeting specific words */
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    
    /* Operations that cross word boundaries */
    __int128 shifted = x << 65;  /* Moves bits from low to high word */
    __int128 arith_shifted = x >> 95;  /* Arithmetic shift */
    
    /* Comparisons after bitwise operations */
    if ((x & mask_high) > (CONST_A & mask_high)) {
        return shifted;
    } else if ((x & mask_low) < (CONST_B & mask_low)) {
        return arith_shifted;
    }
    
    return x ^ mask_high ^ mask_low;
}

/* Function using overflow builtins with 128-bit values */
int overflow_operations(__int128 a, __int128 b, __int128 *result) {
    __int128 sum, product;
    int overflow_sum, overflow_product;
    
    /* Overflow checks may use double_int comparisons internally */
    overflow_sum = __builtin_add_overflow(a, b, &sum);
    overflow_product = __builtin_mul_overflow(a, b, &product);
    
    /* Comparisons with large constants */
    if (!overflow_sum && sum > CONST_A && sum < CONST_C) {
        *result = sum;
        return 1;
    }
    
    if (!overflow_product && product < CONST_B) {
        *result = product;
        return 2;
    }
    
    *result = 0;
    return 0;
}

/* Dead code with comparisons that compiler may still evaluate */
void dead_code_with_comparisons(void) {
    if (0) {  /* Dead code, but compiler may still analyze constants */
        /* These comparisons should exercise all branches of double_int::cmp */
        volatile int cmp1 = (CONST_A < CONST_B) ? 1 : 0;  /* High equal, low differs */
        volatile int cmp2 = (CONST_B < CONST_C) ? 1 : 0;  /* High differs */
        volatile int cmp3 = (CONST_C > CONST_D) ? 1 : 0;  /* Both differ */
        volatile int cmp4 = (CONST_D == CONST_D) ? 1 : 0; /* Equal */
        
        /* Mixed-type comparisons */
        volatile int cmp5 = (CONST_A > 1000ULL) ? 1 : 0;
        volatile int cmp6 = (CONST_E < -1000LL) ? 1 : 0;
    }
}

/* Preprocessor conditions with constant evaluations */
#if ((__int128)0x123456789ABCDEF0ULL << 64) > ((__int128)0x123456789ABCDEF0ULL << 63)
/* This condition is always true and forces compile-time comparison */
#define PREPROC_COMPARE 1
#else
#define PREPROC_COMPARE 0
#endif

/* Main function that exercises all test cases */
int main(void) {
    __int128 test_values[] = {
        CONST_A,
        CONST_B,
        CONST_C,
        CONST_D,
        CONST_E,
        0,
        1000,
        -1000,
        ((__int128)1 << 127) - 1,  /* Max positive */
        (__int128)1 << 127         /* Min negative */
    };
    
    __int128 total_sum = 0;
    
    /* Process global array */
    for (size_t i = 0; i < sizeof(global_array)/sizeof(global_array[0]); i++) {
        total_sum += global_array[i];
    }
    
    /* Test range analysis */
    for (int i = 0; i < 5; i++) {
        total_sum += range_analysis_test(test_values[i], test_values[i+1]);
    }
    
    /* Test loops */
    total_sum += loop_with_128bit_iv(CONST_D, CONST_A);
    
    /* Test bitwise operations */
    for (int i = 0; i < 5; i++) {
        total_sum += bitwise_operations(test_values[i]);
    }
    
    /* Test overflow operations */
    __int128 overflow_result;
    for (int i = 0; i < 4; i++) {
        overflow_operations(test_values[i], test_values[i+1], &overflow_result);
        total_sum += overflow_result;
    }
    
    /* Call dead code function */
    dead_code_with_comparisons();
    
    /* Print a simple checksum (just low 64 bits for simplicity) */
    unsigned long long checksum = (unsigned long long)total_sum + 
                                  (unsigned long long)(total_sum >> 64);
    
    printf("Checksum: %llu\n", checksum);
    printf("Preprocessor compare result: %d\n", PREPROC_COMPARE);
    
    return 0;
}
