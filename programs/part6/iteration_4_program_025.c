/* test_double_int_cmp.c - Comprehensive test for double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <limits.h>

/* Approach 1: Large integer constants with compile-time comparisons */
#ifdef __SIZEOF_INT128__
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;
#else
/* Fallback for compilers without __int128 support */
typedef struct { uint64_t hi; uint64_t lo; } int128_t;
#endif

/* Static assertions with large constants */
#define STATIC_ASSERT(cond) typedef char static_assert_##__LINE__[(cond)?1:-1]

#ifdef __SIZEOF_INT128__
/* These will trigger double_int::cmp during constant folding */
STATIC_ASSERT(((int128_t)1 << 70) > 0);
STATIC_ASSERT(((int128_t)1 << 70) > ((int128_t)1 << 69));
STATIC_ASSERT(((int128_t)1 << 100) > ((int128_t)1 << 90));
STATIC_ASSERT(((int128_t)-1 << 70) < 0);
STATIC_ASSERT(((int128_t)0x7FFFFFFFFFFFFFFFLL << 64) > 0x7FFFFFFFFFFFFFFFLL);

/* Test equality comparisons */
STATIC_ASSERT(((int128_t)0x123456789ABCDEF0ULL << 64) == 
              ((int128_t)0x123456789ABCDEF0ULL << 64));
STATIC_ASSERT(((int128_t)0x123456789ABCDEF0ULL << 64) != 
              ((int128_t)0xFEDCBA9876543210ULL << 64));
#endif

/* Approach 2: Builtin overflow operations */
void test_overflow_builtins(void) {
    long long a, b, result;
    int overflow;
    
    /* Test 1: Multiplication that overflows 64-bit */
    a = 0x7FFFFFFFFFFFFFFFLL;  /* Max positive int64_t */
    b = 2;
    overflow = __builtin_mul_overflow(a, b, &result);
    printf("Overflow test 1: %lld * 2 overflow? %s\n", 
           a, overflow ? "YES" : "NO");
    
    /* Test 2: Large multiplication with overflow detection */
    a = 0x123456789ABCDEFLL;
    b = 0xFEDCBA987654321LL;
    overflow = __builtin_mul_overflow(a, b, &result);
    printf("Overflow test 2: large multiplication overflow? %s\n",
           overflow ? "YES" : "NO");
    
    /* Test 3: Addition overflow */
    a = 0x7FFFFFFFFFFFFFFFLL;
    b = 1;
    overflow = __builtin_add_overflow(a, b, &result);
    printf("Overflow test 3: max_int64 + 1 overflow? %s\n",
           overflow ? "YES" : "NO");
    
    /* Test 4: Compile-time overflow check */
    if (__builtin_constant_p(__builtin_mul_overflow_p(0x7FFFFFFFFFFFFFFFLL, 
                                                      2, 
                                                      (long long)0))) {
        printf("Compile-time overflow check performed\n");
    }
}

/* Approach 3: Range analysis with complex conditions */
void test_range_analysis(int x) {
    /* Complex range analysis that uses double_int comparisons */
    if (x > 1000 && x < 2000) {
        /* This multiplication's range calculation uses double_int::cmp */
        long long y = (long long)x * x;
        
        /* Nested conditions for more complex range analysis */
        if (y > 1000000 && y < 4000000) {
            long long z = y * 2;
            printf("Range test: x=%d, y=%lld, z=%lld\n", x, y, z);
            
            /* Additional comparison that might use double_int */
            if (z > 2000000 && z < 8000000) {
                printf("Nested range condition passed\n");
            }
        }
    }
    
    /* Test with negative ranges */
    if (x < -1000 && x > -2000) {
        long long y = (long long)x * x;  /* Positive result */
        if (y > 1000000 && y < 4000000) {
            printf("Negative range test passed: x=%d, y=%lld\n", x, y);
        }
    }
}

/* Approach 4: Loop induction variables with large steps */
void test_loop_induction(void) {
    /* Loop with induction variable that might trigger wrap analysis */
    for (int64_t i = 0; i < 1000; i += 0x7FFFFFFFFFFFFFFFLL / 1000) {
        /* The compiler might analyze this loop using double_int comparisons */
        printf("Loop iteration with large step: %lld\n", (long long)i);
        if (i > 0x3FFFFFFFFFFFFFFFLL) {
            printf("Crossed half-way point\n");
            break;
        }
    }
}

/* Approach 5: Bitwise operations with large shifts */
void test_large_shifts(void) {
    uint64_t large_val = 0xFFFFFFFFFFFFFFFFULL;
    
    /* Operations that create wide integers */
    uint64_t shifted = large_val << 2;  /* This overflows 64-bit */
    
    /* Comparisons that might use double_int internally */
    if ((shifted >> 2) == large_val) {
        printf("Shift test passed: reversible shift\n");
    }
    
    /* Test with compile-time large shifts */
    #ifdef __SIZEOF_INT128__
    int128_t huge = ((int128_t)1 << 120);
    if (huge > 0) {
        printf("Large shift test: 1 << 120 is positive\n");
    }
    #endif
}

/* Approach 6: Mixed signed/unsigned comparisons */
void test_mixed_comparisons(void) {
    int64_t signed_val = -100;
    uint64_t unsigned_val = 1000;
    
    /* These comparisons might trigger different code paths in double_int::cmp */
    if (signed_val < (int64_t)unsigned_val) {
        printf("Mixed comparison 1: signed < unsigned\n");
    }
    
    if ((uint64_t)signed_val > unsigned_val) {
        /* This might be false due to wrap-around */
        printf("Mixed comparison 2: (unsigned)signed > unsigned\n");
    }
    
    /* Large mixed comparisons */
    int64_t large_signed = 0x7FFFFFFFFFFFFFFFLL;
    uint64_t large_unsigned = 0xFFFFFFFFFFFFFFFFULL;
    
    if (large_signed < (int64_t)large_unsigned) {
        printf("Large mixed comparison passed\n");
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
    test_range_analysis(-1500);
    printf("\n");
    
    /* Test 3: Loop induction */
    printf("3. Testing loop induction:\n");
    test_loop_induction();
    printf("\n");
    
    /* Test 4: Large shifts */
    printf("4. Testing large shifts:\n");
    test_large_shifts();
    printf("\n");
    
    /* Test 5: Mixed comparisons */
    printf("5. Testing mixed signed/unsigned comparisons:\n");
    test_mixed_comparisons();
    printf("\n");
    
    /* Additional compile-time tests */
    #ifdef __SIZEOF_INT128__
    printf("6. Runtime tests with 128-bit integers:\n");
    
    /* Runtime comparisons of large values */
    int128_t a = ((int128_t)1 << 70);
    int128_t b = ((int128_t)1 << 69);
    
    if (a > b) {
        printf("Runtime 128-bit comparison: 1<<70 > 1<<69\n");
    }
    
    if (a != b) {
        printf("Runtime 128-bit inequality: 1<<70 != 1<<69\n");
    }
    
    /* Test with maximum values */
    int128_t max128 = ((int128_t)0x7FFFFFFFFFFFFFFFLL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    int128_t min128 = ((int128_t)1 << 127);
    
    if (max128 > min128) {
        printf("Runtime max/min comparison passed\n");
    }
    #endif
    
    printf("\n=== All tests completed ===\n");
    
    return 0;
}
