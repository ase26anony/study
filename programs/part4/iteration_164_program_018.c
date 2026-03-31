/* autofdo_phi_condition_test.c
 * 
 * This program generates control flow patterns where boolean values (0/1)
 * flow through PHI nodes into conditional comparisons, with intermediate
 * SSA copy chains. The runtime behavior creates distinct profile patterns
 * that will be captured by AutoFDO profiling.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HOT_ITERATIONS 1000000
#define WARM_ITERATIONS 10000
#define COLD_ITERATIONS 10

/* Function 1: Simple PHI-to-conditional with direct copy chain */
int process_mode1(int mode, int iterations) {
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        int phi_val;
        int tmp1, tmp2, cmp_var;
        
        /* Create PHI node based on mode */
        if (mode == 1) {
            phi_val = 1;  /* Hot path value */
        } else {
            phi_val = 0;  /* Cold path value */
        }
        
        /* Create SSA copy chain: phi -> tmp1 -> tmp2 -> cmp_var */
        tmp1 = phi_val;
        tmp2 = tmp1;
        cmp_var = tmp2;
        
        /* Add arithmetic that doesn't break single-assignment pattern */
        cmp_var = cmp_var + 0;
        
        /* Conditional using PHI-derived value directly */
        if (cmp_var) {  /* This becomes: if (phi_val) after optimization */
            result += i * 2;  /* Hot computation */
        } else {
            result += i / 2;  /* Cold computation */
        }
        
        /* Another conditional with explicit comparison */
        int tmp3 = cmp_var;
        if (tmp3 == 1) {
            result ^= i;
        }
    }
    
    return result;
}

/* Function 2: Nested PHI patterns with multiple predecessors */
int process_mode2(int mode, int iterations) {
    int result = 0;
    int outer_flag = (mode > 0);
    
    for (int i = 0; i < iterations; i++) {
        int phi_val1, phi_val2;
        int chain1, chain2, final_cmp;
        
        /* Complex PHI pattern with two levels */
        if (i % 100 == 0) {
            phi_val1 = outer_flag ? 1 : 0;
        } else {
            phi_val1 = (mode % 2) ? 1 : 0;
        }
        
        /* Second PHI in same basic block */
        if (result > 1000000) {
            phi_val2 = 0;
        } else {
            phi_val2 = phi_val1;  /* Copy from first PHI */
        }
        
        /* Longer copy chain */
        chain1 = phi_val2;
        chain2 = chain1;
        int chain3 = chain2;
        final_cmp = chain3;
        
        /* Multiple conditionals using the same PHI-derived value */
        if (final_cmp != 0) {
            result += i * i;
            
            /* Nested conditional */
            int tmp = final_cmp;
            if (tmp == 1) {
                result -= i;
            }
        } else {
            result += i % 100;
        }
        
        /* Loop with PHI-derived condition */
        int loop_ctrl = final_cmp;
        int j = 0;
        while (j < 10 && loop_ctrl) {
            result ^= j;
            j++;
            /* Break the loop early in cold path */
            if (!loop_ctrl) break;
        }
    }
    
    return result;
}

/* Function 3: PHI in loop header with varying trip counts */
int process_mode3(int mode, int iterations) {
    int result = 0;
    int use_fast_path = (mode == 3);
    
    /* Outer loop with PHI-dependent inner loop */
    for (int outer = 0; outer < iterations / 100; outer++) {
        int inner_limit;
        int phi_control;
        
        /* PHI-like pattern created by conditional assignment */
        if (use_fast_path && outer > 10) {
            phi_control = 1;
            inner_limit = 1000;  /* Hot: large inner loop */
        } else {
            phi_control = 0;
            inner_limit = 10;    /* Cold: small inner loop */
        }
        
        /* Copy chain */
        int c1 = phi_control;
        int c2 = c1;
        int control_var = c2;
        
        /* Inner loop with PHI-derived control */
        for (int inner = 0; inner < inner_limit; inner++) {
            if (control_var) {
                result += outer * inner;
            } else {
                result += outer + inner;
            }
            
            /* Conditional inside loop using same control */
            int tmp = control_var;
            if (tmp == 1) {
                result ^= (outer << 3);
            }
        }
    }
    
    return result;
}

/* Function 4: Multiple PHI nodes feeding into same conditional */
int process_mode4(int mode, int iterations) {
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        int phi_a, phi_b, combined;
        
        /* Two independent PHI nodes */
        if (i % 7 == 0) {
            phi_a = (mode == 4) ? 1 : 0;
        } else {
            phi_a = 0;
        }
        
        if (i % 13 == 0) {
            phi_b = 1;
        } else {
            phi_b = (mode > 2) ? 1 : 0;
        }
        
        /* Combine PHI results */
        combined = phi_a & phi_b;
        
        /* Copy chain */
        int chain1 = combined;
        int chain2 = chain1;
        int final_val = chain2;
        
        /* Conditional with combined PHI value */
        if (final_val) {
            result += i * 3;
        } else if (final_val == 0) {
            result += i / 3;
        }
    }
    
    return result;
}

/* Function 5: Recursive pattern with PHI propagation */
int recursive_processor(int depth, int max_depth, int flag) {
    if (depth >= max_depth) return 1;
    
    int phi_val;
    int result = 0;
    
    /* PHI-like value based on flag and depth */
    if (flag && depth < max_depth / 2) {
        phi_val = 1;
    } else {
        phi_val = 0;
    }
    
    /* Copy chain */
    int tmp = phi_val;
    int cmp_var = tmp;
    
    /* Conditional controlling recursion */
    if (cmp_var) {
        result += recursive_processor(depth + 1, max_depth, flag);
        result += recursive_processor(depth + 2, max_depth, flag);
    } else {
        result += recursive_processor(depth + 1, max_depth, 0);
    }
    
    /* Another conditional with explicit comparison */
    if (cmp_var == 1) {
        result ^= depth;
    }
    
    return result;
}

int process_mode5(int mode, int iterations) {
    int result = 0;
    int use_deep_recursion = (mode == 5);
    
    for (int i = 0; i < iterations; i++) {
        result += recursive_processor(0, 10, use_deep_recursion);
    }
    
    return result;
}

/* Main function with profile-generating runtime behavior */
int main(int argc, char *argv[]) {
    int mode = 1;  /* Default to hot mode */
    int total_iterations = HOT_ITERATIONS;
    
    /* Parse command line arguments */
    if (argc > 1) {
        mode = atoi(argv[1]);
        if (argc > 2) {
            total_iterations = atoi(argv[2]);
        }
    }
    
    printf("Running mode %d with %d iterations\n", mode, total_iterations);
    
    int result = 0;
    clock_t start = clock();
    
    /* Warm-up phase with mixed behavior */
    printf("Warm-up phase...\n");
    for (int warm = 0; warm < WARM_ITERATIONS; warm++) {
        result ^= process_mode1(warm % 3, 10);
    }
    
    /* Main processing based on mode */
    printf("Main processing phase...\n");
    switch (mode) {
        case 1:  /* Hot path dominant */
            result += process_mode1(1, total_iterations);
            result += process_mode2(1, total_iterations / 2);
            break;
            
        case 2:  /* Mixed hot/cold */
            result += process_mode1(0, total_iterations / 3);
            result += process_mode3(2, total_iterations);
            break;
            
        case 3:  /* Loop-heavy */
            result += process_mode3(3, total_iterations * 2);
            break;
            
        case 4:  /* Multiple PHI patterns */
            result += process_mode4(4, total_iterations);
            result += process_mode2(4, total_iterations / 4);
            break;
            
        case 5:  /* Recursive pattern */
            result += process_mode5(5, total_iterations / 10);
            break;
            
        default:  /* Cold path dominant */
            result += process_mode1(0, COLD_ITERATIONS);
            result += process_mode2(0, COLD_ITERATIONS);
            break;
    }
    
    /* Cold path execution (rarely taken) */
    if (mode == 99) {  /* Special test mode */
        result += process_mode1(0, 5);
        result += process_mode4(0, 5);
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Result: %d\n", result);
    printf("Time elapsed: %.2f seconds\n", elapsed);
    
    return 0;
}
