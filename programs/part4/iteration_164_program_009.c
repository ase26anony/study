/* autofdo_phi_patterns.c
 * 
 * This program generates specific control flow patterns to trigger
 * GCC's AutoFDO profile analysis for PHI-to-conditional propagation.
 * It creates boolean values (0/1) flowing through PHI nodes into
 * conditional comparisons with SSA copy chains in between.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ========== Pattern 1: Simple PHI-to-Condition with Copy Chain ========== */

int pattern1_phi_to_cond(int mode, int iterations) {
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        int value_from_phi;
        int tmp1, tmp2, tmp3;
        
        /* Create two different paths that set values 0 or 1 */
        if (i % 100 < 95) {  /* Hot path - 95% of iterations */
            value_from_phi = 1;  /* Set to 1 in hot path */
        } else {  /* Cold path - 5% of iterations */
            value_from_phi = 0;  /* Set to 0 in cold path */
        }
        
        /* PHI node would be created here by compiler for value_from_phi */
        
        /* Create SSA copy chain to trigger the while loop walking back */
        tmp1 = value_from_phi;      /* First copy */
        tmp2 = tmp1;                /* Second copy */
        tmp3 = tmp2 + 0;            /* Third copy with arithmetic that doesn't change value */
        
        /* Conditional using the PHI-derived value through copy chain */
        if (tmp3) {  /* Direct use in if condition - should trigger uncovered logic */
            result += i * 2;  /* Hot computation */
        } else {
            result += i / 2;  /* Cold computation */
        }
        
        /* Another conditional with explicit comparison */
        if (tmp3 == 1) {  /* Explicit equality comparison */
            result += 3;
        }
        
        /* Yet another with != 0 */
        if (tmp3 != 0) {
            result += 5;
        }
    }
    
    return result;
}

/* ========== Pattern 2: Nested PHI Patterns with Complex Control Flow ========== */

int pattern2_nested_phis(int mode, int size) {
    int sum = 0;
    int* data = (int*)malloc(size * sizeof(int));
    
    if (!data) return 0;
    
    /* Initialize with pattern */
    for (int i = 0; i < size; i++) {
        data[i] = (i * 7) % 13;
    }
    
    /* Outer loop with PHI-based condition */
    for (int i = 0; i < size; i++) {
        int outer_flag;
        
        /* Set flag differently based on mode */
        if (mode == 1) {
            outer_flag = (i < size * 9 / 10) ? 1 : 0;  /* 90% hot */
        } else {
            outer_flag = (i < size / 10) ? 1 : 0;      /* 10% hot */
        }
        
        /* Copy chain for outer flag */
        int chain1 = outer_flag;
        int chain2 = chain1;
        int chain3 = chain2;
        
        /* Inner loop controlled by PHI-derived value */
        while (chain3) {  /* Use in loop condition */
            int inner_flag;
            
            /* Another PHI inside the loop */
            if (data[i] > 5) {
                inner_flag = 1;
            } else {
                inner_flag = 0;
            }
            
            /* Copy chain for inner flag */
            int inner_chain1 = inner_flag;
            int inner_chain2 = inner_chain1;
            
            /* Conditional using inner PHI-derived value */
            if (inner_chain2 == 1) {  /* Explicit comparison */
                sum += data[i] * 3;
                break;  /* Exit inner loop */
            } else {
                sum += 1;
                data[i]--;
            }
            
            /* Update outer chain to eventually exit */
            chain3 = 0;  /* Will cause PHI update on next iteration */
        }
        
        /* Another conditional after loop */
        if (chain1) {
            sum += i;
        }
    }
    
    free(data);
    return sum;
}

/* ========== Pattern 3: Multiple Predecessor PHI with Arithmetic ========== */

int pattern3_multi_pred_phi(int iterations) {
    int total = 0;
    int a = 0, b = 0;
    
    for (int i = 0; i < iterations; i++) {
        int selector;
        int phi_result;
        
        /* Complex branching to create multiple predecessors */
        if (i % 3 == 0) {
            a++;
            selector = 1;
        } else if (i % 3 == 1) {
            b++;
            selector = 0;
        } else {
            a += b;
            selector = (a > b) ? 1 : 0;
        }
        
        /* This creates a PHI with 3 different predecessors */
        phi_result = selector;
        
        /* Long copy chain */
        int c1 = phi_result;
        int c2 = c1;
        int c3 = c2 + 0;  /* Arithmetic that preserves value */
        int c4 = c3;
        int c5 = c4;
        
        /* Multiple conditionals using the chain */
        if (c5) {
            total += a * 2;
        }
        
        if (c3 == 1) {
            total += b * 3;
        }
        
        /* Nested conditional */
        if (c2) {
            if (c4 != 0) {
                total += 7;
            }
        }
    }
    
    return total;
}

/* ========== Pattern 4: Function Calls with PHI Propagation ========== */

int helper_hot_path(int x) {
    return x * x + 1;
}

int helper_cold_path(int x) {
    return x / 2;
}

int pattern4_function_calls(int mode, int limit) {
    int accumulator = 0;
    int use_hot_path;
    
    for (int i = 0; i < limit; i++) {
        /* PHI-like selection based on mode and iteration */
        if (mode == 1) {
            use_hot_path = (i % 100 < 98) ? 1 : 0;  /* 98% hot */
        } else {
            use_hot_path = (i % 100 < 2) ? 1 : 0;   /* 2% hot */
        }
        
        /* Copy chain */
        int chain_a = use_hot_path;
        int chain_b = chain_a;
        int chain_c = chain_b;
        
        /* Call different functions based on PHI-derived value */
        if (chain_c) {
            accumulator += helper_hot_path(i);
        } else {
            accumulator += helper_cold_path(i);
        }
        
        /* Another use in switch-like construct */
        if (chain_b == 1) {
            accumulator += 1000;
        }
    }
    
    return accumulator;
}

/* ========== Pattern 5: Array Processing with PHI-based Conditions ========== */

int pattern5_array_processing(int* array, int size, int threshold) {
    int count = 0;
    int sum = 0;
    
    for (int i = 0; i < size; i++) {
        int is_above_threshold;
        
        /* Set boolean based on comparison */
        if (array[i] > threshold) {
            is_above_threshold = 1;
        } else {
            is_above_threshold = 0;
        }
        
        /* Multi-step copy chain */
        int t1 = is_above_threshold;
        int t2 = t1;
        int t3 = t2 + 0;
        int t4 = t3;
        
        /* Conditional using PHI-derived value */
        if (t4) {
            count++;
            sum += array[i];
            
            /* Nested conditional with another copy */
            int t5 = t2;
            if (t5 == 1) {
                sum += 100;
            }
        } else {
            sum -= array[i];
        }
        
        /* Loop with PHI-derived condition */
        int loop_ctrl = t1;
        int j = 0;
        while (loop_ctrl && j < 3) {
            sum += j;
            j++;
            loop_ctrl = 0;  /* Will create PHI in next iteration */
        }
    }
    
    return sum + count;
}

/* ========== Main Function with Profile-Generating Runtime ========== */

int main(int argc, char** argv) {
    int mode = 1;  /* Default to hot mode */
    int total_iterations = 1000000;
    int result = 0;
    
    /* Parse command line for mode */
    if (argc > 1) {
        mode = atoi(argv[1]);
        if (mode < 1 || mode > 3) mode = 1;
    }
    
    if (argc > 2) {
        total_iterations = atoi(argv[2]);
        if (total_iterations < 1000) total_iterations = 1000;
    }
    
    printf("Running AutoFDO PHI pattern generator - Mode %d, Iterations %d\n", 
           mode, total_iterations);
    
    clock_t start = clock();
    
    /* Phase 1: Warm-up with mixed patterns */
    printf("Phase 1: Warm-up...\n");
    for (int phase = 0; phase < 3; phase++) {
        result ^= pattern1_phi_to_cond(mode, total_iterations / 10);
        result ^= pattern3_multi_pred_phi(total_iterations / 20);
    }
    
    /* Phase 2: Main profile generation */
    printf("Phase 2: Main profile generation...\n");
    
    /* Dominant hot path for mode 1, cold for mode 2 */
    if (mode == 1) {
        /* Hot mode - execute hot paths heavily */
        result += pattern1_phi_to_cond(mode, total_iterations);
        result += pattern2_nested_phis(mode, 10000);
        result += pattern4_function_calls(mode, total_iterations / 2);
    } else if (mode == 2) {
        /* Cold mode - execute cold paths more */
        result += pattern1_phi_to_cond(mode, total_iterations / 10);
        result += pattern2_nested_phis(mode, 1000);
        result += pattern4_function_calls(mode, total_iterations / 20);
    } else {
        /* Mixed mode */
        result += pattern1_phi_to_cond(1, total_iterations / 2);
        result += pattern1_phi_to_cond(2, total_iterations / 2);
        result += pattern2_nested_phis(1, 5000);
        result += pattern2_nested_phis(2, 5000);
    }
    
    /* Phase 3: Array processing pattern */
    printf("Phase 3: Array processing...\n");
    int array_size = 10000;
    int* array = (int*)malloc(array_size * sizeof(int));
    for (int i = 0; i < array_size; i++) {
        array[i] = (i * 13) % 97;
    }
    result += pattern5_array_processing(array, array_size, 50);
    free(array);
    
    /* Phase 4: Final mixed patterns */
    printf("Phase 4: Final patterns...\n");
    result += pattern3_multi_pred_phi(total_iterations / 5);
    result += pattern4_function_calls(mode, total_iterations / 4);
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Completed in %.2f seconds\n", elapsed);
    printf("Result checksum: %d\n", result);
    
    /* Verify result consistency */
    if (mode == 1) {
        printf("Mode 1 (Hot) - Expected large result\n");
    } else if (mode == 2) {
        printf("Mode 2 (Cold) - Expected smaller result\n");
    } else {
        printf("Mode 3 (Mixed) - Expected intermediate result\n");
    }
    
    return 0;
}
