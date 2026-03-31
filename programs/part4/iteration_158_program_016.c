/* test_double_int_cmp.c
 * Compile with: gcc -O2 -Wall -Wextra -c test_double_int_cmp.c -o test.o
 * For more detailed analysis: gcc -O3 -fdump-tree-all -c test_double_int_cmp.c
 */

#include <stdio.h>
#include <stdint.h>

/* Volatile sink to prevent dead code elimination */
static volatile int sink;

/* Helper function to force comparisons */
static int compare_128(__int128 a, __int128 b) {
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

static int compare_u128(unsigned __int128 a, unsigned __int128 b) {
    if (a < b) return -1;
    if (a > b) return 1;
    return 0;
}

/* Function that uses mixed comparisons */
static int mixed_comparisons(__int128 s1, unsigned __int128 u1, 
                             __int128 s2, unsigned __int128 u2) {
    int result = 0;
    
    /* Compare signed with signed */
    if (s1 < s2) result |= 1;
    if (s1 > s2) result |= 2;
    
    /* Compare unsigned with unsigned */
    if (u1 < u2) result |= 4;
    if (u1 > u2) result |= 8;
    
    /* Mixed comparisons (will promote to unsigned) */
    if ((unsigned __int128)s1 < u2) result |= 16;
    if (u1 < (unsigned __int128)s2) result |= 32;
    
    return result;
}

int main(void) {
    /* 1. Wide Integer Constant Expressions */
    /* Create constants with non-zero high 64-bit parts */
    const __int128 big_neg = ((__int128)-1 << 64) | 0x123456789ABCDEF0ULL;
    const __int128 big_pos = ((__int128)0x123456789ABCDEFULL << 64) | 0xFEDCBA9876543210ULL;
    const unsigned __int128 huge = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x123456789ABCDEF0ULL;
    
    /* 2. Control Flow Dependent on Wide Comparisons */
    __int128 a = big_neg;
    __int128 b = big_pos;
    unsigned __int128 ua = huge;
    unsigned __int128 ub = huge >> 1;
    
    /* Test all comparison outcomes */
    
    /* Case 1: High part less (unsigned comparison) */
    /* a.high < b.high (since a is negative, b is positive) */
    if (a < b) {
        sink = 1;  /* Should be taken */
    }
    
    /* Case 2: High part greater */
    /* b.high > a.high */
    if (b > a) {
        sink = 2;  /* Should be taken */
    }
    
    /* Case 3: High parts equal, low part less */
    __int128 c = big_pos;
    __int128 d = big_pos + 1;  /* Same high part, different low part */
    if (c < d) {
        sink = 3;  /* Should be taken */
    }
    
    /* Case 4: High parts equal, low part greater */
    if (d > c) {
        sink = 4;  /* Should be taken */
    }
    
    /* Case 5: Equality */
    __int128 e = big_pos;
    __int128 f = big_pos;
    if (e == f) {
        sink = 5;  /* Should be taken */
    }
    
    /* 3. Array Indexing with Wide Indices */
    int small_array[10] = {0};
    
    /* Use wide integer in bounds checking */
    for (unsigned __int128 i = 0; i < 10; i++) {
        small_array[i] = (int)i;
    }
    
    /* 4. Mixed Signed/Unsigned Contexts */
    /* This will trigger unsigned comparison in double_int::cmp */
    __int128 signed_val = -1;  /* High bits all 1s in two's complement */
    unsigned __int128 unsigned_val = 0xFFFFFFFFFFFFFFFFULL;  /* Only low 64 bits set */
    
    /* Important: This comparison promotes to unsigned! */
    if ((unsigned __int128)signed_val < unsigned_val) {
        /* signed_val as unsigned is 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF
         * unsigned_val is 0x0000000000000000FFFFFFFFFFFFFFFF
         * So signed_val > unsigned_val as unsigned */
        sink = 6;  /* Should NOT be taken */
    } else {
        sink = 7;  /* Should be taken */
    }
    
    /* Ternary operator with wide comparison */
    __int128 result = (a < b) ? a : b;
    sink = (int)result;
    
    /* 5. Compiler Builtins and Intrinsics */
    __int128 x = big_pos;
    __int128 y = big_neg;
    __int128 sum;
    
    /* Overflow checking builtin */
    if (__builtin_add_overflow(x, y, &sum)) {
        sink = 8;
    } else {
        sink = 9;
    }
    
    /* Bit counting builtins */
    unsigned __int128 bits = huge;
    int leading_zeros = __builtin_clzg(bits, 0);
    sink = leading_zeros;
    
    /* Switch statement with wide constants (requires lowering) */
    unsigned __int128 switch_val = huge >> 64;
    switch (switch_val) {
        case 0x123456789ABCDEFULL:
            sink = 100;
            break;
        case 0x0ULL:
            sink = 101;
            break;
        default:
            sink = 102;
    }
    
    /* Complex arithmetic that might trigger constant folding */
    const __int128 complex_expr = 
        ((big_pos * 3) >> 2) + (big_neg / 5) - (huge % 1000);
    
    /* Force comparisons in function calls */
    int cmp1 = compare_128(big_pos, big_neg);
    int cmp2 = compare_u128(huge, huge >> 1);
    int cmp3 = mixed_comparisons(big_neg, huge, big_pos, huge >> 2);
    
    sink = cmp1 + cmp2 + cmp3;
    
    /* Print results to prevent optimization */
    printf("Results: %d %d %d %d %d\n", 
           (int)(a < b), (int)(b > a), (int)(c < d), 
           (int)(d > c), (int)(e == f));
    
    return 0;
}
