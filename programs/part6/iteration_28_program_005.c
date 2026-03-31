/* test_fixed_value.c - Test program to trigger fixed-value range analysis coverage */

#include <stdint.h>
#include <stdio.h>

/* Global variables to create inter-procedural data flow */
static unsigned long global_counter = 0;
static int global_threshold = 1000;

/* Function with loop that may trigger high part comparison */
__attribute__((noinline))
void test_loop_with_boundary(unsigned limit) {
    /* This loop creates a value range analysis scenario */
    for (unsigned i = 0; i < limit; i++) {
        /* Complex arithmetic that may produce values requiring precise range tracking */
        unsigned long val = i * 3U;
        
        /* Comparison near boundary - compiler may analyze range of 'val' */
        if (val > 0xFFFFFFFFUL) {
            /* This branch should be analyzed for range */
            global_counter += val;
        }
        
        /* Bit manipulation that creates specific bit patterns */
        unsigned shifted = (val << 2) | 0x3;
        
        /* Another boundary check */
        if (shifted == 0xFFFFFFF0U) {
            break;
        }
    }
}

/* Function targeting the specific uncovered condition:
   a_high == 0 && a_low > max_s (after zext) */
__attribute__((noinline))
void test_zero_high_large_low(void) {
    /* Create values where high part is 0 but low part is large */
    
    /* Use 64-bit operations that stay within 32-bit range but have specific patterns */
    uint64_t large_val = 0x00000000FFFFFFFFULL;  /* High: 0, Low: all 1s */
    
    /* Operations that maintain zero high part but manipulate low part */
    for (int i = 0; i < 100; i++) {
        /* Shift operations that might trigger fixed-value analysis */
        uint64_t shifted = large_val << i;
        
        /* Comparison that might be analyzed by the uncovered code */
        if (shifted > 0xFFFFFFFFULL) {
            /* This should trigger range analysis */
            global_counter += (shifted & 0xFFFFFFFF);
        }
        
        /* Create value just above power-of-two boundary */
        uint64_t boundary_val = (1ULL << 32) + i;  /* High: 1, Low: i */
        
        /* This comparison might trigger different path */
        if (boundary_val <= 0x100000000ULL) {
            global_threshold--;
        }
    }
}

/* Function with multiplication that can overflow */
__attribute__((noinline))
int test_multiplication_range(int a, int b) {
    /* Multiplication that might be analyzed for overflow */
    int result = a * b;
    
    /* Comparison that forces range analysis */
    if (result > 1000000) {
        return 1;
    }
    
    /* Bitwise operations for specific patterns */
    unsigned pattern = 0x80000000U;
    unsigned masked = result & pattern;
    
    if (masked == pattern) {
        return 2;
    }
    
    return 0;
}

/* Function using __int128 for wider fixed-point analysis */
__attribute__((noinline))
void test_128bit_operations(void) {
    __int128 large = ((__int128)1 << 64) + 12345;
    __int128 mask = ((__int128)0xFFFFFFFF << 64) | 0xFFFFFFFF;
    
    /* Operations that might trigger fixed-value analysis */
    __int128 result = large & mask;
    
    if (result > 0) {
        global_counter += (unsigned long)result;
    }
}

/* Function with nested loops and complex exit conditions */
__attribute__((noinline))
void test_complex_loop_ranges(int n) {
    int sum = 0;
    
    /* Outer loop with induction variable */
    for (int i = 0; i < n; i++) {
        /* Inner loop with dependency on outer */
        for (int j = 0; j < i; j++) {
            /* Arithmetic that creates value ranges */
            int val = i * j - 100;
            
            /* Multiple comparisons for range analysis */
            if (val > 0 && val < 1000) {
                sum += val;
            } else if (val >= 1000) {
                sum -= 1000;
            }
            
            /* Bit manipulation */
            unsigned bits = (unsigned)val;
            bits = (bits >> 16) | (bits << 16);  /* byte swap */
            
            if (bits == 0x12345678) {
                break;
            }
        }
        
        /* Exit condition based on computed value */
        if (sum > 50000) {
            break;
        }
    }
    
    global_threshold = sum % 1000;
}

/* Function that creates values with specific bit patterns */
__attribute__((noinline))
void test_bit_patterns(void) {
    /* Create values where high part is 0, low part has specific patterns */
    unsigned patterns[] = {
        0x00000000,  /* All zeros */
        0x00000001,  /* Single bit */
        0x7FFFFFFF,  /* Max positive 31-bit */
        0x80000000,  /* High bit set */
        0xFFFFFFFF,  /* All ones */
    };
    
    for (int i = 0; i < 5; i++) {
        uint64_t extended = (uint64_t)patterns[i];
        
        /* Operations that might trigger the uncovered comparison */
        uint64_t shifted = extended << 32;
        
        /* This comparison: high part == 0, low part > something */
        if (extended > 0x7FFFFFFFULL) {
            global_counter++;
        }
        
        /* Another comparison pattern */
        if (shifted > 0xFFFFFFFF00000000ULL) {
            global_threshold++;
        }
    }
}

/* Main function to drive all tests */
int main(int argc, char *argv[]) {
    /* Test 1: Loop with boundary values */
    test_loop_with_boundary(1000);
    
    /* Test 2: Specific pattern for uncovered code */
    test_zero_high_large_low();
    
    /* Test 3: Multiplication range analysis */
    int mul_result = test_multiplication_range(1000, 2000);
    
    /* Test 4: 128-bit operations */
    test_128bit_operations();
    
    /* Test 5: Complex loop ranges */
    test_complex_loop_ranges(100);
    
    /* Test 6: Bit pattern tests */
    test_bit_patterns();
    
    /* Use results to prevent dead code elimination */
    printf("Results: counter=%lu, threshold=%d, mul=%d\n", 
           global_counter, global_threshold, mul_result);
    
    return 0;
}
