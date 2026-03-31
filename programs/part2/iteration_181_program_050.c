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
    
    // Additional variables for the target instruction
    int x = argc * 6;
    int y = argc + 19;
    int z = argc * 7 - 2;
    
    // Result accumulator to prevent dead code elimination
    int result = 0;
    
    // Loop to provide scheduling context and prevent elimination
    int iterations = (argc > 1) ? 100 : 200;
    for (int i = 0; i < iterations; ++i) {
        // Use volatile to prevent constant folding
        volatile int mod_base = 13;
        
        // Create a conditional jump that depends on runtime values
        // This should generate a simple conditional jump (simplejump_p)
        if (((i + argc) % mod_base) == 0) {
            // Jump to label with a safe target instruction
            goto target_label;
        }
        
        // Some computations to create register pressure
        a = b + c;
        d = e ^ f;
        g = h * 2;
        
        // Skip the target instruction when not jumping
        continue;
        
    target_label:
        // TARGET INSTRUCTION: This should be the candidate for delay slot filling
        // Simple arithmetic with distinct variables not used in jump condition
        x = y + z;
        
        // Additional instruction after label to ensure it's not alone
        a = b - c;
        
        // Continue with other operations
        d = e | f;
        g = h + 1;
    }
    
    // Use all variables to create observable side effects
    result = a + b + c + d + e + f + g + h + x + y + z;
    
    // Print result to prevent complete optimization
    printf("Result: %d\n", result);
    
    return result & 0xFF;
}
