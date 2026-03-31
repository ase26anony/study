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

#define CONST_D_HIGH 0x0000000000000000ULL  /* Small high word */
#define CONST_D_LOW  0xFFFFFFFFFFFFFFFFULL  /* Max low word */
#define CONST_D (((__int128)CONST_D_HIGH << 64) | CONST_D_LOW)

#define CONST_E_HIGH 0xFFFFFFFFFFFFFFFFULL  /* Max high word */
#define CONST_E_LOW  0x0000000000000000ULL  /* Zero low word */
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

/* Structure with 128-bit fields */
struct wide_struct {
    __int128 field1;
    __int128 field2;
    unsigned long long normal_field;
};

/* Global structure with 128-bit constants */
static const struct wide_struct global_struct = {
    .field1 = CONST_A,
    .field2 = CONST_B,
    .normal_field = 0xDEADBEEF
};

/* Test 1: Function with 128-bit comparisons for VRP */
__int128 test_range_analysis(__int128 x, __int128 y) {
    /* Comparisons that should trigger double_int::cmp in VRP */
    if (x < CONST_A) {
        if (y > CONST_B) {
            return x + y;
        } else if (y == CONST_C) {
            return x - y;
        }
    } else if (x > CONST_C) {
        if (y < CONST_D) {
            return x * y;
        }
    }
    
    /* Ternary with mixed types */
    return (x > 0) ? (__int128)100 : (__int128)-100;
}

/* Test 2: Loop with 128-bit induction variable */
__int128 test_loop_comparisons(__int128 start, __int128 end) {
    __int128 sum = 0;
    
    /* Loop that compares 128-bit values */
    for (__int128 i = start; i < end; i += (end - start) / 100) {
        /* Force comparisons in loop condition */
        if (i < CONST_D) {
            sum += i;
        } else if (i > CONST_E) {
            sum -= i;
        } else {
            sum ^= i;
        }
    }
    
    return sum;
}

/* Test 3: Bitwise operations crossing word boundaries */
__int128 test_bitwise_ops(__int128 x) {
    /* Masks targeting specific words */
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    __int128 mask_mid = ((__int128)0x0000FFFFFFFF0000ULL << 64) | 0x0000FFFFFFFF0000ULL;
    
    /* Operations that require reasoning about both words */
    __int128 result = (x & mask_high) >> 64;
    result |= (x & mask_low) << 64;
    result ^= mask_mid;
    
    /* Shift operations that cross word boundaries */
    result = (result << 65) | (result >> 63);
    
    return result;
}

/* Test 4: Overflow checking with 128-bit values */
int test_overflow_ops(__int128 a, __int128 b, __int128 *sum, __int128 *prod) {
    /* These builtins may use double_int comparisons internally */
    int overflow_add = __builtin_add_overflow(a, b, sum);
    int overflow_mul = __builtin_mul_overflow(a, b, prod);
    
    return overflow_add || overflow_mul;
}

/* Test 5: Mixed-type comparisons and conversions */
long long test_mixed_comparisons(__int128 big_val, unsigned long long small_val) {
    /* Comparisons between different types force conversions */
    if (big_val < small_val) {
        return (long long)(big_val >> 64);
    } else if (big_val > (__int128)small_val * 1000) {
        return (long long)(big_val & 0xFFFFFFFFFFFFFFFFULL);
    }
    
    /* Ternary with mixed types */
    __int128 temp = (small_val > 100) ? CONST_A : CONST_B;
    return (long long)(temp % 1000000);
}

/* Test 6: Dead code with compile-time comparisons */
void dead_code_paths(void) {
    /* Dead code that still gets evaluated at compile time */
    if (0) {
        /* These comparisons should be evaluated during constant folding */
        static_assert(CONST_A != CONST_B, "Constants differ");
        static_assert(CONST_C > CONST_A, "C > A");
        static_assert(CONST_D < CONST_E, "D < E");
        
        /* Complex dead expressions */
        __int128 dead_var = CONST_A + CONST_B;
        if (dead_var > CONST_C && dead_var < CONST_E) {
            dead_var = dead_var * 2;
        }
    }
    
    /* Another dead branch with different comparisons */
    if (__builtin_constant_p(CONST_A)) {
        /* This branch might be taken during compilation */
        volatile int dummy = 1;
        (void)dummy;
    }
}

/* Test 7: Switch statement with 128-bit values (simulated) */
int test_switch_simulation(__int128 val) {
    /* GCC doesn't allow 128-bit in switch directly, but we can simulate */
    if (val == CONST_A) return 1;
    else if (val == CONST_B) return 2;
    else if (val == CONST_C) return 3;
    else if (val == CONST_D) return 4;
    else if (val == CONST_E) return 5;
    else if (val == 0) return 0;
    else return -1;
}

/* Test 8: Array operations with 128-bit values */
__int128 test_array_operations(void) {
    __int128 sum = 0;
    
    /* Sum global array - forces processing of all constants */
    for (int i = 0; i < (int)(sizeof(global_array) / sizeof(global_array[0])); i++) {
        sum += global_array[i];
        
        /* Compare with various constants during summation */
        if (global_array[i] < CONST_A) {
            sum += 1;
        } else if (global_array[i] > CONST_E) {
            sum -= 1;
        }
    }
    
    return sum;
}

/* Test 9: Complex conditional expressions */
__int128 test_complex_conditionals(__int128 a, __int128 b, __int128 c) {
    /* Nested comparisons that should trigger multiple double_int::cmp calls */
    __int128 result = 0;
    
    if ((a < b) && (b < c)) {
        result = a + b + c;
    } else if ((a > CONST_A) || (b < CONST_B)) {
        result = a - b;
    } else if ((c >= CONST_D) && (c <= CONST_E)) {
        result = b * c;
    } else {
        result = (a > 0) ? a : -a;
    }
    
    /* Additional comparison in return expression */
    return (result > 0) ? result : -result;
}

/* Main function that exercises all tests */
int main(void) {
    /* Initialize with values that exercise different comparison paths */
    __int128 test_val1 = CONST_A;
    __int128 test_val2 = CONST_B;
    __int128 test_val3 = CONST_C;
    __int128 test_val4 = CONST_D;
    __int128 test_val5 = CONST_E;
    
    /* Call test functions */
    __int128 result1 = test_range_analysis(test_val1, test_val2);
    __int128 result2 = test_loop_comparisons(test_val4, test_val5);
    __int128 result3 = test_bitwise_ops(test_val3);
    
    __int128 sum, prod;
    int overflow = test_overflow_ops(test_val1, test_val2, &sum, &prod);
    
    long long result4 = test_mixed_comparisons(test_val3, 0x123456789ABCDEFULL);
    
    dead_code_paths();
    
    int switch_result = test_switch_simulation(test_val1);
    
    __int128 result5 = test_array_operations();
    
    __int128 result6 = test_complex_conditionals(test_val1, test_val2, test_val3);
    
    /* Compute a simple checksum to verify execution */
    __int128 checksum = result1 + result2 + result3 + sum + prod + result4 + 
                       result5 + result6 + switch_result + overflow;
    
    /* Print low 64 bits of checksum */
    printf("Checksum (low 64 bits): %llx\n", 
           (unsigned long long)(checksum & 0xFFFFFFFFFFFFFFFFULL));
    
    /* Also print some intermediate results for verification */
    printf("Result1 low: %llx\n", (unsigned long long)(result1 & 0xFFFFFFFFFFFFFFFFULL));
    printf("Result2 low: %llx\n", (unsigned long long)(result2 & 0xFFFFFFFFFFFFFFFFULL));
    printf("Sum low: %llx\n", (unsigned long long)(sum & 0xFFFFFFFFFFFFFFFFULL));
    
    return 0;
}
