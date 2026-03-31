#include <stdio.h>
#include <stdint.h>

#define NUM_VARS 32
#define LOOP_ITERATIONS 100

// Force no inlining to keep RTL complex
__attribute__((noinline)) 
static uint32_t high_pressure_computation(uint32_t input) {
    // Declare many variables to create register pressure
    register uint32_t a0 = input + 1;
    register uint32_t a1 = input * 2;
    register uint32_t a2 = input ^ 0x55555555;
    register uint32_t a3 = input - 17;
    register uint32_t a4 = input | 0xAAAAAAAA;
    register uint32_t a5 = input & 0x33333333;
    register uint32_t a6 = input << 3;
    register uint32_t a7 = input >> 2;
    register uint32_t a8 = input * 3 + 7;
    register uint32_t a9 = input / 5 + 11;
    
    // More variables with different computations
    uint32_t b0 = a0 * a1 + 13;
    uint32_t b1 = a2 - a3 * 2;
    uint32_t b2 = a4 ^ a5 ^ 0x12345678;
    uint32_t b3 = a6 | (a7 << 1);
    uint32_t b4 = a8 + a9 * 3;
    uint32_t b5 = (a0 & a1) | (a2 & a3);
    uint32_t b6 = a4 * 7 - a5;
    uint32_t b7 = a6 ^ a7 ^ a8;
    uint32_t b8 = a9 + (a0 << 2);
    uint32_t b9 = a1 * a2 - a3;
    
    // Even more variables - each with unique computation
    uint32_t c0 = b0 + b1 * 2;
    uint32_t c1 = b2 - b3 + 19;
    uint32_t c2 = b4 ^ b5 ^ b6;
    uint32_t c3 = b7 | b8 & 0xFF;
    uint32_t c4 = b9 * 11 + 23;
    uint32_t c5 = (b0 & 0xF0F0F0F0) | (b1 & 0x0F0F0F0F);
    uint32_t c6 = b2 * 3 + b3 * 5;
    uint32_t c7 = b4 - b5 + b6 - b7;
    uint32_t c8 = b8 << 3 | b9 >> 1;
    uint32_t c9 = (b0 + b1 + b2 + b3) & 0xFFFFFFFF;
    
    // Variables that will be used across loops - creating long live ranges
    uint32_t d0 = c0 + input;  // Will be used in loop
    uint32_t d1 = c1 * input;  // Will be used in loop
    uint32_t d2 = c2 ^ input;  // Will be used in loop
    uint32_t d3 = c3 | input;  // Will be used in loop
    uint32_t d4 = c4 - input;  // Will be used in loop
    
    // Complex control flow to create challenging liveness patterns
    uint32_t result = 0;
    
    // Outer loop - uses variables defined outside
    for (int i = 0; i < LOOP_ITERATIONS; i++) {
        // Use variables defined outside loop - these may need rematerialization
        uint32_t temp0 = d0 + i;
        uint32_t temp1 = d1 - i;
        uint32_t temp2 = d2 ^ i;
        uint32_t temp3 = d3 | i;
        uint32_t temp4 = d4 * i;
        
        // Inner loop with conditional - creates merging points
        for (int j = 0; j < 5; j++) {
            // Use different sets of variables based on condition
            if (j % 2 == 0) {
                // Use first set of variables
                result += temp0 + temp1 + c0 + c1;
                result ^= a0 + a1 + b0 + b1;
            } else {
                // Use second set of variables
                result += temp2 + temp3 + c2 + c3;
                result ^= a2 + a3 + b2 + b3;
            }
            
            // More computations that keep many values live
            uint32_t inner_temp = temp4 + j;
            result += inner_temp;
            
            // Use more variables to increase pressure
            if (i % 3 == 0) {
                result += c4 + c5 + c6;
            } else {
                result += c7 + c8 + c9;
            }
        }
        
        // Use all the 'a' variables periodically to keep them live
        if (i % 7 == 0) {
            result += a4 + a5 + a6 + a7 + a8 + a9;
        }
        
        // Conditional that uses many variables
        if (i % 11 == 0) {
            result += b4 + b5 + b6 + b7 + b8 + b9;
        }
    }
    
    // Final computation using all variables to ensure none are optimized away
    result += a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9;
    result += b0 + b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9;
    result += c0 + c1 + c2 + c3 + c4 + c5 + c6 + c7 + c8 + c9;
    result += d0 + d1 + d2 + d3 + d4;
    
    return result;
}

// Another noinline function to create more compilation context
__attribute__((noinline))
static uint32_t secondary_computation(uint32_t seed) {
    uint32_t x = seed;
    
    // Chain of dependent computations
    for (int i = 0; i < 20; i++) {
        x = x * 1103515245 + 12345;
        x = (x >> 16) & 0x7FFF;
        x = x ^ (x << 13);
        x = x * 1664525 + 1013904223;
    }
    
    return x;
}

int main() {
    uint32_t input = 0x12345678;
    
    // Call the high pressure function multiple times
    uint32_t total = 0;
    for (int i = 0; i < 10; i++) {
        uint32_t result1 = high_pressure_computation(input + i);
        uint32_t result2 = secondary_computation(input - i);
        total += result1 + result2;
    }
    
    printf("Result: %u\n", total);
    
    // Also test with different inputs
    uint32_t test_values[] = {1, 100, 1000, 10000, 100000};
    for (int i = 0; i < 5; i++) {
        uint32_t r = high_pressure_computation(test_values[i]);
        printf("Test %d: %u\n", i, r);
    }
    
    return 0;
}
