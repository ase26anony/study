#include <stdio.h>
#include <stdint.h>

// Prevent inlining to maintain complex RTL structure
__attribute__((noinline))
static uint64_t high_pressure_computation(int* data, int n) {
    // Initialize many variables from input to create distinct values
    int v0 = data[0] ^ 0x55AA55AA;
    int v1 = data[1] + 0x12345678;
    int v2 = data[2] * 0x89ABCDEF;
    int v3 = data[3] | 0xF0F0F0F0;
    int v4 = data[4] & 0x0F0F0F0F;
    int v5 = data[5] << 3;
    int v6 = data[6] >> 2;
    int v7 = data[7] + data[0];
    int v8 = data[8] - data[1];
    int v9 = data[9] * data[2];
    int v10 = data[10] | data[3];
    int v11 = data[11] & data[4];
    int v12 = data[12] ^ data[5];
    int v13 = data[13] + 0x11111111;
    int v14 = data[14] * 0x33333333;
    int v15 = data[15] | 0xCCCCCCCC;
    
    // Create rematerialization candidates - pure functions of inputs
    // These will have long live ranges and be cheap to recompute
    int cand1 = v0 + 0x1000;      // Simple addition
    int cand2 = v1 & 0x00FF00FF;  // Mask operation
    int cand3 = v2 << 2;          // Simple shift
    int cand4 = v3 ^ 0xAAAAAAAA;  // XOR with constant
    int cand5 = v4 + v5;          // Addition of two values
    int cand6 = v6 & 0x0000FFFF;  // Mask to lower 16 bits
    int cand7 = v7 << 1;          // Multiply by 2
    int cand8 = v8 ^ v9;          // XOR of two values
    
    // Keep the candidates live across many operations
    // by using them in a complex control flow structure
    
    uint64_t result = 0;
    
    // Outer loop creates complex liveness patterns
    for (int i = 0; i < n; i++) {
        // Inner loop with conditional branches
        for (int j = 0; j < 4; j++) {
            // Use different sets of variables in different branches
            // to create merging points with many live values
            if (j % 2 == 0) {
                // Use first set of candidates and variables
                int temp1 = cand1 + v10 + i;
                int temp2 = cand2 & v11 & j;
                int temp3 = cand3 | v12 | (i * j);
                result += temp1 + temp2 + temp3;
                
                // More computations to increase pressure
                int temp4 = v13 * cand4 / (j + 1);
                int temp5 = v14 ^ cand5 ^ (i << j);
                int temp6 = v15 + cand6 - (j * 3);
                result += temp4 + temp5 + temp6;
            } else {
                // Use second set of candidates and variables
                int temp7 = cand7 * v0 * (j + 2);
                int temp8 = cand8 + v1 + (i << 1);
                int temp9 = cand1 - v2 - (j * 4);
                result += temp7 + temp8 + temp9;
                
                // More computations with different combinations
                int temp10 = v3 & cand2 & 0x7F7F7F7F;
                int temp11 = v4 | cand3 | 0x80808080;
                int temp12 = v5 ^ cand4 ^ 0x55555555;
                result += temp10 + temp11 + temp12;
            }
            
            // Additional computations that use all candidates
            // to ensure they stay live throughout the loop
            int mix1 = cand5 + cand6 + cand7 + cand8;
            int mix2 = cand1 & cand2 & cand3 & cand4;
            result += (mix1 | mix2) * (i + j);
            
            // Force spill/reload pressure with many intermediate values
            int t1 = v6 + v7 + v8;
            int t2 = v9 - v10 - v11;
            int t3 = v12 * v13 * v14;
            int t4 = v15 | v0 | v1;
            int t5 = v2 & v3 & v4;
            int t6 = v5 ^ v6 ^ v7;
            int t7 = v8 + v9 + v10;
            int t8 = v11 - v12 - v13;
            
            // Use all temporaries in a way that prevents dead code elimination
            result += t1 + t2 + t3 + t4 + t5 + t6 + t7 + t8;
        }
        
        // Use candidates outside inner loop too
        if (i % 3 == 0) {
            result += cand1 + cand3 + cand5 + cand7;
        } else if (i % 3 == 1) {
            result += cand2 + cand4 + cand6 + cand8;
        } else {
            result += cand1 * cand2 * cand3;
        }
    }
    
    // Final use of all candidates to ensure they're live until the end
    result += cand1 + cand2 + cand3 + cand4 + cand5 + cand6 + cand7 + cand8;
    
    // Also use all original variables in final computation
    result += v0 + v1 + v2 + v3 + v4 + v5 + v6 + v7;
    result += v8 + v9 + v10 + v11 + v12 + v13 + v14 + v15;
    
    return result;
}

// Another noinline function to create more compilation context
__attribute__((noinline))
static uint64_t process_data(int* data, int size) {
    // Call the high pressure function multiple times
    // to create different register allocation contexts
    uint64_t sum = 0;
    for (int i = 0; i < 3; i++) {
        sum += high_pressure_computation(data + i * 5, size / 2);
    }
    return sum;
}

int main() {
    // Initialize test data
    int data[32];
    for (int i = 0; i < 32; i++) {
        data[i] = i * 12345 + 6789;
    }
    
    // Perform computation
    uint64_t result = process_data(data, 8);
    
    // Print result to prevent optimization
    printf("Result: %llu\n", (unsigned long long)result);
    
    return 0;
}
