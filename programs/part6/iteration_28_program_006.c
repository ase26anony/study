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
    /* This loop should create range analysis where a_high might be compared with max_r */
    for (int i = 0; i < n; i++) {
        /* Complex arithmetic that might trigger fixed-point analysis */
        int val = i * 3 + 7;
        if (val > n * 2) {
            global_counter += val;
        }
    }
}

/* Function targeting a_high == 0 && a_low > max_s condition */
__attribute__((noinline))
void test_zero_high_large_low(unsigned int bits) {
    /* Create values where high part is 0 but low part is large */
    unsigned long long mask = (1ULL << bits) - 1;
    
    /* This value has high part 0, low part with specific bit pattern */
    unsigned long long value = mask + 1; /* Just above the mask boundary */
    
    /* Operations that might trigger the specific comparison */
    if (value > mask) {
        global_counter += 1;
    }
    
    /* Another pattern: value with all bits set in low part */
    unsigned long long max_low = ~0ULL >> (64 - bits);
    if (value == max_low + 1) {
        global_counter += 2;
    }
}

/* Function with bitwise operations and shifts */
__attribute__((noinline))
void test_bitwise_range(unsigned int shift) {
    /* Create values that require precise range tracking */
    unsigned int x = 0xFFFFFFFF;
    
    /* Right shift creating values with zero high bits */
    unsigned int y = x >> shift;
    
    /* Left shift that might overflow into high part */
    unsigned int z = 1U << (shift - 1);
    
    /* Comparisons that might trigger the uncovered logic */
    if (y > (1U << 16)) {
        global_counter += y;
    }
    
    if (z > 0x7FFFFFFF) {
        global_counter += z;
    }
}

/* Function using 128-bit integers (if available) */
#ifdef __SIZEOF_INT128__
__attribute__((noinline))
void test_128bit_range(void) {
    __int128 large_val = ((__int128)1 << 64) - 1;
    __int128 shifted = large_val << 2;
    
    /* This might trigger comparisons with different high/low parts */
    if (shifted > large_val) {
        global_counter += 1;
    }
}
#endif

/* Function with multiplication that can overflow */
__attribute__((noinline))
void test_multiplication_range(unsigned int a, unsigned int b) {
    /* Multiplication that might require range analysis */
    unsigned long long product = (unsigned long long)a * b;
    
    /* Compare against boundary values */
    unsigned long long boundary = 1ULL << 32;
    if (product > boundary) {
        global_counter += product % 1000;
    }
    
    /* Another comparison targeting zero high part */
    if (product < (1ULL << 63)) {
        /* product has high part 0 */
        if (product > (1ULL << 32) - 1) {
            global_counter += 1;
        }
    }
}

/* Function with complex control flow and value propagation */
__attribute__((noinline))
void test_complex_propagation(int iterations) {
    int x = 0;
    int y = 1000;
    
    for (int i = 0; i < iterations; i++) {
        /* Create value ranges that change during loop */
        x = x * 2 + 1;
        y = y / 2;
        
        /* Branch with value-dependent condition */
        if (x > y) {
            global_counter += x - y;
        } else if (x == y) {
            global_counter += 1;
        }
        
        /* Use __builtin_expect to hint branch prediction */
        if (__builtin_expect(x > 10000, 0)) {
            break;
        }
    }
}

/* Function that returns a value with known range */
__attribute__((const))
static unsigned int get_range_boundary(void) {
    return 0xFFFF;
}

/* Main function to call all test cases */
int main(int argc, char *argv[]) {
    /* Call functions with different parameters to trigger various range analyses */
    
    /* Test 1: Loop with maximum range */
    test_loop_max_range(1000);
    
    /* Test 2: Target zero high part with large low part */
    /* Use different bit widths to trigger different i_f_bits values */
    test_zero_high_large_low(16);
    test_zero_high_large_low(32);
    test_zero_high_large_low(48);
    
    /* Test 3: Bitwise operations */
    test_bitwise_range(8);
    test_bitwise_range(16);
    test_bitwise_range(24);
    
    /* Test 4: 128-bit integers if available */
    #ifdef __SIZEOF_INT128__
    test_128bit_range();
    #endif
    
    /* Test 5: Multiplication with potential overflow */
    test_multiplication_range(0xFFFFFFFF, 2);
    test_multiplication_range(0xFFFF, 0xFFFF);
    
    /* Test 6: Complex value propagation */
    test_complex_propagation(50);
    
    /* Additional test: Use function with known range */
    unsigned int bound = get_range_boundary();
    if (global_counter > bound) {
        printf("Global counter exceeded boundary: %lu > %u\n", global_counter, bound);
    }
    
    /* Prevent dead code elimination */
    volatile_var = global_counter > 0 ? 1 : 0;
    
    return 0;
}
