#include <stdio.h>
#include <stdint.h>

#define NUM_VARS 32
#define LOOP_ITERATIONS 100

// Force no inlining to maintain complex RTL structure
static __attribute__((noinline)) 
uint64_t high_pressure_calculation(uint64_t seed) {
    // Declare many distinct variables to create register pressure
    register uint64_t a0 = seed + 1;
    register uint64_t a1 = seed * 2;
    register uint64_t a2 = seed ^ 0x12345678;
    register uint64_t a3 = seed - 777;
    register uint64_t a4 = seed << 3;
    register uint64_t a5 = seed >> 2;
    register uint64_t a6 = seed * 3 + 1;
    register uint64_t a7 = seed & 0xF0F0F0F0;
    register uint64_t a8 = seed | 0x0F0F0F0F;
    register uint64_t a9 = seed % 1001;
    
    // More variables to increase pressure
    uint64_t b0 = a0 * 2;
    uint64_t b1 = a1 + a2;
    uint64_t b2 = a3 ^ a4;
    uint64_t b3 = a5 - a6;
    uint64_t b4 = a7 & a8;
    uint64_t b5 = a9 << 1;
    uint64_t b6 = a0 >> 2;
    uint64_t b7 = a1 * a2;
    uint64_t b8 = a3 + a4 + a5;
    uint64_t b9 = a6 ^ a7 ^ a8;
    
    // Create rematerialization candidates - pure functions of inputs
    // These will have long live ranges across the loop
    uint64_t remat_candidate1 = a0 * 3 + 17;  // Cheap: a0*3+17
    uint64_t remat_candidate2 = a1 & 0xFF00;  // Cheap: a1 & mask
    uint64_t remat_candidate3 = a2 << 2;      // Cheap: a2 << 2
    uint64_t remat_candidate4 = a3 ^ 0xAAAA;  // Cheap: a3 ^ constant
    uint64_t remat_candidate5 = a4 + 12345;   // Cheap: a4 + constant
    
    // Complex control flow to create challenging liveness patterns
    uint64_t accumulator = 0;
    
    // Outer loop - keeps many variables live
    for (int i = 0; i < LOOP_ITERATIONS; i++) {
        // Use rematerialization candidates inside loop
        // They were defined outside, creating cross-block liveness
        uint64_t temp1 = remat_candidate1 * i;
        uint64_t temp2 = remat_candidate2 + i;
        uint64_t temp3 = remat_candidate3 ^ i;
        uint64_t temp4 = remat_candidate4 - i;
        uint64_t temp5 = remat_candidate5 & i;
        
        // Nested conditional to create merge points
        if (i % 3 == 0) {
            // Use one set of variables
            accumulator += a0 + b0 + temp1;
            accumulator ^= a1 * b1 + temp2;
        } else if (i % 3 == 1) {
            // Use different set
            accumulator += a2 + b2 + temp3;
            accumulator ^= a3 * b3 + temp4;
        } else {
            // Use another set
            accumulator += a4 + b4 + temp5;
            accumulator ^= a5 * b5 + remat_candidate1;
        }
        
        // Inner loop with more computations
        for (int j = 0; j < 5; j++) {
            // More computations using many live variables
            uint64_t inner_temp = (a6 + j) * (b6 - j);
            accumulator += inner_temp;
            
            // Conditional inside inner loop
            if (j % 2 == 0) {
                accumulator ^= a7 + b7 + remat_candidate2;
            } else {
                accumulator ^= a8 + b8 + remat_candidate3;
            }
        }
        
        // More independent computations to keep values live
        uint64_t c0 = b0 * i + 11;
        uint64_t c1 = b1 ^ i ^ 0xCC;
        uint64_t c2 = b2 + i * 7;
        uint64_t c3 = b3 & i & 0xAA;
        uint64_t c4 = b4 << (i % 8);
        
        // Use all these new values
        accumulator += c0 + c1 + c2 + c3 + c4;
        
        // Force all original variables to stay live by using them
        // in a way that can't be optimized away
        a0 = a0 ^ (accumulator & 1);
        a1 = a1 + (accumulator & 2);
        a2 = a2 - (accumulator & 4);
        a3 = a3 * ((accumulator & 8) ? 2 : 1);
        a4 = a4 | (accumulator & 16);
    }
    
    // Final combination using all variables to ensure none are dead
    uint64_t result = accumulator;
    result += a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7 + a8 + a9;
    result += b0 + b1 + b2 + b3 + b4 + b5 + b6 + b7 + b8 + b9;
    result += remat_candidate1 + remat_candidate2 + remat_candidate3 + 
              remat_candidate4 + remat_candidate5;
    
    return result;
}

// Another high-pressure function with different pattern
static __attribute__((noinline))
uint64_t another_pressure_function(uint64_t x, uint64_t y) {
    // Create many intermediate values
    uint64_t v1 = x * y + 1;
    uint64_t v2 = x ^ y ^ 0x1234;
    uint64_t v3 = (x << 3) | (y >> 2);
    uint64_t v4 = x % (y + 1);
    uint64_t v5 = y - x * 2;
    uint64_t v6 = (x & 0xFF) * (y & 0xFF);
    uint64_t v7 = x + y * 3;
    uint64_t v8 = (x ^ 0x5555) + (y ^ 0xAAAA);
    uint64_t v9 = x * x - y * y;
    uint64_t v10 = (x | y) & 0xF0F0F0F0;
    
    // Rematerialization candidates
    uint64_t rc1 = v1 + 42;      // v1 + constant
    uint64_t rc2 = v2 << 1;      // v2 << 1
    uint64_t rc3 = v3 & 0xFF00;  // v3 & mask
    uint64_t rc4 = v4 ^ 0x3333;  // v4 ^ constant
    
    // Complex computation using all values
    uint64_t sum = 0;
    for (int i = 0; i < 50; i++) {
        // Switch statement creates multiple control flow paths
        switch (i % 4) {
            case 0:
                sum += rc1 * i + v1;
                sum ^= v2 + rc2;
                break;
            case 1:
                sum += rc3 - i + v3;
                sum ^= v4 * rc4;
                break;
            case 2:
                sum += v5 * v6 + rc1;
                sum ^= v7 - rc2;
                break;
            case 3:
                sum += v8 ^ v9 ^ rc3;
                sum ^= v10 + rc4;
                break;
        }
        
        // Additional computations to increase pressure
        for (int j = 0; j < 3; j++) {
            uint64_t t1 = v1 + j;
            uint64_t t2 = v2 - j;
            uint64_t t3 = v3 * j;
            uint64_t t4 = v4 ^ j;
            
            sum += t1 + t2 + t3 + t4;
            
            // Use remat candidates in inner loop
            if (j % 2 == 0) {
                sum ^= rc1 + rc2;
            } else {
                sum ^= rc3 + rc4;
            }
        }
    }
    
    return sum;
}

int main() {
    uint64_t seed = 0xDEADBEEF;
    
    // Call high-pressure functions multiple times
    uint64_t result1 = high_pressure_calculation(seed);
    uint64_t result2 = another_pressure_function(seed, seed + 1);
    uint64_t result3 = high_pressure_calculation(seed + 2);
    
    // Combine results to ensure all computations matter
    uint64_t final_result = result1 ^ result2 + result3;
    
    printf("Result: %llu\n", (unsigned long long)final_result);
    
    // Verify with a simple check (deterministic)
    if (final_result == 0) {
        printf("Unexpected zero result\n");
    }
    
    return 0;
}
