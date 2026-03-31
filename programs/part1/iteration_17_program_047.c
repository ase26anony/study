/* test-double-int.c - Target GCC's double_int comparison logic */
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Define 128-bit constants that cross 64-bit boundaries */
#define HIGH_BIT_64   0x8000000000000000ULL
#define MAX_64        0xFFFFFFFFFFFFFFFFULL
#define MID_128       0x80000000000000008000000000000000ULL
#define LARGE_128     0xFFFFFFFF00000000FFFFFFFF00000000ULL

/* Force compile-time evaluation with static assertions */
_Static_assert(((__int128)HIGH_BIT_64 << 64) > (__int128)0, 
               "High-bit shift must be positive");
_Static_assert(((__int128)MAX_64) < ((__int128)MAX_64 << 64),
               "Shifted max must be larger");

/* Function to create value ranges that span both words */
static __int128 create_range(int selector) {
    switch (selector) {
        case 0: return (__int128)0;
        case 1: return (__int128)MAX_64;                    /* Low word max */
        case 2: return (__int128)HIGH_BIT_64 << 64;         /* High word bit 63 set */
        case 3: return (__int128)MAX_64 << 64;              /* High word all bits */
        case 4: return ~((__int128)0);                      /* All bits set (signed -1) */
        case 5: return ((__int128)MAX_64 << 64) | MAX_64;   /* Max positive signed */
        case 6: return (__int128)HIGH_BIT_64 << 63;         /* Bit 126 set */
        default: return (__int128)selector;
    }
}

/* Force range analysis with loop induction variables */
static unsigned __int128 analyze_ranges(void) {
    unsigned __int128 checksum = 0;
    
    /* Loop with induction variable crossing 64-bit boundary */
    for (unsigned __int128 i = MAX_64 - 10; i < MAX_64 + 10; i++) {
        /* Comparisons that exercise high-word logic */
        if (i < MAX_64) checksum += 1;
        if (i > MAX_64) checksum += 2;
        if (i == MAX_64) checksum += 4;
        
        /* Mixed-type comparisons */
        if (i < (unsigned long long)MAX_64) checksum += 8;
        if ((__int128)i < (__int128)MAX_64) checksum += 16;
    }
    
    return checksum;
}

/* Test overflow operations requiring wide comparisons */
static __int128 test_overflow_ops(void) {
    __int128 result = 0;
    __int128 a, b;
    int overflow;
    
    /* Test near overflow boundaries */
    a = ((__int128)MAX_64 >> 2);
    b = ((__int128)MAX_64 >> 1);
    
    /* These should trigger overflow checking with double_int comparisons */
    overflow = __builtin_add_overflow(a, b, &result);
    if (overflow) result = -1;
    
    overflow = __builtin_mul_overflow(a, (__int128)3, &result);
    if (overflow) result = -2;
    
    /* Test signed overflow at boundary */
    a = (__int128)HIGH_BIT_64 << 64;  /* Most negative */
    b = (__int128)-1;
    overflow = __builtin_add_overflow(a, b, &result);
    if (!overflow) result = -3;
    
    return result;
}

/* Bitwise operations crossing 64-bit boundaries */
static unsigned __int128 test_bitwise_ops(void) {
    unsigned __int128 x = LARGE_128;
    unsigned __int128 y = ~LARGE_128;
    unsigned __int128 checksum = 0;
    
    /* Operations that mix high and low words */
    checksum += x & y;
    checksum += x | y;
    checksum += x ^ y;
    
    /* Shifts crossing word boundaries */
    checksum += x << 1;
    checksum += x >> 1;
    checksum += y << 32;
    checksum += y >> 32;
    checksum += x << 64;
    checksum += x >> 64;
    
    /* Compound operations */
    checksum += (x & ((unsigned __int128)MAX_64 << 64)) | (y & MAX_64);
    checksum += (x << 1) & (y >> 1);
    
    return checksum;
}

/* Switch statement with __int128 case labels */
static int test_switch(__int128 value) {
    /* Force compiler to generate comparison tree */
    switch ((unsigned __int128)value & 0xF) {
        case 0: return 0;
        case ((unsigned __int128)MAX_64 & 0xF): return 1;
        case ((unsigned __int128)(MAX_64 << 64) & 0xF): return 2;
        case ((unsigned __int128)((MAX_64 << 64) | MAX_64) & 0xF): return 3;
        default: return -1;
    }
}

/* Variadic function to force conversions */
static void test_variadic(__int128 a, __int128 b) {
    /* These conversions may trigger double_int operations */
    printf("Comparison: %lld vs %lld\n", 
           (long long)(a >> 64), (long long)(b >> 64));
    printf("Low words: %llu %llu\n",
           (unsigned long long)(a & MAX_64),
           (unsigned long long)(b & MAX_64));
}

/* Main test driver */
int main(void) {
    unsigned __int128 checksum = 0;
    __int128 values[8];
    
    /* Initialize array with values that exercise different comparison paths */
    values[0] = (__int128)0;
    values[1] = (__int128)MAX_64;                    /* High=0, Low=max */
    values[2] = (__int128)HIGH_BIT_64 << 64;         /* High=bit63, Low=0 */
    values[3] = (__int128)MAX_64 << 64;              /* High=max, Low=0 */
    values[4] = ~((__int128)0);                      /* High=max, Low=max (signed -1) */
    values[5] = ((__int128)MAX_64 << 64) | MAX_64;   /* Max positive signed */
    values[6] = (__int128)HIGH_BIT_64 << 63;         /* Bit 126 set */
    values[7] = (__int128)-1 * values[2];            /* Negative of value[2] */
    
    /* Test 1: Compare values where high words differ */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (values[i] < values[j]) checksum += 1;
            if (values[i] > values[j]) checksum += 2;
            if (values[i] == values[j]) checksum += 4;
            if (values[i] <= values[j]) checksum += 8;
            if (values[i] >= values[j]) checksum += 16;
        }
    }
    
    /* Test 2: Range analysis with loops */
    checksum += analyze_ranges();
    
    /* Test 3: Overflow operations */
    __int128 overflow_result = test_overflow_ops();
    checksum += (unsigned __int128)overflow_result;
    
    /* Test 4: Bitwise operations */
    checksum += test_bitwise_ops();
    
    /* Test 5: Switch with __int128 */
    for (int i = 0; i < 8; i++) {
        checksum += test_switch(values[i]);
    }
    
    /* Test 6: Mixed-precision comparisons */
    for (int i = 0; i < 8; i++) {
        /* Compare with 64-bit types */
        if (values[i] < (long long)MAX_64) checksum += 1;
        if ((unsigned __int128)values[i] > (unsigned long long)MAX_64) checksum += 2;
        
        /* Ternary with mixed types */
        __int128 temp = (i & 1) ? values[i] : (__int128)(i);
        checksum += (unsigned __int128)temp;
    }
    
    /* Test 7: Built-in functions */
    for (int i = 0; i < 8; i++) {
        /* Count leading zeros in high word */
        int clz = __builtin_clzll((unsigned long long)(values[i] >> 64));
        checksum += clz;
        
        /* Population count */
        int pop = __builtin_popcountll((unsigned long long)(values[i] & MAX_64));
        checksum += pop;
        
        /* Branch prediction hints */
        if (__builtin_expect(values[i] < values[(i+1)&7], 0)) {
            checksum += 1;
        }
    }
    
    /* Test 8: Create value ranges using helper */
    for (int i = 0; i < 8; i++) {
        __int128 val = create_range(i);
        checksum += (unsigned __int128)val;
    }
    
    /* Print checksum to prevent dead code elimination */
    printf("Checksum (high 64 bits): %llu\n", 
           (unsigned long long)(checksum >> 64));
    printf("Checksum (low 64 bits): %llu\n",
           (unsigned long long)(checksum & MAX_64));
    
    /* Force variadic conversions */
    test_variadic(values[2], values[5]);
    
    return 0;
}
