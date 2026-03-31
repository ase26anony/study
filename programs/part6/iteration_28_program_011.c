/* test_fixed_value.c
 * 
 * This test program is designed to trigger GCC's fixed-point range analysis
 * logic, specifically targeting the uncovered comparison block in fixed-value.cc
 * where a_high == max_r (0) and a_low > max_s.
 * 
 * Compile with: gcc -O3 -fstrict-overflow -c test_fixed_value.c -o test.o
 */

#include <stdint.h>
#include <limits.h>

/* Global variables to create inter-procedural data flow */
static unsigned long global_counter = 0;
static volatile unsigned long volatile_counter = 0; /* Prevent some optimizations */

/* Function that creates a value with high part 0 and low part > max_s
 * This targets: a_high == max_r && a_low.ugt(max_s)
 */
__attribute__((noinline))
unsigned long long create_large_low_part(unsigned int bits) {
    /* Create a value where high part is 0, low part has specific bit pattern */
    unsigned long long value = 0;
    
    /* Set bits in the low part based on input parameter */
    if (bits > 0 && bits < 64) {
        /* Create a value with all bits set in low part up to 'bits' */
        value = (1ULL << bits) - 1;
        
        /* Ensure high part remains 0 by masking */
        value &= ~(0xFFFFFFFF00000000ULL);
    }
    
    return value;
}

/* Function with loop where index may reach maximum value
 * This could trigger: a_high.sgt(max_r)
 */
__attribute__((noinline))
void loop_with_boundary_check(int n) {
    int i;
    
    /* Loop with boundary that compiler can analyze */
    for (i = 0; i < n; i++) {
        /* Complex exit condition that may engage range analysis */
        if (i == n - 1) {
            global_counter += i;
        }
        
        /* Arithmetic that produces values requiring range tracking */
        int shifted = i << 3;
        int multiplied = shifted * 7;
        
        /* Comparison near boundary */
        if (multiplied > (n * 7 * 8 - 1)) {
            volatile_counter = multiplied;
        }
    }
}

/* Function using bitwise operations to create specific patterns */
__attribute__((noinline))
unsigned long long bitwise_pattern(unsigned int seed) {
    unsigned long long result = 0;
    
    /* Create pattern where high part is 0, low part has specific bits set */
    result = (unsigned long long)seed;
    
    /* Shift to create interesting bit patterns */
    result = (result << 16) | (result >> 16);
    result = result ^ 0xAAAAAAAAAAAAAAAAULL;
    
    /* Mask to ensure high part is 0 */
    result &= 0x00000000FFFFFFFFULL;
    
    /* Add a large value to the low part */
    result += 0xFFFFFF00ULL;
    
    return result;
}

/* Function with __int128 operations (may engage fixed-value analysis) */
__attribute__((noinline))
__int128 int128_operations(__int128 a, __int128 b) {
    __int128 result = a * b;
    
    /* Shift operation that may trigger fixed-value analysis */
    result = result >> 32;
    
    /* Comparison that could engage the target logic */
    if (result > (__int128)0x7FFFFFFFFFFFFFFFLL) {
        volatile_counter = (unsigned long)result;
    }
    
    return result;
}

/* Function that specifically targets the uncovered condition */
__attribute__((noinline))
void target_specific_condition(void) {
    /* Create values that when analyzed as fixed-point:
     * - Have high part = 0 (max_r)
     * - Have low part > max_s (after zext)
     */
    
    /* max_s is set to -1 then zext(i_f_bits)
     * For typical i_f_bits values, max_s becomes a large unsigned value
     * with specific bits set in the low part
     */
    
    unsigned long long values[] = {
        0x00000000FFFFFFFFULL,  /* High=0, Low=all bits set in 32-bit */
        0x00000000FFFFFFFEULL,  /* High=0, Low=almost all bits */
        0x0000000080000000ULL,  /* High=0, Low=MSB set in 32-bit */
        0x0000000100000000ULL,  /* High=0, Low=bit 32 set (if 64-bit) */
    };
    
    for (int i = 0; i < 4; i++) {
        unsigned long long val = values[i];
        
        /* Operations that maintain high=0 but manipulate low part */
        unsigned long long shifted = val << 1;
        unsigned long long masked = shifted & 0x00000000FFFFFFFFULL;
        
        /* Complex expression that compiler needs to analyze */
        unsigned long long complex_val = (masked * 3) / 2;
        
        /* Comparison that may trigger the target logic */
        if (complex_val > 0x7FFFFFFFULL) {
            global_counter += complex_val;
        }
    }
}

/* Function with nested loops and complex conditions */
__attribute__((noinline))
void nested_loop_analysis(int outer, int inner) {
    for (int i = 0; i < outer; i++) {
        int i_squared = i * i;
        
        for (int j = 0; j < inner; j++) {
            int product = i_squared * j;
            
            /* Condition that creates range analysis opportunities */
            if (product > (outer * inner * 2)) {
                /* This may trigger overflow analysis */
                unsigned long long big_val = (unsigned long long)product;
                big_val = big_val << 32;
                
                /* Check if high part is 0 but low part is large */
                if ((big_val >> 32) == 0 && (big_val & 0xFFFFFFFFULL) > 0x7FFFFFFFULL) {
                    volatile_counter = big_val;
                }
            }
        }
    }
}

/* Main function that exercises all test cases */
int main(int argc, char *argv[]) {
    /* Test 1: Create values with specific bit patterns */
    unsigned long long pattern1 = create_large_low_part(32);
    unsigned long long pattern2 = create_large_low_part(31);
    
    /* Test 2: Loop with boundary analysis */
    loop_with_boundary_check(1000);
    
    /* Test 3: Bitwise pattern generation */
    unsigned long long bitwise_result = bitwise_pattern(0x12345678);
    
    /* Test 4: __int128 operations */
    __int128 a = 0x123456789ABCDEF0LL;
    __int128 b = 0xFEDCBA9876543210LL;
    __int128 int128_result = int128_operations(a, b);
    
    /* Test 5: Specifically target the uncovered condition */
    target_specific_condition();
    
    /* Test 6: Nested loop analysis */
    nested_loop_analysis(50, 100);
    
    /* Additional tests with different optimization hints */
    
    /* Use __builtin_expect to influence branch prediction */
    if (__builtin_expect(global_counter > 1000, 0)) {
        volatile_counter = global_counter;
    }
    
    /* Create a value that might overflow in analysis */
    unsigned int x = 0xFFFFFFFF;
    unsigned int y = x + 1;  /* Overflow to 0 */
    
    /* Complex expression for range analysis */
    unsigned long long z = ((unsigned long long)x * y) >> 16;
    
    /* Final check that might trigger the target comparison */
    if (z > 0x00000000FFFFFFFFULL) {
        return 1;
    }
    
    return 0;
}
