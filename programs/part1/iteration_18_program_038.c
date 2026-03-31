/* double_int_test.c - Test program to trigger double_int::cmp coverage */
#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Large 128-bit constants with varying high/low word patterns */
#define HIGH_A 0x123456789ABCDEF0ULL
#define LOW_A  0xFEDCBA9876543210ULL
#define HIGH_B 0x123456789ABCDEF0ULL  /* Same high as A */
#define LOW_B  0xFEDCBA9876543211ULL  /* Different low */
#define HIGH_C 0x123456789ABCDEF1ULL  /* Different high */
#define LOW_C  0xFEDCBA9876543210ULL

/* Construct 128-bit constants */
static const __int128 CONST_A = ((__int128)HIGH_A << 64) | LOW_A;
static const __int128 CONST_B = ((__int128)HIGH_B << 64) | LOW_B;
static const __int128 CONST_C = ((__int128)HIGH_C << 64) | LOW_C;
static const __int128 CONST_D = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL; /* -1 */
static const __int128 CONST_E = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL; /* Max signed */

/* Global arrays with 128-bit constants - forces compile-time evaluation */
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

/* Static assertions to force compile-time comparisons */
_Static_assert(CONST_A < CONST_C, "Compile-time comparison 1");
_Static_assert(CONST_B > CONST_A, "Compile-time comparison 2");
_Static_assert(CONST_D < CONST_E, "Compile-time comparison 3");

/* Function 1: Range analysis with comparisons in both high and low words */
__int128 range_compare(__int128 x, __int128 y) {
    /* These comparisons should trigger double_int::cmp in VRP */
    if (x < CONST_A) {
        return x + 1;
    }
    if (x > CONST_B) {
        return x - 1;
    }
    if (y < CONST_C) {
        return y + CONST_A;
    }
    return x * y;
}

/* Function 2: Mixed-type comparisons and conversions */
__int128 mixed_type_ops(unsigned long long ull_val, __int128 i128_val) {
    /* Force promotion and comparison */
    __int128 result = 0;
    
    /* Compare __int128 with unsigned long long */
    if (i128_val < ull_val) {
        result = (__int128)ull_val - i128_val;
    } else {
        result = i128_val - (__int128)ull_val;
    }
    
    /* Ternary with mixed types */
    __int128 temp = (ull_val > 1000) ? CONST_A : CONST_B;
    
    /* Compare results */
    if (result < temp) {
        return result;
    }
    return temp;
}

/* Function 3: Bitwise operations crossing word boundaries */
__int128 bitwise_cross_boundary(__int128 val) {
    /* Masks targeting specific words */
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    __int128 mask_low  = 0xFFFFFFFFFFFFFFFFULL;
    
    /* Operations that affect both words */
    __int128 shifted = val << 65;  /* Shift into high word */
    __int128 masked = val & mask_high;
    __int128 combined = (val & mask_high) | (val & mask_low);
    
    /* Comparisons after bit manipulation */
    if (shifted < CONST_A) {
        return masked;
    }
    if (combined > CONST_B) {
        return shifted;
    }
    return val;
}

/* Function 4: Arithmetic with overflow checking */
__int128 arithmetic_with_overflow(__int128 a, __int128 b) {
    __int128 sum, product;
    int overflow;
    
    /* Overflow addition - may trigger internal comparisons */
    overflow = __builtin_add_overflow(a, b, &sum);
    if (overflow) {
        /* Compare with extreme values */
        if (a > 0 && b > 0 && sum < a) {
            return CONST_E;  /* Near max */
        }
        if (a < 0 && b < 0 && sum > a) {
            return CONST_D;  /* Near min */
        }
    }
    
    /* Overflow multiplication */
    overflow = __builtin_mul_overflow(a, b, &product);
    if (overflow) {
        /* Comparisons for overflow analysis */
        if (a > CONST_A && b > CONST_B) {
            return CONST_E;
        }
        if (a < CONST_D && b < CONST_D) {
            return CONST_D;
        }
    }
    
    return overflow ? (a / 2 + b / 2) : product;
}

/* Function 5: Loop with __int128 induction variable */
__int128 loop_with_128bit_iv(__int128 start, __int128 end) {
    __int128 sum = 0;
    __int128 i;
    
    /* Loop bound comparisons should trigger double_int::cmp */
    if (start >= end) {
        return 0;
    }
    
    /* Loop with 128-bit counter */
    for (i = start; i < end; i = i + 1) {
        /* Break early to avoid long runtime */
        if (i - start > 100) {
            break;
        }
        sum += i;
        
        /* Additional comparison inside loop */
        if (i < CONST_A) {
            sum += 1;
        }
        if (i > CONST_B) {
            sum -= 1;
        }
    }
    
    return sum;
}

/* Function 6: Dead code with constant comparisons */
void dead_code_comparisons(void) {
    /* Dead code that still gets parsed and potentially constant-folded */
    if (0) {
        /* These comparisons should be evaluated during compilation */
        if (CONST_A < CONST_B) {
            __builtin_unreachable();
        }
        if (CONST_C > CONST_A) {
            __builtin_unreachable();
        }
        if (((__int128)0x8000000000000000ULL << 64) < 0) {
            __builtin_unreachable();
        }
    }
    
    /* Another dead path */
    if (__builtin_constant_p(CONST_A)) {
        /* This should always be true, but creates comparison context */
        volatile int dummy = (CONST_A == CONST_A);
        (void)dummy;
    }
}

/* Function 7: Switch statement with large constants (simulated with if-else) */
int switch_like_comparison(__int128 val) {
    /* Simulate switch with large constants */
    if (val == CONST_A) {
        return 1;
    } else if (val == CONST_B) {
        return 2;
    } else if (val == CONST_C) {
        return 3;
    } else if (val == CONST_D) {
        return 4;
    } else if (val == CONST_E) {
        return 5;
    }
    return 0;
}

/* Main function that exercises all test cases */
int main(void) {
    __int128 result = 0;
    __int128 temp;
    
    printf("Testing 128-bit comparisons for double_int::cmp coverage\n");
    
    /* 1. Test range comparisons */
    temp = range_compare(CONST_A - 1, CONST_B + 1);
    result += temp;
    
    /* 2. Test mixed type operations */
    temp = mixed_type_ops(0xFFFFFFFFFFFFFFFFULL, CONST_A);
    result += temp;
    
    /* 3. Test bitwise operations */
    temp = bitwise_cross_boundary(CONST_C);
    result += temp;
    
    /* 4. Test arithmetic with overflow */
    temp = arithmetic_with_overflow(CONST_A / 2, CONST_B / 2);
    result += temp;
    
    /* 5. Test loop with 128-bit induction variable */
    temp = loop_with_128bit_iv(CONST_A - 50, CONST_A + 50);
    result += temp;
    
    /* 6. Force dead code evaluation */
    dead_code_comparisons();
    
    /* 7. Test switch-like comparisons */
    int code = switch_like_comparison(CONST_B);
    result += code;
    
    /* 8. Process global array */
    for (size_t i = 0; i < sizeof(global_array)/sizeof(global_array[0]); i++) {
        result += global_array[i];
    }
    
    /* 9. Additional direct comparisons */
    if (CONST_A < CONST_B) {
        result += 1;
    }
    if (CONST_C > CONST_A) {
        result += 2;
    }
    if (CONST_D < CONST_E) {
        result += 3;
    }
    
    /* 10. Shift operations that cross word boundaries */
    __int128 shifted = CONST_A << 96;  /* Most bits in high word */
    if (shifted < 0) {
        result += shifted;
    } else {
        result -= shifted;
    }
    
    /* Print a simple checksum (just lower 64 bits for simplicity) */
    unsigned long long checksum = (unsigned long long)result + 
                                  (unsigned long long)(result >> 64);
    printf("Result checksum: 0x%016llx\n", checksum);
    
    return 0;
}
