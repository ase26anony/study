/* test_double_int_cmp.c - Comprehensive test for double_int comparison logic */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

/* Force 128-bit integer support */
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* ==================== SECTION 1: Constant Folding with Large Integers ==================== */

/* Static assertions with large constants - forces compile-time comparison */
#define STATIC_ASSERT(cond) _Static_assert(cond, #cond)

/* Large constants that require double_int representation */
const int128_t VERY_LARGE_POS = ((int128_t)1 << 70);
const int128_t VERY_LARGE_NEG = -((int128_t)1 << 70);
const int128_t HUGE_PRODUCT = ((int128_t)0x7FFFFFFFFFFFFFFF) * 2;
const uint128_t LARGE_UNSIGNED = ((uint128_t)1 << 72);

/* Compile-time comparisons that should trigger double_int::cmp */
STATIC_ASSERT(VERY_LARGE_POS > 0);
STATIC_ASSERT(VERY_LARGE_NEG < 0);
STATIC_ASSERT(VERY_LARGE_POS > VERY_LARGE_NEG);
STATIC_ASSERT(LARGE_UNSIGNED > (uint128_t)0xFFFFFFFFFFFFFFFF);
STATIC_ASSERT(HUGE_PRODUCT > INT64_MAX);

/* Template-like macro for compile-time comparison */
#define COMPARE_CONSTANTS(a, b, op) \
    do { \
        if (__builtin_constant_p((a) op (b))) { \
            static volatile int result = ((a) op (b)) ? 1 : 0; \
            (void)result; \
        } \
    } while(0)

/* ==================== SECTION 2: GCC Builtins with Overflow ==================== */

/* Test overflow builtins that use double_int internally */
void test_overflow_builtins(void) {
    long long a, b;
    long long res;
    int overflow;
    
    /* Case 1: Multiplication that overflows 64-bit */
    a = 0x7FFFFFFFFFFFFFFFLL; /* Max int64_t */
    b = 2;
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("Mul overflow test: %lld * 2 overflowed? %s\n", 
           a, overflow ? "YES" : "NO");
    
    /* Case 2: Addition with potential overflow */
    a = 0x7FFFFFFFFFFFFFFFLL;
    b = 1;
    overflow = __builtin_add_overflow(a, b, &res);
    printf("Add overflow test: %lld + 1 overflowed? %s\n",
           a, overflow ? "YES" : "NO");
    
    /* Case 3: Subtraction with underflow */
    a = (-0x7FFFFFFFFFFFFFFFLL - 1);
    b = 1;
    overflow = __builtin_sub_overflow(a, b, &res);
    printf("Sub overflow test: %lld - 1 overflowed? %s\n",
           a, overflow ? "YES" : "NO");
    
    /* Constant overflow checks */
    if (__builtin_constant_p(__builtin_mul_overflow_p(0x7FFFFFFFFFFFFFFFLL, 
                                                       2, 
                                                       (long long)0))) {
        printf("Constant overflow check triggered\n");
    }
}

/* ==================== SECTION 3: Range Analysis with Complex Conditions ==================== */

/* Functions that create complex value ranges */
void test_range_analysis(int x) {
    /* Create known bounds for x */
    if (x > 1000 && x < 2000) {
        /* This multiplication creates a range that needs double_int comparison */
        int64_t y = (int64_t)x * (int64_t)x;
        
        /* Further comparisons with large constants */
        if (y > ((int64_t)1 << 40)) {
            printf("Range test: y = %lld is very large\n", (long long)y);
        }
        
        /* Nested range analysis */
        if (x > 1500) {
            int64_t z = y * 2;
            if (z > ((int64_t)1 << 41)) {
                printf("Nested range: z = %lld exceeds threshold\n", (long long)z);
            }
        }
    }
    
    /* Test with negative ranges */
    if (x < -1000 && x > -2000) {
        int64_t y = (int64_t)x * (int64_t)x;
        if (y > ((int64_t)1 << 40)) {
            printf("Negative range test: y = %lld is positive large\n", (long long)y);
        }
    }
}

/* Loop with induction variable analysis */
void test_induction_variables(void) {
    for (int64_t i = 0x7000000000000000LL; 
         i < 0x7000000000000000LL + 100; 
         i += 0x100000000LL) {
        /* Large step value causes wrap-around analysis */
        int64_t j = i * 3;
        if (j > 0x7FFFFFFFFFFFFFFFLL) {
            printf("Induction: i=%lld, j overflowed\n", (long long)i);
        }
    }
}

/* ==================== SECTION 4: 128-bit Integer Operations ==================== */

/* Direct operations on 128-bit integers */
void test_128bit_operations(void) {
    int128_t a = ((int128_t)1 << 70) + 123;
    int128_t b = ((int128_t)1 << 69) + 456;
    int128_t c = ((int128_t)1 << 70) + 123; /* Same as a */
    
    /* Comparisons that should use double_int::cmp */
    if (a > b) {
        printf("128-bit: a > b (expected)\n");
    }
    
    if (a == c) {
        printf("128-bit: a == c (expected)\n");
    }
    
    if (b < a) {
        printf("128-bit: b < a (expected)\n");
    }
    
    /* Arithmetic with comparison */
    int128_t sum = a + b;
    if (sum > a && sum > b) {
        printf("128-bit: sum > both operands\n");
    }
    
    /* Division with large values */
    int128_t quotient = a / 2;
    if (quotient < a) {
        printf("128-bit: quotient < original\n");
    }
}

/* ==================== SECTION 5: Mixed-width Operations ==================== */

/* Operations mixing different integer widths */
void test_mixed_width_operations(void) {
    /* 64-bit and 128-bit mixing */
    int64_t small = 0x7FFFFFFFFFFFFFFFLL;
    int128_t large = ((int128_t)1 << 70);
    
    /* Comparisons between different widths */
    if (large > small) {
        printf("Mixed width: 128-bit > 64-bit\n");
    }
    
    /* Arithmetic with promotion */
    int128_t result = large + small;
    if (result > large) {
        printf("Mixed width: large + small > large\n");
    }
    
    /* Shift operations */
    int128_t shifted = small << 10;
    if (shifted > small) {
        printf("Mixed width: shifted > original\n");
    }
}

/* ==================== SECTION 6: Compile-time Function Evaluation ==================== */

/* Constexpr-like function for C */
static inline int128_t compute_large_value(int n) 
    __attribute__((always_inline));
    
static inline int128_t compute_large_value(int n) {
    return ((int128_t)n << 60) + 123456789;
}

/* Use in constant context */
void test_compile_time_eval(void) {
    /* Force constant evaluation through builtin */
    if (__builtin_constant_p(compute_large_value(1) > compute_large_value(0))) {
        const int result = (compute_large_value(1) > compute_large_value(0)) ? 1 : 0;
        printf("Compile-time eval: %d\n", result);
    }
    
    /* Array size based on large constant comparison */
    char buffer[(compute_large_value(5) > compute_large_value(3)) ? 100 : 200];
    printf("Buffer size based on 128-bit comparison: %zu\n", sizeof(buffer));
}

/* ==================== MAIN TEST DRIVER ==================== */

int main(int argc, char *argv[]) {
    int test_value;
    
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
    /* Seed for reproducible tests */
    if (argc > 1) {
        test_value = atoi(argv[1]);
    } else {
        test_value = 1500; /* Mid-range value */
    }
    
    /* Execute all test sections */
    printf("1. Testing overflow builtins:\n");
    test_overflow_builtins();
    printf("\n");
    
    printf("2. Testing range analysis:\n");
    test_range_analysis(test_value);
    test_range_analysis(-test_value);
    printf("\n");
    
    printf("3. Testing induction variables:\n");
    test_induction_variables();
    printf("\n");
    
    printf("4. Testing 128-bit operations:\n");
    test_128bit_operations();
    printf("\n");
    
    printf("5. Testing mixed-width operations:\n");
    test_mixed_width_operations();
    printf("\n");
    
    printf("6. Testing compile-time evaluation:\n");
    test_compile_time_eval();
    printf("\n");
    
    /* Final verification using large constants */
    printf("7. Final verification with extreme values:\n");
    
    /* Test all comparison operators with large values */
    const int128_t max_128 = ~((int128_t)1 << 127);
    const int128_t min_128 = ((int128_t)1 << 127);
    
    /* These comparisons should all trigger double_int::cmp */
    if (VERY_LARGE_POS < max_128) {
        printf("  VERY_LARGE_POS < MAX_128 ✓\n");
    }
    
    if (VERY_LARGE_NEG > min_128) {
        printf("  VERY_LARGE_NEG > MIN_128 ✓\n");
    }
    
    if (VERY_LARGE_POS != VERY_LARGE_NEG) {
        printf("  VERY_LARGE_POS != VERY_LARGE_NEG ✓\n");
    }
    
    if (LARGE_UNSIGNED > 0) {
        printf("  LARGE_UNSIGNED > 0 ✓\n");
    }
    
    /* Runtime assertion based on large constant comparison */
    assert(VERY_LARGE_POS > 0);
    assert(VERY_LARGE_NEG < 0);
    assert(HUGE_PRODUCT > INT64_MAX);
    
    printf("\n=== All tests completed successfully ===\n");
    
    return 0;
}
