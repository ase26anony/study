/* test_double_int_cmp.c - Exercise GCC's double_int::cmp method */

#include <stdio.h>
#include <stdint.h>

/* Force compiler to use wide integer operations */
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* Volatile sink to prevent optimization */
volatile int sink;

/* Function that performs comparisons and returns results */
static int compare_int128(int128_t a, int128_t b) {
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

static int compare_uint128(uint128_t a, uint128_t b) {
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

/* Mixed signed/unsigned comparison */
static int compare_mixed(int128_t a, uint128_t b) {
    if (a < (int128_t)b) return -1;
    if (a > (int128_t)b) return 1;
    return 0;
}

int main(void) {
    int result = 0;
    
    /* 1. Wide Integer Constant Expressions */
    /* Create constants with non-zero high 64-bit parts */
    const uint128_t huge_const1 = ((uint128_t)0xFFFFFFFFFFFFFFFFULL << 64) | 0x123456789ABCDEF0ULL;
    const uint128_t huge_const2 = ((uint128_t)0xFFFFFFFFFFFFFFFEULL << 64) | 0xFEDCBA9876543210ULL;
    const uint128_t huge_const3 = ((uint128_t)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0000000000000000ULL;
    
    const int128_t signed_huge1 = ((int128_t)0x7FFFFFFFFFFFFFFFULL << 64) | 0x123456789ABCDEF0ULL;
    const int128_t signed_huge2 = ((int128_t)0x7FFFFFFFFFFFFFFEULL << 64) | 0xFEDCBA9876543210ULL;
    const int128_t signed_huge3 = -((int128_t)0x7FFFFFFFFFFFFFFFULL << 63);
    
    /* 2. Control Flow Dependent on Wide Comparisons */
    
    /* Test 1: High part less (unsigned) */
    if (huge_const2 < huge_const1) {
        result |= 1;  /* Should execute */
    }
    
    /* Test 2: High part greater (unsigned) */
    if (huge_const1 > huge_const2) {
        result |= 2;  /* Should execute */
    }
    
    /* Test 3: High part equal, low part less */
    uint128_t var1 = huge_const1;
    uint128_t var2 = huge_const1 + 1;
    if (var1 < var2) {
        result |= 4;  /* Should execute */
    }
    
    /* Test 4: High part equal, low part greater */
    if (var2 > var1) {
        result |= 8;  /* Should execute */
    }
    
    /* Test 5: Equality */
    if (huge_const1 == var1) {
        result |= 16;  /* Should execute */
    }
    
    /* Test 6: Signed comparisons with different high parts */
    if (signed_huge2 < signed_huge1) {
        result |= 32;  /* Should execute */
    }
    
    /* 3. Array Indexing with Wide Indices */
    {
        char arr[1000];
        uint128_t index = 500;
        
        /* Bounds check using wide comparison */
        if (index < sizeof(arr)) {
            arr[index] = 'X';
            result |= 64;
        }
        
        /* Loop with wide counter */
        for (uint128_t i = 0; i < 10; i++) {
            arr[i] = (char)i;
            result |= 128;
        }
    }
    
    /* 4. Mixed Signed/Unsigned Contexts */
    {
        int128_t s = -1;
        uint128_t u = 0xFFFFFFFFFFFFFFFFULL;
        
        /* This triggers unsigned comparison after conversion */
        if ((uint128_t)s < u) {
            result |= 256;  /* Should execute: 0xFF...FF < 0x00...FF */
        }
        
        if (s < (int128_t)u) {
            result |= 512;  /* Should execute: -1 < huge positive */
        }
    }
    
    /* 5. Compiler Builtins and Intrinsics */
    {
        int128_t a = signed_huge1;
        int128_t b = 1;
        int128_t sum;
        
        /* Overflow check may involve wide comparisons */
        if (__builtin_add_overflow(a, b, &sum)) {
            result |= 1024;
        } else {
            result |= 2048;
        }
        
        /* Count leading zeros on wide integer */
        uint128_t x = huge_const1;
        int clz = __builtin_clzg(x, 0);
        result |= (clz << 12);
    }
    
    /* 6. Switch statement with wide constants (requires conversion) */
    {
        uint64_t selector = 0;
        
        /* Compare against wide constants in switch */
        switch (selector) {
            case 0:
                if (huge_const1 > huge_const2) result |= 4096;
                break;
            case 1:
                if (huge_const1 < huge_const2) result |= 8192;
                break;
            default:
                if (huge_const1 == huge_const2) result |= 16384;
                break;
        }
    }
    
    /* 7. Ternary operator with wide comparison */
    {
        uint128_t max_val = (huge_const1 > huge_const2) ? huge_const1 : huge_const2;
        uint128_t min_val = (huge_const1 < huge_const2) ? huge_const1 : huge_const2;
        
        if (max_val > min_val) {
            result |= 32768;
        }
    }
    
    /* 8. Function calls that perform comparisons */
    result |= compare_int128(signed_huge1, signed_huge2) + 65536;
    result |= compare_uint128(huge_const1, huge_const2) + 131072;
    result |= compare_mixed(signed_huge1, huge_const1) + 262144;
    
    /* 9. Arithmetic that creates new values for comparison */
    {
        uint128_t prod = huge_const1 * 2;
        uint128_t sum = huge_const2 + huge_const3;
        
        if (prod > sum) {
            result |= 524288;
        }
        
        /* Shift operations */
        uint128_t shifted = huge_const1 >> 64;
        if (shifted > 0) {
            result |= 1048576;
        }
    }
    
    /* 10. Explicit comparison of all outcomes */
    /* Force all comparison results */
    int cmp_results[5] = {0};
    
    /* High part less */
    uint128_t small_high = ((uint128_t)0x1000ULL << 64);
    uint128_t large_high = ((uint128_t)0x2000ULL << 64);
    cmp_results[0] = (small_high < large_high) ? -1 : 0;
    
    /* High part greater */
    cmp_results[1] = (large_high > small_high) ? 1 : 0;
    
    /* High equal, low less */
    uint128_t same_high_low1 = large_high | 0x1000ULL;
    uint128_t same_high_low2 = large_high | 0x2000ULL;
    cmp_results[2] = (same_high_low1 < same_high_low2) ? -1 : 0;
    
    /* High equal, low greater */
    cmp_results[3] = (same_high_low2 > same_high_low1) ? 1 : 0;
    
    /* Equality */
    cmp_results[4] = (same_high_low1 == same_high_low1) ? 0 : 99;
    
    /* Aggregate all results to volatile sink */
    sink = result;
    for (int i = 0; i < 5; i++) {
        sink += cmp_results[i];
    }
    
    /* Print to prevent optimization */
    printf("Result: %d\n", result);
    
    return 0;
}
