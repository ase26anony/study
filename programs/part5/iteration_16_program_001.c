#include <stdio.h>
#include <stdint.h>

#define NUM_VARS 40
#define LOOP_ITERATIONS 100

// Force no inlining to maintain complex RTL structure
__attribute__((noinline))
static uint64_t high_pressure_computation(uint64_t seed) {
    // Declare many variables to create register pressure
    register uint64_t v0 = seed + 1;
    register uint64_t v1 = seed * 2;
    register uint64_t v2 = seed ^ 0x55555555;
    register uint64_t v3 = seed << 3;
    register uint64_t v4 = seed >> 2;
    register uint64_t v5 = seed + 0x12345678;
    register uint64_t v6 = seed * 3;
    register uint64_t v7 = seed ^ 0xAAAAAAAA;
    register uint64_t v8 = seed << 1;
    register uint64_t v9 = seed >> 1;
    register uint64_t v10 = seed + 0x87654321;
    register uint64_t v11 = seed * 5;
    register uint64_t v12 = seed ^ 0x33333333;
    register uint64_t v13 = seed << 2;
    register uint64_t v14 = seed >> 3;
    register uint64_t v15 = seed + 0x11111111;
    
    // Create rematerialization candidates - pure functions of inputs
    // These will have long live ranges across the loop
    uint64_t r0 = v0 + 0x1000;  // Candidate 1: v0 + constant
    uint64_t r1 = v1 & 0xFF;    // Candidate 2: v1 & mask
    uint64_t r2 = v2 << 4;      // Candidate 3: v2 << shift
    uint64_t r3 = v3 ^ 0xCC;    // Candidate 4: v3 ^ constant
    uint64_t r4 = v4 | 0xAA;    // Candidate 5: v4 | mask
    uint64_t r5 = v5 * 2;       // Candidate 6: v5 * constant
    uint64_t r6 = v6 - 100;     // Candidate 7: v6 - constant
    uint64_t r7 = v7 + v8;      // Candidate 8: v7 + v8
    
    // More variables to increase pressure
    uint64_t t0, t1, t2, t3, t4, t5, t6, t7;
    uint64_t u0, u1, u2, u3, u4, u5, u6, u7;
    
    // Complex control flow with nested loops
    uint64_t result = 0;
    
    // Outer loop - remat candidates defined outside, used inside
    for (int i = 0; i < LOOP_ITERATIONS; i++) {
        // Use rematerialization candidates here
        // They need to stay live across many operations
        t0 = r0 + i;
        t1 = r1 * i;
        t2 = r2 ^ i;
        t3 = r3 & i;
        t4 = r4 | i;
        t5 = r5 - i;
        t6 = r6 + r7;
        t7 = r7 * r0;
        
        // Inner loop with conditional branches
        for (int j = 0; j < 5; j++) {
            // More computations using different variable sets
            u0 = v0 + v1 + j;
            u1 = v2 * v3 - j;
            u2 = v4 ^ v5 ^ j;
            u3 = v6 & v7 & j;
            u4 = v8 | v9 | j;
            u5 = v10 * v11 + j;
            u6 = v12 - v13 - j;
            u7 = v14 ^ v15 ^ j;
            
            // Conditional that uses remat candidates
            if (j % 2 == 0) {
                // Use one set of variables
                t0 += u0 + r0;
                t1 += u1 + r1;
                t2 += u2 + r2;
            } else {
                // Use different set
                t3 += u3 + r3;
                t4 += u4 + r4;
                t5 += u5 + r5;
            }
            
            // More independent computations to keep values live
            v0 = v0 * 3 + 1;
            v1 = v1 ^ 0x1234;
            v2 = v2 + v3;
            v3 = v3 * 2;
            v4 = v4 ^ v5;
            v5 = v5 + 7;
            v6 = v6 * 5;
            v7 = v7 ^ 0xABCD;
            v8 = v8 + v9;
            v9 = v9 * 11;
            v10 = v10 ^ 0xDCBA;
            v11 = v11 + v12;
            v12 = v12 * 13;
            v13 = v13 ^ 0x2468;
            v14 = v14 + v15;
            v15 = v15 * 17;
        }
        
        // Use all remat candidates in final computation
        result += t0 + t1 + t2 + t3 + t4 + t5 + t6 + t7;
        result += r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7;
        
        // Force all variables to stay live by using them
        result ^= v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7;
        result += v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    }
    
    // Final combination using all variables
    result += r0 * 2;
    result ^= r1 << 1;
    result += r2 >> 2;
    result ^= r3 & 0xFF;
    result += r4 | 0x55;
    result ^= r5 * 3;
    result += r6 - 50;
    result ^= r7 + r0;
    
    return result;
}

// Another noinline function to create more pressure
__attribute__((noinline))
static uint64_t create_more_pressure(uint64_t a, uint64_t b) {
    uint64_t x0 = a + b;
    uint64_t x1 = a * b;
    uint64_t x2 = a ^ b;
    uint64_t x3 = a << (b & 0xF);
    uint64_t x4 = a >> (b & 0x7);
    uint64_t x5 = a + 0xDEADBEEF;
    uint64_t x6 = b + 0xCAFEBABE;
    uint64_t x7 = a * 3 + b * 7;
    uint64_t x8 = (a & 0xFF) | (b & 0xFF00);
    uint64_t x9 = (a ^ 0x1234) + (b ^ 0x5678);
    
    // Keep these live across many operations
    for (int i = 0; i < 50; i++) {
        x0 = x0 * x1 + i;
        x1 = x1 ^ x2 ^ i;
        x2 = x2 + x3 - i;
        x3 = x3 * x4 * (i + 1);
        x4 = x4 ^ x5 ^ (i * 2);
        x5 = x5 + x6 + (i * 3);
        x6 = x6 * x7 - i;
        x7 = x7 ^ x8 ^ (i + 5);
        x8 = x8 + x9 + (i * 7);
        x9 = x9 * x0 + (i * 11);
    }
    
    return x0 + x1 + x2 + x3 + x4 + x5 + x6 + x7 + x8 + x9;
}

int main() {
    uint64_t seed = 0x123456789ABCDEF0ULL;
    
    // Chain computations to increase overall register pressure
    uint64_t result1 = high_pressure_computation(seed);
    uint64_t result2 = create_more_pressure(result1, seed);
    uint64_t result3 = high_pressure_computation(result2);
    
    // Final deterministic result
    uint64_t final_result = result1 ^ result2 ^ result3;
    
    printf("Result: 0x%016llX\n", (unsigned long long)final_result);
    
    // Also print intermediate results to prevent optimization
    printf("Intermediate: 0x%016llX 0x%016llX 0x%016llX\n",
           (unsigned long long)result1,
           (unsigned long long)result2,
           (unsigned long long)result3);
    
    return 0;
}
