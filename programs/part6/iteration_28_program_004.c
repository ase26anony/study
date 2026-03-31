/* test_fixed_value.c - Test program to trigger fixed-point range analysis coverage */

#include <stdio.h>
#include <stdint.h>
#include <limits.h>

/* Global variables to create complex data flow */
static unsigned long global_counter = 0;
static volatile int volatile_var = 0; /* Prevent some optimizations */

/* Function with loop where index may reach maximum value */
__attribute__((noinline))
void test_loop_max_range(int n) {
    /* Loop with induction variable that compiler can analyze */
    for (int i = 0; i < n; i++) {
        /* Complex exit condition to force range analysis */
        if (i > n - 2) {
            global_counter += i;
        }
    }
    
    /* After loop, i's range should be [0, n-1] */
    /* Create value that might trigger a_high.sgt(max_r) */
    int j = n - 1;
    if (j > 1000) {
        global_counter += j;
    }
}

/* Function targeting a_high == 0 && a_low > max_s case */
__attribute__((noinline, const))
unsigned long long create_large_low_part(unsigned bits) {
    /* Create value where high part is 0 but low part has specific bit pattern */
    unsigned long long mask = (1ULL << bits) - 1;
    return mask + 1; /* Value just above the mask boundary */
}

/* Function with arithmetic producing zero high part but large low part */
__attribute__((noinline))
void test_zero_high_large_low(void) {
    /* Operations that might produce values analyzed as fixed-point */
    unsigned long long x = create_large_low_part(31);
    unsigned long long y = x * 2;
    
    /* Comparison that might trigger the uncovered condition */
    if (y > (1ULL << 32)) {
        global_counter += (unsigned)y;
    }
    
    /* Bitwise operations for precise bit patterns */
    unsigned long long z = (x << 1) | 1;
    if (z != 0) {
        volatile_var = (int)z;
    }
}

/* Function using __int128 for wider fixed-point analysis */
__attribute__((noinline))
void test_128bit_range(void) {
    __int128 large_val = (__int128)1 << 60;
    __int128 multiplied = large_val * 3;
    
    /* Split into high/low parts conceptually */
    if (multiplied > (__int128)1 << 61) {
        global_counter += (unsigned long)multiplied;
    }
}

/* Function with shifting operations that affect range analysis */
__attribute__((noinline))
void test_shift_patterns(unsigned shift) {
    unsigned base = 0x80000000U; /* High bit set */
    
    /* Right shift creates values with zero high bits */
    unsigned right_shifted = base >> shift;
    
    /* Left shift might create large low part */
    unsigned left_shifted = 1U << (shift & 0x1F);
    
    /* Combined operation */
    unsigned combined = (right_shifted + left_shifted) & 0xFFFFFFFF;
    
    if (combined > 0x7FFFFFFF) {
        global_counter += combined;
    }
}

/* Function with complex control flow and value ranges */
__attribute__((noinline))
void test_complex_ranges(int limit) {
    int i = 0;
    int j = limit;
    
    /* Loop with two induction variables */
    while (i < j) {
        /* Arithmetic that creates values near boundaries */
        int diff = j - i;
        int mid = (i + j) / 2;
        
        if (diff > 1000) {
            /* Might trigger overflow analysis */
            int prod = diff * mid;
            if (__builtin_expect(prod > 1000000, 0)) {
                global_counter += prod;
            }
        }
        
        i++;
        j--;
    }
}

/* Function specifically targeting the uncovered comparison */
__attribute__((noinline))
void target_uncovered_comparison(unsigned f_bits) {
    /*
     * The uncovered code sets:
     * max_r = 0
     * max_s = (unsigned)-1 zero-extended by f_bits
     * 
     * We need a value where:
     * - high part == 0 (equals max_r)
     * - low part > max_s (after zero extension)
     */
    
    /* Create value that might be analyzed as having high=0, low>max_s */
    unsigned long long max_for_bits = (1ULL << f_bits) - 1;
    unsigned long long target_val = max_for_bits + 1;
    
    /* Operations that maintain high part as 0 */
    unsigned long long doubled = target_val * 2;
    unsigned long long masked = doubled & ((1ULL << (f_bits * 2)) - 1);
    
    /* Comparison that might trigger the specific path */
    if (masked > max_for_bits) {
        volatile_var = (int)masked;
    }
    
    /* Another approach: use 32-bit values that get extended */
    unsigned int small = 0xFFFFFFFFU;
    unsigned long long extended = (unsigned long long)small + 1;
    
    if (extended > 0xFFFFFFFFULL) {
        global_counter += extended;
    }
}

/* Main function that exercises all test cases */
int main(void) {
    /* Test various scenarios to trigger different range analysis paths */
    
    /* Test 1: Loop with maximum range */
    test_loop_max_range(1000);
    test_loop_max_range(0x7FFFFFFF);
    
    /* Test 2: Zero high part, large low part */
    test_zero_high_large_low();
    
    /* Test 3: 128-bit range analysis */
    test_128bit_range();
    
    /* Test 4: Shift patterns */
    for (unsigned s = 1; s < 32; s += 8) {
        test_shift_patterns(s);
    }
    
    /* Test 5: Complex ranges */
    test_complex_ranges(500);
    test_complex_ranges(2000);
    
    /* Test 6: Direct targeting of uncovered comparison */
    /* Try different f_bits values that might match i_f_bits in the uncovered code */
    target_uncovered_comparison(16);  /* Common for fixed-point types */
    target_uncovered_comparison(31);  /* Near 32-bit boundary */
    target_uncovered_comparison(32);  /* Exactly 32 bits */
    target_uncovered_comparison(48);  /* For 64-bit analysis */
    
    /* Additional stress test with various bit patterns */
    for (int i = 0; i < 100; i++) {
        unsigned val = (unsigned)i * 0x55555555U;
        if (val > 0x80000000U) {
            global_counter += val;
        }
    }
    
    printf("Test completed. Global counter: %lu\n", global_counter);
    return 0;
}
