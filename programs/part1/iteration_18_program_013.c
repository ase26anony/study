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
static const __int128 CONST_D = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0ULL;
static const __int128 CONST_E = ((__int128)0x8000000000000000ULL << 64) | 0x0ULL; /* Min signed */

/* Global arrays with 128-bit constants - forces compile-time evaluation */
static const __int128 global_array[] = {
    CONST_A,
    CONST_B,
    CONST_C,
    CONST_D,
    CONST_E,
    -CONST_A,
    -CONST_B,
    ((__int128)0x0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL,
};

/* Test 1: Range analysis with comparisons */
__int128 test_range_analysis(__int128 x, __int128 y) {
    /* Comparisons that should trigger double_int::cmp in VRP */
    if (x < CONST_A) {
        if (y > CONST_B) {
            return x + y;
        }
    }
    
    /* Compare with different high words */
    if (x >= CONST_C) {
        return x - CONST_A;
    }
    
    /* Compare with equal high words but different low words */
    if (x <= CONST_B && x >= CONST_A) {
        return x & CONST_D;  /* Mask high word */
    }
    
    return x ^ y;
}

/* Test 2: Loop with 128-bit induction variable */
__int128 test_loop_comparisons(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop with 128-bit bounds - compiler must compare induction variable */
    for (__int128 i = start; i < end; i += ((__int128)1 << 32)) {
        /* Cross-word boundary operations */
        sum += i & CONST_D;  /* Only high word */
        sum += i & ~CONST_D; /* Only low word */
        
        /* Comparison in loop condition */
        if (i > CONST_E) {
            sum >>= 4;  /* Arithmetic shift on signed */
        }
    }
    
    return sum;
}

/* Test 3: Overflow checking with 128-bit values */
int test_overflow_ops(__int128 a, __int128 b, __int128 *result) {
    __int128 sum, prod;
    
    /* Builtin overflow checks may use double_int comparisons internally */
    if (__builtin_add_overflow(a, b, &sum)) {
        return -1;
    }
    
    if (__builtin_mul_overflow(a, b, &prod)) {
        return -2;
    }
    
    /* Compare results with constants */
    if (sum > CONST_A && prod < CONST_C) {
        *result = sum ^ prod;
        return 1;
    }
    
    *result = sum + prod;
    return 0;
}

/* Test 4: Shift operations crossing word boundaries */
__int128 test_cross_word_shifts(__int128 val) {
    __int128 result = 0;
    
    /* Left shift moving bits from low to high word */
    result |= (val << 65) & CONST_D;  /* High word only */
    result |= (val << 32) & ~CONST_D; /* Low word only */
    
    /* Right shift on negative value (arithmetic shift) */
    if (val < 0) {
        result |= (val >> 96);  /* Shift into low word */
    }
    
    /* Compare shifted values */
    __int128 shifted = val << 72;
    if (shifted > CONST_B) {
        result ^= 0x5555;
    }
    
    return result;
}

/* Test 5: Mixed-type comparisons and conversions */
__int128 test_mixed_type_comparisons(__int128 a, unsigned long long b) {
    /* Compare 128-bit with 64-bit - requires promotion */
    if (a > b) {
        /* Ternary with mixed types */
        __int128 temp = (b % 2) ? a : (__int128)b;
        
        /* Compare with constant */
        if (temp < CONST_A) {
            return temp | CONST_D;
        }
    }
    
    /* Compare with different constant */
    if (a <= CONST_C) {
        return a & (__int128)b;
    }
    
    return a ^ ((__int128)b << 64);
}

/* Test 6: Dead code with constant comparisons (still evaluated by compiler) */
void dead_code_comparisons(void) {
    /* These comparisons should be evaluated at compile time */
    if (0) {  /* Dead code, but constants still processed */
        if (CONST_A < CONST_B) {
            /* High words equal, low words differ */
            volatile int dummy = 1;
            (void)dummy;
        }
        
        if (CONST_B < CONST_C) {
            /* High words differ */
            volatile int dummy = 2;
            (void)dummy;
        }
        
        if (CONST_D > CONST_E) {
            /* Both high and low differ */
            volatile int dummy = 3;
            (void)dummy;
        }
    }
}

/* Test 7: Static assertions with 128-bit comparisons */
#define STATIC_ASSERT_CONST(cond) _Static_assert(cond, "128-bit constant assertion")

/* Force compile-time evaluation of 128-bit comparisons */
STATIC_ASSERT_CONST(CONST_A != CONST_B);  /* Same high, different low */
STATIC_ASSERT_CONST(CONST_A < CONST_C);   /* Different high */
STATIC_ASSERT_CONST(CONST_D > CONST_E);   /* Both words differ */
STATIC_ASSERT_CONST((CONST_A >> 64) == HIGH_A);  /* Extract high word */

/* Test 8: Preprocessor conditions with builtin constant checks */
#if defined(__GNUC__) && __GNUC__ >= 4
/* These may force early constant evaluation */
#define CHECK_CONSTANT(expr) __builtin_constant_p(expr)
#else
#define CHECK_CONSTANT(expr) 0
#endif

/* Test 9: Array operations forcing constant folding */
__int128 process_global_array(void) {
    __int128 sum = 0;
    const size_t count = sizeof(global_array) / sizeof(global_array[0]);
    
    for (size_t i = 0; i < count; i++) {
        /* Compare array elements with constants */
        if (global_array[i] > CONST_A) {
            sum += global_array[i] & CONST_D;
        } else if (global_array[i] < CONST_E) {
            sum += global_array[i] | CONST_D;
        } else {
            sum ^= global_array[i];
        }
    }
    
    return sum;
}

int main(void) {
    /* Initialize with values that will exercise different comparison paths */
    __int128 test_val1 = CONST_A - 1;
    __int128 test_val2 = CONST_B + 1;
    __int128 test_val3 = CONST_C;
    __int128 test_val4 = -CONST_A;
    
    /* Run tests */
    __int128 result1 = test_range_analysis(test_val1, test_val2);
    __int128 result2 = test_loop_comparisons(test_val4, test_val3);
    __int128 result3 = test_cross_word_shifts(test_val1);
    __int128 result4 = test_mixed_type_comparisons(test_val2, 0x123456789ABCDEFULL);
    
    __int128 overflow_result;
    int overflow_ret = test_overflow_ops(test_val1, test_val2, &overflow_result);
    
    /* Process global array */
    __int128 array_result = process_global_array();
    
    /* Call dead code function (compiler still processes constants) */
    dead_code_comparisons();
    
    /* Combine results into a simple checksum */
    __int128 final_result = result1 ^ result2 ^ result3 ^ result4 ^ 
                           overflow_result ^ array_result;
    
    /* Print low 64 bits as hex for verification */
    unsigned long long low = (unsigned long long)final_result;
    unsigned long long high = (unsigned long long)(final_result >> 64);
    
    printf("Result: 0x%016llx%016llx\n", high, low);
    printf("Overflow test returned: %d\n", overflow_ret);
    
    /* Additional compile-time checks */
    if (CHECK_CONSTANT(CONST_A < CONST_B)) {
        printf("Constant folding detected\n");
    }
    
    return 0;
}
