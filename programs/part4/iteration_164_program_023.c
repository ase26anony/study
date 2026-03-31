/* autofdo_phi_conditional.c
 * 
 * This program generates execution patterns to trigger GCC's AutoFDO
 * PHI-to-conditional analysis in auto-profile.cc lines 1312-1333.
 * 
 * Compilation and usage:
 * 1. First compilation (with existing profile or empty):
 *    gcc -O2 -fauto-profile autofdo_phi_conditional.c -o autofdo_test
 *    
 * 2. Run with dominant hot path:
 *    ./autofdo_test 1  # Generates profile with hot paths
 *    
 * 3. Recompile with collected profile:
 *    gcc -O2 -fauto-profile -Wauto-profile autofdo_phi_conditional.c -o autofdo_test_opt
 *    
 * 4. Run optimized version:
 *    ./autofdo_test_opt 2  # Tests cold paths
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define HOT_ITERATIONS 1000000
#define WARM_ITERATIONS 10000
#define COLD_ITERATIONS 10

/* Global to prevent optimization */
volatile int global_counter = 0;
volatile int checksum = 0;

/* Function 1: Complex PHI pattern with SSA copy chain */
int process_data_phi_chain(int mode, int size) {
    int result = 0;
    int i, j;
    
    /* Create varying profile based on mode */
    int limit = (mode == 1) ? HOT_ITERATIONS : 
                (mode == 2) ? WARM_ITERATIONS : COLD_ITERATIONS;
    
    for (i = 0; i < limit; i++) {
        int phi_val;
        int tmp1, tmp2, tmp3;
        
        /* Create branching that feeds into PHI */
        if (i % 3 == 0) {
            /* Hot path for mode 1, cold for others */
            phi_val = (mode == 1) ? 1 : 0;
        } else if (i % 3 == 1) {
            /* Medium path */
            phi_val = (mode == 2) ? 1 : 0;
        } else {
            /* Always cold */
            phi_val = 0;
        }
        
        /* PHI-like assignment through multiple basic blocks */
        int branch_selector = i % 4;
        int phi_result;
        
        /* Simulate PHI node with multiple predecessors */
        switch (branch_selector) {
            case 0:
                phi_result = phi_val;  /* From first branch */
                break;
            case 1:
                phi_result = (mode == 1) ? 1 : 0;  /* Hot branch */
                break;
            case 2:
                phi_result = (i < limit/2) ? 1 : 0;  /* Split path */
                break;
            default:
                phi_result = 0;  /* Cold branch */
                break;
        }
        
        /* Create SSA copy chain to trigger while loop in uncovered code */
        tmp1 = phi_result;      /* First assignment */
        tmp2 = tmp1;            /* Copy through SSA */
        tmp3 = tmp2 + 0;        /* Arithmetic that doesn't break pattern */
        int cmp_var = tmp3;     /* Final variable for comparison */
        
        /* Multiple comparison types using the PHI-derived value */
        
        /* Type 1: Direct use in if condition (triggers integer_zerop/integer_onep check) */
        if (cmp_var) {  /* cmp_var is 0 or 1 from PHI */
            /* Hot path for mode 1 */
            result += i * 2;
            global_counter++;
        } else {
            /* Cold path */
            result -= i;
        }
        
        /* Type 2: Explicit equality comparison */
        if (cmp_var == 1) {  /* Explicit comparison with 1 */
            result += i % 100;
            checksum ^= i;
        }
        
        /* Type 3: Not-equal comparison */
        if (cmp_var != 0) {
            for (j = 0; j < 10; j++) {
                result += j * cmp_var;
            }
        }
        
        /* Nested conditional with PHI-derived value */
        int nested_tmp = cmp_var;
        while (nested_tmp > 0) {  /* Loop condition using PHI value */
            result += nested_tmp;
            nested_tmp--;
            if (result > 1000000) result %= 1000000;
        }
    }
    
    return result;
}

/* Function 2: PHI pattern in loop condition */
int process_array_with_phi(int* data, int size, int mode) {
    int sum = 0;
    int i;
    
    /* Create PHI-to-conditional in loop condition */
    for (i = 0; i < size; i++) {
        int should_process;
        
        /* Complex branching to create PHI */
        if (data[i] > 100) {
            should_process = (mode == 1) ? 1 : 0;  /* Hot in mode 1 */
        } else if (data[i] > 50) {
            should_process = (mode == 2) ? 1 : 0;  /* Hot in mode 2 */
        } else {
            should_process = 0;  /* Always cold */
        }
        
        /* SSA copy chain */
        int chain1 = should_process;
        int chain2 = chain1;
        int chain3 = chain2 * 1;  /* Maintains SSA single assignment pattern */
        int final_flag = chain3;
        
        /* Use in conditional with constant comparison */
        if (final_flag == 1) {  /* Triggers integer_onep check */
            sum += data[i] * 2;
            global_counter += data[i] % 100;
        } else if (final_flag == 0) {  /* Triggers integer_zerop check */
            sum -= data[i] / 2;
        }
        
        /* Another conditional block */
        int tmp_flag = final_flag;
        while (tmp_flag) {  /* Loop using PHI-derived value */
            sum += tmp_flag;
            tmp_flag = 0;  /* Single iteration */
            checksum ^= sum;
        }
    }
    
    return sum;
}

/* Function 3: Nested PHI patterns with function calls */
int nested_phi_pattern(int depth, int mode) {
    if (depth <= 0) return 1;
    
    int result = 0;
    int branch_decision;
    
    /* Create PHI based on multiple conditions */
    if (mode == 1) {
        branch_decision = (depth % 2 == 0) ? 1 : 0;
    } else if (mode == 2) {
        branch_decision = (depth % 3 == 0) ? 1 : 0;
    } else {
        branch_decision = 0;  /* Cold path */
    }
    
    /* SSA propagation chain */
    int val1 = branch_decision;
    int val2 = val1;
    int val3 = val2 + 0;
    int cond_var = val3;
    
    /* Multiple conditional blocks with same PHI-derived value */
    if (cond_var) {
        result += nested_phi_pattern(depth - 1, mode) * 2;
    }
    
    if (cond_var == 1) {
        result += nested_phi_pattern(depth - 2, mode);
    }
    
    if (cond_var != 0) {
        for (int i = 0; i < depth; i++) {
            result += i * cond_var;
        }
    }
    
    return result;
}

/* Function 4: Real computation with PHI-to-conditional for profile diversity */
int compute_primes_with_phi(int limit, int mode) {
    int count = 0;
    int i, j;
    
    for (i = 2; i <= limit; i++) {
        int is_prime;
        
        /* PHI-like decision: process differently based on mode */
        if (mode == 1) {
            /* Hot path: check all numbers thoroughly */
            is_prime = 1;
            for (j = 2; j * j <= i; j++) {
                if (i % j == 0) {
                    is_prime = 0;
                    break;
                }
            }
        } else if (mode == 2) {
            /* Warm path: only check odd numbers thoroughly */
            is_prime = (i % 2 != 0);
            if (is_prime && i > 2) {
                for (j = 3; j * j <= i; j += 2) {
                    if (i % j == 0) {
                        is_prime = 0;
                        break;
                    }
                }
            }
        } else {
            /* Cold path: simple check */
            is_prime = (i == 2 || i == 3 || i == 5 || i == 7);
        }
        
        /* SSA copy chain */
        int prime_flag = is_prime;
        int flag_copy1 = prime_flag;
        int flag_copy2 = flag_copy1;
        int final_prime_flag = flag_copy2;
        
        /* Conditional using PHI-derived value */
        if (final_prime_flag) {
            count++;
            global_counter += i;
        }
        
        /* Another conditional block */
        if (final_prime_flag == 1) {
            checksum ^= i;
        }
    }
    
    return count;
}

/* Main function with different execution modes */
int main(int argc, char** argv) {
    int mode = 1;  /* Default to hot path mode */
    int result = 0;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    printf("Running mode %d\n", mode);
    
    /* Warm-up phase (creates initial profile annotations) */
    clock_t start = clock();
    
    /* Phase 1: Process data with PHI chains */
    result += process_data_phi_chain(mode, 1000);
    
    /* Phase 2: Array processing with PHI patterns */
    int array_size = (mode == 1) ? 10000 : 1000;
    int* data = malloc(array_size * sizeof(int));
    for (int i = 0; i < array_size; i++) {
        data[i] = (i * 17) % 1000;
    }
    result += process_array_with_phi(data, array_size, mode);
    free(data);
    
    /* Phase 3: Nested PHI patterns */
    int depth = (mode == 1) ? 10 : 5;
    result += nested_phi_pattern(depth, mode);
    
    /* Phase 4: Real computation for profile diversity */
    int prime_limit = (mode == 1) ? 10000 : 
                     (mode == 2) ? 5000 : 1000;
    result += compute_primes_with_phi(prime_limit, mode);
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Result: %d\n", result);
    printf("Global counter: %d\n", global_counter);
    printf("Checksum: %d\n", checksum);
    printf("Elapsed time: %.3f seconds\n", elapsed);
    
    /* Verification output */
    printf("Verification: mode=%d, result_hash=%u\n", 
           mode, (unsigned int)(result ^ checksum ^ global_counter));
    
    return 0;
}
