/* double_int_test.c - Test program to trigger double_int::cmp coverage */

#include <stdio.h>
#include <stdint.h>
#include <assert.h>

/* Global arrays with __int128 constants spanning full 128-bit range */
static const __int128 global_constants[] = {
    /* High word differs */
    ((__int128)0x0000000000000000ULL << 64) | 0xFFFFFFFFFFFFFFFFULL, /* 0x0:FFFFFFFFFFFFFFFF */
    ((__int128)0x0000000000000001ULL << 64) | 0x0000000000000000ULL, /* 0x1:0000000000000000 */
    ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0000000000000000ULL, /* -1:0000000000000000 */
    ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL, /* MAX positive */
    ((__int128)0x8000000000000000ULL << 64) | 0x0000000000000000ULL, /* MIN negative */
    
    /* High words equal, low words differ */
    ((__int128)0x123456789ABCDEF0ULL << 64) | 0x1111111111111111ULL,
    ((__int128)0x123456789ABCDEF0ULL << 64) | 0x2222222222222222ULL,
    ((__int128)0x123456789ABCDEF0ULL << 64) | 0x3333333333333333ULL,
    
    /* Edge cases for comparisons */
    ((__int128)0x0000000000000000ULL << 64) | 0x0000000000000000ULL, /* Zero */
    ((__int128)0x0000000000000000ULL << 64) | 0x0000000000000001ULL, /* One */
    ((__int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL, /* -1 */
};

/* Static assertions forcing compile-time comparisons */
static_assert((((__int128)0x123456789ABCDEF0ULL << 64) | 0x1111111111111111ULL) < 
              (((__int128)0x123456789ABCDEF0ULL << 64) | 0x2222222222222222ULL),
              "Compile-time comparison 1");

static_assert((((__int128)0x0000000000000001ULL << 64) | 0x0000000000000000ULL) > 
              (((__int128)0x0000000000000000ULL << 64) | 0xFFFFFFFFFFFFFFFFULL),
              "Compile-time comparison 2");

/* Dead code with comparisons that compiler may evaluate */
static const __int128 dead_code_comparisons(void) {
    if (0) { /* Dead code, but compiler may still evaluate constants */
        __int128 a = ((__int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAAULL;
        __int128 b = ((__int128)0x5555555555555555ULL << 64) | 0xBBBBBBBBBBBBBBBBULL;
        __int128 c = ((__int128)0x6666666666666666ULL << 64) | 0xAAAAAAAAAAAAAAAAULL;
        
        /* These comparisons should trigger double_int::cmp with various cases */
        if (a < b) return a;  /* High equal, low differs */
        if (c > a) return c;  /* High differs */
        if (b > 0) return b;  /* Comparison with zero */
    }
    return 0;
}

/* Function using __int128 comparisons for range analysis */
__int128 range_analysis_test(__int128 x, __int128 y) {
    /* Constants for comparison */
    const __int128 THRESHOLD_HIGH = ((__int128)0x1000000000000000ULL << 64) | 0x0000000000000000ULL;
    const __int128 THRESHOLD_LOW = ((__int128)0x0FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    
    /* Comparisons that should trigger VRP analysis */
    if (x > THRESHOLD_HIGH && y < THRESHOLD_LOW) {
        return x - y;
    }
    
    if (x < 0 && y > 0) {
        return x + y;
    }
    
    /* Mixed-type comparison */
    unsigned long long ull = 0xFFFFFFFFFFFFFFFFULL;
    if (x > (__int128)ull) {
        return x >> 2;
    }
    
    return x * y;
}

/* Function with bitwise operations crossing word boundaries */
__int128 bitwise_operations(__int128 a, __int128 b) {
    /* Masks targeting specific words */
    const __int128 MASK_HIGH = ((__int128)0xFFFFFFFFFFFFFFFFULL << 64);
    const __int128 MASK_LOW = 0xFFFFFFFFFFFFFFFFULL;
    
    /* Operations that require reasoning about both words */
    __int128 result = 0;
    
    /* Left shift moving bits from low to high word */
    result = a << 65;
    
    /* Right shift on negative value (arithmetic shift) */
    __int128 neg = ((__int128)0x8000000000000000ULL << 64) | 0x123456789ABCDEF0ULL;
    result |= neg >> 70;
    
    /* Bitwise operations with word-specific masks */
    result |= (a & MASK_HIGH) | (b & MASK_LOW);
    
    return result;
}

/* Function with overflow checking */
int overflow_checks(__int128 a, __int128 b) {
    __int128 sum, product;
    int overflow_sum, overflow_product;
    
    /* These builtins may use double_int comparisons internally */
    overflow_sum = __builtin_add_overflow(a, b, &sum);
    overflow_product = __builtin_mul_overflow(a, b, &product);
    
    /* Comparisons after overflow checks */
    if (!overflow_sum && sum > 0) {
        return 1;
    }
    
    if (!overflow_product && product < a && product < b) {
        return 2;
    }
    
    return 0;
}

/* Loop with __int128 induction variable */
void loop_with_128bit_induction(void) {
    /* Loop bounds that differ in both high and low words */
    __int128 start = ((__int128)0x0000000000001000ULL << 64) | 0x0000000000000000ULL;
    __int128 end = ((__int128)0x0000000000001001ULL << 64) | 0x0000000000000000ULL;
    __int128 step = ((__int128)0x0000000000000000ULL << 64) | 0x1000000000000000ULL;
    
    volatile __int128 sum = 0; /* volatile to prevent complete optimization */
    
    /* Loop with comparisons at each iteration */
    for (__int128 i = start; i < end; i += step) {
        sum += i;
        
        /* Additional comparison inside loop */
        if (i > (start + step * 2)) {
            sum -= step;
        }
    }
    
    /* Prevent unused variable warning */
    (void)sum;
}

/* Function using ternary operator with mixed types */
__int128 ternary_mixed_types(int selector, unsigned long long base) {
    /* Ternary with __int128 and narrower type */
    __int128 large_const = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    
    return selector ? large_const : (__int128)base;
}

/* Main test function */
int main(void) {
    __int128 result = 0;
    __int128 checksum = 0;
    
    /* Test 1: Process global constants */
    for (size_t i = 0; i < sizeof(global_constants)/sizeof(global_constants[0]); i++) {
        checksum += global_constants[i];
    }
    
    /* Test 2: Range analysis with comparisons */
    __int128 test_val1 = ((__int128)0x2000000000000000ULL << 64) | 0x0000000000000000ULL;
    __int128 test_val2 = ((__int128)0x0000000000000000ULL << 64) | 0x8000000000000000ULL;
    
    result = range_analysis_test(test_val1, test_val2);
    checksum += result;
    
    /* Test 3: Bitwise operations */
    __int128 a = ((__int128)0x5555555555555555ULL << 64) | 0xAAAAAAAAAAAAAAAAULL;
    __int128 b = ((__int128)0x3333333333333333ULL << 64) | 0xCCCCCCCCCCCCCCCCULL;
    
    result = bitwise_operations(a, b);
    checksum += result;
    
    /* Test 4: Overflow checks */
    checksum += overflow_checks(a, b);
    
    /* Test 5: Loop with 128-bit induction */
    loop_with_128bit_induction();
    
    /* Test 6: Ternary with mixed types */
    result = ternary_mixed_types(1, 0xFFFFFFFFFFFFFFFFULL);
    checksum += result;
    
    /* Print checksum (simplified for demonstration) */
    unsigned long long low = (unsigned long long)checksum;
    unsigned long long high = (unsigned long long)(checksum >> 64);
    
    printf("Checksum: 0x%016llx%016llx\n", high, low);
    printf("Test completed successfully.\n");
    
    return 0;
}
