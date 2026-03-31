/* test_double_int_cmp.c - Comprehensive test for double_int comparison logic */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>

/* Approach 1: Large integer constants and static assertions */
#ifdef __cplusplus
#define STATIC_ASSERT _Static_assert
#else
#define STATIC_ASSERT static_assert
#endif

/* Force compiler to handle 128-bit constants */
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* Large constants that require double_int representation */
const int128_t VERY_LARGE_POS = ((int128_t)1 << 70);
const int128_t VERY_LARGE_NEG = -((int128_t)1 << 70);
const uint128_t HUGE_UNSIGNED = ((uint128_t)1 << 100) + ((uint128_t)1 << 70);

/* Static assertions that force compile-time comparisons */
STATIC_ASSERT(VERY_LARGE_POS > 0, "Large positive constant");
STATIC_ASSERT(VERY_LARGE_NEG < 0, "Large negative constant");
STATIC_ASSERT(HUGE_UNSIGNED > VERY_LARGE_POS, "Unsigned comparison");
STATIC_ASSERT(((int128_t)1 << 65) < VERY_LARGE_POS, "Shift comparison");
STATIC_ASSERT(-VERY_LARGE_POS == VERY_LARGE_NEG, "Negation comparison");

/* Approach 2: Builtin overflow operations */
void test_overflow_builtins(void) {
    long long a, b;
    long long res;
    int overflow;
    
    /* Test cases that may trigger double_int comparisons in overflow checks */
    a = 0x7FFFFFFFFFFFFFFFLL; /* LLONG_MAX */
    b = 2;
    
    /* Multiplication overflow - internal logic uses double_int comparisons */
    overflow = __builtin_mul_overflow(a, b, &res);
    printf("Mul overflow test: %lld * 2 overflow? %s\n", 
           a, overflow ? "YES" : "NO");
    
    /* Addition overflow */
    overflow = __builtin_add_overflow(a, 1, &res);
    printf("Add overflow test: LLONG_MAX + 1 overflow? %s\n",
           overflow ? "YES" : "NO");
    
    /* Subtraction overflow */
    a = 0x8000000000000000LL; /* LLONG_MIN */
    overflow = __builtin_sub_overflow(a, 1, &res);
    printf("Sub overflow test: LLONG_MIN - 1 overflow? %s\n",
           overflow ? "YES" : "NO");
    
    /* Constant overflow checks */
    if (__builtin_constant_p(__builtin_mul_overflow_p(0x7FFFFFFFFFFFFFFFLL, 
                                                      2, 0LL))) {
        printf("Constant overflow check triggered\n");
    }
}

/* Approach 3: Range analysis with complex conditions */
void test_range_analysis(int x) {
    /* Create conditions that force VRP to use double_int comparisons */
    if (x > 1000 && x < 2000) {
        /* Multiplication that may overflow 64-bit but handled with double_int */
        int64_t y = (int64_t)x * (int64_t)x;
        
        /* Nested conditions for more complex range analysis */
        if (y > 1000000 && y < 4000000) {
            int64_t z = y * x; /* Further multiplication */
            printf("Range test: x=%d, y=%lld, z=%lld\n", x, (long long)y, (long long)z);
        }
    }
    
    /* Test with very large ranges */
    if (x > -1000000000 && x < 1000000000) {
        /* Force compiler to analyze large ranges */
        int64_t big = (int64_t)x * 1000000000LL;
        if (big > -5000000000000000000LL && big < 5000000000000000000LL) {
            printf("Large range test passed: %lld\n", (long long)big);
        }
    }
}

/* Approach 4: Loop induction variables with large steps */
void test_loop_induction(void) {
    /* Loop with large step that may cause wrap-around analysis */
    for (int64_t i = 0; i < 100; i += 0x100000000LL) {
        /* This loop may trigger double_int comparisons in loop analysis */
        printf("Loop induction: i = %lld\n", (long long)i);
    }
    
    /* More complex loop with multiplication */
    for (int j = 1000; j < 10000; j += 100) {
        int64_t prod = (int64_t)j * j * j;
        if (prod > 1000000000LL) {
            printf("Large product: j=%d, prod=%lld\n", j, (long long)prod);
        }
    }
}

/* Approach 5: Direct 128-bit arithmetic and comparisons */
void test_128bit_operations(void) {
    int128_t a = ((int128_t)1 << 70) + 123;
    int128_t b = ((int128_t)1 << 69) + 456;
    int128_t c = ((int128_t)1 << 71) - 789;
    
    /* Runtime comparisons of 128-bit values */
    if (a > b) printf("128-bit: a > b\n");
    if (b < c) printf("128-bit: b < c\n");
    if (a != c) printf("128-bit: a != c\n");
    
    /* Arithmetic that may trigger internal double_int comparisons */
    int128_t sum = a + b;
    int128_t diff = c - a;
    int128_t prod = b * 1000;
    
    if (sum > diff) printf("128-bit sum > diff\n");
    if (prod > a) printf("128-bit prod > a\n");
    
    /* Test with mixed 64-bit and 128-bit */
    int64_t sixty_four = 0x7FFFFFFFFFFFFFFFLL;
    int128_t mixed = a + sixty_four;
    if (mixed > a) printf("Mixed 64/128-bit: mixed > a\n");
}

/* Approach 6: Template metaprogramming (C++ only) */
#ifdef __cplusplus
template <int128_t N>
struct LargeCompare {
    static const bool greater_than_2_65 = N > (int128_t(1) << 65);
    static const bool less_than_neg_2_66 = N < -(int128_t(1) << 66);
    static const bool is_positive = N > 0;
    
    static void print() {
        printf("Template N=%s: gt_2^65=%d, lt_neg_2^66=%d, pos=%d\n",
               (N == (int128_t(1) << 70)) ? "2^70" : 
               (N == -(int128_t(1) << 70)) ? "-2^70" : "other",
               greater_than_2_65, less_than_neg_2_66, is_positive);
    }
};

void test_templates(void) {
    LargeCompare<(int128_t(1) << 70)>::print();
    LargeCompare<-(int128_t(1) << 70)>::print();
    LargeCompare<(int128_t(1) << 65) + 1>::print();
}
#endif

/* Approach 7: Bitfield and enum tests */
enum big_enum : int128_t {
    BIG_ENUM_A = (int128_t)1 << 70,
    BIG_ENUM_B = (int128_t)1 << 69,
    BIG_ENUM_C = (int128_t)1 << 71
};

void test_enum_comparisons(void) {
    /* Compare enum values - may trigger double_int comparisons */
    if (BIG_ENUM_A > BIG_ENUM_B) printf("Enum: A > B\n");
    if (BIG_ENUM_C > BIG_ENUM_A) printf("Enum: C > A\n");
    
    /* Use in switch - forces comparisons */
    big_enum e = BIG_ENUM_B;
    switch (e) {
        case BIG_ENUM_A: printf("Case A\n"); break;
        case BIG_ENUM_B: printf("Case B\n"); break;
        case BIG_ENUM_C: printf("Case C\n"); break;
    }
}

/* Main test driver */
int main(int argc, char *argv[]) {
    printf("=== Testing double_int::cmp coverage ===\n\n");
    
    /* Test 1: Overflow builtins */
    printf("1. Testing overflow builtins:\n");
    test_overflow_builtins();
    printf("\n");
    
    /* Test 2: Range analysis */
    printf("2. Testing range analysis:\n");
    for (int i = 1001; i < 1005; i++) {
        test_range_analysis(i);
    }
    printf("\n");
    
    /* Test 3: Loop induction */
    printf("3. Testing loop induction:\n");
    test_loop_induction();
    printf("\n");
    
    /* Test 4: 128-bit operations */
    printf("4. Testing 128-bit operations:\n");
    test_128bit_operations();
    printf("\n");
    
    /* Test 5: Enum comparisons */
    printf("5. Testing enum comparisons:\n");
    test_enum_comparisons();
    printf("\n");
    
    /* Test 6: Templates (C++ only) */
#ifdef __cplusplus
    printf("6. Testing template metaprogramming:\n");
    test_templates();
    printf("\n");
#endif
    
    /* Test 7: Complex constant expressions */
    printf("7. Testing complex constant expressions:\n");
    const int128_t complex_expr = ((int128_t)1 << 70) + ((int128_t)1 << 65) - 1;
    const int128_t another_expr = ((int128_t)1 << 71) - ((int128_t)1 << 66);
    
    /* Force compile-time comparison */
    if (__builtin_constant_p(complex_expr > another_expr)) {
        printf("Complex constant comparison evaluated at compile time\n");
    }
    
    /* Runtime verification */
    if (complex_expr < another_expr) {
        printf("Runtime: complex_expr < another_expr\n");
    } else {
        printf("Runtime: complex_expr >= another_expr\n");
    }
    
    /* Test 8: Division and modulus with large numbers */
    printf("\n8. Testing division/modulus:\n");
    int128_t dividend = (int128_t)1 << 100;
    int128_t divisor = (int128_t)1 << 70;
    int128_t quotient = dividend / divisor;
    int128_t remainder = dividend % divisor;
    
    printf("Division: 2^100 / 2^70 = 2^%lld\n", (long long)(quotient >> 1));
    printf("Remainder: %s\n", remainder == 0 ? "Zero" : "Non-zero");
    
    /* Final validation */
    printf("\n=== All tests completed ===\n");
    
    /* Return success if we got here */
    return 0;
}
