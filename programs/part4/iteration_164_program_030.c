/* autofdo_phi_conditional.c
 * Generates PHI-to-conditional patterns for AutoFDO profile analysis
 * Compile: gcc -O2 -fauto-profile autofdo_phi_conditional.c -o autofdo_test
 * Run: ./autofdo_test 1 (hot path) then ./autofdo_test 2 (cold path)
 * Recompile with profile: gcc -O2 -fauto-profile -Wauto-profile autofdo_phi_conditional.c -o autofdo_optimized
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define HOT_ITERATIONS 10000000
#define COLD_ITERATIONS 100
#define ARRAY_SIZE 10000

/* Function 1: Complex PHI pattern with SSA copy chain in hot loop */
uint64_t process_hot_path(int mode, int limit) {
    uint64_t sum = 0;
    int* data = (int*)malloc(ARRAY_SIZE * sizeof(int));
    
    // Initialize array
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i % 100;
    }
    
    // Main hot loop with PHI-to-conditional pattern
    for (int iter = 0; iter < limit; iter++) {
        int phi_value;
        int prev = iter % 2;
        
        // Create PHI node predecessors with constant values
        if (prev == 0) {
            // First predecessor sets to 1
            phi_value = 1;  // Constant 1
        } else {
            // Second predecessor sets to 0  
            phi_value = 0;  // Constant 0
        }
        
        // Create SSA copy chain: phi -> tmp1 -> tmp2 -> cmp_var
        int tmp1 = phi_value;
        int tmp2 = tmp1;
        int cmp_var = tmp2;
        
        // Add more copy operations to ensure while loop executes
        int tmp3 = cmp_var;
        int tmp4 = tmp3 + 0;  // Arithmetic that doesn't break pattern
        int final_cmp = tmp4;
        
        // Multiple comparison types using PHI-derived value
        if (final_cmp) {  // Direct use in if condition
            // Hot path - executed frequently
            for (int i = 0; i < ARRAY_SIZE; i++) {
                if (data[i] > 50) {
                    sum += data[i];
                }
            }
        } else {
            // Cold path - rarely executed
            for (int i = 0; i < 10; i++) {
                sum += i;
            }
        }
        
        // Another PHI pattern with explicit comparison
        int phi2_value;
        if (iter % 3 == 0) {
            phi2_value = 1;
        } else {
            phi2_value = 0;
        }
        
        // Longer copy chain
        int chain1 = phi2_value;
        int chain2 = chain1;
        int chain3 = chain2;
        
        if (chain3 == 1) {  // Explicit equality comparison
            // Another hot path
            sum += iter * 2;
        }
        
        // Loop condition using PHI-derived value
        int loop_control;
        if (mode == 1) {
            loop_control = 1;
        } else {
            loop_control = 0;
        }
        
        int loop_var = loop_control;
        while (loop_var) {  // PHI value in loop condition
            sum += 1;
            loop_var = 0;  // Break after one iteration
        }
    }
    
    free(data);
    return sum;
}

/* Function 2: Nested conditionals with varying PHI patterns */
uint64_t process_nested_branches(int depth, int width) {
    uint64_t result = 0;
    
    for (int i = 0; i < width; i++) {
        int phi_base;
        
        // Create PHI with multiple predecessors
        if (i % 4 == 0) {
            phi_base = 1;
        } else if (i % 4 == 1) {
            phi_base = 0;
        } else if (i % 4 == 2) {
            phi_base = 1;
        } else {
            phi_base = 0;
        }
        
        // Copy through multiple SSA variables
        int copy1 = phi_base;
        int copy2 = copy1;
        int copy3 = copy2;
        
        // Deeply nested conditionals using PHI value
        if (copy3) {
            result += process_inner_hot(i, depth);
        } else {
            result += process_inner_cold(i);
        }
        
        // Another pattern with != 0 comparison
        int phi_neg;
        if (i < width / 2) {
            phi_neg = 0;
        } else {
            phi_neg = 1;
        }
        
        int neg_copy = phi_neg;
        if (neg_copy != 0) {  // != 0 comparison
            result += i * 3;
        }
    }
    
    return result;
}

/* Hot inner function called from frequent path */
uint64_t process_inner_hot(int base, int depth) {
    uint64_t sum = base;
    
    for (int d = 0; d < depth; d++) {
        int inner_phi;
        
        // Another PHI pattern inside hot function
        if (d % 2 == 0) {
            inner_phi = 1;
        } else {
            inner_phi = 0;
        }
        
        int inner_copy = inner_phi;
        int inner_copy2 = inner_copy;
        
        if (inner_copy2) {
            sum += d * 2;
        } else {
            sum += d;
        }
    }
    
    return sum;
}

/* Cold inner function rarely called */
uint64_t process_inner_cold(int base) {
    return base / 2;
}

/* Function 3: Array processing with data-dependent PHI values */
uint64_t process_array_with_phi(int* array, int size, int threshold) {
    uint64_t total = 0;
    
    for (int i = 0; i < size; i++) {
        int phi_select;
        
        // PHI value depends on array data
        if (array[i] > threshold) {
            phi_select = 1;
        } else {
            phi_select = 0;
        }
        
        // Multi-step copy chain
        int step1 = phi_select;
        int step2 = step1;
        int step3 = step2 + 0;  // Preserve SSA single-assignment pattern
        int final_val = step3;
        
        // Use in conditional with both hot and cold paths
        if (final_val) {
            // Hot path for high values
            total += array[i] * 2;
            
            // Nested conditional with another PHI
            int nested_phi;
            if (array[i] > threshold * 2) {
                nested_phi = 1;
            } else {
                nested_phi = 0;
            }
            
            int nested_copy = nested_phi;
            if (nested_copy == 1) {  // Explicit == 1 comparison
                total += 1000;
            }
        } else {
            // Cold path for low values
            total += 1;
        }
    }
    
    return total;
}

/* Main function with different execution modes */
int main(int argc, char** argv) {
    int mode = 1;  // Default to hot mode
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    uint64_t final_result = 0;
    int iterations;
    
    // Warm-up phase
    printf("Starting execution mode %d\n", mode);
    
    if (mode == 1) {
        // HOT MODE: Execute hot paths millions of times
        iterations = HOT_ITERATIONS;
        
        // Process hot path with complex PHI patterns
        final_result += process_hot_path(1, iterations / 1000);
        
        // Process with nested branches (deep hot paths)
        final_result += process_nested_branches(10, 1000);
        
        // Array processing with data-dependent PHI
        int* data_array = (int*)malloc(ARRAY_SIZE * sizeof(int));
        for (int i = 0; i < ARRAY_SIZE; i++) {
            data_array[i] = i;
        }
        final_result += process_array_with_phi(data_array, ARRAY_SIZE, 5000);
        free(data_array);
        
    } else if (mode == 2) {
        // COLD MODE: Execute cold paths only
        iterations = COLD_ITERATIONS;
        
        // Mostly execute cold branches
        final_result += process_hot_path(2, iterations);
        
        // Shallow nested branches (more cold paths)
        final_result += process_nested_branches(2, 100);
        
        // Array processing favoring cold paths
        int* data_array = (int*)malloc(1000 * sizeof(int));
        for (int i = 0; i < 1000; i++) {
            data_array[i] = i % 10;  // Low values -> cold paths
        }
        final_result += process_array_with_phi(data_array, 1000, 100);
        free(data_array);
        
    } else {
        // MIXED MODE: Balanced execution
        iterations = HOT_ITERATIONS / 10;
        
        // Mix of hot and cold
        final_result += process_hot_path(0, iterations);
        final_result += process_nested_branches(5, 500);
    }
    
    // Verification checksum
    printf("Final result: %lu\n", final_result);
    printf("Mode %d completed with %d logical iterations\n", mode, iterations);
    
    return 0;
}
