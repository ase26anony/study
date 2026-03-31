/* test_autofdo_phi_cond.c
 * 
 * This program generates patterns to trigger AutoFDO's PHI-to-conditional
 * analysis in auto-profile.cc lines 1312-1333.
 * 
 * Compilation for coverage:
 * 1. First compilation (with empty or existing profile):
 *    g++ -O2 -fauto-profile -o test_autofdo test_autofdo_phi_cond.c
 * 2. Run with hot path to generate profile:
 *    ./test_autofdo 1 > /dev/null
 * 3. Recompile with generated profile:
 *    g++ -O2 -fauto-profile -Wauto-profile -o test_autofdo_opt test_autofdo_phi_cond.c
 * 
 * For debugging dumps:
 *    g++ -O2 -fauto-profile -fdump-tree-afdo -fdump-tree-afdo-details test_autofdo_phi_cond.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HOT_ITERATIONS 1000000
#define COLD_ITERATIONS 100
#define ARRAY_SIZE 1000

/* Function 1: Direct PHI-to-conditional with SSA copy chain */
int process_hot_path(int mode, int iterations) {
    int sum = 0;
    int i, j;
    
    /* Create varying profile counts for different paths */
    for (i = 0; i < iterations; i++) {
        int phi_val;
        int tmp1, tmp2, tmp3;
        
        /* Create PHI node with values from different predecessors */
        if (i % 100 < 95) {  /* Hot path - 95% of iterations */
            phi_val = 1;  /* Value 1 from this predecessor */
        } else {  /* Cold path - 5% of iterations */
            phi_val = 0;  /* Value 0 from this predecessor */
        }
        
        /* Chain of SSA assignments to trigger the while loop */
        tmp1 = phi_val;      /* First copy */
        tmp2 = tmp1;         /* Second copy */
        tmp3 = tmp2 + 0;     /* Arithmetic that doesn't change value */
        
        /* Multiple comparison types using the PHI-derived value */
        if (tmp3) {  /* Direct use in if condition */
            /* Hot computation path */
            for (j = 0; j < 10; j++) {
                sum += (i * j) % 100;
            }
        } else {
            /* Cold computation path */
            sum -= 1;
        }
        
        /* Another conditional with explicit comparison */
        if (tmp3 == 1) {  /* Explicit equality with 1 */
            sum += i % 10;
        }
        
        /* Use in loop condition */
        int loop_ctrl = tmp3;
        while (loop_ctrl) {
            sum += 1;
            loop_ctrl = 0;  /* Execute once */
        }
    }
    
    return sum;
}

/* Function 2: Nested PHI patterns with complex control flow */
int process_complex_pattern(int mode, int iterations) {
    int result = 0;
    int data[ARRAY_SIZE];
    int i;
    
    /* Initialize array */
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i * 2;
    }
    
    for (i = 0; i < iterations; i++) {
        int cond1, cond2;
        int tmp_a, tmp_b, tmp_c;
        
        /* Create two independent PHI sources */
        if (mode == 1) {
            cond1 = (i < iterations * 0.8) ? 1 : 0;
        } else {
            cond1 = (i % 3 == 0) ? 1 : 0;
        }
        
        cond2 = (data[i % ARRAY_SIZE] > 1000) ? 1 : 0;
        
        /* Chain of assignments for cond1 */
        tmp_a = cond1;
        tmp_b = tmp_a;
        
        /* Chain of assignments for cond2 */
        tmp_c = cond2;
        
        /* Nested conditionals with PHI-derived values */
        if (tmp_b) {
            /* Hot path with inner conditional */
            if (tmp_c) {
                result += data[i % ARRAY_SIZE] / 2;
            } else {
                result += data[i % ARRAY_SIZE] / 4;
            }
            
            /* Another use in loop */
            int k = tmp_b;
            do {
                result += 1;
                k--;
            } while (k > 0);
        } else {
            /* Cold path */
            if (tmp_c != 0) {  /* Explicit inequality with 0 */
                result -= data[i % ARRAY_SIZE] / 8;
            }
        }
        
        /* Switch-like pattern using PHI value */
        switch (tmp_b) {
            case 1:
                result += 100;
                break;
            case 0:
                result += 10;
                break;
        }
    }
    
    return result;
}

/* Function 3: PHI from function calls with varying profile counts */
int process_with_function_calls(int mode, int iterations) {
    int total = 0;
    int i;
    
    /* Helper function that returns 0/1 based on complex condition */
    int get_phi_value(int idx, int mode) {
        static int counter = 0;
        counter++;
        
        if (mode == 1) {
            return (counter % 100 < 90) ? 1 : 0;  /* 90% hot */
        } else {
            return (idx % 7 == 0) ? 1 : 0;  /* ~14% hot */
        }
    }
    
    for (i = 0; i < iterations; i++) {
        int base_val = get_phi_value(i, mode);
        int chain1, chain2, final_cond;
        
        /* Multi-step SSA copy chain */
        chain1 = base_val;
        chain2 = chain1;
        final_cond = chain2;
        
        /* Conditional with the PHI-derived value */
        if (final_cond) {
            /* Call hot function */
            total += hot_calculation(i);
        } else {
            /* Call cold function */
            total += cold_calculation(i);
        }
        
        /* Another use with different comparison */
        if (final_cond == 1) {
            total += i * 2;
        } else if (final_cond != 1) {
            total += i;
        }
    }
    
    return total;
}

/* Hot and cold helper functions */
int hot_calculation(int x) {
    /* Complex enough to not be optimized away */
    int y = x * x;
    y = (y >> 3) | (y << 5);
    return y % 1000;
}

int cold_calculation(int x) {
    /* Different computation pattern */
    int y = x * 3;
    y = y ^ (y >> 2);
    return y % 100;
}

/* Function 4: Loop-carried PHI dependencies */
int process_loop_carried_phi(int iterations) {
    int sum = 0;
    int prev_hot = 0;  /* Initial value */
    int i;
    
    for (i = 0; i < iterations; i++) {
        int current_hot;
        int tmp1, tmp2;
        
        /* PHI that depends on previous iteration */
        if (i == 0) {
            current_hot = 0;
        } else {
            current_hot = (sum % 100 > 50) ? 1 : 0;
        }
        
        /* Mix with previous value */
        int phi_val = (prev_hot && current_hot) ? 1 : 0;
        
        /* SSA copy chain */
        tmp1 = phi_val;
        tmp2 = tmp1;
        
        /* Use in conditional */
        if (tmp2) {
            sum += i * 3;
        } else {
            sum += i;
        }
        
        /* Update for next iteration */
        prev_hot = current_hot;
        
        /* Nested loop with PHI-derived condition */
        int inner_ctrl = tmp2;
        for (int j = 0; j < inner_ctrl * 5; j++) {
            sum += j % 10;
        }
    }
    
    return sum;
}

/* Main function with different execution modes */
int main(int argc, char *argv[]) {
    int mode = 1;  /* Default to hot mode */
    int iterations = HOT_ITERATIONS;
    int result = 0;
    
    /* Parse command line */
    if (argc > 1) {
        mode = atoi(argv[1]);
        if (mode < 1 || mode > 3) mode = 1;
    }
    
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations < 1) iterations = HOT_ITERATIONS;
    }
    
    /* Warm-up phase (different profile) */
    printf("Starting warm-up...\n");
    result += process_hot_path(2, COLD_ITERATIONS);
    
    /* Main execution with selected mode */
    printf("Running mode %d with %d iterations...\n", mode, iterations);
    
    switch (mode) {
        case 1:  /* Hot mode - triggers heavily annotated blocks */
            result += process_hot_path(1, iterations);
            result += process_complex_pattern(1, iterations / 10);
            result += process_with_function_calls(1, iterations / 5);
            break;
            
        case 2:  /* Mixed mode */
            result += process_hot_path(2, iterations / 2);
            result += process_complex_pattern(2, iterations / 4);
            result += process_loop_carried_phi(iterations / 3);
            break;
            
        case 3:  /* Cold mode */
            result += process_hot_path(3, COLD_ITERATIONS);
            result += process_with_function_calls(3, COLD_ITERATIONS * 2);
            break;
    }
    
    /* Final processing with different pattern */
    printf("Final processing...\n");
    result += process_loop_carried_phi(iterations / 20);
    
    /* Output checksum for verification */
    printf("Result checksum: %d\n", result % 1000000);
    
    return 0;
}
