/* autofdo_phi_conditional.c
 * 
 * This program generates execution patterns specifically designed to trigger
 * GCC's AutoFDO PHI-to-conditional analysis in auto-profile.cc lines 1312-1333.
 * The patterns include:
 * 1. Boolean values (0/1) flowing through PHI nodes into conditional comparisons
 * 2. Chains of SSA assignments between PHI definitions and condition uses
 * 3. Hot loops with annotated basic blocks
 * 4. Multiple comparison types using PHI-derived values
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Global variables to prevent optimization */
volatile int global_seed = 42;
volatile int global_result = 0;

/* Function 1: Simple PHI-to-conditional with direct copy chain */
int phi_conditional_direct(int mode, int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        int value_from_phi;
        int pred_value;
        
        /* Create branching that feeds into PHI */
        if (i % 3 == 0) {
            pred_value = 1;  /* Hot path - frequently taken */
        } else {
            pred_value = 0;  /* Cold path */
        }
        
        /* PHI node implicitly created here */
        value_from_phi = pred_value;
        
        /* Chain of SSA copies to trigger the while loop in auto-profile.cc */
        int tmp1 = value_from_phi;
        int tmp2 = tmp1;
        int tmp3 = tmp2 + 0;  /* Arithmetic that doesn't break single-assignment */
        int cmp_var = tmp3;
        
        /* Conditional using PHI-derived value - multiple comparison types */
        if (cmp_var) {  /* Direct use in if condition */
            sum += i * 2;  /* Hot computation */
        }
        
        if (cmp_var == 1) {  /* Explicit equality comparison */
            sum += i % 7;
        }
        
        /* Another SSA copy chain */
        int another_copy = cmp_var;
        int final_copy = another_copy;
        
        if (final_copy != 0) {  /* Inequality comparison */
            sum += (i & 0xFF);
        }
    }
    
    return sum;
}

/* Function 2: Nested loops with complex PHI patterns */
int phi_conditional_nested(int mode, int outer_iter, int inner_iter) {
    int total = 0;
    int outer_control;
    
    /* Outer loop with varying behavior */
    for (int o = 0; o < outer_iter; o++) {
        /* Determine PHI source based on mode and iteration */
        int phi_source;
        
        if (mode == 1) {
            /* Hot mode - mostly true */
            phi_source = (o % 10 != 9) ? 1 : 0;
        } else {
            /* Cold mode - mostly false */
            phi_source = (o % 10 == 0) ? 1 : 0;
        }
        
        /* PHI node with multiple predecessors in inner loop */
        for (int i = 0; i < inner_iter; i++) {
            int inner_value;
            
            /* Complex branching for PHI input */
            if (i < inner_iter / 2) {
                inner_value = phi_source;
            } else {
                inner_value = (i % 3 == 0) ? phi_source : !phi_source;
            }
            
            /* PHI equivalent */
            int phi_result = inner_value;
            
            /* Extended SSA copy chain */
            int chain1 = phi_result;
            int chain2 = chain1;
            int chain3 = chain2;
            int chain4 = chain3 + 0;
            int chain5 = chain4;
            
            /* Multiple conditionals with the PHI-derived value */
            while (chain5) {  /* Use in loop condition */
                total += (o * i) % 100;
                chain5 = 0;  /* Single iteration */
            }
            
            if (chain4 == 1) {
                total += i * 3;
            }
            
            /* Additional computation to prevent dead code elimination */
            total += (phi_result * o) & 0xF;
        }
    }
    
    return total;
}

/* Function 3: Switch-based PHI with call site variations */
int phi_conditional_switch(int mode, int iterations) {
    int result = 0;
    int switch_control;
    
    for (int i = 0; i < iterations; i++) {
        /* Determine value based on complex conditions */
        int base_value;
        
        switch (mode) {
            case 1:  /* Hot path dominant */
                base_value = (i % 100 < 95) ? 1 : 0;
                break;
            case 2:  /* Mixed */
                base_value = (i % 2 == 0) ? 1 : 0;
                break;
            case 3:  /* Cold path dominant */
                base_value = (i % 100 < 5) ? 1 : 0;
                break;
            default:
                base_value = 0;
        }
        
        /* PHI-like selection with multiple assignments */
        int phi_var;
        if (i & 1) {
            phi_var = base_value;
        } else {
            phi_var = !base_value;
        }
        
        /* Multi-step SSA propagation */
        int step1 = phi_var;
        int step2 = step1;
        int step3 = step2 + 0;
        int step4 = step3;
        int step5 = step4;
        
        /* Conditional using the propagated value */
        if (step5) {
            result += i * i;
        }
        
        if (step4 == 1) {
            result += i % 19;
        }
        
        /* Call to external function with PHI-derived value */
        result += helper_function(step3, i);
    }
    
    return result;
}

/* Helper function to create additional call site diversity */
int helper_function(int condition, int index) {
    static int counter = 0;
    int local = 0;
    
    /* PHI pattern within helper */
    int helper_phi;
    if (condition) {
        helper_phi = index % 7;
    } else {
        helper_phi = index % 13;
    }
    
    /* SSA copy chain */
    int h1 = helper_phi;
    int h2 = h1;
    int h3 = h2;
    
    if (h3 > 0) {
        local = index * 2;
    }
    
    counter++;
    return local + (counter & 0xFF);
}

/* Function 4: Array processing with data-dependent PHI */
int phi_conditional_array(int* data, int size, int threshold) {
    int count = 0;
    int sum = 0;
    
    for (int i = 0; i < size; i++) {
        /* Data-dependent boolean */
        int is_above_threshold = (data[i] > threshold) ? 1 : 0;
        
        /* PHI with multiple potential sources */
        int selected_value;
        if (i > size / 2) {
            selected_value = is_above_threshold;
        } else {
            selected_value = (data[i] % 2 == 0) ? is_above_threshold : 0;
        }
        
        /* Long SSA copy chain */
        int v1 = selected_value;
        int v2 = v1;
        int v3 = v2 + 0;
        int v4 = v3;
        int v5 = v4;
        int v6 = v5;
        int v7 = v6;
        
        /* Multiple conditionals */
        if (v7) {
            count++;
            sum += data[i];
        }
        
        if (v6 == 1) {
            sum += i;
        }
        
        /* Nested conditional with same PHI source */
        while (v5) {
            sum += 1;
            v5 = 0;
        }
    }
    
    return sum + count;
}

/* Function 5: Recursive PHI patterns */
int phi_conditional_recursive(int depth, int current, int limit) {
    if (depth >= limit || current <= 0) {
        return current;
    }
    
    /* PHI-like value based on recursion depth */
    int recursive_phi;
    if (depth % 3 == 0) {
        recursive_phi = 1;
    } else {
        recursive_phi = (current % 2 == 0) ? 1 : 0;
    }
    
    /* SSA propagation */
    int r1 = recursive_phi;
    int r2 = r1;
    int r3 = r2;
    
    int result = 0;
    
    /* Conditional using PHI value */
    if (r3) {
        result += depth * 10;
    }
    
    if (r2 == 1) {
        result += current * 5;
    }
    
    /* Recursive calls with different PHI behaviors */
    result += phi_conditional_recursive(depth + 1, current - 1, limit);
    result += phi_conditional_recursive(depth + 1, current - 2, limit);
    
    return result;
}

/* Main function with different execution modes */
int main(int argc, char** argv) {
    int mode = 1;  /* Default to hot mode */
    int total_iterations = 1000000;
    
    /* Parse command line arguments */
    if (argc > 1) {
        mode = atoi(argv[1]);
        if (argc > 2) {
            total_iterations = atoi(argv[2]);
        }
    }
    
    printf("Running AutoFDO PHI-Conditional Test - Mode %d, Iterations %d\n", 
           mode, total_iterations);
    
    int final_result = 0;
    clock_t start = clock();
    
    /* Warm-up phase - establish baseline profile */
    for (int warm = 0; warm < 1000; warm++) {
        final_result += phi_conditional_direct(mode, 100);
    }
    
    /* Main measurement phase with different patterns */
    if (mode == 1) {
        /* Hot mode - execute hot paths heavily */
        printf("Executing HOT mode patterns...\n");
        
        /* Pattern 1: Direct PHI with many iterations */
        final_result += phi_conditional_direct(mode, total_iterations);
        
        /* Pattern 2: Nested loops */
        final_result += phi_conditional_nested(mode, 100, total_iterations / 100);
        
        /* Pattern 3: Array processing */
        int array_size = 10000;
        int* data = malloc(array_size * sizeof(int));
        for (int i = 0; i < array_size; i++) {
            data[i] = (i * 31) % 1000;
        }
        final_result += phi_conditional_array(data, array_size, 500);
        free(data);
        
    } else if (mode == 2) {
        /* Mixed mode - balanced execution */
        printf("Executing MIXED mode patterns...\n");
        
        final_result += phi_conditional_direct(mode, total_iterations / 2);
        final_result += phi_conditional_nested(mode, 50, total_iterations / 50);
        final_result += phi_conditional_switch(mode, total_iterations);
        
    } else if (mode == 3) {
        /* Cold mode - execute cold paths */
        printf("Executing COLD mode patterns...\n");
        
        final_result += phi_conditional_direct(mode, total_iterations / 10);
        final_result += phi_conditional_recursive(0, 20, 5);
        final_result += phi_conditional_switch(mode, total_iterations / 5);
        
    } else {
        /* Test all modes */
        printf("Executing ALL mode patterns...\n");
        
        for (int test_mode = 1; test_mode <= 3; test_mode++) {
            final_result += phi_conditional_direct(test_mode, total_iterations / 3);
            final_result += phi_conditional_nested(test_mode, 30, total_iterations / 30);
        }
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Final result: %d\n", final_result);
    printf("Execution time: %.3f seconds\n", elapsed);
    printf("Checksum: %u\n", (unsigned int)final_result);
    
    /* Store in global to prevent dead code elimination */
    global_result = final_result;
    
    return 0;
}
