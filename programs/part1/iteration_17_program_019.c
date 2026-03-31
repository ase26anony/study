/* test-double-int.c */
#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Define 128-bit constants that cross 64-bit boundaries */
#define HIGH_BIT_64    0x8000000000000000ULL
#define MAX_64         0xFFFFFFFFFFFFFFFFULL
#define MID_128        0x123456789ABCDEF0ULL

/* Force compile-time evaluation with static assertions */
_Static_assert(((__int128)HIGH_BIT_64 << 64) > 0, 
               "High-bit shift should be positive");
_Static_assert(((__int128)MAX_64) < ((__int128)HIGH_BIT_64 << 64),
               "Comparison with high word difference");

/* Test function that exercises __int128 range analysis */
static __int128 process_value(unsigned __int128 x, int shift) {
    /* Operations that may overflow and require wide comparisons */
    __int128 result = (__int128)x;
    
    /* Force high-word comparisons */
    if (x > ((unsigned __int128)MAX_64 << 64 | MAX_64)) {
        result = result >> 1;
    }
    
    /* Bitwise operations crossing 64-bit boundary */
    result = result & (((__int128)MAX_64 << 64) | MID_128);
    result = result | (((__int128)HIGH_BIT_64) << 32);
    
    return result;
}

/* Function with __int128 comparisons for VRP */
static int compare_range(__int128 a, __int128 b) {
    int cmp = 0;
    
    /* Direct comparisons that should use double_int::cmp */
    if (a < b) cmp = -1;
    else if (a > b) cmp = 1;
    
    /* Additional comparisons through ternary operator */
    cmp += (a == b) ? 0 : ((a < b) ? -2 : 2);
    
    return cmp;
}

/* Test overflow operations */
static int check_overflow(__int128 a, __int128 b) {
    __int128 sum, diff, prod;
    int overflow = 0;
    
    /* Use builtins for overflow checking */
    overflow |= __builtin_add_overflow(a, b, &sum);
    overflow |= __builtin_sub_overflow(a, b, &diff);
    overflow |= __builtin_mul_overflow(a, b, &prod);
    
    /* Comparisons after overflow checks */
    if (sum > diff) overflow++;
    if (prod < a && prod < b) overflow++;
    
    return overflow;
}

/* Switch with __int128 cases (compile-time constants) */
static const char* classify_int128(__int128 val) {
    /* GCC should generate comparison trees for these cases */
    switch (val) {
        case ((__int128)0):
            return "zero";
        case ((__int128)1 << 64):
            return "2^64";
        case -((__int128)1 << 64):
            return "-2^64";
        case ((__int128)MAX_64 << 64):
            return "max_high";
        default:
            if (val > 0) return "positive";
            if (val < 0) return "negative";
            return "unknown";
    }
}

/* Mixed-precision operations */
static __int128 mixed_operations(size_t count, long long base) {
    unsigned __int128 acc = 0;
    
    for (size_t i = 0; i < count; i++) {
        /* Mix 128-bit and 64-bit operations */
        __int128 temp = (__int128)base * i;
        
        /* Force conversions and comparisons */
        if ((unsigned __int128)temp > (unsigned __int128)acc) {
            acc = temp;
        }
        
        /* Bitwise operations across boundary */
        acc = (acc << 1) | (i & 1);
        
        /* Compare with 64-bit boundary */
        if (acc > (unsigned __int128)MAX_64) {
            acc = acc & (((unsigned __int128)MAX_64 << 64) | MAX_64);
        }
    }
    
    return (__int128)acc;
}

/* Array processing with __int128 */
static unsigned long long process_array(__int128 arr[], int size) {
    unsigned long long checksum = 0;
    __int128 max_val = ((__int128)1 << 127) - 1;  /* INT128_MAX approx */
    __int128 min_val = -((__int128)1 << 127);     /* INT128_MIN approx */
    
    for (int i = 0; i < size; i++) {
        /* Comparisons that exercise high/low word logic */
        if (arr[i] > max_val) {
            arr[i] = max_val;
        }
        if (arr[i] < min_val) {
            arr[i] = min_val;
        }
        
        /* Accumulate with overflow */
        checksum += (unsigned long long)(arr[i] & MAX_64);
        checksum += (unsigned long long)((arr[i] >> 64) & MAX_64);
        
        /* Compare adjacent elements */
        if (i > 0) {
            int cmp = compare_range(arr[i], arr[i-1]);
            checksum += cmp;
        }
    }
    
    return checksum;
}

int main(void) {
    /* Create array of __int128 values that exercise different comparison paths */
    __int128 test_values[8];
    unsigned long long checksum = 0;
    
    /* Initialize with values that differ in high words */
    test_values[0] = ((__int128)HIGH_BIT_64 << 64) | 0x1;  /* High word: 0x8000..., low: 1 */
    test_values[1] = ((__int128)HIGH_BIT_64 << 64) | 0x2;  /* Same high, different low */
    test_values[2] = ((__int128)(HIGH_BIT_64 >> 1) << 64) | 0x1;  /* Different high */
    test_values[3] = -test_values[0];  /* Negative value */
    test_values[4] = 0;                /* Zero */
    test_values[5] = MAX_64;           /* Max 64-bit */
    test_values[6] = ((__int128)MAX_64 << 64) | MAX_64;  /* Max 128-bit unsigned approx */
    test_values[7] = ((__int128)1 << 127) - 1;  /* Near INT128_MAX */
    
    /* Process array to force comparisons */
    checksum = process_array(test_values, 8);
    
    /* Test specific comparison scenarios */
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 8; j++) {
            if (i != j) {
                /* Force comparison evaluation */
                checksum += (test_values[i] < test_values[j]) ? 1 : 0;
                checksum += (test_values[i] > test_values[j]) ? 2 : 0;
                checksum += (test_values[i] == test_values[j]) ? 4 : 0;
            }
        }
    }
    
    /* Test overflow operations */
    checksum += check_overflow(test_values[0], test_values[1]);
    checksum += check_overflow(test_values[3], test_values[4]);
    
    /* Test mixed precision */
    __int128 mixed = mixed_operations(100, 1000);
    checksum += (unsigned long long)(mixed & MAX_64);
    checksum += (unsigned long long)((mixed >> 64) & MAX_64);
    
    /* Test classification */
    for (int i = 0; i < 8; i++) {
        const char* cls = classify_int128(test_values[i]);
        checksum += cls[0];  /* Use first char as checksum */
    }
    
    /* Test process_value function */
    for (int i = 0; i < 8; i++) {
        __int128 processed = process_value(test_values[i], i);
        checksum += (unsigned long long)(processed & MAX_64);
    }
    
    /* Use __builtin_expect with __int128 comparisons */
    __int128 a = test_values[0];
    __int128 b = test_values[1];
    if (__builtin_expect(a < b, 1)) {
        checksum += 1000;
    }
    if (__builtin_expect(a > b, 0)) {
        checksum += 2000;
    }
    
    /* Force printf conversion (triggers type conversions) */
    printf("Checksum: %llu\n", checksum);
    printf("Value 0: 0x%016llx%016llx\n", 
           (unsigned long long)(test_values[0] >> 64),
           (unsigned long long)(test_values[0] & MAX_64));
    
    /* Additional constant folding tests */
    _Static_assert(((__int128)0x7FFFFFFFFFFFFFFFLL << 64) > 0xFFFFFFFFFFFFFFFFULL,
                   "Cross-word constant comparison");
    
    return 0;
}
