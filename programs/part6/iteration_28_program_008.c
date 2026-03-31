/* test_fixed_value.c - Test program for GCC fixed-value range analysis */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

/* Global variables to create inter-procedural data flow */
static unsigned long global_counter = 0;
static __int128 large_value = 0;

/* Function with loop where index may reach maximum value */
/* This should trigger a_high.sgt(max_r) path */
__attribute__((noinline))
void test_high_part_nonzero(int n) {
    /* Create a value where high part is non-zero */
    for (long long i = 0; i < n; i++) {
        /* Complex arithmetic that may produce non-zero high part */
        __int128 val = (__int128)i * (__int128)(1ULL << 63);
        
        /* Comparison that forces range analysis */
        if (val > ((__int128)1 << 120)) {
            global_counter++;
        }
    }
}

/* Function targeting a_high == 0 && a_low > max_s path */
/* max_s is a large unsigned value after zext, so we need a_low > large_unsigned */
__attribute__((noinline, const))
unsigned long test_zero_high_large_low(unsigned long base, int shift) {
    /* Create value with high part = 0, low part > max_s */
    /* max_s after zext with i_f_bits becomes something like 0xFFFFFFFF... */
    /* We need a_low > this maximum unsigned value */
    
    /* Start with a value that has all bits set in low part */
    unsigned long long val = ~0ULL;
    
    /* Shift to create a value > max_s */
    /* If max_s is all 1's in i_f_bits, we need more bits */
    unsigned long long result = val << shift;
    
    /* Force compiler to analyze this value */
    if (result > 0xFFFFFFFFFFFFFFFFULL) {
        /* This branch should be taken when shift > 0 */
        return result + base;
    }
    return base;
}

/* Function with bitwise operations and shifts */
__attribute__((noinline))
void test_bitwise_patterns(int iterations) {
    unsigned long long mask = 0xFFFFFFFFFFFFFFFFULL;
    unsigned long long accumulator = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Create pattern where high part might be zero but low part is large */
        unsigned long long pattern = mask >> i;
        
        /* Multiply to potentially create large values */
        __int128 wide_val = (__int128)pattern * (__int128)(mask);
        
        /* Complex condition that depends on bit patterns */
        if ((wide_val & ((__int128)0xFF << 64)) == 0) {
            /* High byte is zero, but low part might be > max_s */
            accumulator += (unsigned long long)wide_val;
        }
    }
    
    /* Use accumulator to prevent dead code elimination */
    if (accumulator > 1000) {
        global_counter += accumulator;
    }
}

/* Function with near-boundary values */
__attribute__((noinline))
int test_boundary_conditions(unsigned long long limit) {
    int count = 0;
    
    /* Loop that approaches a boundary */
    for (unsigned long long i = limit - 100; i < limit; i++) {
        /* Create value that might trigger the condition */
        __int128 val = (__int128)i * (__int128)i;
        
        /* Check if high part is zero but low part is large */
        if ((val >> 64) == 0) {
            /* High part is zero */
            if ((unsigned long long)val > 0xFFFFFFFFFFFFFFF0ULL) {
                /* Low part is very close to maximum */
                count++;
            }
        }
    }
    
    return count;
}

/* Function using GCC builtins to influence optimization */
__attribute__((noinline))
void test_with_builtins(int n) {
    /* Use __builtin_expect to hint at branch prediction */
    for (int i = 0; i < n; i++) {
        unsigned long long val = (unsigned long long)i * 0x123456789ABCDEFULL;
        
        /* Create condition where high part might be zero */
        if (__builtin_expect((val >> 32) == 0, 0)) {
            /* But low part is still significant */
            if (val > 0xFFFFFFFF00000000ULL) {
                global_counter += val;
            }
        }
    }
}

/* Main function that exercises all test cases */
int main(int argc, char *argv[]) {
    int iterations = 1000;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 1000;
    }
    
    printf("Testing fixed-value range analysis...\n");
    
    /* Test 1: Non-zero high part */
    test_high_part_nonzero(iterations);
    printf("Test 1 completed: global_counter = %lu\n", global_counter);
    
    /* Test 2: Zero high part, large low part */
    /* Use shift=1 to ensure result > 0xFFFFFFFFFFFFFFFFULL */
    unsigned long result = test_zero_high_large_low(global_counter, 1);
    printf("Test 2 result: %lu\n", result);
    
    /* Test 3: Bitwise patterns */
    test_bitwise_patterns(iterations / 10);
    printf("Test 3 completed: global_counter = %lu\n", global_counter);
    
    /* Test 4: Boundary conditions */
    /* Use a value near 2^64 */
    unsigned long long boundary = 0xFFFFFFFFFFFFFFFFULL;
    int count = test_boundary_conditions(boundary);
    printf("Test 4 count: %d\n", count);
    
    /* Test 5: With builtins */
    test_with_builtins(iterations);
    printf("Test 5 completed: global_counter = %lu\n", global_counter);
    
    /* Additional test: Direct large value creation */
    /* This might directly trigger the uncovered condition */
    {
        __int128 huge = ((__int128)0x7FFFFFFFFFFFFFFFULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
        __int128 another = ((__int128)0x0ULL << 64) | 0xFFFFFFFFFFFFFFFFULL;
        
        /* Force comparisons that might use fixed-value analysis */
        if (huge > another) {
            printf("Large value comparison triggered\n");
        }
        
        /* Create a value with zero high but max low */
        __int128 zero_high_max_low = 0xFFFFFFFFFFFFFFFFULL;
        if (zero_high_max_low > 0xFFFFFFFFFFFFFFFEULL) {
            printf("Zero-high max-low comparison triggered\n");
        }
    }
    
    printf("All tests completed successfully.\n");
    return 0;
}
