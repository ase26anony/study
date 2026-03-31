/* test_double_int_cmp.c - Exercise GCC's double_int::cmp method */
#include <stdio.h>
#include <stdint.h>

/* Force compiler to use wide integer operations */
typedef __int128 int128_t;
typedef unsigned __int128 uint128_t;

/* Volatile sink to prevent optimization */
volatile int sink;

/* Function to force comparisons */
int compare_int128(int128_t a, int128_t b) {
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

int compare_uint128(uint128_t a, uint128_t b) {
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

/* Mixed signed/unsigned comparison */
int compare_mixed(int128_t a, uint128_t b) {
    return (a < b) ? -1 : (a > b) ? 1 : 0;
}

int main(void) {
    /* Large constants that require 128-bit representation */
    const uint128_t HUGE_CONST = ((uint128_t)0xFFFFFFFFFFFFFFFFULL << 64) | 0x123456789ABCDEF0ULL;
    const int128_t NEG_HUGE = -((int128_t)0x7FFFFFFFFFFFFFFFULL << 63);
    const uint128_t MEDIUM_CONST = ((uint128_t)0x8000000000000000ULL << 64) | 0x0ULL;
    
    /* Test case 1: High part less (unsigned comparison) */
    {
        uint128_t a = HUGE_CONST;
        uint128_t b = HUGE_CONST + ((uint128_t)1 << 64);
        sink = compare_uint128(a, b);  /* Should return -1 */
    }
    
    /* Test case 2: High part greater (unsigned comparison) */
    {
        uint128_t a = HUGE_CONST + ((uint128_t)1 << 64);
        uint128_t b = HUGE_CONST;
        sink = compare_uint128(a, b);  /* Should return 1 */
    }
    
    /* Test case 3: High parts equal, low part less */
    {
        uint128_t a = HUGE_CONST;
        uint128_t b = HUGE_CONST + 1;
        sink = compare_uint128(a, b);  /* Should return -1 */
    }
    
    /* Test case 4: High parts equal, low part greater */
    {
        uint128_t a = HUGE_CONST + 1;
        uint128_t b = HUGE_CONST;
        sink = compare_uint128(a, b);  /* Should return 1 */
    }
    
    /* Test case 5: Equality */
    {
        uint128_t a = HUGE_CONST;
        uint128_t b = HUGE_CONST;
        sink = compare_uint128(a, b);  /* Should return 0 */
    }
    
    /* Test case 6: Mixed signed/unsigned with negative values */
    {
        int128_t a = -1;  /* All bits set in two's complement */
        uint128_t b = 0xFFFFFFFFFFFFFFFFULL;  /* Only low 64 bits set */
        sink = compare_mixed(a, b);  /* Forces unsigned comparison of high parts */
    }
    
    /* Test case 7: Array indexing with wide integers */
    {
        char arr[1000];
        uint128_t index = 500;
        if (index < sizeof(arr)) {
            sink = arr[index];  /* Bounds check uses comparison */
        }
    }
    
    /* Test case 8: Loop with wide integer counter */
    {
        uint128_t start = HUGE_CONST;
        uint128_t end = HUGE_CONST + 10;
        for (uint128_t i = start; i < end; i++) {
            sink += (int)i;  /* Loop condition uses comparison */
        }
    }
    
    /* Test case 9: Switch with wide constants (may be lowered to if-chain) */
    {
        uint128_t val = MEDIUM_CONST;
        switch (val) {
            case ((uint128_t)0x8000000000000000ULL << 64):
                sink = 1;
                break;
            case ((uint128_t)0x8000000000000001ULL << 64):
                sink = 2;
                break;
            default:
                sink = 3;
        }
    }
    
    /* Test case 10: Builtin overflow checks */
    {
        int128_t a = ((int128_t)0x7FFFFFFFFFFFFFFFULL << 64);
        int128_t b = 1;
        int128_t result;
        if (__builtin_add_overflow(a, b, &result)) {
            sink = 4;  /* Overflow path */
        }
    }
    
    /* Test case 11: Bit operations with wide integers */
    {
        uint128_t a = HUGE_CONST;
        uint128_t b = HUGE_CONST >> 1;
        if (__builtin_clzg(a, 0) < __builtin_clzg(b, 0)) {
            sink = 5;
        }
    }
    
    /* Test case 12: Constant folding with arithmetic */
    {
        const uint128_t c1 = HUGE_CONST + 100;
        const uint128_t c2 = HUGE_CONST + 200;
        const int cmp_result = (c1 < c2) ? -1 : (c1 > c2) ? 1 : 0;
        sink = cmp_result;  /* Should be folded at compile time */
    }
    
    /* Test case 13: Ternary operator with wide comparison */
    {
        uint128_t x = HUGE_CONST;
        uint128_t y = HUGE_CONST + ((uint128_t)1 << 63);
        int result = (x < y) ? 100 : 200;
        sink = result;
    }
    
    /* Test case 14: Function pointer to force code generation */
    {
        int (*cmp_ptr)(int128_t, int128_t) = compare_int128;
        sink = cmp_ptr(NEG_HUGE, NEG_HUGE + 1);
    }
    
    printf("Test completed (sink = %d)\n", sink);
    return 0;
}
