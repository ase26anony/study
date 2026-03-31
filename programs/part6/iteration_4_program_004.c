/* test_double_int_cmp.c - Comprehensive test for double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Test 1: Static assertions with large 128-bit constants */
static_assert(sizeof(__int128) == 16, "128-bit support required");

/* Large constants that require double_int representation */
const __int128 VERY_LARGE_POS = ((__int128_t)1 << 70);
const __int128 VERY_LARGE_NEG = -((__int128_t)1 << 70);
const __int128 HUGE_VAL = ((__int128_t)0x7FFFFFFFFFFFFFFF << 64) | 0xFFFFFFFFFFFFFFFF;
const __int128 MAX_INT128 = ((__int128_t)1 << 127) - 1;
const __int128 MIN_INT128 = -((__int128_t)1 << 127);

/* Static comparisons that force compile-time evaluation */
static_assert(VERY_LARGE_POS > 0, "Large positive comparison");
static_assert(VERY_LARGE_NEG < 0, "Large negative comparison");
static_assert(VERY_LARGE_POS > VERY_LARGE_NEG, "Cross-sign comparison");
static_assert(HUGE_VAL > VERY_LARGE_POS, "Both high parts differ");
static_assert(MAX_INT128 > MIN_INT128, "Extreme bounds comparison");

/* Test 2: Builtin overflow operations that use double_int internally */
void test_overflow_builtins(void) {
    long long a, b;
    long long res;
    int overflow;
    
    /* Test cases designed to trigger overflow comparisons */
    a = 0x7FFFFFFFFFFFFFFFLL; /* LLONG_MAX */
    b = 2;
    
    /* Multiplication overflow - internal double_int comparison */
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("Mul overflow test: %lld * %lld -> overflow=%d\n", a, b, overflow);
    
    /* Addition overflow */
    overflow = __builtin_add_overflow(a, 1, &res);
    printf("Add overflow test: %lld + 1 -> overflow=%d\n", a, overflow);
    
    /* Subtraction overflow */
    a = -0x7FFFFFFFFFFFFFFFLL - 1; /* LLONG_MIN */
    overflow = __builtin_sub_overflow(a, 1, &res);
    printf("Sub overflow test: %lld - 1 -> overflow=%d\n", a, overflow);
    
    /* Constant overflow checks */
    if (__builtin_constant_p(__builtin_mul_overflow_p(0x7FFFFFFFFFFFFFFFLL, 2, 0LL))) {
        printf("Constant overflow detection active\n");
    }
}

/* Test 3: Range analysis with complex conditions */
void test_range_analysis(int x) {
    /* Create known bounds that VRP will analyze */
    if (x > 1000 && x < 2000) {
        /* Multiplication that requires double_int for range calculation */
        long long y = (long long)x * x;
        
        /* Nested conditions for more complex range analysis */
        if (y > 1000000 && y < 4000000) {
            printf("Range analysis test passed: y=%lld\n", y);
        }
        
        /* Large shift operations */
        __int128 big_shift = (__int128)x << 40;
        if (big_shift > ((__int128)1 << 50)) {
            printf("Large shift in range: %lld\n", (long long)(big_shift >> 40));
        }
    }
    
    /* Test with negative ranges */
    if (x < -1000 && x > -2000) {
        __int128 neg_product = (__int128)x * x; /* Positive result */
        if (neg_product > 1000000) {
            printf("Negative range product: %lld\n", (long long)neg_product);
        }
    }
}

/* Test 4: Loop induction variables with potential overflow */
void test_loop_induction(void) {
    /* Loop with large step that might trigger wrap analysis */
    for (int64_t i = 0x7000000000000000LL; 
         i < 0x7000000000000000LL + 1000; 
         i += 0x100000000LL) {
        /* Operations that require double_int comparisons for bounds checking */
        if (i > 0x7000000000000000LL && i < 0x8000000000000000LL) {
            printf("Loop induction: i=%llx\n", (unsigned long long)i);
        }
    }
}

/* Test 5: Direct 128-bit arithmetic and comparisons */
void test_direct_128bit_ops(void) {
    __int128 a = ((__int128_t)1 << 70) + 12345;
    __int128 b = ((__int128_t)1 << 70) + 12346;
    __int128 c = ((__int128_t)1 << 69);
    __int128 d = -((__int128_t)1 << 70);
    
    /* Various comparison patterns */
    int cmp1 = (a < b) ? -1 : (a > b) ? 1 : 0;
    int cmp2 = (a > c) ? 1 : (a < c) ? -1 : 0;
    int cmp3 = (d < a) ? -1 : (d > a) ? 1 : 0;
    
    printf("128-bit comparisons: %d, %d, %d\n", cmp1, cmp2, cmp3);
    
    /* Arithmetic that might overflow 64-bit */
    __int128 product = (__int128)0x7FFFFFFFFFFFFFFFLL * 2;
    if (product > 0x7FFFFFFFFFFFFFFFLL) {
        printf("128-bit product overflow detected\n");
    }
    
    /* Mixed-size comparisons */
    if (a > 0x7FFFFFFFFFFFFFFFLL) {
        printf("128-bit > 64-bit max comparison\n");
    }
}

/* Test 6: Compile-time template-like comparisons (C version) */
#define COMPARE_128(a, b) \
    ((a) < (b) ? -1 : ((a) > (b) ? 1 : 0))

/* Force evaluation at translation time */
static const int compile_time_cmp = 
    __builtin_choose_expr(
        __builtin_constant_p(VERY_LARGE_POS > 100),
        COMPARE_128(VERY_LARGE_POS, 100),
        0);

/* Test 7: Enumeration with large values */
enum big_enum : __int128 {
    BIG_ENUM_A = ((__int128_t)1 << 70),
    BIG_ENUM_B = ((__int128_t)1 << 71),
    BIG_ENUM_C = ((__int128_t)1 << 72)
};

void test_enum_comparisons(void) {
    /* Comparisons between enum values */
    if (BIG_ENUM_B > BIG_ENUM_A) {
        printf("Large enum comparison passed\n");
    }
    
    /* Switch with large enum - forces comparison during lowering */
    __int128 val = BIG_ENUM_B;
    switch (val) {
        case BIG_ENUM_A: printf("Case A\n"); break;
        case BIG_ENUM_B: printf("Case B\n"); break;
        case BIG_ENUM_C: printf("Case C\n"); break;
        default: printf("Default case\n");
    }
}

/* Test 8: Bitfield operations with wide types */
struct wide_bitfield {
    __int128 wide_field:70; /* Non-portable but may trigger internal paths */
    unsigned long long normal_field:60;
};

void test_bitfield_ops(void) {
    struct wide_bitfield wbf = {0};
    wbf.wide_field = ((__int128_t)1 << 69) - 1;
    
    /* Operations that might require double_int comparisons */
    if (wbf.wide_field > 0) {
        printf("Wide bitfield positive\n");
    }
}

/* Test 9: Division and modulus with large values */
void test_division_large(void) {
    __int128 dividend = ((__int128_t)1 << 100) + 12345;
    __int128 divisor = ((__int128_t)1 << 50) + 6789;
    
    /* Division requires magnitude comparison */
    __int128 quotient = dividend / divisor;
    __int128 remainder = dividend % divisor;
    
    printf("Large division: quotient=%llx, remainder=%llx\n",
           (unsigned long long)(quotient >> 64), 
           (unsigned long long)remainder);
    
    /* Compare quotient with expected range */
    if (quotient > 0 && quotient < ((__int128_t)1 << 50)) {
        printf("Quotient in expected range\n");
    }
}

/* Main test driver */
int main(void) {
    printf("=== Testing double_int::cmp coverage ===\n");
    
    /* Test 2: Overflow builtins */
    test_overflow_builtins();
    
    /* Test 3: Range analysis with sample values */
    test_range_analysis(1500);
    test_range_analysis(-1500);
    
    /* Test 4: Loop induction */
    test_loop_induction();
    
    /* Test 5: Direct 128-bit operations */
    test_direct_128bit_ops();
    
    /* Test 7: Enum comparisons */
    test_enum_comparisons();
    
    /* Test 8: Bitfield operations */
    test_bitfield_ops();
    
    /* Test 9: Division with large values */
    test_division_large();
    
    /* Runtime validation of compile-time comparisons */
    if (compile_time_cmp == 1) {
        printf("Compile-time comparison validated at runtime\n");
    }
    
    /* Additional runtime checks */
    __int128 runtime_a = ((__int128_t)1 << 70) + 1000;
    __int128 runtime_b = ((__int128_t)1 << 70) + 2000;
    
    if (runtime_a < runtime_b && 
        VERY_LARGE_POS > 0 && 
        VERY_LARGE_NEG < 0) {
        printf("All runtime comparisons PASS\n");
    } else {
        printf("Runtime comparison FAIL\n");
        return 1;
    }
    
    printf("=== All tests completed ===\n");
    return 0;
}
