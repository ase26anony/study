/* test_double_int_cmp.c - Comprehensive test for double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Force 128-bit integer support */
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* Static assertions with large constants */
#define CONCAT(a, b) a##b
#define STATIC_ASSERT(cond, msg) \
    typedef char CONCAT(static_assert_, __LINE__)[(cond) ? 1 : -1]

/* Test 1: Direct large constant comparisons */
static void test_large_constants(void) {
    /* Constants larger than 64 bits */
    const int128_t huge_pos = ((int128_t)1 << 70) + 12345;
    const int128_t huge_neg = -((int128_t)1 << 70) - 67890;
    const int128_t medium = ((int128_t)1 << 65);
    const int128_t zero = 0;
    
    /* Compile-time comparisons that should trigger double_int::cmp */
    STATIC_ASSERT(huge_pos > medium, "Large positive comparison");
    STATIC_ASSERT(huge_neg < zero, "Large negative comparison");
    STATIC_ASSERT(huge_pos != huge_neg, "Large inequality");
    STATIC_ASSERT(medium > zero, "Medium positive comparison");
    
    /* Test both high and low parts differ */
    const int128_t a = ((int128_t)0x123456789ABCDEF0 << 64) | 0xFEDCBA9876543210ULL;
    const int128_t b = ((int128_t)0x123456789ABCDEF0 << 64) | 0xFEDCBA9876543211ULL;
    STATIC_ASSERT(a < b, "Low part difference");
    
    const int128_t c = ((int128_t)0x123456789ABCDEF1 << 64) | 0x0ULL;
    const int128_t d = ((int128_t)0x123456789ABCDEF0 << 64) | 0xFFFFFFFFFFFFFFFFULL;
    STATIC_ASSERT(d < c, "High part dominates low part");
}

/* Test 2: Builtin overflow operations */
static void test_overflow_builtins(void) {
    long long x, y;
    long long result;
    int overflow;
    
    /* Test cases that should trigger overflow comparisons */
    x = 0x7FFFFFFFFFFFFFFFLL; /* Max int64_t */
    y = 2;
    
    overflow = __builtin_mul_overflow(x, y, &result);
    if (__builtin_constant_p(overflow)) {
        /* Force constant evaluation path */
        STATIC_ASSERT(sizeof(x) == 8, "64-bit long long");
    }
    
    /* Another overflow test */
    x = 0x8000000000000000LL; /* Min int64_t */
    y = -1;
    overflow = __builtin_smul_overflow(x, y, &result);
    
    /* Test with unsigned */
    unsigned long long ux = 0xFFFFFFFFFFFFFFFFULL;
    unsigned long long uy = 2;
    unsigned long long uresult;
    int uoverflow = __builtin_umul_overflow(ux, uy, &uresult);
    
    /* Use results to prevent dead code elimination */
    printf("Overflow tests: %d %d %d\n", overflow, uoverflow, __builtin_constant_p(overflow));
}

/* Test 3: Range analysis with complex conditions */
static void test_range_analysis(int input) {
    /* Create conditions that force VRP to use double_int comparisons */
    int x = input;
    
    if (x > 1000 && x < 2000) {
        /* Multiplication that could overflow 32-bit */
        long long y = (long long)x * x;
        
        /* Nested conditions for range refinement */
        if (y > 1500000LL && y < 3000000LL) {
            long long z = y * 2;
            
            /* Force comparison of potentially large values */
            if (z > 2500000LL) {
                printf("Range path 1: %lld\n", z);
            }
        }
    }
    
    /* Test with negative ranges */
    if (x < -1000 && x > -2000) {
        long long y = (long long)x * x;
        if (y > 1000000LL && y < 4000000LL) {
            printf("Range path 2: %lld\n", y);
        }
    }
    
    /* Large step loop that might trigger wrap analysis */
    for (long long i = 0x7000000000000000LL; 
         i < 0x7000000000000100LL; 
         i += 0x100000000LL) {
        /* Comparison in loop condition uses double_int */
        if (i > 0x7000000000000080LL) {
            printf("Loop value: %lld\n", i);
        }
    }
}

/* Test 4: Template metaprogramming (C++ version available) */
#ifdef __cplusplus
template <int128_t N, int128_t M>
struct LargeCompare {
    static const bool less = N < M;
    static const bool equal = N == M;
    static const bool greater = N > M;
    
    /* Force instantiation with different high/low combinations */
    static void verify() {
        static_assert(LargeCompare<N, M>::less == (N < M), "Template comparison error");
        static_assert(LargeCompare<N, M>::greater == (N > M), "Template comparison error");
    }
};

/* Instantiate with various large values */
template struct LargeCompare<(int128_t)1 << 65, (int128_t)1 << 66>;
template struct LargeCompare<(int128_t)-1 << 70, (int128_t)1 << 70>;
template struct LargeCompare<
    ((int128_t)0x12345678 << 64) | 0x9ABCDEF0,
    ((int128_t)0x12345678 << 64) | 0x9ABCDEF1>;
#endif

/* Test 5: Enumeration with large values */
enum big_enum : int128_t {
    BIG_ENUM_A = ((int128_t)1 << 72),
    BIG_ENUM_B = ((int128_t)1 << 72) + 1,
    BIG_ENUM_C = ((int128_t)1 << 73)
};

/* Test 6: Complex compile-time expressions */
static const int128_t compile_time_expr = 
    (((int128_t)1 << 70) * 3) / 2 + 100;

STATIC_ASSERT(compile_time_expr > ((int128_t)1 << 70), "Complex expression");

/* Test 7: Bitfield and mode attributes (GCC extensions) */
typedef int128_t __attribute__((mode(TI))) ti_int;
typedef unsigned int128_t __attribute__((mode(TI))) ti_uint;

static void test_mode_attribute(void) {
    ti_int a = ((ti_int)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    ti_int b = ((ti_int)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543211ULL;
    
    /* These comparisons should use double_int internally */
    if (a < b) {
        printf("TI mode comparison correct\n");
    }
    
    /* Test division which requires magnitude comparison */
    ti_int c = a / 2;
    if (c > 0) {
        printf("TI division positive\n");
    }
}

/* Test 8: Mixed-size operations */
static void test_mixed_size(void) {
    /* Operations between 64-bit and 128-bit values */
    uint64_t u64 = 0xFFFFFFFFFFFFFFFFULL;
    int128_t i128 = (int128_t)u64 * u64; /* 64x64 -> 128 multiplication */
    
    /* Comparisons at boundaries */
    int128_t boundary = (int128_t)1 << 63;
    if (i128 > boundary) {
        printf("Mixed size: 64x64 > 2^63\n");
    }
    
    /* Test with sign extension */
    int64_t i64 = -1000000000000000000LL;
    int128_t extended = i64;
    if (extended < 0) {
        printf("Sign extended negative\n");
    }
}

/* Test 9: __builtin_constant_p forcing */
static void test_constant_p(void) {
    const int128_t const_val = ((int128_t)1 << 75) + 123;
    
    if (__builtin_constant_p(const_val > 0)) {
        /* Force constant evaluation path */
        STATIC_ASSERT(const_val > 0, "Builtin constant positive");
    }
    
    /* Complex constant expression */
    const int128_t expr = (const_val * 2) / 3;
    if (__builtin_constant_p(expr)) {
        STATIC_ASSERT(expr > ((int128_t)1 << 74), "Complex constant expr");
    }
}

/* Main test driver */
int main(int argc, char *argv[]) {
    printf("Starting double_int::cmp coverage tests...\n");
    
    /* Run all tests */
    test_large_constants();
    test_overflow_builtins();
    
    /* Use command line input for range analysis */
    int input = 1500;
    if (argc > 1) {
        input = atoi(argv[1]);
    }
    test_range_analysis(input);
    
    #ifdef __cplusplus
    LargeCompare<(int128_t)1 << 65, (int128_t)1 << 66>::verify();
    #endif
    
    test_mode_attribute();
    test_mixed_size();
    test_constant_p();
    
    /* Runtime validation */
    const int128_t runtime_a = ((int128_t)1 << 70) + 100;
    const int128_t runtime_b = ((int128_t)1 << 70) + 200;
    
    if (runtime_a < runtime_b) {
        printf("Runtime 128-bit comparison PASS\n");
    } else {
        printf("Runtime 128-bit comparison FAIL\n");
        return 1;
    }
    
    /* Test edge cases */
    const int128_t max_low = 0xFFFFFFFFFFFFFFFFULL;
    const int128_t min_high = (int128_t)1 << 64;
    
    if (max_low < min_high) {
        printf("Edge case comparison PASS\n");
    } else {
        printf("Edge case comparison FAIL\n");
        return 1;
    }
    
    printf("All tests completed successfully!\n");
    return 0;
}
