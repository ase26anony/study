#include <stdio.h>

int main(int argc, char **argv) {
    // Initialize variables with argc-dependent values to prevent optimization
    volatile int init = argc;
    
    // Variables for the jump condition - kept separate from target instruction
    int cond_a = init + 1;
    int cond_b = init * 2;
    int cond_mod = 13;
    
    // Variables for the target instruction (must not overlap with jump resources)
    int target_x = init + 3;
    int target_y = init + 4;
    int target_res = 0;
    
    // Additional variables to ensure target isn't the only instruction in block
    int aux_a = init + 5;
    int aux_b = init + 6;
    int aux_res = 0;
    
    // Loop counter to provide scheduling context
    int iterations = (argc > 1) ? 100 : 200;
    
    // Result accumulator to prevent dead code elimination
    int checksum = 0;
    
    for (int i = 0; i < iterations; ++i) {
        // Create runtime-dependent condition to prevent constant folding
        // Using modulo with prime number to ensure non-trivial pattern
        if ((i + cond_a) % cond_mod == 0) {
            // This should generate a simple conditional jump
            goto delay_candidate_label;
        }
        
        // Some computation to make the CFG non-trivial
        checksum += i * 2;
        continue;
        
delay_candidate_label:
        // TARGET INSTRUCTION: Simple, safe arithmetic operation
        // Must be non-jump, non-trapping, and not reference jump resources
        target_res = target_x + target_y;  // Simple addition - safe
        
        // Additional instruction to ensure target isn't alone in basic block
        aux_res = aux_a ^ aux_b;  // Bitwise XOR - also safe
        
        // Use results to prevent elimination
        checksum += target_res + aux_res;
        
        // Reset variables to break live ranges across iterations
        target_x = init + i + 3;
        target_y = init + i + 4;
        aux_a = init + i + 5;
        aux_b = init + i + 6;
    }
    
    // Additional computations to create register pressure
    int final_compute = 0;
    for (int i = 0; i < 10; ++i) {
        final_compute += checksum * i;
        final_compute ^= (target_res + aux_res);
    }
    
    // Print result to create observable side effect
    printf("Result: %d (checksum: %d)\n", final_compute, checksum);
    
    return final_compute != 0 ? 0 : 1;
}
