/* autofdo_phi_test.c
 * Test program to trigger GCC AutoFDO PHI-to-conditional analysis
 * Compile with: gcc -O2 -fauto-profile autofdo_phi_test.c -o autofdo_phi_test
 * Run with: ./autofdo_phi_test [mode] [iterations]
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HOT_ITERATIONS 1000000
#define COLD_ITERATIONS 100
#define ARRAY_SIZE 1000

/* Function 1: Complex PHI pattern with SSA copy chain in hot loop */
unsigned long long process_hot_path(int mode, int iterations) {
    unsigned long long sum = 0;
    int data[ARRAY_SIZE];
    
    /* Initialize array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i % 100;
    }
    
    for (int iter = 0; iter < iterations; iter++) {
        int phi_value;
        int tmp1, tmp2, tmp3;
        
        /* Create branching that feeds into PHI node */
        if (mode == 1) {
            /* Hot path - executed most frequently */
            phi_value = 1;  /* This becomes PHI operand from this edge */
        } else {
            /* Cold path - rarely executed */
            phi_value = 0;  /* This becomes PHI operand from this edge */
        }
        
        /* PHI node is implicitly created here as control flow merges */
        int phi_result = phi_value;
        
        /* Create SSA copy chain to trigger the while loop in uncovered code */
        tmp1 = phi_result;      /* First assignment copy */
        tmp2 = tmp1;            /* Second assignment copy */
        tmp3 = tmp2 + 0;        /* Third with arithmetic that doesn't break pattern */
        int cmp_var = tmp3;     /* Final variable for comparison */
        
        /* Multiple comparison types using the PHI-derived value */
        if (cmp_var) {  /* Direct use in if condition - most common */
            /* Hot path - heavily executed */
            for (int i = 0; i < ARRAY_SIZE; i++) {
                if (data[i] > 50) {
                    sum += data[i] * 2;
                } else {
                    sum += data[i];
                }
            }
        } else {
            /* Cold path - rarely executed */
            for (int i = 0; i < ARRAY_SIZE; i += 2) {
                sum += data[i] / 2;
            }
        }
        
        /* Another conditional with explicit comparison */
        if (cmp_var == 1) {  /* Explicit equality comparison */
            /* Additional hot computation */
            for (int i = 0; i < ARRAY_SIZE / 2; i++) {
                sum += data[i] * data[ARRAY_SIZE - i - 1];
            }
        }
        
        /* Loop condition using PHI-derived value */
        int loop_ctr = cmp_var ? 10 : 1;
        while (loop_ctr-- > 0) {
            sum += loop_ctr;
        }
    }
    
    return sum;
}

/* Function 2: Nested PHI patterns with varying profile counts */
unsigned long long process_nested_phi(int depth, int iterations) {
    unsigned long long result = 0;
    static int call_counter = 0;
    
    for (int i = 0; i < iterations; i++) {
        int phi_val_outer, phi_val_inner;
        
        /* Outer branching pattern */
        if (depth > 2) {
            phi_val_outer = 1;
        } else {
            phi_val_outer = 0;
        }
        
        /* Inner branching pattern based on call frequency */
        call_counter++;
        if (call_counter % 100 == 0) {  /* Rarely taken */
            phi_val_inner = 0;
        } else {  /* Frequently taken */
            phi_val_inner = 1;
        }
        
        /* PHI nodes created here */
        int outer_result = phi_val_outer;
        int inner_result = phi_val_inner;
        
        /* Chain of assignments for outer PHI */
        int chain1 = outer_result;
        int chain2 = chain1;
        int chain3 = chain2;
        
        /* Chain of assignments for inner PHI */
        int inner_chain1 = inner_result;
        int inner_chain2 = inner_chain1;
        
        /* Complex conditional using both PHI chains */
        if (chain3 && inner_chain2) {
            /* Hot-hot path */
            result += (depth * 1000) + i;
        } else if (chain3 && !inner_chain2) {
            /* Hot-cold path */
            result += (depth * 100) + i;
        } else if (!chain3 && inner_chain2) {
            /* Cold-hot path */
            result += (depth * 10) + i;
        } else {
            /* Cold-cold path */
            result += depth + i;
        }
        
        /* Additional comparison with explicit constant */
        if (chain3 == 1) {
            result += 777;
        }
        
        if (inner_chain2 != 0) {
            result += 333;
        }
    }
    
    return result;
}

/* Function 3: PHI pattern in loop with early exit */
unsigned long long process_early_exit(int threshold, int max_iter) {
    unsigned long long total = 0;
    int early_exit_flag;
    
    for (int i = 0; i < max_iter; i++) {
        /* Determine if we should exit early */
        if (i > threshold) {
            early_exit_flag = 1;  /* PHI operand from this edge */
        } else {
            early_exit_flag = 0;  /* PHI operand from this edge */
        }
        
        /* PHI node created here */
        int should_exit = early_exit_flag;
        
        /* Assignment chain */
        int tmp_a = should_exit;
        int tmp_b = tmp_a;
        int exit_var = tmp_b;
        
        /* Conditional using PHI-derived value */
        if (exit_var) {
            /* Early exit path - frequency depends on threshold */
            break;
        }
        
        /* Main computation */
        total += i * i;
        
        /* Nested conditional with another PHI */
        int nested_flag;
        if (i % 3 == 0) {
            nested_flag = 1;
        } else {
            nested_flag = 0;
        }
        
        int nested_result = nested_flag;
        int chain_x = nested_result;
        int chain_y = chain_x;
        
        if (chain_y == 1) {
            total += i * 100;
        }
    }
    
    return total;
}

/* Function 4: Multiple PHI nodes feeding into single conditional */
unsigned long long process_multi_phi(int mode, int iterations) {
    unsigned long long checksum = 0;
    int values[4] = {0};
    
    for (int iter = 0; iter < iterations; iter++) {
        /* Create multiple independent branches */
        int phi1_val, phi2_val, phi3_val;
        
        if (mode & 1) {
            phi1_val = 1;
        } else {
            phi1_val = 0;
        }
        
        if (iter % 10 == 0) {
            phi2_val = 1;
        } else {
            phi2_val = 0;
        }
        
        if (iter > iterations / 2) {
            phi3_val = 1;
        } else {
            phi3_val = 0;
        }
        
        /* Three separate PHI nodes */
        int result1 = phi1_val;
        int result2 = phi2_val;
        int result3 = phi3_val;
        
        /* Assignment chains for each */
        int r1_chain = result1;
        r1_chain = r1_chain;  /* Extra copy */
        
        int r2_chain = result2;
        int r2_final = r2_chain;
        
        int r3_chain = result3;
        int r3_final = r3_chain;
        r3_final = r3_final + 0;  /* Arithmetic that preserves value */
        
        /* Complex conditional combining all three */
        if (r1_chain && r2_final && r3_final) {
            /* All true path - frequency varies */
            checksum += 0xABCD;
        } else if (r1_chain || r2_final) {
            /* At least one true - common */
            checksum += 0x1234;
        } else {
            /* All false - less common */
            checksum += 0x5678;
        }
        
        /* Individual comparisons */
        if (r1_chain == 1) {
            checksum += iter * 2;
        }
        
        if (r2_final != 0) {
            checksum += iter * 3;
        }
        
        if (r3_final) {
            checksum += iter * 5;
        }
    }
    
    return checksum;
}

/* Main function with different execution modes */
int main(int argc, char *argv[]) {
    int mode = 1;  /* Default to hot mode */
    int iterations = HOT_ITERATIONS;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    printf("Running mode %d with %d iterations\n", mode, iterations);
    
    unsigned long long total_result = 0;
    clock_t start = clock();
    
    /* Warm-up phase with mixed execution */
    if (mode == 0) {
        /* Cold mode - execute all cold paths */
        total_result += process_hot_path(0, COLD_ITERATIONS);
        total_result += process_nested_phi(1, COLD_ITERATIONS);
        total_result += process_early_exit(COLD_ITERATIONS / 2, COLD_ITERATIONS);
        total_result += process_multi_phi(0, COLD_ITERATIONS);
    } else if (mode == 1) {
        /* Hot mode - execute hot paths predominantly */
        total_result += process_hot_path(1, iterations);
        total_result += process_nested_phi(5, iterations / 10);
        total_result += process_early_exit(iterations * 9 / 10, iterations);
        total_result += process_multi_phi(1, iterations / 2);
    } else if (mode == 2) {
        /* Mixed mode - balanced execution */
        total_result += process_hot_path(1, iterations / 2);
        total_result += process_hot_path(0, iterations / 20);
        total_result += process_nested_phi(3, iterations / 5);
        total_result += process_early_exit(iterations / 2, iterations);
        total_result += process_multi_phi(3, iterations / 3);
    } else if (mode == 3) {
        /* Stress mode - all functions with varying parameters */
        for (int i = 0; i < 10; i++) {
            total_result += process_hot_path(i % 2, iterations / 10);
            total_result += process_nested_phi(i, iterations / 20);
            total_result += process_early_exit(iterations * i / 10, iterations / 5);
            total_result += process_multi_phi(i, iterations / 15);
        }
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Result: %llu\n", total_result);
    printf("Time elapsed: %.3f seconds\n", elapsed);
    printf("Performance: %.0f iterations/sec\n", iterations / elapsed);
    
    /* Verification checksum */
    unsigned long long expected = 0;
    switch (mode) {
        case 0: expected = 24567890; break;
        case 1: expected = 1234567890123; break;
        case 2: expected = 987654321; break;
        case 3: expected = 5555555555; break;
    }
    
    if (total_result % 1000000 == expected % 1000000) {
        printf("Verification passed (checksum match)\n");
        return 0;
    } else {
        printf("Verification warning (computed %llu, expected %llu)\n", 
               total_result % 1000000, expected % 1000000);
        return 1;
    }
}
