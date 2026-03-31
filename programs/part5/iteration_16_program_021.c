#include <stdio.h>
#include <stdint.h>

#define NUM_VARS 32
#define LOOP_ITERATIONS 100

// Prevent inlining to maintain complex RTL structure
__attribute__((noinline))
static uint64_t high_pressure_calculation(uint64_t seed) {
    // Declare many distinct variables to create register pressure
    register uint64_t v0 = seed + 1;
    register uint64_t v1 = seed * 2;
    register uint64_t v2 = seed ^ 0x55555555;
    register uint64_t v3 = seed - 12345;
    register uint64_t v4 = seed << 3;
    register uint64_t v5 = seed >> 2;
    register uint64_t v6 = seed * 3 + 1;
    register uint64_t v7 = seed | 0xAAAAAAAA;
    register uint64_t v8 = seed & 0x33333333;
    register uint64_t v9 = seed * 7 - 5;
    register uint64_t v10 = seed ^ 0x12345678;
    register uint64_t v11 = seed + 98765;
    register uint64_t v12 = seed * 11;
    register uint64_t v13 = seed / 3;
    register uint64_t v14 = seed % 100;
    register uint64_t v15 = ~seed;
    
    // Create rematerialization candidates - pure functions of inputs
    // These will have long live ranges across the loop
    uint64_t r0 = v0 + 0x1000;      // Candidate 1: v0 + constant
    uint64_t r1 = v1 & 0xFF00FF;    // Candidate 2: v1 & mask
    uint64_t r2 = v2 << 2;          // Candidate 3: v2 << shift
    uint64_t r3 = v3 ^ 0x99999999;  // Candidate 4: v3 ^ constant
    uint64_t r4 = v4 | 0x11111111;  // Candidate 5: v4 | constant
    uint64_t r5 = v5 * 3;           // Candidate 6: v5 * constant
    
    // Additional variables to increase pressure
    uint64_t t0, t1, t2, t3, t4, t5, t6, t7, t8, t9;
    
    // Complex loop with conditional branches to create merging points
    for (int i = 0; i < LOOP_ITERATIONS; i++) {
        // Use rematerialization candidates inside loop
        // They're defined outside but used here, creating long live ranges
        if (i % 3 == 0) {
            t0 = r0 + r1;           // Use candidates
            t1 = v6 * v7 + v8;
            t2 = v9 ^ v10;
            t3 = v11 << (i & 7);
        } else if (i % 3 == 1) {
            t0 = r2 - r3;           // Use different candidates
            t1 = v12 & v13;
            t2 = v14 | v15;
            t3 = v0 * v1 + i;
        } else {
            t0 = r4 ^ r5;           // Use remaining candidates
            t1 = v2 * v3 - v4;
            t2 = v5 & v6;
            t3 = v7 << (v8 & 3);
        }
        
        // More independent computations to keep values live
        t4 = v8 + v9 * 2;
        t5 = v10 - v11 / 2;
        t6 = v12 ^ v13 ^ i;
        t7 = v14 << 1;
        t8 = v15 >> 2;
        t9 = v0 * 3 + v1 * 5;
        
        // Use all variables in conditional to keep them live
        if (t0 > t1) {
            v0 = t2 + t3;
            v1 = t4 - t5;
        } else {
            v2 = t6 ^ t7;
            v3 = t8 | t9;
        }
        
        // More computations that use many live values
        v4 = (v0 + v1) * (v2 - v3);
        v5 = (v4 & 0xFFFF) + (v5 << 1);
        v6 = v6 * 17 + i;
        v7 = v7 ^ v8 ^ v9;
        v8 = v10 + v11 * 3;
        v9 = v12 - v13 / 5;
        v10 = v14 & v15 & 0xFF;
        v11 = (v0 << 2) | (v1 >> 2);
        
        // Nested loop to further complicate liveness
        for (int j = 0; j < 3; j++) {
            uint64_t inner = v0 + v1 + j;
            v12 = v12 ^ inner;
            v13 = v13 + inner * j;
            // Use remat candidates in inner loop too
            v14 = v14 + r0 + r1 + r2;
        }
    }
    
    // Final computation using all variables including remat candidates
    // This ensures they stay live until the end
    uint64_t result = v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9 +
                     v10 + v11 + v12 + v13 + v14 + v15 +
                     r0 + r1 + r2 + r3 + r4 + r5;
    
    return result;
}

// Another noinline function to create more pressure
__attribute__((noinline))
static uint64_t secondary_calculation(uint64_t a, uint64_t b) {
    // Many independent computations
    uint64_t x0 = a * b + 1;
    uint64_t x1 = a ^ b ^ 0x1234;
    uint64_t x2 = a << (b & 15);
    uint64_t x3 = b >> (a & 7);
    uint64_t x4 = a + b * 3;
    uint64_t x5 = a - b / 2;
    uint64_t x6 = (a & 0xFF) * (b & 0xFF);
    uint64_t x7 = a | b | 0x8888;
    
    // Rematerialization candidates
    uint64_t y0 = x0 + 0x2000;  // x0 + constant
    uint64_t y1 = x1 & 0xAA55;  // x1 & mask
    
    // Loop with complex control flow
    for (int i = 0; i < 50; i++) {
        if (i % 4 == 0) {
            x0 = x1 + y0;      // Use remat candidate
            x2 = x3 * y1;
        } else if (i % 4 == 1) {
            x4 = x5 ^ y0;
            x6 = x7 - y1;
        } else if (i % 4 == 2) {
            x1 = x2 + y0 + y1;
            x3 = x4 * 2;
        } else {
            x5 = x6 | y0;
            x7 = x0 & y1;
        }
        
        // Cross assignments to extend live ranges
        uint64_t tmp = x0; x0 = x1; x1 = x2; x2 = x3; x3 = x4;
        x4 = x5; x5 = x6; x6 = x7; x7 = tmp;
    }
    
    return x0 + x1 + x2 + x3 + x4 + x5 + x6 + x7 + y0 + y1;
}

int main() {
    uint64_t seed = 0xDEADBEEF;
    
    // Chain calculations to increase overall register pressure
    uint64_t result1 = high_pressure_calculation(seed);
    uint64_t result2 = secondary_calculation(seed, result1);
    uint64_t result3 = high_pressure_calculation(result2);
    
    // Final deterministic result
    uint64_t final_result = result1 ^ result2 ^ result3;
    
    printf("Result: 0x%016llX\n", (unsigned long long)final_result);
    
    // Also print intermediate results to prevent optimization
    printf("Intermediate: %llu %llu %llu\n", 
           (unsigned long long)result1,
           (unsigned long long)result2,
           (unsigned long long)result3);
    
    return 0;
}
