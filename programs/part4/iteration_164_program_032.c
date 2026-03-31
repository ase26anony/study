/* autofdo_phi_conditional_test.c
 * 
 * This program generates specific control flow patterns to trigger GCC's
 * AutoFDO PHI-to-conditional analysis in auto-profile.cc lines 1312-1333.
 * The patterns include:
 * 1. Boolean values (0/1) flowing through PHI nodes into conditional comparisons
 * 2. Chains of SSA assignments between PHI and conditional
 * 3. Hot and cold paths with varying execution frequencies
 * 4. Complex nested control flow for profile annotation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HOT_ITERATIONS 1000000
#define WARM_ITERATIONS 100000
#define COLD_ITERATIONS 100

/* Function 1: Simple PHI-to-conditional with direct assignment chain */
int phi_conditional_direct(int mode, int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        int pred_value;
        
        /* Create branching that leads to PHI node */
        if (i % 3 == 0) {
            pred_value = 1;  /* Hot path value */
        } else if (i % 3 == 1) {
            pred_value = 0;  /* Warm path value */
        } else {
            pred_value = (mode > 0) ? 1 : 0;  /* Mode-dependent */
        }
        
        /* PHI node would be created here after SSA */
        int phi_result = pred_value;
        
        /* Chain of single assignments to trigger while loop in auto-profile.cc */
        int tmp1 = phi_result;
        int tmp2 = tmp1;
        int cmp_var = tmp2;
        int final_cmp = cmp_var + 0;  /* Arithmetic that preserves value */
        
        /* Conditional using PHI-derived value - triggers uncovered code */
        if (final_cmp) {
            /* Hot path when final_cmp == 1 */
            sum += i * 2;
        } else {
            /* Cold path when final_cmp == 0 */
            sum += i;
        }
    }
    
    return sum;
}

/* Function 2: Nested PHI patterns with complex assignment chains */
int phi_conditional_nested(int mode, int iterations) {
    int sum = 0;
    int outer_loop = iterations / 100;
    
    for (int j = 0; j < outer_loop; j++) {
        int inner_pred;
        
        /* Outer branching affecting inner loop */
        if (j % 10 == 0) {
            inner_pred = 1;
        } else if (j % 10 == 5) {
            inner_pred = 0;
        } else {
            inner_pred = (mode % 2) ? 1 : 0;
        }
        
        /* Multiple assignment chain */
        int chain1 = inner_pred;
        int chain2 = chain1;
        int chain3 = chain2;
        
        for (int i = 0; i < 100; i++) {
            int inner_value;
            
            /* Inner branching creates another PHI */
            if (i < 50) {
                inner_value = chain3;
            } else {
                inner_value = (chain3 == 0) ? 1 : 0;
            }
            
            /* More assignment chains */
            int tmp_a = inner_value;
            int tmp_b = tmp_a;
            int tmp_c = tmp_b;
            
            /* Multiple comparison types */
            if (tmp_c == 1) {
                sum += i * j + 1;
            } else if (tmp_c != 0) {
                sum += i * j - 1;
            } else {
                sum += i * j;
            }
            
            /* Another conditional with different comparison */
            int check_var = tmp_c;
            while (check_var && i < 75) {
                sum += check_var;
                check_var = 0;  /* Break after one iteration */
            }
        }
    }
    
    return sum;
}

/* Function 3: Array-based PHI patterns with varying trip counts */
int phi_conditional_array(int mode, int size) {
    int* data = (int*)malloc(size * sizeof(int));
    int sum = 0;
    
    /* Initialize with pattern */
    for (int i = 0; i < size; i++) {
        data[i] = i % 10;
    }
    
    /* Process array with PHI-dependent conditions */
    for (int i = 0; i < size; i++) {
        int selector;
        
        /* Create PHI pattern based on array values */
        if (data[i] < 3) {
            selector = 1;
        } else if (data[i] < 6) {
            selector = 0;
        } else {
            selector = (mode > 1) ? 1 : 0;
        }
        
        /* Assignment chain */
        int val1 = selector;
        int val2 = val1;
        int val3 = val2;
        int val4 = val3 + 0;  /* Preserve through arithmetic */
        
        /* Conditional with explicit comparison */
        if (val4 == 1) {
            /* Hot path for selector == 1 */
            sum += data[i] * 3;
            
            /* Nested condition with another PHI */
            int nested_sel = (i % 2 == 0) ? val4 : 0;
            int nested_copy = nested_sel;
            
            if (nested_copy) {
                sum += 100;
            }
        } else {
            /* Cold path */
            sum += data[i];
            
            /* Different comparison type */
            if (val4 != 1) {
                sum -= 50;
            }
        }
        
        /* Loop condition using PHI-derived value */
        int loop_var = val4;
        int k = 0;
        while (loop_var && k < 5) {
            sum += k;
            k++;
            loop_var = (k < 3) ? loop_var : 0;
        }
    }
    
    free(data);
    return sum;
}

/* Function 4: Recursive PHI patterns with call site variations */
int phi_conditional_recursive(int depth, int mode, int* call_count) {
    if (depth <= 0) {
        return 0;
    }
    
    (*call_count)++;
    
    int local_selector;
    
    /* PHI-like selection based on depth and mode */
    if (depth % 3 == 0) {
        local_selector = 1;
    } else if (depth % 3 == 1) {
        local_selector = 0;
    } else {
        local_selector = (mode % 3 == 0) ? 1 : 0;
    }
    
    /* Multi-step assignment chain */
    int step1 = local_selector;
    int step2 = step1;
    int step3 = step2;
    int step4 = step3;
    int final_val = step4;
    
    int result = 0;
    
    /* Conditional using the PHI-derived value */
    if (final_val) {
        /* Hot recursive path */
        result = depth * 10;
        result += phi_conditional_recursive(depth - 1, mode, call_count);
        result += phi_conditional_recursive(depth - 2, mode, call_count);
    } else {
        /* Cold recursive path */
        result = depth * 5;
        result += phi_conditional_recursive(depth - 1, mode, call_count);
    }
    
    /* Another conditional with different comparison */
    int check = final_val;
    if (check == 1) {
        result += 1000;
    }
    
    return result;
}

/* Function 5: Mixed patterns for comprehensive coverage */
int phi_conditional_mixed(int mode, int iterations) {
    int sum = 0;
    
    for (int phase = 0; phase < 3; phase++) {
        int phase_selector;
        
        /* Phase-dependent PHI */
        switch (phase) {
            case 0: phase_selector = (mode == 1) ? 1 : 0; break;
            case 1: phase_selector = 1; break;  /* Always hot in phase 1 */
            case 2: phase_selector = 0; break;  /* Always cold in phase 2 */
        }
        
        /* Complex assignment network */
        int net1 = phase_selector;
        int net2 = net1;
        int net3 = net2;
        int net4 = net3 + 0;
        int net5 = net4;
        
        int phase_iterations = (phase == 0) ? iterations : iterations / 10;
        
        for (int i = 0; i < phase_iterations; i++) {
            int inner_selector;
            
            /* Inner PHI based on multiple conditions */
            if (i % 100 == 0) {
                inner_selector = net5;
            } else if (i % 100 == 99) {
                inner_selector = (net5 == 0) ? 1 : 0;
            } else {
                inner_selector = (i % 2 == 0) ? net5 : !net5;
            }
            
            /* Assignment chain */
            int a = inner_selector;
            int b = a;
            int c = b;
            int d = c;
            
            /* Multiple conditional forms */
            if (d) {
                sum += i * phase * 3;
                
                /* Nested with another PHI */
                int nested = (sum % 2 == 0) ? d : 0;
                if (nested == 1) {
                    sum += 777;
                }
            } else {
                sum += i * phase;
                
                if (d != 1) {
                    sum -= 111;
                }
            }
            
            /* Loop condition with PHI value */
            int loop_ctrl = d;
            int counter = 0;
            do {
                if (loop_ctrl) {
                    sum += counter * 7;
                }
                counter++;
                loop_ctrl = (counter < 3) ? loop_ctrl : 0;
            } while (loop_ctrl);
        }
    }
    
    return sum;
}

/* Main function with different execution modes */
int main(int argc, char** argv) {
    int mode = 1;  /* Default to hot mode */
    int total_iterations = HOT_ITERATIONS;
    
    /* Parse command line arguments */
    if (argc > 1) {
        mode = atoi(argv[1]);
        if (argc > 2) {
            total_iterations = atoi(argv[2]);
        }
    }
    
    printf("Running AutoFDO PHI-Conditional Test - Mode %d, Iterations %d\n", 
           mode, total_iterations);
    
    int total_sum = 0;
    clock_t start_time = clock();
    
    /* Warm-up phase with moderate iterations */
    printf("Phase 1: Warm-up...\n");
    total_sum += phi_conditional_direct(mode, WARM_ITERATIONS);
    
    /* Main execution phase with pattern variations */
    printf("Phase 2: Main execution...\n");
    
    /* Execute different patterns based on mode */
    switch (mode) {
        case 1:  /* Hot mode - maximize hot path execution */
            total_sum += phi_conditional_direct(mode, total_iterations);
            total_sum += phi_conditional_nested(mode, total_iterations / 10);
            total_sum += phi_conditional_array(mode, total_iterations / 100);
            break;
            
        case 2:  /* Balanced mode */
            total_sum += phi_conditional_direct(mode, total_iterations / 2);
            total_sum += phi_conditional_nested(mode, total_iterations / 20);
            total_sum += phi_conditional_mixed(mode, total_iterations / 5);
            break;
            
        case 3:  /* Recursive-heavy mode */
            {
                int call_count = 0;
                total_sum += phi_conditional_recursive(15, mode, &call_count);
                printf("Recursive calls: %d\n", call_count);
            }
            total_sum += phi_conditional_array(mode, total_iterations / 50);
            break;
            
        case 4:  /* Cold mode - maximize cold path execution */
            total_sum += phi_conditional_direct(0, total_iterations);  /* Force cold */
            total_sum += phi_conditional_mixed(0, total_iterations / 2);
            break;
            
        default:  /* Mixed mode */
            total_sum += phi_conditional_direct(mode, total_iterations);
            total_sum += phi_conditional_nested(mode, total_iterations / 10);
            total_sum += phi_conditional_array(mode, total_iterations / 100);
            total_sum += phi_conditional_mixed(mode, total_iterations / 5);
            break;
    }
    
    /* Final phase with verification */
    printf("Phase 3: Verification...\n");
    total_sum += phi_conditional_direct(mode, COLD_ITERATIONS);
    
    clock_t end_time = clock();
    double elapsed = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    
    printf("Total checksum: %d\n", total_sum);
    printf("Execution time: %.3f seconds\n", elapsed);
    printf("Mode %d completed successfully.\n", mode);
    
    return 0;
}
