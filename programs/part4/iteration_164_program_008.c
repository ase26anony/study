/* autofdo_phi_conditional_test.c
 * 
 * This program generates specific control flow patterns to trigger
 * GCC's AutoFDO profile analysis for PHI-to-conditional propagation.
 * 
 * Compilation and usage:
 * 1. First compilation (with existing profile or empty):
 *    gcc -O2 -fauto-profile autofdo_phi_conditional_test.c -o test_program
 *    
 * 2. Run with dominant mode to generate profile data:
 *    ./test_program 1 > /dev/null
 *    
 * 3. Recompile with collected profile:
 *    gcc -O2 -fauto-profile -Wauto-profile autofdo_phi_conditional_test.c -o test_program_opt
 *    
 * 4. For debugging: Add -fdump-tree-afdo -fdump-tree-afdo-details
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HOT_ITERATIONS 1000000
#define WARM_ITERATIONS 10000
#define COLD_ITERATIONS 10

/* Function 1: Complex PHI pattern with SSA copy chains */
int process_with_phi_chains(int mode, int iterations) {
    int result = 0;
    int i, j;
    
    for (i = 0; i < iterations; i++) {
        int phi_val;
        int tmp1, tmp2, tmp3, cmp_var;
        
        /* Create branching that feeds into PHI node */
        if (mode == 1) {
            /* Hot path - executed frequently */
            phi_val = 1;  /* Will be PHI input from this edge */
        } else if (mode == 2) {
            /* Medium path */
            phi_val = (i % 100 == 0) ? 1 : 0;
        } else {
            /* Cold path */
            phi_val = 0;  /* Will be PHI input from this edge */
        }
        
        /* PHI node is conceptually here (created by compiler) */
        int phi_result = phi_val;
        
        /* Create SSA copy chain to trigger the while loop walking back */
        tmp1 = phi_result;      /* First assignment */
        tmp2 = tmp1;            /* Second assignment */
        tmp3 = tmp2 + 0;        /* Arithmetic that preserves value */
        cmp_var = tmp3;         /* Final copy before comparison */
        
        /* Multiple comparison types using the PHI-derived value */
        
        /* Type 1: Direct use in if condition */
        if (cmp_var) {  /* if (phi_derived) */
            result += i * 2;  /* Hot computation */
        } else {
            result -= i;      /* Cold computation */
        }
        
        /* Type 2: Explicit equality comparison */
        if (cmp_var == 1) {  /* if (phi_derived == 1) */
            for (j = 0; j < 10; j++) {
                result += (i * j) % 7;
            }
        }
        
        /* Type 3: Not-equal comparison */
        if (cmp_var != 0) {  /* if (phi_derived != 0) */
            result ^= (i << 3);
        }
        
        /* Nested conditional with PHI-derived value */
        int nested_tmp = cmp_var;
        if (nested_tmp) {
            if (i % 3 == 0) {
                result += 7;
            }
        }
    }
    
    return result;
}

/* Function 2: Loop condition with PHI-derived value */
int process_with_phi_in_loop(int mode, int iterations) {
    int result = 0;
    int outer_iter = (mode == 1) ? iterations : iterations / 100;
    
    for (int i = 0; i < outer_iter; i++) {
        int loop_control;
        
        /* Different predecessors set different values */
        if (i < outer_iter / 2) {
            loop_control = 1;  /* PHI input from first half */
        } else {
            loop_control = (mode == 1) ? 1 : 0;  /* PHI input from second half */
        }
        
        /* PHI node here */
        int phi_control = loop_control;
        
        /* SSA copy chain */
        int chain1 = phi_control;
        int chain2 = chain1;
        int chain3 = chain2;
        int final_control = chain3;
        
        /* Use PHI-derived value as loop condition */
        int counter = 0;
        while (final_control && counter < 5) {  /* while (phi_derived) */
            result += (i * counter) % 11;
            counter++;
            
            /* Modify control to eventually exit */
            if (counter == 3) {
                final_control = 0;  /* Break the loop */
            }
        }
        
        /* Another conditional with copy chain */
        int tmp_a = phi_control;
        int tmp_b = tmp_a;
        if (tmp_b == 1) {
            result ^= i;
        }
    }
    
    return result;
}

/* Function 3: Complex nested PHI patterns */
int complex_phi_patterns(int mode, int size) {
    int result = 0;
    int* data = malloc(size * sizeof(int));
    
    if (!data) return 0;
    
    /* Initialize array with pattern */
    for (int i = 0; i < size; i++) {
        data[i] = i % 100;
    }
    
    /* Process array with PHI-dependent branching */
    for (int i = 0; i < size; i++) {
        int selector;
        
        /* Complex predecessor conditions */
        if (mode == 1) {
            selector = (data[i] > 50) ? 1 : 0;
        } else if (mode == 2) {
            selector = (data[i] % 3 == 0) ? 1 : 0;
        } else {
            selector = 0;
        }
        
        /* PHI node */
        int phi_sel = selector;
        
        /* Multiple SSA copies */
        int copy1 = phi_sel;
        int copy2 = copy1;
        int copy3 = copy2 + 0;  /* Preserve through arithmetic */
        int copy4 = copy3;
        
        /* Multiple conditional uses */
        if (copy4) {
            result += data[i] * 2;
            
            /* Nested conditional with another copy chain */
            int nested_copy = copy4;
            if (nested_copy == 1) {
                result -= data[i] / 2;
            }
        } else {
            result += data[i];
        }
        
        /* Another independent PHI pattern */
        int alt_selector;
        if (i % 7 == 0) {
            alt_selector = 1;
        } else {
            alt_selector = (mode == 1) ? 1 : 0;
        }
        
        int phi_alt = alt_selector;
        int chain_a = phi_alt;
        int chain_b = chain_a;
        
        if (chain_b != 0) {
            result ^= data[i];
        }
    }
    
    free(data);
    return result;
}

/* Function 4: Mixed hot/cold paths with PHI propagation */
int mixed_path_phi(int mode, int iterations) {
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        int hot_path = (mode == 1) ? 1 : 0;
        
        /* Function call to create varying profile at call sites */
        if (hot_path) {
            result += helper_hot(i);
        } else {
            result += helper_cold(i);
        }
        
        /* PHI pattern after call */
        int post_call_val;
        if (result > 1000) {
            post_call_val = 1;
        } else {
            post_call_val = hot_path;
        }
        
        int phi_post = post_call_val;
        int tmp_x = phi_post;
        int tmp_y = tmp_x;
        
        if (tmp_y) {
            result += i % 19;
        }
    }
    
    return result;
}

/* Helper functions to create varying call site profiles */
int helper_hot(int x) {
    int r = 0;
    for (int i = 0; i < 10; i++) {
        r += (x + i) % 13;
    }
    return r;
}

int helper_cold(int x) {
    return x % 17;
}

/* Main function with different execution modes */
int main(int argc, char** argv) {
    int mode = 1;  /* Default to hot mode */
    int total_result = 0;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    printf("Running mode %d\n", mode);
    
    /* Warm-up phase */
    clock_t start = clock();
    
    /* Execute different patterns based on mode */
    switch (mode) {
        case 1:  /* Hot mode - dominant execution */
            total_result += process_with_phi_chains(1, HOT_ITERATIONS);
            total_result += process_with_phi_in_loop(1, HOT_ITERATIONS / 10);
            total_result += complex_phi_patterns(1, 10000);
            total_result += mixed_path_phi(1, HOT_ITERATIONS / 5);
            break;
            
        case 2:  /* Medium mode - mixed profile */
            total_result += process_with_phi_chains(2, WARM_ITERATIONS);
            total_result += process_with_phi_in_loop(2, WARM_ITERATIONS / 10);
            total_result += complex_phi_patterns(2, 1000);
            total_result += mixed_path_phi(2, WARM_ITERATIONS / 5);
            break;
            
        case 3:  /* Cold mode - rare paths */
            total_result += process_with_phi_chains(3, COLD_ITERATIONS);
            total_result += process_with_phi_in_loop(3, COLD_ITERATIONS);
            total_result += complex_phi_patterns(3, 100);
            total_result += mixed_path_phi(3, COLD_ITERATIONS);
            break;
            
        default:
            /* Mixed execution to create complex profile */
            total_result += process_with_phi_chains(1, HOT_ITERATIONS / 2);
            total_result += process_with_phi_chains(2, WARM_ITERATIONS);
            total_result += process_with_phi_chains(3, COLD_ITERATIONS);
            break;
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Result: %d\n", total_result);
    printf("Time: %.3f seconds\n", elapsed);
    
    return 0;
}
