/* test_fixed_value.c - Test program to trigger fixed-value range analysis coverage */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Function attributes to help with value propagation */
#define LIKELY(x) __builtin_expect(!!(x), 1)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#define PURE __attribute__((pure))

/* Global variables to create inter-procedural data flow */
static volatile uint64_t g_seed = 0x12345678;
static uint64_t g_max_val = 0;

/* Function 1: Targets a_high.sgt(max_r) path */
/* Creates a value where high part > 0 */
__attribute__((noinline))
static uint64_t func_high_gt_zero(int iterations) {
    uint64_t result = 0;
    
    /* Loop with induction variable that may exceed 32-bit range */
    for (int64_t i = 0; i < iterations; i++) {
        /* Create value with high bits set when i is large */
        result = (uint64_t)i << 32;
        
        /* Complex exit condition to force range analysis */
        if (UNLIKELY(result > 0xFFFFFFFF00000000ULL)) {
            break;
        }
    }
    
    return result;
}

/* Function 2: Targets a_high == max_r && a_low.ugt(max_s) path */
/* Creates value where high part is 0 but low part is large */
__attribute__((noinline))
static uint64_t func_zero_high_large_low(int shift) {
    uint64_t value = 0;
    
    /* Start with all bits set in low part */
    value = ~(uint64_t)0;
    
    /* Right shift to clear high bits, leaving only low bits */
    value >>= (64 - shift);
    
    /* Add 1 to potentially exceed max_s boundary */
    value += 1;
    
    /* Mask to ensure high part stays 0 */
    value &= ((1ULL << shift) - 1);
    
    return value;
}

/* Function 3: Uses bitwise operations to create boundary values */
__attribute__((noinline))
static uint64_t func_bitwise_boundary(int bits) {
    uint64_t mask = (1ULL << bits) - 1;
    uint64_t value = 0;
    
    /* Create value just at the boundary */
    value = mask;
    
    /* Sometimes exceed the boundary by 1 */
    if (LIKELY(bits < 63)) {
        value += (g_seed & 1);  /* Randomly add 0 or 1 */
    }
    
    return value;
}

/* Function 4: Complex loop with multiple exit conditions */
__attribute__((noinline))
static uint64_t func_complex_loop(int limit) {
    uint64_t acc = 0;
    uint64_t prev = 0;
    
    for (int i = 0; i < limit; i++) {
        /* Mix of arithmetic operations */
        acc = (acc * 1103515245 + 12345) & 0x7FFFFFFF;
        
        /* Left shift to potentially set high bits */
        uint64_t shifted = acc << (i % 32);
        
        /* Conditional that depends on value range */
        if (shifted > 0xFFFFFFFFULL && (shifted >> 32) > 0) {
            /* This path creates values with high part > 0 */
            prev = shifted;
        } else if (shifted == 0xFFFFFFFFULL) {
            /* This path creates exact boundary value */
            prev = shifted + 1;  /* Exceeds by 1 */
        }
        
        /* Early exit based on complex condition */
        if (prev > (1ULL << 62)) {
            break;
        }
    }
    
    return prev;
}

/* Function 5: Uses 128-bit arithmetic to trigger fixed-point analysis */
__attribute__((noinline))
static uint64_t func_128bit_ops(uint64_t a, uint64_t b) {
    __uint128_t wide = (__uint128_t)a * (__uint128_t)b;
    
    /* Extract high and low parts separately */
    uint64_t high = (uint64_t)(wide >> 64);
    uint64_t low = (uint64_t)wide;
    
    /* Conditions that test both high and low parts */
    if (high == 0) {
        /* When high is 0, low might still be large */
        if (low > 0xFFFFFFFFULL) {
            return low;
        }
    }
    
    return high;
}

/* Function 6: Recursive function to create complex value flow */
__attribute__((noinline))
static uint64_t func_recursive(int depth, uint64_t val) {
    if (depth <= 0) {
        return val;
    }
    
    /* Shift right to potentially clear high bits */
    uint64_t shifted = val >> 1;
    
    /* Recursive call with modified value */
    uint64_t result = func_recursive(depth - 1, shifted);
    
    /* Left shift back, but may lose bits */
    return result << 1;
}

/* Main test driver */
int main(int argc, char *argv[]) {
    uint64_t results[6] = {0};
    int test_cases = 6;
    
    /* Test different bit widths to trigger various i_f_bits values */
    int bit_widths[] = {16, 32, 48, 64};
    
    for (int i = 0; i < 4; i++) {
        int bits = bit_widths[i];
        
        /* Call functions with different parameters to create
           various value ranges for the compiler to analyze */
        
        /* 1. Test high part > 0 */
        results[0] = func_high_gt_zero(1 << (bits / 2));
        
        /* 2. Test high part == 0, low part large */
        results[1] = func_zero_high_large_low(bits);
        
        /* 3. Test boundary values */
        results[2] = func_bitwise_boundary(bits);
        
        /* 4. Test complex loop */
        results[3] = func_complex_loop(100);
        
        /* 5. Test 128-bit operations */
        results[4] = func_128bit_ops(0xFFFFFFFFULL, 0xFFFFFFFFULL);
        
        /* 6. Test recursive value flow */
        results[5] = func_recursive(10, (1ULL << bits) - 1);
        
        /* Use results to prevent dead code elimination */
        for (int j = 0; j < 6; j++) {
            g_max_val ^= results[j];
        }
    }
    
    /* Final output to ensure program has observable behavior */
    printf("Result: %llu\n", (unsigned long long)g_max_val);
    
    return 0;
}
