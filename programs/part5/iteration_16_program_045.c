#include <stdio.h>
#include <stdint.h>

#define NUM_VARS 32
#define LOOP_ITERATIONS 100

// Force no inlining to keep RTL complex
static __attribute__((noinline)) 
uint64_t high_pressure_computation(const int32_t* input) {
    // Declare many variables to create register pressure
    register int32_t v0  = input[0]  + 1;
    register int32_t v1  = input[1]  - 2;
    register int32_t v2  = input[2]  * 3;
    register int32_t v3  = input[3]  / 4;
    register int32_t v4  = input[4]  | 0x55;
    register int32_t v5  = input[5]  & 0xAA;
    register int32_t v6  = input[6]  ^ 0xFF;
    register int32_t v7  = input[7]  << 1;
    register int32_t v8  = input[8]  >> 2;
    register int32_t v9  = input[9]  + v0;
    register int32_t v10 = input[10] - v1;
    register int32_t v11 = input[11] * v2;
    register int32_t v12 = input[12] / v3;
    register int32_t v13 = input[13] | v4;
    register int32_t v14 = input[14] & v5;
    register int32_t v15 = input[15] ^ v6;
    
    // Create rematerialization candidates - pure functions of inputs
    // These will have long live ranges across the loop
    int32_t remat_candidate1 = v0 * 37 + v1;  // Pure computation
    int32_t remat_candidate2 = v2 & 0x7F;     // Cheap bitwise op
    int32_t remat_candidate3 = v3 << 3;       // Cheap shift
    int32_t remat_candidate4 = v4 ^ 0x1234;   // Cheap XOR
    int32_t remat_candidate5 = v5 + 0xABCD;   // Cheap addition
    
    // More variables to increase pressure
    int32_t v16 = input[16] + 11;
    int32_t v17 = input[17] - 22;
    int32_t v18 = input[18] * 33;
    int32_t v19 = input[19] / 44;
    int32_t v20 = input[20] | 0x99;
    int32_t v21 = input[21] & 0x66;
    int32_t v22 = input[22] ^ 0x33;
    int32_t v23 = input[23] << 4;
    int32_t v24 = input[24] >> 3;
    int32_t v25 = input[25] + v16;
    int32_t v26 = input[26] - v17;
    int32_t v27 = input[27] * v18;
    int32_t v28 = input[28] / v19;
    int32_t v29 = input[29] | v20;
    int32_t v30 = input[30] & v21;
    int32_t v31 = input[31] ^ v22;
    
    // More rematerialization candidates
    int32_t remat_candidate6 = v16 * 41 + v17;
    int32_t remat_candidate7 = v18 & 0x3F;
    int32_t remat_candidate8 = v19 << 2;
    int32_t remat_candidate9 = v20 ^ 0x5678;
    int32_t remat_candidate10 = v21 + 0xDEF0;
    
    uint64_t accumulator = 0;
    
    // Complex loop with conditional branches to create merging points
    for (int i = 0; i < LOOP_ITERATIONS; i++) {
        // Use rematerialization candidates inside loop
        // They were defined outside, creating long live ranges
        int32_t temp1 = remat_candidate1 + i;
        int32_t temp2 = remat_candidate2 - i;
        int32_t temp3 = remat_candidate3 * i;
        int32_t temp4 = remat_candidate4 ^ i;
        int32_t temp5 = remat_candidate5 & i;
        
        // Conditional branch using different sets of variables
        if (i & 1) {
            // Use first set of variables
            accumulator += v0 + v1 + v2 + v3 + v4 + v5;
            accumulator += temp1 * temp2;
            
            // More computations to keep values live
            int32_t t1 = v6 * v7 + v8;
            int32_t t2 = v9 & v10 | v11;
            int32_t t3 = v12 ^ v13 << 2;
            accumulator += t1 + t2 + t3;
        } else {
            // Use second set of variables
            accumulator += v16 + v17 + v18 + v19 + v20 + v21;
            accumulator += temp3 * temp4;
            
            // Different computations
            int32_t t4 = v22 * v23 + v24;
            int32_t t5 = v25 & v26 | v27;
            int32_t t6 = v28 ^ v29 << 1;
            accumulator += t4 + t5 + t6;
        }
        
        // Use more rematerialization candidates
        if (i % 3 == 0) {
            accumulator += remat_candidate6 * remat_candidate7;
            accumulator += remat_candidate8 ^ remat_candidate9;
        } else if (i % 3 == 1) {
            accumulator += remat_candidate10 & remat_candidate1;
            accumulator += remat_candidate2 | remat_candidate3;
        } else {
            accumulator += remat_candidate4 - remat_candidate5;
            accumulator += remat_candidate6 + remat_candidate7;
        }
        
        // Nested loop to increase complexity
        for (int j = 0; j < 3; j++) {
            // Use all variables to keep them live
            accumulator += v30 + v31;
            accumulator += (v0 << j) & (v16 >> j);
            
            // More rematerialization usage
            int32_t nested_temp = remat_candidate1 + remat_candidate6;
            accumulator += nested_temp * j;
        }
        
        // Complex expression using many live variables
        accumulator += ((v0 * v1) + (v2 & v3) - (v4 | v5) ^ 
                       (v16 * v17) + (v18 & v19) - (v20 | v21)) * i;
    }
    
    // Final combination using all variables and rematerialization candidates
    uint64_t final_result = accumulator;
    final_result += remat_candidate1 * 1000;
    final_result += remat_candidate2 * 100;
    final_result += remat_candidate3 * 10;
    final_result += remat_candidate4;
    final_result += remat_candidate5;
    final_result += remat_candidate6 * 10000;
    final_result += remat_candidate7 * 1000;
    final_result += remat_candidate8 * 100;
    final_result += remat_candidate9 * 10;
    final_result += remat_candidate10;
    
    // Use all variables in final computation to prevent dead code elimination
    final_result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7 + v8 + v9;
    final_result += v10 + v11 + v12 + v13 + v14 + v15 + v16 + v17;
    final_result += v18 + v19 + v20 + v21 + v22 + v23 + v24 + v25;
    final_result += v26 + v27 + v28 + v29 + v30 + v31;
    
    return final_result;
}

// Another noinline function to create more compilation context
static __attribute__((noinline))
uint64_t process_data(const int32_t* data, int size) {
    uint64_t total = 0;
    
    for (int i = 0; i < size; i += NUM_VARS) {
        // Call the high pressure function multiple times
        total += high_pressure_computation(data + i);
        
        // Additional computations between calls
        for (int j = 0; j < 5; j++) {
            total = (total << 3) | (total >> 61);  // Rotate
            total ^= data[i % size] * 0x9E3779B9;  // Mix with constant
        }
    }
    
    return total;
}

int main() {
    // Initialize data array
    int32_t data[NUM_VARS * 4];
    for (int i = 0; i < NUM_VARS * 4; i++) {
        data[i] = i * 3 + 7;  // Simple but non-trivial pattern
    }
    
    // Perform computation
    uint64_t result = process_data(data, NUM_VARS * 4);
    
    // Print result to prevent optimization
    printf("Result: %llu\n", (unsigned long long)result);
    
    // Also use in a conditional to ensure computation isn't dead
    if (result > 1000000) {
        printf("Large result detected\n");
    }
    
    return 0;
}
