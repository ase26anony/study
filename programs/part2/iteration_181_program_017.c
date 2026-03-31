#include <stdio.h>

int main(int argc, char **argv) {
    // Initialize variables with argc-dependent values to prevent optimization
    int a = argc * 3;
    int b = argc + 7;
    int c = argc * 2 - 5;
    int d = argc + 11;
    int e = argc * 4 - 3;
    int f = argc + 17;
    int g = argc * 5;
    int h = argc + 23;
    
    // Result accumulator to prevent dead code elimination
    int result = 0;
    
    // Loop to provide scheduling context and prevent elimination
    int iterations = (argc > 1) ? 100 : 200;
    for (int i = 0; i < iterations; ++i) {
        // Use different variables for jump condition vs target instruction
        int cond_var1 = a + i;
        int cond_var2 = b * 3;
        
        // Create a conditional jump that depends on runtime values
        // The modulo operation prevents compile-time optimization
        if ((cond_var1 + cond_var2) % 13 == 0) {
            // This goto creates a simplejump_p instruction
            goto target_label;
        }
        
        // Some other computation to make the block non-trivial
        g = h ^ i;
        continue;
        
        // Target label with a safe, non-jump instruction
        // Uses different variables than the jump condition
        target_label:
        // Simple arithmetic operation that shouldn't trap
        // and doesn't reference condition codes or special registers
        d = e + f;
        
        // Additional operation to ensure target isn't isolated
        a = b & c;
        
        // Update result to prevent elimination
        result += d + a;
    }
    
    // More computations using modified variables
    result += g * 2;
    
    // Print result to create observable side effect
    printf("Result: %d\n", result);
    
    return result & 0xFF;
}
