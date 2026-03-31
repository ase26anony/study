/* autofdo_phi_patterns.c
 * Generates PHI-to-conditional patterns for AutoFDO profile analysis
 * Compile with: gcc -O2 -fauto-profile autofdo_phi_patterns.c -o autofdo_phi_patterns
 * Run with: ./autofdo_phi_patterns <mode> <iterations>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <time.h>

#define HOT_ITERATIONS 1000000
#define COLD_ITERATIONS 100
#define ARRAY_SIZE 1000

/* Function 1: Simple PHI pattern with direct copy chain */
uint64_t phi_pattern_simple(int mode, int iterations) {
    uint64_t sum = 0;
    int i, j;
    
    for (i = 0; i < iterations; i++) {
        int cond_base;
        
        /* Create two predecessor blocks with different constant assignments */
        if (mode == 1) {
            /* Hot path - executed frequently */
            cond_base = 1;
        } else {
            /* Cold path - rarely executed */
            cond_base = 0;
        }
        
        /* PHI node implicitly created here - merges the two values */
        int phi_result = cond_base;
        
        /* Create SSA copy chain to trigger the while loop walking back */
        int tmp1 = phi_result;
        int tmp2 = tmp1;
        int tmp3 = tmp2 + 0;  /* Arithmetic that doesn't break single-assignment */
        int cmp_var = tmp3;
        
        /* Conditional using PHI-derived value - triggers uncovered analysis */
        if (cmp_var) {  /* Direct use in if condition */
            /* Hot block when mode=1 */
            for (j = 0; j < 100; j++) {
                sum += j * j;
            }
        } else {
            /* Cold block when mode=0 */
            sum += i;
        }
        
        /* Another comparison type */
        if (cmp_var == 1) {  /* Explicit equality comparison */
            sum += 7;
        }
    }
    
    return sum;
}

/* Function 2: Nested PHI patterns with complex control flow */
uint64_t phi_pattern_complex(int mode, int outer_iter, int inner_iter) {
    uint64_t result = 0;
    int i, j;
    
    for (i = 0; i < outer_iter; i++) {
        int base_value;
        
        /* Multiple predecessor blocks with different constants */
        if (i % 3 == 0) {
            base_value = 1;
        } else if (i % 3 == 1) {
            base_value = 0;
        } else {
            base_value = (mode == 2) ? 1 : 0;
        }
        
        /* PHI node */
        int phi_val = base_value;
        
        /* Longer copy chain */
        int chain1 = phi_val;
        int chain2 = chain1;
        int chain3 = chain2;
        int final_cond = chain3;
        
        /* Loop condition using PHI-derived value */
        while (final_cond && j < inner_iter) {
            result += i * j;
            j++;
            
            /* Nested conditional with another PHI */
            int inner_cond;
            if (j % 2 == 0) {
                inner_cond = 1;
            } else {
                inner_cond = 0;
            }
            
            int inner_phi = inner_cond;
            int inner_copy = inner_phi;
            
            if (inner_copy != 0) {  /* Inequality comparison */
                result += 3;
            }
        }
        
        /* Switch-like pattern */
        int switch_var;
        if (mode == 1) {
            switch_var = 1;
        } else {
            switch_var = 0;
        }
        
        int phi_for_switch = switch_var;
        int switch_copy = phi_for_switch;
        
        if (switch_copy) {
            result += 1000;
        }
    }
    
    return result;
}

/* Function 3: Array processing with PHI-based conditions */
uint64_t array_processing_with_phi(int* data, int size, int threshold) {
    uint64_t sum = 0;
    int i;
    
    for (i = 0; i < size; i++) {
        int should_process;
        
        /* Create PHI based on array value comparison */
        if (data[i] > threshold) {
            should_process = 1;
        } else {
            should_process = 0;
        }
        
        /* PHI node */
        int process_flag = should_process;
        
        /* Copy chain across basic block */
        int flag_copy1 = process_flag;
        int flag_copy2 = flag_copy1;
        
        /* Conditional using PHI-derived value */
        if (flag_copy2 == 1) {
            /* Hot processing path for high values */
            sum += data[i] * data[i];
            
            /* Nested condition with another PHI */
            int extra_processing;
            if (data[i] > threshold * 2) {
                extra_processing = 1;
            } else {
                extra_processing = 0;
            }
            
            int extra_phi = extra_processing;
            if (extra_phi) {
                sum += 100;
            }
        } else {
            /* Cold path for low values */
            sum += data[i];
        }
    }
    
    return sum;
}

/* Function 4: Recursive pattern with PHI propagation */
uint64_t recursive_phi_pattern(int depth, int max_depth, int toggle) {
    if (depth >= max_depth) {
        return 1;
    }
    
    int branch_flag;
    
    /* Different constants in recursive branches */
    if (toggle) {
        branch_flag = 1;
    } else {
        branch_flag = 0;
    }
    
    /* PHI-like value propagation through recursion */
    int current_flag = branch_flag;
    int flag_copy = current_flag;
    
    uint64_t result = 0;
    
    if (flag_copy) {
        /* Hot recursive branch */
        result += recursive_phi_pattern(depth + 1, max_depth, toggle ^ 1) * 3;
    } else {
        /* Cold recursive branch */
        result += recursive_phi_pattern(depth + 1, max_depth, toggle) * 2;
    }
    
    /* Another conditional using the same PHI-derived value */
    if (flag_copy == 1) {
        result += depth * 10;
    }
    
    return result;
}

/* Main function with profile-generating runtime behavior */
int main(int argc, char** argv) {
    int mode = 1;  /* Default to hot mode */
    int iterations = HOT_ITERATIONS;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    srand(time(NULL));
    uint64_t total_result = 0;
    
    /* Warm-up phase with mixed behavior */
    printf("Starting warm-up phase...\n");
    for (int warm = 0; warm < 1000; warm++) {
        total_result ^= phi_pattern_simple(warm % 3, 100);
    }
    
    /* Main execution with mode-dependent behavior */
    printf("Running mode %d with %d iterations...\n", mode, iterations);
    
    switch (mode) {
        case 1:  /* Hot mode - heavily exercises PHI-to-conditional hot paths */
            total_result += phi_pattern_simple(1, iterations);
            total_result += phi_pattern_complex(1, iterations / 100, 50);
            break;
            
        case 2:  /* Mixed mode - balanced hot/cold paths */
            total_result += phi_pattern_simple(0, iterations / 2);
            total_result += phi_pattern_simple(1, iterations / 2);
            total_result += phi_pattern_complex(2, iterations / 200, 30);
            break;
            
        case 3:  /* Cold mode - mostly cold paths */
            total_result += phi_pattern_simple(0, iterations);
            total_result += phi_pattern_complex(0, iterations / 50, 10);
            break;
            
        default:
            printf("Unknown mode. Using default hot mode.\n");
            total_result += phi_pattern_simple(1, iterations);
    }
    
    /* Array processing with data-dependent PHI conditions */
    int data[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = rand() % 1000;
    }
    
    int threshold = (mode == 1) ? 200 : 800;  /* Different thresholds for different modes */
    total_result += array_processing_with_phi(data, ARRAY_SIZE, threshold);
    
    /* Recursive patterns */
    total_result += recursive_phi_pattern(0, 10, mode == 1);
    
    /* Verification output */
    printf("Result checksum: %lu\n", total_result);
    
    /* Additional runs to ensure profile coverage */
    if (mode == 1) {
        /* Extra hot runs for profile dominance */
        for (int extra = 0; extra < 5; extra++) {
            total_result ^= phi_pattern_simple(1, 10000);
        }
    }
    
    return (int)(total_result % 1000);
}
