/* test_double_int_cmp.c - Exercise double_int::cmp unsigned comparison logic */

#include <stdio.h>
#include <stdint.h>

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

/* Force compiler to generate comparisons in different contexts */
volatile int sink;

int main(void) {
    /* 1. Wide Integer Constant Expressions with non-zero high parts */
    const __int128 big1 = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    const __int128 big2 = ((__int128)0x123456789ABCDEF0ULL << 64) | 0x0123456789ABCDEFULL;
    const __int128 big3 = ((__int128)0x123456789ABCDEF1ULL << 64) | 0xFEDCBA9876543210ULL;
    const __int128 big4 = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFEDCBA9876543210ULL;
    
    const unsigned __int128 ubig1 = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0000000000000000ULL;
    const unsigned __int128 ubig2 = ((unsigned __int128)0xFFFFFFFFFFFFFFFEULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    const unsigned __int128 ubig3 = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL << 64) | 0x0000000000000001ULL;
    
    /* 2. Control Flow Dependent on Wide Comparisons */
    
    /* Case 1: High part less (returns -1) */
    if (big2 < big1) {  /* high equal, low less */
        sink = 1;
    }
    
    /* Case 2: High part greater (returns 1) */
    if (big3 > big1) {  /* high greater */
        sink = 2;
    }
    
    /* Case 3: High part equal, low part less (returns -1) */
    const __int128 big5 = ((__int128)0x123456789ABCDEF0ULL << 64) | 0x0000000000000000ULL;
    if (big5 < big1) {  /* high equal, low less */
        sink = 3;
    }
    
    /* Case 4: High part equal, low part greater (returns 1) */
    const __int128 big6 = ((__int128)0x123456789ABCDEF0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    if (big6 > big1) {  /* high equal, low greater */
        sink = 4;
    }
    
    /* Case 5: Equality (returns 0) */
    if (big1 == big4) {  /* completely equal */
        sink = 5;
    }
    
    /* 3. Array Indexing with Wide Indices */
    int small_array[10] = {0};
    __int128 index = 5;
    if (index >= 0 && index < 10) {
        small_array[index] = 42;
    }
    
    /* Loop with wide counter */
    for (__int128 i = 0; i < 5; i++) {
        small_array[i] = (int)i;
    }
    
    /* 4. Mixed Signed/Unsigned Contexts */
    
    /* Signed with high bit set vs unsigned */
    __int128 s_neg = -1;  /* 0xFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF */
    unsigned __int128 u_max = ~(unsigned __int128)0;
    
    if (s_neg < u_max) {  /* Will compare as unsigned due to promotion */
        sink = 6;
    }
    
    /* Ternary operator with wide comparison */
    __int128 result = (big1 < big2) ? big1 : big2;
    sink = (int)result;
    
    /* 5. Compiler Builtins and Intrinsics */
    
    /* Overflow checking */
    __int128 a = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
    __int128 b = 1;
    __int128 overflow_result;
    
    if (__builtin_add_overflow(a, b, &overflow_result)) {
        sink = 7;
    }
    
    /* Bit counting builtins */
    unsigned __int128 uval = 0x8000000000000000ULL;
    int leading_zeros = __builtin_clzg(uval, 0);
    sink = leading_zeros;
    
    /* 6. Switch statement with wide constants */
    __int128 switch_val = big1;
    switch (switch_val == big1 ? 1 : 
            switch_val < big2 ? 2 : 
            switch_val > big3 ? 3 : 0) {
        case 1: sink = 100; break;
        case 2: sink = 200; break;
        case 3: sink = 300; break;
        default: sink = 400; break;
    }
    
    /* 7. Function calls that force comparisons */
    int cmp1 = compare_128(big1, big2);  /* high equal, low less -> -1 */
    int cmp2 = compare_128(big3, big1);  /* high greater -> 1 */
    int cmp3 = compare_128(big1, big4);  /* equal -> 0 */
    int cmp4 = compare_128(big1, big5);  /* high equal, low greater -> 1 */
    int cmp5 = compare_128(big5, big1);  /* high equal, low less -> -1 */
    
    int cmp6 = compare_u128(ubig1, ubig2);  /* high greater -> 1 */
    int cmp7 = compare_u128(ubig2, ubig1);  /* high less -> -1 */
    int cmp8 = compare_u128(ubig1, ubig3);  /* high equal, low less -> -1 */
    
    /* Aggregate results to prevent optimization */
    volatile int total = cmp1 + cmp2 + cmp3 + cmp4 + cmp5 + cmp6 + cmp7 + cmp8;
    (void)total;
    
    /* Print something to prevent complete optimization */
    printf("Test completed. Comparisons performed: %d\n", 
           cmp1 + cmp2 + cmp3 + cmp4 + cmp5 + cmp6 + cmp7 + cmp8);
    
    return 0;
}
