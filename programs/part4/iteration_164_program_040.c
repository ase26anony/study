/* autofdo_phi_conditional.c
 * 
 * This program generates execution patterns specifically designed to trigger
 * GCC's AutoFDO PHI-to-conditional analysis in auto-profile.cc lines 1312-1333.
 * The patterns include:
 * 1. Boolean values (0/1) flowing through PHI nodes into conditional comparisons
 * 2. Chains of SSA assignments between PHI definitions and conditionals
 * 3. Hot loops with annotated basic blocks
 * 4. Multiple comparison types using PHI-derived values
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ========== Pattern 1: Simple PHI-to-conditional with SSA chains ========== */

/* This function creates a PHI node that selects between 0 and 1 from different
 * predecessor blocks, with SSA copy chains before the conditional */
int pattern1_phi_to_conditional(int mode, int iterations) {
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        int phi_value;
        int tmp1, tmp2, tmp3;
        
        /* Create two different predecessor paths that merge at PHI */
        if (mode == 1) {
            /* Hot path - executed millions of times */
            phi_value = 1;  /* This becomes PHI operand from this edge */
        } else {
            /* Cold path - rarely executed */
            phi_value = 0;  /* This becomes PHI operand from this edge */
        }
        
        /* SSA copy chain to trigger the while loop in uncovered code */
        tmp1 = phi_value;      /* First assignment copy */
        tmp2 = tmp1;           /* Second assignment copy */
        tmp3 = tmp2 + 0;       /* Assignment with trivial arithmetic */
        
        /* Conditional using the PHI-derived value - triggers uncovered analysis */
        if (tmp3) {  /* Direct use in if condition */
            result += i * 2;
        } else {
            result += i;
        }
        
        /* Another conditional with explicit comparison */
        if (tmp3 == 1) {  /* Explicit equality comparison */
            result += 7;
        }
        
        /* Chain continues with more copies */
        int tmp4 = tmp3;
        int tmp5 = tmp4;
        
        /* Use in while condition */
        int counter = 3;
        while (tmp5 && counter > 0) {  /* PHI-derived in loop condition */
            result += counter;
            counter--;
        }
    }
    
    return result;
}

/* ========== Pattern 2: Nested PHI patterns with complex control flow ========== */

int pattern2_nested_phi(int seed, int limit) {
    int total = 0;
    int outer_phi_val;
    
    /* Outer loop creates PHI at loop header */
    for (int j = 0; j < limit; j++) {
        /* PHI at loop header based on previous iteration */
        if (j == 0) {
            outer_phi_val = (seed % 2);  /* Initial value */
        } else {
            /* PHI selects between two different update strategies */
            if (total % 1000 > 500) {
                outer_phi_val = 1;
            } else {
                outer_phi_val = 0;
            }
        }
        
        /* SSA copy chain */
        int chain1 = outer_phi_val;
        int chain2 = chain1;
        int chain3 = chain2;
        
        /* Inner conditional structure */
        for (int k = 0; k < 10; k++) {
            int inner_phi_val;
            
            /* Create inner PHI based on outer value */
            if (chain3) {
                inner_phi_val = (k % 3 == 0) ? 1 : 0;
            } else {
                inner_phi_val = (k % 2 == 0) ? 1 : 0;
            }
            
            /* More SSA copies */
            int inner_tmp1 = inner_phi_val;
            int inner_tmp2 = inner_tmp1;
            
            /* Conditional with PHI-derived value */
            if (inner_tmp2 != 0) {  /* Inequality comparison */
                total += j * k + 1;
            } else {
                total += j + k;
            }
        }
        
        /* Another conditional using outer chain */
        if (chain3 == 1) {
            total += 100;
        }
    }
    
    return total;
}

/* ========== Pattern 3: PHI in switch-like pattern ========== */

int pattern3_switch_phi(int value, int scale) {
    int result = 0;
    int phi_selector;
    
    /* Multiple predecessor blocks feeding into PHI */
    if (value < 10) {
        phi_selector = 0;
        result += value * 2;
    } else if (value < 20) {
        phi_selector = 1;
        result += value * 3;
    } else if (value < 30) {
        phi_selector = 0;
        result += value * 4;
    } else {
        phi_selector = 1;
        result += value * 5;
    }
    
    /* SSA copy chain */
    int copy1 = phi_selector;
    int copy2 = copy1;
    int copy3 = copy2 + 0;  /* Trivial arithmetic to maintain SSA pattern */
    
    /* Multiple conditionals using the PHI-derived value */
    for (int i = 0; i < scale; i++) {
        if (copy3) {
            result += i * 11;
        }
        
        if (copy3 == 1) {
            result += i * 7;
        }
        
        /* Nested conditional */
        int tmp = copy3;
        while (tmp && i < 5) {
            result += i * 13;
            i++;
            tmp = 0;  /* Break condition */
        }
    }
    
    return result;
}

/* ========== Pattern 4: Complex loop with PHI-based exit conditions ========== */

int pattern4_loop_phi_exit(int base, int max_iter) {
    int sum = 0;
    int continue_flag;
    
    /* Initial PHI value */
    continue_flag = 1;
    
    for (int i = 0; i < max_iter && continue_flag; i++) {
        int inner_sum = 0;
        
        /* PHI updated based on complex condition */
        if (i % 100 == 0) {
            continue_flag = 0;  /* Rarely taken exit */
        } else {
            continue_flag = 1;  /* Usually taken */
        }
        
        /* SSA propagation through multiple variables */
        int flag_copy1 = continue_flag;
        int flag_copy2 = flag_copy1;
        int flag_copy3 = flag_copy2;
        
        /* Inner computation using PHI-derived value */
        for (int j = 0; j < 50; j++) {
            if (flag_copy3) {
                inner_sum += (i * j) % 97;
            } else {
                inner_sum += (i + j) % 97;
            }
            
            /* Additional conditional */
            if (flag_copy3 == 1) {
                inner_sum += j * 3;
            }
        }
        
        sum += inner_sum;
        
        /* Chain continues */
        int final_flag = flag_copy3;
        if (!final_flag) {
            sum += 9999;  /* Rare bonus */
        }
    }
    
    return sum;
}

/* ========== Pattern 5: Recursive pattern with PHI propagation ========== */

int pattern5_recursive_phi(int depth, int breadth, int *counter) {
    if (depth <= 0) {
        return 1;
    }
    
    int total = 0;
    int local_phi;
    
    /* PHI value based on depth */
    if (depth % 2 == 0) {
        local_phi = 1;
    } else {
        local_phi = 0;
    }
    
    /* SSA chain */
    int chain_a = local_phi;
    int chain_b = chain_a;
    int chain_c = chain_b;
    
    /* Conditional using PHI-derived value */
    if (chain_c) {
        total += depth * 100;
    }
    
    /* Recursive calls with PHI propagation */
    for (int i = 0; i < breadth; i++) {
        (*counter)++;
        
        int child_result = pattern5_recursive_phi(depth - 1, breadth, counter);
        
        /* PHI-based adjustment */
        if (chain_c == 1) {
            total += child_result * 2;
        } else {
            total += child_result;
        }
    }
    
    /* Another conditional */
    if (chain_c != 0) {
        total += 77;
    }
    
    return total;
}

/* ========== Main driver with profile-generating behavior ========== */

int main(int argc, char *argv[]) {
    int mode = 1;  /* Default to hot mode */
    int iterations = 1000000;  /* Default iterations for hot path */
    
    /* Parse command line arguments */
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    printf("Running AutoFDO PHI pattern generator - Mode: %d, Iterations: %d\n", 
           mode, iterations);
    
    int total_result = 0;
    clock_t start = clock();
    
    /* Execute different patterns based on mode to generate varied profiles */
    switch (mode) {
        case 1:  /* Hot mode - dominates profile */
            printf("Executing HOT path patterns...\n");
            
            /* Pattern 1: Simple PHI with many iterations */
            total_result += pattern1_phi_to_conditional(1, iterations);
            
            /* Pattern 2: Nested PHI with moderate iterations */
            total_result += pattern2_nested_phi(42, iterations / 100);
            
            /* Pattern 3: Switch-like PHI */
            for (int i = 0; i < iterations / 10; i++) {
                total_result += pattern3_switch_phi(i % 40, 5);
            }
            
            /* Pattern 4: Loop with PHI exit */
            total_result += pattern4_loop_phi_exit(100, iterations / 50);
            
            /* Pattern 5: Recursive PHI */
            {
                int counter = 0;
                total_result += pattern5_recursive_phi(6, 3, &counter);
            }
            break;
            
        case 2:  /* Warm mode - mixed profile */
            printf("Executing WARM path patterns...\n");
            
            total_result += pattern1_phi_to_conditional(0, iterations / 10);
            total_result += pattern2_nested_phi(123, iterations / 1000);
            
            for (int i = 0; i < 1000; i++) {
                total_result += pattern3_switch_phi(i % 40, 3);
            }
            
            total_result += pattern4_loop_phi_exit(50, 1000);
            
            {
                int counter = 0;
                total_result += pattern5_recursive_phi(4, 2, &counter);
            }
            break;
            
        case 3:  /* Cold mode - rarely executed */
            printf("Executing COLD path patterns...\n");
            
            total_result += pattern1_phi_to_conditional(0, 100);
            total_result += pattern2_nested_phi(999, 50);
            
            for (int i = 0; i < 20; i++) {
                total_result += pattern3_switch_phi(i % 40, 1);
            }
            
            total_result += pattern4_loop_phi_exit(10, 20);
            
            {
                int counter = 0;
                total_result += pattern5_recursive_phi(2, 1, &counter);
            }
            break;
            
        default:
            printf("Unknown mode. Using mixed execution.\n");
            
            /* Mix of all modes */
            total_result += pattern1_phi_to_conditional(mode % 2, iterations / 100);
            total_result += pattern2_nested_phi(mode * 17, iterations / 1000);
            
            for (int i = 0; i < 5000; i++) {
                total_result += pattern3_switch_phi((i + mode) % 40, 2);
            }
            
            total_result += pattern4_loop_phi_exit(mode * 25, 500);
            
            {
                int counter = 0;
                total_result += pattern5_recursive_phi(3 + (mode % 3), 1 + (mode % 2), &counter);
            }
            break;
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Total result: %d\n", total_result);
    printf("Execution time: %.3f seconds\n", elapsed);
    
    /* Output a checksum for verification */
    unsigned int checksum = 0;
    unsigned char *bytes = (unsigned char *)&total_result;
    for (size_t i = 0; i < sizeof(total_result); i++) {
        checksum += bytes[i];
    }
    printf("Checksum: 0x%08x\n", checksum);
    
    return 0;
}
