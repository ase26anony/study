/* autofdo_phi_conditional.c
 * Test program to trigger AutoFDO PHI-to-conditional analysis
 * Compile: gcc -O2 -fauto-profile autofdo_phi_conditional.c -o autofdo_test
 * Run: ./autofdo_test 1  # Dominant hot path
 *      ./autofdo_test 2  # Cold path
 * Recompile with profile: gcc -O2 -fauto-profile -Wauto-profile autofdo_phi_conditional.c -o autofdo_optimized
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HOT_ITERATIONS 10000000
#define WARM_ITERATIONS 1000000
#define COLD_ITERATIONS 100

/* Function 1: Complex PHI pattern with SSA copy chains in hot loop */
unsigned long long process_hot_data(int mode, int size) {
    unsigned long long checksum = 0;
    int* data = (int*)malloc(size * sizeof(int));
    
    // Initialize array with pattern
    for (int i = 0; i < size; i++) {
        data[i] = i % 100;
    }
    
    // Hot loop with PHI-to-conditional pattern
    for (int iter = 0; iter < HOT_ITERATIONS; iter++) {
        int phi_value = 0;  // Will flow through PHI nodes
        
        // Create branching that sets phi_value to 0 or 1
        if (mode == 1) {
            // Hot path - executed most frequently
            phi_value = 1;
        } else {
            // Cold path
            phi_value = 0;
        }
        
        // Multiple SSA copy chain to trigger while loop walking
        int tmp1 = phi_value;
        int tmp2 = tmp1;
        int tmp3 = tmp2 + 0;  // Arithmetic that preserves value
        int cmp_var = tmp3;
        
        // PHI-like behavior through loop carried dependency
        int prev = 0;
        for (int i = 0; i < size; i++) {
            // Complex PHI: prev = (i == 0) ? cmp_var : data[i-1]
            int phi_result = (i == 0) ? cmp_var : prev;
            
            // Chain of assignments from PHI result
            int chain1 = phi_result;
            int chain2 = chain1;
            int final_cmp = chain2;
            
            // Conditional using PHI-derived value (triggers uncovered code)
            if (final_cmp == 1) {
                // Hot path inside loop
                data[i] = data[i] * 2 + 1;
                checksum += data[i];
            } else if (final_cmp != 0) {
                // Rarely taken path
                data[i] = data[i] / 2;
                checksum -= data[i];
            } else {
                // Cold path
                data[i] = data[i] + 1;
                checksum += data[i] * 3;
            }
            
            prev = data[i] % 2;  // Creates loop-carried dependency for PHI
        }
        
        // Another PHI pattern with nested conditionals
        int outer_phi = 0;
        if (checksum % 1000 > 500) {
            outer_phi = 1;
        }
        
        // More SSA copies
        int copy1 = outer_phi;
        int copy2 = copy1;
        
        // Different comparison type
        while (copy2) {
            // This loop rarely executes
            checksum = checksum >> 1;
            copy2 = 0;
        }
    }
    
    free(data);
    return checksum;
}

/* Function 2: Nested conditionals with PHI propagation */
unsigned long long process_cold_data(int mode, int size) {
    unsigned long long checksum = 0;
    
    for (int iter = 0; iter < COLD_ITERATIONS; iter++) {
        int phi_base = (mode == 2) ? 1 : 0;
        
        // Complex SSA chain
        int a = phi_base;
        int b = a;
        int c = b;
        int d = c + 0;
        
        // Switch-like PHI pattern
        int switch_var = 0;
        if (iter % 3 == 0) {
            switch_var = d;
        } else if (iter % 3 == 1) {
            switch_var = 1;
        } else {
            switch_var = 0;
        }
        
        // Chain assignments
        int chain_a = switch_var;
        int chain_b = chain_a;
        
        // Conditional with PHI-derived value
        if (chain_b == 1) {
            // Cold path
            for (int i = 0; i < size; i++) {
                checksum += i * iter;
            }
        } else {
            // Even colder path
            checksum += iter;
        }
        
        // Another PHI pattern
        int loop_phi = 0;
        for (int i = 0; i < 10; i++) {
            // PHI: loop_phi = (i == 0) ? chain_b : loop_phi
            int phi_val = (i == 0) ? chain_b : loop_phi;
            
            // Assignment chain
            int t1 = phi_val;
            int t2 = t1;
            
            if (t2) {
                checksum += i * 100;
            }
            
            loop_phi = checksum % 2;
        }
    }
    
    return checksum;
}

/* Function 3: Mixed hot/cold paths with function calls */
unsigned long long mixed_processing(int mode, int size) {
    static int persistent_state = 0;
    unsigned long long checksum = 0;
    
    // Warm-up phase
    for (int i = 0; i < WARM_ITERATIONS; i++) {
        int warm_phi = (i < WARM_ITERATIONS / 2) ? 1 : 0;
        
        // SSA copies
        int w1 = warm_phi;
        int w2 = w1;
        
        if (w2 == 1) {
            // Hot during warm-up
            persistent_state = (persistent_state + i) % 100;
        }
    }
    
    // Main processing with mode-dependent PHI
    int main_phi = 0;
    if (mode == 1) {
        main_phi = 1;
    } else if (mode == 2) {
        main_phi = persistent_state > 50 ? 1 : 0;
    } else {
        main_phi = 0;
    }
    
    // Long chain of assignments
    int m1 = main_phi;
    int m2 = m1;
    int m3 = m2 + 0;
    int m4 = m3;
    int final_cond = m4;
    
    // Multiple conditionals using the PHI-derived value
    for (int i = 0; i < size * 100; i++) {
        // Create PHI that depends on loop iteration
        int loop_phi = (i == 0) ? final_cond : (checksum % 2);
        
        // Assignment chain inside loop
        int l1 = loop_phi;
        int l2 = l1;
        
        if (l2) {
            // Hot path when mode=1
            checksum += i * 3 + persistent_state;
        } else {
            // Cold path
            checksum += i;
        }
        
        // Nested conditional with another PHI
        if (i % 100 == 0) {
            int nested_phi = (checksum % 1000 > 500) ? 1 : 0;
            int n1 = nested_phi;
            
            if (n1 == 1) {
                checksum += 777;
            }
        }
    }
    
    return checksum;
}

/* Function 4: Array processing with data-dependent PHI nodes */
unsigned long long array_phi_pattern(int* data, int size, int threshold) {
    unsigned long long checksum = 0;
    int prev = 0;
    
    for (int i = 0; i < size; i++) {
        // Complex PHI: depends on comparison result
        int cmp_result = data[i] > threshold;
        
        // SSA chain
        int c1 = cmp_result;
        int c2 = c1;
        int use_var = c2;
        
        // PHI that merges values from different sources
        int merge_phi = (i == 0) ? use_var : (prev ^ use_var);
        
        // More assignments
        int m1 = merge_phi;
        int m2 = m1;
        
        // Conditional using PHI-derived value
        if (m2 == 1) {
            data[i] = data[i] * 2;
            checksum += data[i];
        } else {
            data[i] = data[i] / 2;
            checksum -= data[i];
        }
        
        prev = data[i] % 2;
        
        // Another PHI pattern in nested loop
        for (int j = 0; j < 5; j++) {
            int inner_phi = (j == 0) ? m2 : (j % 2);
            int i1 = inner_phi;
            
            if (i1) {
                checksum += j * 100;
            }
        }
    }
    
    return checksum;
}

int main(int argc, char** argv) {
    if (argc < 2) {
        printf("Usage: %s <mode> [size]\n", argv[0]);
        printf("  mode 1: Hot path dominant (run for profiling)\n");
        printf("  mode 2: Cold path testing\n");
        return 1;
    }
    
    int mode = atoi(argv[1]);
    int size = (argc > 2) ? atoi(argv[2]) : 1000;
    
    if (size <= 0) size = 1000;
    
    printf("Starting AutoFDO test with mode=%d, size=%d\n", mode, size);
    
    clock_t start = clock();
    
    // Execute different patterns based on mode
    unsigned long long checksum1 = process_hot_data(mode, size);
    printf("Phase 1 checksum: %llu\n", checksum1);
    
    unsigned long long checksum2 = process_cold_data(mode, size / 10);
    printf("Phase 2 checksum: %llu\n", checksum2);
    
    unsigned long long checksum3 = mixed_processing(mode, size);
    printf("Phase 3 checksum: %llu\n", checksum3);
    
    // Array processing with data-dependent PHI
    int* array = (int*)malloc(size * sizeof(int));
    for (int i = 0; i < size; i++) {
        array[i] = (i * 17) % 1000;
    }
    
    unsigned long long checksum4 = array_phi_pattern(array, size, 500);
    printf("Phase 4 checksum: %llu\n", checksum4);
    
    free(array);
    
    unsigned long long final_checksum = checksum1 ^ checksum2 ^ checksum3 ^ checksum4;
    printf("Final checksum: %llu\n", final_checksum);
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Execution time: %.2f seconds\n", elapsed);
    
    return 0;
}
