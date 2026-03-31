/* autofdo_phi_test.c
 * Test program to trigger GCC AutoFDO PHI-to-conditional analysis
 * Compile with: gcc -O2 -fauto-profile autofdo_phi_test.c -o autofdo_phi_test
 * Run with: ./autofdo_phi_test <mode> <iterations>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define HOT_ITERATIONS 10000000
#define WARM_ITERATIONS 1000000
#define COLD_ITERATIONS 1000

/* Global checksum for preventing optimization */
volatile uint64_t global_checksum = 0;

/* Function 1: Simple PHI-to-conditional with copy chain */
uint64_t phi_chain_pattern(int mode, int iterations) {
    uint64_t sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        int value1 = 0;
        int value2 = 0;
        
        /* Create branching that leads to PHI node */
        if (mode == 1) {
            /* Hot path - executed frequently */
            value1 = 1;
            value2 = 1;
        } else if (mode == 2) {
            /* Warm path */
            value1 = 1;
            value2 = 0;
        } else {
            /* Cold path */
            value1 = 0;
            value2 = 0;
        }
        
        /* PHI node equivalent - value depends on incoming edge */
        int phi_result;
        if (i % 2 == 0) {
            phi_result = value1;
        } else {
            phi_result = value2;
        }
        
        /* Chain of SSA copies to trigger while loop in auto-profile.cc */
        int tmp1 = phi_result;
        int tmp2 = tmp1;
        int tmp3 = tmp2 + 0;  /* Arithmetic that doesn't break pattern */
        int cmp_var = tmp3;
        
        /* Multiple comparison types using PHI-derived value */
        if (cmp_var) {  /* Direct use in if condition */
            sum += i * 2;
        }
        
        if (cmp_var == 1) {  /* Explicit equality comparison */
            sum += i * 3;
        }
        
        if (cmp_var != 0) {  /* Inequality comparison */
            sum += i * 5;
        }
        
        /* Use in loop condition */
        int loop_ctrl = cmp_var;
        int j = 0;
        while (loop_ctrl && j < 10) {
            sum += j;
            j++;
            loop_ctrl = (j < 5) ? cmp_var : 0;  /* Another PHI-like pattern */
        }
    }
    
    return sum;
}

/* Function 2: Nested loops with complex PHI patterns */
uint64_t nested_phi_pattern(int mode, int iterations) {
    uint64_t sum = 0;
    int outer_limit = (mode == 1) ? 100 : 10;
    
    for (int outer = 0; outer < outer_limit; outer++) {
        int inner_control = 0;
        
        /* Different assignments in different paths */
        if (outer % 3 == 0) {
            inner_control = 1;  /* Hot path assignment */
        } else if (outer % 3 == 1) {
            inner_control = (mode == 1) ? 1 : 0;  /* Mode-dependent */
        } else {
            inner_control = 0;  /* Cold path */
        }
        
        /* PHI node from loop carried dependency */
        int phi_val = inner_control;
        for (int inner = 0; inner < iterations / 1000; inner++) {
            /* Update phi_val creating loop-carried dependency */
            int old_phi = phi_val;
            
            /* Complex SSA copy chain */
            int chain1 = old_phi;
            int chain2 = chain1;
            int chain3 = chain2;
            int final_val = chain3;
            
            /* Multiple conditional uses */
            if (final_val) {
                sum += outer * inner * 7;
            }
            
            if (final_val == 1 && outer > 5) {
                sum += inner * 11;
            }
            
            /* Prepare for next iteration - creates PHI at loop header */
            phi_val = (inner % 2 == 0) ? 1 : 0;
        }
    }
    
    return sum;
}

/* Function 3: Array processing with data-dependent PHI */
uint64_t array_based_phi(int* data, int size, int threshold) {
    uint64_t sum = 0;
    int prev_was_hot = 0;
    
    for (int i = 0; i < size; i++) {
        int current_hot = (data[i] > threshold) ? 1 : 0;
        
        /* PHI node: depends on previous iteration */
        int phi_result;
        if (i == 0) {
            phi_result = 0;
        } else {
            phi_result = prev_was_hot;
        }
        
        /* SSA copy chain */
        int tmp_a = phi_result;
        int tmp_b = tmp_a;
        int tmp_c = tmp_b + 0;
        int use_val = tmp_c;
        
        /* Conditional based on PHI-derived value */
        if (use_val) {
            sum += data[i] * 2;
        } else {
            sum += data[i];
        }
        
        if (use_val == 1 && current_hot) {
            sum += 1000;
        }
        
        /* Update for next iteration */
        prev_was_hot = current_hot;
    }
    
    return sum;
}

/* Function 4: Switch-based PHI pattern */
uint64_t switch_phi_pattern(int mode, int iterations) {
    uint64_t sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        int case_val = i % 4;
        int phi_base = 0;
        
        switch (case_val) {
            case 0:  /* Hot case in mode 1 */
                phi_base = (mode == 1) ? 1 : 0;
                break;
            case 1:  /* Always hot */
                phi_base = 1;
                break;
            case 2:  /* Mode-dependent */
                phi_base = (mode == 2) ? 1 : 0;
                break;
            case 3:  /* Always cold */
                phi_base = 0;
                break;
        }
        
        /* Multiple PHI-like assignments */
        int phi1 = phi_base;
        int phi2 = (i % 3 == 0) ? phi1 : 0;
        
        /* Extended SSA chain */
        int chain_start = phi2;
        int mid1 = chain_start;
        int mid2 = mid1;
        int mid3 = mid2;
        int final_cond = mid3;
        
        /* Various conditional uses */
        if (final_cond) {
            sum += i * 13;
        }
        
        if (final_cond == 1 && case_val < 2) {
            sum += i * 17;
        }
        
        /* Nested condition with PHI */
        int nested_ctrl = final_cond;
        for (int j = 0; j < 5; j++) {
            if (nested_ctrl) {
                sum += j * 19;
            }
            nested_ctrl = (j % 2 == 0) ? final_cond : 0;
        }
    }
    
    return sum;
}

/* Function 5: Recursive PHI pattern */
uint64_t recursive_phi_helper(int depth, int max_depth, int hot_path) {
    if (depth >= max_depth) {
        return 1;
    }
    
    /* PHI-like value based on recursion path */
    int current_val;
    if (depth == 0) {
        current_val = hot_path;
    } else {
        /* This creates a PHI in the recursive return */
        current_val = (depth % 2 == 0) ? 1 : 0;
    }
    
    /* SSA copies */
    int copy1 = current_val;
    int copy2 = copy1;
    int use_val = copy2;
    
    uint64_t sum = 0;
    if (use_val) {
        sum += depth * 23;
    }
    
    /* Recursive calls create complex control flow */
    sum += recursive_phi_helper(depth + 1, max_depth, hot_path);
    
    if (use_val == 1 && depth > 1) {
        sum += depth * 29;
    }
    
    return sum;
}

uint64_t recursive_phi_pattern(int mode, int iterations) {
    uint64_t sum = 0;
    int hot_path = (mode == 1) ? 1 : 0;
    
    for (int i = 0; i < iterations / 100; i++) {
        sum += recursive_phi_helper(0, 10, hot_path);
    }
    
    return sum;
}

/* Main function with different execution modes */
int main(int argc, char** argv) {
    int mode = 1;  /* Default to hot mode */
    int iterations = HOT_ITERATIONS;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    printf("Running AutoFDO PHI test with mode=%d, iterations=%d\n", 
           mode, iterations);
    
    uint64_t total_sum = 0;
    clock_t start = clock();
    
    /* Phase 1: Warm-up with simple pattern */
    total_sum += phi_chain_pattern(mode, iterations / 10);
    
    /* Phase 2: Main computation with nested patterns */
    total_sum += nested_phi_pattern(mode, iterations);
    
    /* Phase 3: Array-based pattern */
    int array_size = 10000;
    int* data = malloc(array_size * sizeof(int));
    for (int i = 0; i < array_size; i++) {
        data[i] = (mode == 1) ? (i % 100) : (i % 10);
    }
    total_sum += array_based_phi(data, array_size, 50);
    free(data);
    
    /* Phase 4: Switch-based pattern */
    total_sum += switch_phi_pattern(mode, iterations / 2);
    
    /* Phase 5: Recursive pattern */
    total_sum += recursive_phi_pattern(mode, iterations / 5);
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    /* Store in global to prevent dead code elimination */
    global_checksum = total_sum;
    
    printf("Checksum: %lu\n", total_sum);
    printf("Elapsed time: %.2f seconds\n", elapsed);
    printf("Mode %d completed successfully\n", mode);
    
    return 0;
}
