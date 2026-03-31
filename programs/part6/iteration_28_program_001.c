/* test_fixed_value.c - Test program to trigger fixed-point range analysis coverage */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Global variables to create complex data flow */
static unsigned long global_counter = 0;
static volatile int volatile_var = 0; /* Prevent some optimizations */

/* Function that creates a value with high part 0 and large low part */
__attribute__((noinline))
static uint64_t create_large_low_part(unsigned bits) {
    /* Create value where high part is 0, low part has specific bit pattern */
    uint64_t mask = (1ULL << bits) - 1;
    return mask; /* All bits set in low 'bits' positions */
}

/* Function targeting a_high.sgt(max_r) path */
__attribute__((noinline))
static int test_high_part_greater(void) {
    int sum = 0;
    
    /* Loop with induction variable that may exceed range */
    for (int i = 0; i < 1000; i++) {
        /* Complex expression that compiler might analyze as fixed-point */
        int val = i * 1073741824; /* 2^30 - may cause overflow analysis */
        
        /* Comparison that depends on value range */
        if (__builtin_expect(val > 0, 1)) {
            sum += val;
        }
        
        /* Bit manipulation that creates specific patterns */
        unsigned shifted = (unsigned)i << 16;
        if (shifted > 0x7FFFFFFF) {
            sum += 1;
        }
    }
    
    return sum;
}

/* Function targeting a_high == max_r && a_low.ugt(max_s) path */
__attribute__((noinline))
static int test_zero_high_large_low(void) {
    unsigned int result = 0;
    
    /* Use 64-bit operations where high 32 bits are 0 but low 32 bits are large */
    for (unsigned i = 0; i < 256; i++) {
        /* Create value with high part 0, low part with specific bit pattern */
        uint64_t val = create_large_low_part(24 + (i & 7));
        
        /* Comparison that might trigger the uncovered condition */
        if (val > 0x00FFFFFF) { /* Compare with value where high byte is 0 */
            result += (unsigned int)val;
        }
        
        /* More complex expression with shifting */
        uint64_t shifted = val << 8;
        if (shifted > 0x0000FFFFFFFFFFFFULL) {
            result += 1;
        }
    }
    
    return result;
}

/* Function with __int128 operations that may trigger fixed-value analysis */
__attribute__((noinline))
static int test_128bit_operations(void) {
    unsigned __int128 large_val = 0;
    int sum = 0;
    
    /* Create 128-bit value with specific pattern */
    for (int i = 0; i < 100; i++) {
        /* Value where high part is 0, low part is large */
        large_val = ((unsigned __int128)0xFFFFFFFFFFFFFFFFULL) >> (i & 31);
        
        /* Comparison that might trigger range analysis */
        if (large_val > 0x7FFFFFFFFFFFFFFFULL) {
            sum += i;
        }
        
        /* Bit manipulation */
        large_val = large_val << 1;
        if (large_val == 0) {
            sum += 1;
        }
    }
    
    return sum;
}

/* Function with complex loop bounds and exit conditions */
__attribute__((noinline))
static int test_complex_bounds(int limit) {
    int arr[256];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 256; i++) {
        arr[i] = i * 3;
    }
    
    /* Loop with complex exit condition */
    for (int i = 0; i < limit && i < 256; i++) {
        /* Expression that might be analyzed as fixed-point */
        int idx = (i * 7) & 255;
        
        /* Access with bounds check */
        if (idx >= 0 && idx < 256) {
            sum += arr[idx];
        }
        
        /* Additional comparison near boundary */
        if (i > 250) {
            sum += arr[255 - (i - 250)];
        }
    }
    
    return sum;
}

/* Function using bitwise operations to create specific patterns */
__attribute__((noinline))
static unsigned test_bit_patterns(void) {
    unsigned patterns[] = {
        0x00000000,
        0x0000FFFF,
        0x00FFFFFF,
        0x7FFFFFFF,
        0x80000000,
        0xFFFFFFFF
    };
    
    unsigned result = 0;
    
    for (int i = 0; i < 6; i++) {
        /* Create value with specific high/low patterns */
        unsigned val = patterns[i];
        
        /* Shift to create different high/low distributions */
        unsigned shifted = val << 8;
        
        /* Comparisons that might trigger the uncovered logic */
        if ((val & 0xFF000000) == 0 && val > 0x00FFFFFF) {
            result += 1;
        }
        
        if (shifted > 0xFFFFFF00) {
            result += 2;
        }
        
        /* Bit manipulation */
        unsigned masked = val & 0x7FFFFFFF;
        if (masked == val) {
            result += 4;
        }
    }
    
    return result;
}

/* Main function that exercises all test cases */
int main(void) {
    int total = 0;
    
    /* Prevent compiler from optimizing everything away */
    if (volatile_var) {
        return 0;
    }
    
    printf("Testing fixed-point range analysis coverage...\n");
    
    /* Exercise different code paths */
    total += test_high_part_greater();
    total += test_zero_high_large_low();
    total += test_128bit_operations();
    total += test_complex_bounds(300);
    total += test_bit_patterns();
    
    /* Use the result to prevent dead code elimination */
    printf("Result: %d\n", total);
    
    return total != 0 ? 0 : 1;
}
