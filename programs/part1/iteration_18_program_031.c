/* double-int-test.c - Test program to trigger double_int::cmp comparisons */
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

/* Full 128-bit constants */
static const __int128 CONST_A = ((__int128)HIGH_A << 64) | LOW_A;
static const __int128 CONST_B = ((__int128)HIGH_B << 64) | LOW_B;
static const __int128 CONST_C = ((__int128)HIGH_C << 64) | LOW_C;
static const __int128 CONST_D = ((__int128)0x8000000000000000ULL << 64) | 0x0ULL; /* Min signed */
static const __int128 CONST_E = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL; /* Max signed */

/* Global arrays with 128-bit constants - forces compile-time initialization */
static const __int128 global_array[] = {
    CONST_A,
    CONST_B,
    CONST_C,
    CONST_D,
    CONST_E,
    ((__int128)0x0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL,  /* Only low word set */
    ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0ULL,  /* Only high word set */
};

/* Static assertions forcing compile-time comparisons */
_Static_assert(CONST_A < CONST_C, "CONST_A should be less than CONST_C");
_Static_assert(CONST_B > CONST_A, "CONST_B should be greater than CONST_A");
_Static_assert(CONST_A != CONST_B, "CONST_A and CONST_B should differ");

/* Dead code with comparisons that may still be evaluated during optimization */
static __int128 dead_code_comparisons(__int128 x) {
    if (0) {  /* Dead branch, but constants may be compared during early passes */
        if (CONST_A < CONST_B) return CONST_A;
        if (CONST_C > CONST_B) return CONST_C;
        if (((__int128)0x1ULL << 120) > CONST_A) return 0;
    }
    return x;
}

/* Function with range analysis on 128-bit values */
__int128 range_analysis_test(__int128 x, __int128 y) {
    /* Comparisons that should trigger VRP analysis */
    if (x < CONST_A) {
        if (y > CONST_B) {
            return x + y;
        }
    }
    
    /* Ternary with mixed-type comparisons */
    __int128 result = (x < 100ULL) ? CONST_A : CONST_B;
    
    /* Nested comparisons */
    if (x > CONST_D && x < CONST_E) {
        result = result | y;
    }
    
    return result;
}

/* Loop with 128-bit induction variable */
void loop_with_128bit_iv(__int128 start, __int128 end) {
    volatile __int128 sum = 0;  /* volatile to prevent complete optimization */
    
    /* Loop that compares 128-bit values */
    for (__int128 i = start; i < end; i += ((__int128)1 << 64)) {
        sum += i;
        
        /* Additional comparison inside loop */
        if (i > CONST_A) {
            sum += CONST_B;
        }
    }
    
    /* Prevent unused variable warning */
    (void)sum;
}

/* Bitwise operations crossing word boundaries */
__int128 bitwise_operations(__int128 x) {
    /* Masks targeting specific words */
    __int128 mask_low = 0xFFFFFFFFFFFFFFFFULL;
    __int128 mask_high = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    
    /* Operations that require reasoning about both words */
    __int128 shifted = x << 65;  /* Crosses word boundary */
    __int128 masked = (x & mask_high) | (CONST_A & mask_low);
    
    /* Arithmetic shift on negative value */
    __int128 neg = -CONST_A;
    __int128 arith_shifted = neg >> 96;  /* Large right shift */
    
    return shifted + masked + arith_shifted;
}

/* Overflow checking with 128-bit values */
int overflow_checks(__int128 a, __int128 b) {
    __int128 sum, product;
    int overflow_add, overflow_mul;
    
    /* Builtins that may use double_int comparisons internally */
    overflow_add = __builtin_add_overflow(a, b, &sum);
    overflow_mul = __builtin_mul_overflow(a, CONST_A, &product);
    
    /* Comparisons of results */
    if (sum > CONST_B || product < CONST_C) {
        return 1;
    }
    
    return overflow_add | overflow_mul;
}

/* Mixed-type comparisons and conversions */
__int128 mixed_type_comparisons(unsigned long long ull, __int128 si128) {
    /* Compare 128-bit with 64-bit */
    if (si128 < ull) {
        return CONST_A;
    }
    
    /* Ternary with type conversion */
    __int128 result = (ull > 1000) ? ((__int128)ull << 64) : si128;
    
    /* Chain of comparisons */
    if (result > CONST_D && result < CONST_E && result != CONST_B) {
        result = result | CONST_C;
    }
    
    return result;
}

/* Structure with 128-bit field */
struct with_128bit {
    __int128 large_value;
    int small_value;
};

/* Switch-like logic (GCC doesn't allow switch on __int128 directly) */
int switch_like_logic(__int128 value) {
    if (value == CONST_A) return 1;
    if (value == CONST_B) return 2;
    if (value == CONST_C) return 3;
    if (value >= CONST_D && value <= CONST_E) return 4;
    return 0;
}

/* Main test function */
int main() {
    /* Initialize with values that will exercise different comparison paths */
    __int128 test_val1 = CONST_A;
    __int128 test_val2 = CONST_B;
    __int128 test_val3 = CONST_C;
    
    /* Test 1: Direct comparisons */
    printf("Test 1 - Direct comparisons:\n");
    printf("CONST_A %s CONST_B\n", (CONST_A < CONST_B) ? "<" : ">=");
    printf("CONST_B %s CONST_C\n", (CONST_B < CONST_C) ? "<" : ">=");
    
    /* Test 2: Range analysis */
    printf("\nTest 2 - Range analysis:\n");
    __int128 range_result = range_analysis_test(test_val1, test_val2);
    printf("Range test result: 0x%016llx%016llx\n", 
           (unsigned long long)(range_result >> 64),
           (unsigned long long)range_result);
    
    /* Test 3: Loop with 128-bit bounds */
    printf("\nTest 3 - Loop with 128-bit IV:\n");
    loop_with_128bit_iv(CONST_D, CONST_D + ((__int128)1 << 68));
    
    /* Test 4: Bitwise operations */
    printf("\nTest 4 - Bitwise operations:\n");
    __int128 bitwise_result = bitwise_operations(test_val1);
    printf("Bitwise result: 0x%016llx%016llx\n",
           (unsigned long long)(bitwise_result >> 64),
           (unsigned long long)bitwise_result);
    
    /* Test 5: Overflow checks */
    printf("\nTest 5 - Overflow checks:\n");
    int overflow_result = overflow_checks(test_val1, test_val2);
    printf("Overflow check result: %d\n", overflow_result);
    
    /* Test 6: Mixed type comparisons */
    printf("\nTest 6 - Mixed type comparisons:\n");
    __int128 mixed_result = mixed_type_comparisons(0xFFFFFFFFFFFFFFFFULL, test_val3);
    printf("Mixed type result: 0x%016llx%016llx\n",
           (unsigned long long)(mixed_result >> 64),
           (unsigned long long)mixed_result);
    
    /* Test 7: Global array processing */
    printf("\nTest 7 - Global array processing:\n");
    __int128 array_sum = 0;
    for (int i = 0; i < sizeof(global_array)/sizeof(global_array[0]); i++) {
        array_sum += global_array[i];
    }
    printf("Array sum: 0x%016llx%016llx\n",
           (unsigned long long)(array_sum >> 64),
           (unsigned long long)array_sum);
    
    /* Test 8: Switch-like logic */
    printf("\nTest 8 - Switch-like logic:\n");
    printf("CONST_A matches: %d\n", switch_like_logic(CONST_A));
    printf("CONST_B matches: %d\n", switch_like_logic(CONST_B));
    printf("CONST_D matches: %d\n", switch_like_logic(CONST_D));
    
    /* Dead code (will be optimized out but may trigger comparisons during compilation) */
    dead_code_comparisons(test_val1);
    
    /* Final verification */
    printf("\nVerification complete.\n");
    
    return 0;
}
