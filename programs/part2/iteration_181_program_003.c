#include <stdio.h>

int main(int argc, char **argv) {
    // Initialize variables with argc-dependent values to prevent optimization
    int a = argc * 3;
    int b = argc + 7;
    int c = argc * 2 - 5;
    int d = argc + 11;
    int e = argc * 4 - 3;
    int f = argc + 13;
    int g = argc * 5;
    int h = argc + 17;
    
    // Result accumulator to create observable side effects
    int result = 0;
    
    // Loop provides scheduling context and prevents elimination
    int iterations = (argc > 1) ? 100 : 200;
    for (int i = 0; i < iterations; ++i) {
        // Use different sets of variables for condition vs target instruction
        // Condition uses variables g and h
        if (((i + g) % 13) == (h % 7)) {
            // This should create a simple conditional jump
            goto target_label;
        }
        
        // Some intermediate computation to separate blocks
        d = e ^ f;
        continue;
        
    target_label:
        // Target instruction: simple arithmetic with different variables (a, b, c)
        // This should be a non-jump, non-trapping instruction
        a = b + c;  // Simple addition, no side effects
        
        // Additional computation after label to ensure it's not a single-instruction block
        d = e ^ f;
        
        // Use result to prevent dead code elimination
        result += a + d;
    }
    
    // More computations using modified variables
    result += a * b + c - d + e * f;
    
    // Print result to create observable output
    printf("Result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
