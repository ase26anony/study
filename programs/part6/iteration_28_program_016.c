/* test_fixed_value.c - Test program to trigger uncovered fixed-value comparison logic */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Use __int128 to ensure we have high/low parts in fixed-point representation */
typedef unsigned __int128 uint128_t;
typedef __int128 int128_t;

/* Global variables to create inter-procedural data flow */
static uint128_t global_max = 0;
static uint128_t global_mask = 0;

/* Function with attribute to encourage value range propagation */
__attribute__((noinline)) 
static uint128_t create_large_low_part(unsigned shift_bits) {
    /* Create a value where high part is 0, low part has specific bit pattern */
    uint128_t val = 1;
    val = (val << shift_bits) - 1;  /* All ones in lower 'shift_bits' bits */
    val += 1;  /* Now low part exceeds the mask of all ones */
    return val;
}

/* Function that should trigger a_high == 0 && a_low > max_s */
__attribute__((noinline, const))
static int test_zero_high_large_low(unsigned fbits) {
    /* Create a value where high part is 0, low part exceeds mask */
    uint128_t mask = ((uint128_t)1 << fbits) - 1;
    uint128_t value = mask + 1;  /* Low part exceeds mask, high part is 0 */
    
    /* Store to globals to create data flow */
    global_mask = mask;
    
    /* Comparison that should trigger the uncovered condition */
    uint128_t high_part = value >> fbits;
    uint128_t low_part = value & mask;
    
    /* This should make compiler analyze: high_part == 0 && low_part > mask */
    if (high_part == 0 && low_part > mask) {
        return 1;
    }
    return 0;
}

/* Function with loop that creates value range analysis */
__attribute__((noinline))
static void loop_with_range_analysis(int n, unsigned fbits) {
    uint128_t mask = ((uint128_t)1 << fbits) - 1;
    uint128_t accumulator = 0;
    
    /* Loop where accumulator may exceed mask in low bits */
    for (int i = 0; i < n; i++) {
        /* Create value with zero high part but potentially large low part */
        uint128_t increment = (i & 1) ? 1 : (mask >> 1);
        accumulator += increment;
        
        /* Force range analysis by using accumulator in condition */
        if (__builtin_expect(accumulator > mask, 0)) {
            /* When accumulator exceeds mask but high part is still 0 */
            uint128_t high_part = accumulator >> fbits;
            if (high_part == 0) {
                /* This should trigger a_high == 0 && a_low > max_s */
                global_max = accumulator;
            }
        }
    }
}

/* Function using multiplication to create large values */
__attribute__((noinline))
static uint128_t multiply_create_range(unsigned a, unsigned b, unsigned fbits) {
    uint128_t mask = ((uint128_t)1 << fbits) - 1;
    uint128_t result = (uint128_t)a * (uint128_t)b;
    
    /* Check if result fits within fbits but low part exceeds mask */
    uint128_t high_part = result >> fbits;
    uint128_t low_part = result & mask;
    
    /* This comparison structure mirrors the uncovered code */
    if (high_part > 0) {
        return result;  /* Would trigger a_high.sgt(max_r) */
    } else if (high_part == 0 && low_part > mask) {
        /* This is impossible mathematically, but compiler might analyze it */
        return result + 1;
    }
    return result;
}

/* Function with complex control flow to engage VRP */
__attribute__((noinline))
static int complex_range_test(unsigned limit, unsigned fbits) {
    uint128_t mask = ((uint128_t)1 << fbits) - 1;
    uint128_t val = 0;
    
    for (unsigned i = 0; i < limit; i++) {
        /* Mix of operations to create complex value ranges */
        val = (val * 3 + i) & ((mask << 1) | 1);
        
        /* Multiple exit conditions for range analysis */
        if (val > (mask << 1)) {
            uint128_t high_part = val >> fbits;
            if (high_part == 1) {  /* high_part == 1 would be > max_r (0) */
                return 1;
            }
        }
        
        if (val == mask + 1) {
            /* val has high_part == 0, low_part == mask + 1 */
            return 2;
        }
    }
    
    /* Final check that might trigger the condition */
    uint128_t high_part = val >> fbits;
    uint128_t low_part = val & mask;
    if (high_part == 0 && low_part > mask) {
        return 3;
    }
    
    return 0;
}

/* Main function that exercises all test cases */
int main(int argc, char *argv[]) {
    int result = 0;
    
    /* Test different fbits values to trigger different i_f_bits in the compiler */
    unsigned fbits_values[] = {16, 32, 64, 96};
    
    for (int i = 0; i < 4; i++) {
        unsigned fbits = fbits_values[i];
        
        /* Test 1: Direct case with zero high, large low */
        result |= test_zero_high_large_low(fbits);
        
        /* Test 2: Loop with range analysis */
        loop_with_range_analysis(100, fbits);
        
        /* Test 3: Multiplication creating boundary values */
        /* Choose values that when multiplied might exceed mask but not shift to high part */
        uint128_t mul_result = multiply_create_range((1U << (fbits/2)) + 1, 
                                                    (1U << (fbits/2)) + 1, 
                                                    fbits);
        (void)mul_result;  /* Use result to prevent optimization */
        
        /* Test 4: Complex control flow */
        result |= complex_range_test(50, fbits);
    }
    
    /* Use globals to prevent dead code elimination */
    if (global_max > 0 || global_mask > 0) {
        printf("Test completed with result: %d\n", result);
    }
    
    return result != 0 ? 0 : 1;
}
