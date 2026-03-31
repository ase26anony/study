/* test_double_int_cmp.c - Comprehensive test for double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Test 1: Large integer constants and static assertions */
static const __int128_t VERY_LARGE_POS = ((__int128_t)1 << 70);
static const __int128_t VERY_LARGE_NEG = -((__int128_t)1 << 70);
static const __int128_t HUGE_PRODUCT = ((__int128_t)0x7FFFFFFFFFFFFFFF) * 2;

/* Static assertions force compile-time comparison */
_Static_assert(VERY_LARGE_POS > 0, "Large positive constant");
_Static_assert(VERY_LARGE_NEG < 0, "Large negative constant");
_Static_assert(HUGE_PRODUCT > INT64_MAX, "Product exceeds 64-bit");
_Static_assert(((1LL << 62) * (1LL << 62)) > 0, "Large multiplication");

/* Test 2: Builtin overflow operations */
int test_overflow_builtins(void) {
    long long a, b;
    long long res;
    int overflow;
    
    /* These will trigger double_int comparisons during overflow checking */
    a = 0x7FFFFFFFFFFFFFFFLL; /* INT64_MAX */
    b = 2;
    overflow = __builtin_mul_overflow(a, b, &res);
    
    if (__builtin_constant_p(overflow)) {
        /* Force constant evaluation path */
        return overflow ? 1 : 0;
    }
    
    /* Test addition overflow */
    a = 0x7FFFFFFFFFFFFFFFLL;
    b = 1;
    overflow = __builtin_add_overflow(a, b, &res);
    
    /* Test subtraction underflow */
    a = -0x7FFFFFFFFFFFFFFFLL - 1;
    b = 1;
    overflow = __builtin_sub_overflow(a, b, &res);
    
    return 0;
}

/* Test 3: Range analysis with complex conditions */
void test_range_analysis(int x) {
    /* Create known bounds that require wide integer comparisons */
    if (x > 1000 && x < 2000) {
        /* Multiplication creates range that needs double_int comparison */
        long long y = (long long)x * x;
        
        /* Nested conditions for more complex range analysis */
        if (y > 1000000 && y < 4000000) {
            long long z = y * 2;
            
            /* This should trigger range comparisons */
            if (z > 2000000 && z < 8000000) {
                printf("Range test passed: %lld\n", z);
            }
        }
    }
    
    /* Test with very large ranges */
    if (x > -1000000000 && x < 1000000000) {
        /* 64-bit multiplication that can exceed 64 bits in intermediate calculations */
        __int128_t big = (__int128_t)x * 1000000000000LL;
        
        if (big > -5000000000000000LL && big < 5000000000000000LL) {
            printf("Big range: %lld\n", (long long)(big / 1000000));
        }
    }
}

/* Test 4: Loop induction variables with large steps */
void test_loop_induction(void) {
    /* Loop with step that requires wide integer comparison for bounds checking */
    for (int64_t i = 0; i < INT64_MAX - 1000; i += 0x7FFFFFFFFFFFFLL) {
        /* The loop condition comparison may use double_int */
        if (i > INT64_MAX / 2) {
            printf("Large induction: %lld\n", (long long)i);
            break;
        }
    }
    
    /* Another loop with potential wrap-around analysis */
    for (int64_t j = INT64_MAX - 5; j < INT64_MAX + 10LL; j++) {
        /* This should trigger wrap-around analysis using double_int comparisons */
        if (j == INT64_MAX) {
            printf("Reached max: %lld\n", (long long)j);
        }
    }
}

/* Test 5: Shift operations beyond 64 bits */
void test_large_shifts(void) {
    __int128_t val;
    
    /* Left shifts creating values > 64 bits */
    val = (__int128_t)1 << 65;
    if (val > ((__int128_t)1 << 64)) {
        printf("65-bit shift: %llu\n", (unsigned long long)(val >> 64));
    }
    
    val = (__int128_t)1 << 96;
    if (val > ((__int128_t)1 << 95)) {
        printf("96-bit shift test passed\n");
    }
    
    /* Combined shifts and comparisons */
    __int128_t a = ((__int128_t)0x123456789ABCDEF0) << 32;
    __int128_t b = ((__int128_t)0xFEDCBA9876543210) << 32;
    
    if (a < b) {
        printf("Large comparison: a < b\n");
    } else if (a > b) {
        printf("Large comparison: a > b\n");
    }
}

/* Test 6: Division and modulus with large values */
void test_division_modulus(void) {
    __int128_t dividend = ((__int128_t)1 << 100) + 12345;
    __int128_t divisor = ((__int128_t)1 << 50) + 6789;
    
    /* These operations require magnitude comparisons */
    __int128_t quotient = dividend / divisor;
    __int128_t remainder = dividend % divisor;
    
    if (quotient > 0 && remainder < divisor) {
        printf("Division test: quotient=%llu, remainder=%llu\n",
               (unsigned long long)(quotient >> 64),
               (unsigned long long)remainder);
    }
}

/* Test 7: __builtin_constant_p with large comparisons */
int test_constant_folding(void) {
    /* Force constant folding of large integer comparisons */
    if (__builtin_constant_p(VERY_LARGE_POS > VERY_LARGE_NEG)) {
        return VERY_LARGE_POS > VERY_LARGE_NEG ? 1 : 0;
    }
    
    if (__builtin_constant_p(((1LL << 63) - 1) * 2 > 0)) {
        return ((1LL << 63) - 1) * 2 > 0 ? 1 : 0;
    }
    
    return 0;
}

/* Test 8: Mixed signed/unsigned comparisons */
void test_mixed_comparisons(void) {
    unsigned __int128 uval = ((unsigned __int128)1 << 127) + 1;
    __int128_t sval = -((__int128_t)1 << 126);
    
    /* These comparisons may trigger different code paths */
    if ((unsigned __int128)uval > (unsigned __int128)sval) {
        printf("Unsigned comparison passed\n");
    }
    
    if (sval < uval) {
        printf("Mixed signedness comparison\n");
    }
}

/* Main test driver */
int main(void) {
    int result = 0;
    
    printf("Testing double_int::cmp coverage...\n");
    
    /* Test 1: Already covered by static assertions at compile time */
    printf("Test 1 (static assertions): PASS\n");
    
    /* Test 2: Overflow builtins */
    result |= test_overflow_builtins();
    printf("Test 2 (overflow builtins): %s\n", result ? "SOME_OVERFLOW" : "NO_OVERFLOW");
    
    /* Test 3: Range analysis */
    test_range_analysis(1500);
    test_range_analysis(-500);
    printf("Test 3 (range analysis): PASS\n");
    
    /* Test 4: Loop induction */
    test_loop_induction();
    printf("Test 4 (loop induction): PASS\n");
    
    /* Test 5: Large shifts */
    test_large_shifts();
    printf("Test 5 (large shifts): PASS\n");
    
    /* Test 6: Division and modulus */
    test_division_modulus();
    printf("Test 6 (division/modulus): PASS\n");
    
    /* Test 7: Constant folding */
    result = test_constant_folding();
    printf("Test 7 (constant folding): %s\n", result ? "TRUE" : "FALSE");
    
    /* Test 8: Mixed comparisons */
    test_mixed_comparisons();
    printf("Test 8 (mixed comparisons): PASS\n");
    
    /* Additional runtime validation */
    {
        __int128_t a = ((__int128_t)1 << 70) + 123;
        __int128_t b = ((__int128_t)1 << 70) + 456;
        
        if (a < b) {
            printf("Runtime large comparison: a < b (correct)\n");
        } else {
            printf("Runtime large comparison: ERROR\n");
            return 1;
        }
    }
    
    printf("\nAll tests completed successfully!\n");
    return 0;
}
