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

#define CONST_D_HIGH 0x0000000000000000ULL  /* Small high word */
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

/* Structure with 128-bit fields */
struct wide_struct {
    __int128 field1;
    __int128 field2;
    unsigned long long normal_field;
};

/* Global structure with 128-bit initializers */
static struct wide_struct global_struct = {
    .field1 = CONST_A,
    .field2 = CONST_B,
    .normal_field = 0xDEADBEEF
};

/* Test 1: Function with 128-bit comparisons for range analysis */
__int128 test_range_analysis(__int128 x, __int128 y) {
    /* Comparisons that should trigger double_int::cmp in VRP */
    if (x > CONST_A) {
        if (y < CONST_B) {
            return x - y;
        } else if (y == CONST_C) {
            return x + y;
        }
    } else if (x == CONST_D) {
        return y << 2;
    }
    
    /* Mixed-type comparisons */
    if (x > (unsigned long long)0xFFFFFFFFULL) {
        return y >> 4;
    }
    
    /* Ternary with 128-bit constants */
    return (x > 0) ? CONST_E : CONST_D;
}

/* Test 2: Loop with 128-bit induction variable */
__int128 test_loop_comparisons(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop with 128-bit bounds - compiler must analyze range */
    for (__int128 i = start; i < end; i += (end - start) / 100) {
        /* Comparisons in loop condition */
        if (i > CONST_A) {
            sum += i;
        } else if (i < CONST_D) {
            sum -= i;
        }
        
        /* Bitwise operations crossing word boundaries */
        __int128 masked = i & (((__int128)0xFFFFFFFFULL << 64) | 0xFFFFFFFFULL);
        sum ^= masked;
    }
    
    return sum;
}

/* Test 3: Overflow operations with 128-bit integers */
int test_overflow_ops(__int128 a, __int128 b) {
    __int128 result;
    int overflow;
    
    /* These may trigger double_int comparisons internally */
    overflow = __builtin_add_overflow(a, b, &result);
    if (overflow) return -1;
    
    overflow = __builtin_mul_overflow(a, CONST_A, &result);
    if (overflow) return -2;
    
    overflow = __builtin_sub_overflow(b, CONST_B, &result);
    if (overflow) return -3;
    
    return 0;
}

/* Test 4: Shift operations that cross word boundaries */
__int128 test_shifts(__int128 value) {
    __int128 result = 0;
    
    /* Left shifts moving bits from low to high word */
    result |= value << 64;    /* All bits move to high word */
    result |= value << 96;    /* Partial move to high word */
    result |= value << 127;   /* Single bit to MSB */
    
    /* Right shifts on negative values (arithmetic shift) */
    __int128 negative = -value;
    result ^= negative >> 64;
    result ^= negative >> 120;
    
    /* Right shift on large positive */
    result |= CONST_E >> 1;
    result |= CONST_E >> 64;
    
    return result;
}

/* Test 5: Dead code with 128-bit constant comparisons */
/* These comparisons should still be evaluated during early optimization passes */
static __int128 dead_code_test(void) {
    if (0) {  /* Dead code, but constants may still be compared */
        /* Cases designed to hit all branches of double_int::cmp */
        
        /* High words equal, low words differ */
        if (CONST_A < CONST_B) { /* Should be true: same high, A.low < B.low */
            return 1;
        }
        
        /* High words differ */
        if (CONST_A < CONST_C) { /* Should be true: A.high < C.high */
            return 2;
        }
        
        if (CONST_C > CONST_A) { /* Should be true: C.high > A.high */
            return 3;
        }
        
        /* Edge cases */
        if (CONST_D < CONST_E) { /* Should be true: D.high < E.high */
            return 4;
        }
        
        if (CONST_E > CONST_D) { /* Should be true: E.high > D.high */
            return 5;
        }
    }
    
    return 0;
}

/* Test 6: Static assertions with 128-bit constants */
/* Force compile-time evaluation of comparisons */
#define STATIC_ASSERT_128(cond) _Static_assert(cond, "128-bit assertion failed")

/* These should trigger double_int::cmp during compilation */
STATIC_ASSERT_128(CONST_A != CONST_B);  /* Same high, different low */
STATIC_ASSERT_128(CONST_A < CONST_C);   /* Different high */
STATIC_ASSERT_128(CONST_D < CONST_E);   /* Edge case comparison */
STATIC_ASSERT_128((CONST_A >> 64) == CONST_A_HIGH);  /* Shift comparison */

/* Test 7: Switch statement with 128-bit values (via hashing) */
unsigned test_switch(__int128 key) {
    /* Convert 128-bit to 64-bit for switch, but comparisons happen before conversion */
    unsigned long long hash = (unsigned long long)(key >> 64) ^ (unsigned long long)key;
    
    switch (hash) {
        case (unsigned long long)(CONST_A >> 64) ^ (unsigned long long)CONST_A:
            return 1;
        case (unsigned long long)(CONST_B >> 64) ^ (unsigned long long)CONST_B:
            return 2;
        case (unsigned long long)(CONST_C >> 64) ^ (unsigned long long)CONST_C:
            return 3;
        default:
            return 0;
    }
}

/* Test 8: Array operations forcing constant folding */
__int128 test_array_ops(void) {
    __int128 sum = 0;
    
    /* Compute sum of global array - forces constant evaluation */
    for (int i = 0; i < (int)(sizeof(global_array) / sizeof(global_array[0])); i++) {
        /* Mix in some comparisons */
        if (global_array[i] > CONST_A) {
            sum += global_array[i];
        } else if (global_array[i] < CONST_D) {
            sum -= global_array[i];
        } else {
            sum ^= global_array[i];
        }
    }
    
    return sum;
}

/* Main function that exercises all tests */
int main(void) {
    __int128 result = 0;
    
    /* Initialize with values that will exercise different comparison paths */
    __int128 test_val1 = CONST_A;
    __int128 test_val2 = CONST_B;
    __int128 test_val3 = CONST_C;
    
    /* Test 1: Range analysis */
    result += test_range_analysis(test_val1, test_val2);
    
    /* Test 2: Loop comparisons */
    result += test_loop_comparisons(CONST_D, CONST_E);
    
    /* Test 3: Overflow operations */
    int overflow_result = test_overflow_ops(test_val1, test_val2);
    result += overflow_result;
    
    /* Test 4: Shift operations */
    result ^= test_shifts(test_val3);
    
    /* Test 5: Dead code (should return 0) */
    result += dead_code_test();
    
    /* Test 7: Switch statement */
    unsigned switch_result = test_switch(test_val1);
    result += switch_result;
    
    /* Test 8: Array operations */
    result += test_array_ops();
    
    /* Also test structure field comparisons */
    if (global_struct.field1 > global_struct.field2) {
        result += 1000;
    }
    
    /* Mixed-type comparison */
    if (test_val1 > 0xFFFFFFFFULL) {
        result += 2000;
    }
    
    /* Print a simple checksum to verify execution */
    unsigned long long checksum = (unsigned long long)(result >> 64) + (unsigned long long)result;
    printf("Checksum: %llu\n", checksum);
    
    return 0;
}
