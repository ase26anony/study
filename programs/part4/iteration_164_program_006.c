/* auto_profile_test.c - Test program for GCC AutoFDO PHI-to-conditional analysis */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HOT_ITERATIONS 10000000
#define WARM_ITERATIONS 1000000
#define COLD_ITERATIONS 100

/* Function 1: Simple PHI-to-conditional with direct copy chain */
int phi_conditional_chain(int mode, int limit) {
    int result = 0;
    
    for (int i = 0; i < limit; i++) {
        int phi_val;
        int tmp1, tmp2, cmp_var;
        
        /* Create PHI node based on mode */
        if (mode == 1) {
            phi_val = 1;  /* Hot path value */
        } else {
            phi_val = 0;  /* Cold path value */
        }
        
        /* SSA copy chain to trigger while loop walking */
        tmp1 = phi_val;      /* First assignment */
        tmp2 = tmp1;         /* Second assignment */
        cmp_var = tmp2 + 0;  /* Third assignment with arithmetic */
        
        /* Conditional using PHI-derived value */
        if (cmp_var == 1) {  /* Explicit equality comparison */
            result += i * 2;  /* Hot computation */
        } else {
            result += i;      /* Cold computation */
        }
    }
    
    return result;
}

/* Function 2: Nested PHI patterns with multiple predecessors */
int nested_phi_pattern(int seed, int iterations) {
    int total = 0;
    int outer_flag = (seed % 3 == 0) ? 1 : 0;
    
    for (int i = 0; i < iterations; i++) {
        int inner_flag;
        int tmp_a, tmp_b, final_flag;
        
        /* Complex PHI creation with multiple conditions */
        if (i % 100 == 0) {
            inner_flag = 1;
        } else if (i % 50 == 0) {
            inner_flag = outer_flag;
        } else {
            inner_flag = 0;
        }
        
        /* Extended SSA copy chain */
        tmp_a = inner_flag;
        for (int j = 0; j < 3; j++) {
            tmp_b = tmp_a;
            tmp_a = tmp_b;  /* Create copy loop in SSA */
        }
        final_flag = tmp_a;
        
        /* Multiple conditional uses */
        if (final_flag) {  /* Direct boolean use */
            total += i * i;
            if (final_flag == 1) {  /* Explicit comparison */
                total += 1;
            }
        } else {
            total -= i;
        }
        
        /* Update outer flag for next iteration */
        outer_flag = (total % 2 == 0) ? 1 : 0;
    }
    
    return total;
}

/* Function 3: PHI in loop condition with array processing */
int phi_loop_condition(int* data, int size, int threshold) {
    int sum = 0;
    int continue_flag = 1;
    int tmp_flag, check_flag;
    
    for (int i = 0; i < size; i++) {
        /* PHI node for loop continuation */
        if (i < threshold) {
            continue_flag = 1;
        } else {
            continue_flag = (data[i] > 0) ? 1 : 0;
        }
        
        /* Multi-step SSA propagation */
        tmp_flag = continue_flag;
        check_flag = tmp_flag;
        
        /* Loop condition using PHI-derived value */
        while (check_flag && i < size) {  /* PHI used in while condition */
            sum += data[i];
            
            /* Update flags to create PHI merges */
            if (sum % 1000 == 0) {
                tmp_flag = 1;
            } else {
                tmp_flag = 0;
            }
            check_flag = tmp_flag;
            
            i++;
            if (i >= size) break;
        }
        
        if (!check_flag) {
            sum -= 100;  /* Cold path penalty */
        }
    }
    
    return sum;
}

/* Function 4: Complex control flow with multiple PHI merges */
int complex_phi_merges(int mode, int depth) {
    int value = 0;
    int flag_chain[4] = {0, 0, 0, 0};
    
    for (int d = 0; d < depth; d++) {
        int current_flag;
        
        /* Multiple predecessor paths creating PHI */
        switch (mode) {
            case 1:  /* Hot path */
                current_flag = 1;
                break;
            case 2:  /* Warm path */
                current_flag = (d % 2 == 0) ? 1 : 0;
                break;
            default: /* Cold path */
                current_flag = 0;
                break;
        }
        
        /* Chain of SSA assignments through array */
        flag_chain[0] = current_flag;
        for (int i = 1; i < 4; i++) {
            flag_chain[i] = flag_chain[i-1];
        }
        
        /* Multiple conditionals with PHI-derived values */
        if (flag_chain[3] != 0) {  /* Inequality comparison */
            value += d * 100;
            if (flag_chain[2] == 1) {
                value += 50;
            }
        } else {
            value -= d;
        }
        
        /* Nested conditional with another PHI */
        int nested_flag;
        if (value > 1000) {
            nested_flag = 1;
        } else {
            nested_flag = flag_chain[1];
        }
        
        int tmp_nested = nested_flag;
        if (tmp_nested) {
            value *= 1.01;
        }
        
        /* Alternate mode for PHI diversity */
        mode = (mode + 1) % 3;
    }
    
    return value;
}

/* Function 5: Real computation with PHI patterns for profile diversity */
int compute_primes_with_phi(int limit, int hot_mode) {
    int count = 0;
    int last_prime = 2;
    
    for (int n = 2; n <= limit; n++) {
        int is_prime = 1;  /* Will become PHI */
        int tmp_is_prime, check_prime;
        
        /* Create PHI based on hot/cold mode */
        if (hot_mode) {
            /* Optimistic path - assume prime */
            is_prime = 1;
        } else {
            /* Conservative path */
            is_prime = 0;
        }
        
        /* SSA copy chain */
        tmp_is_prime = is_prime;
        check_prime = tmp_is_prime;
        
        /* Actual prime checking (creates profile diversity) */
        for (int i = 2; i * i <= n; i++) {
            if (n % i == 0) {
                check_prime = 0;
                break;
            }
        }
        
        /* Conditional using PHI-derived value */
        if (check_prime == 1) {  /* Explicit comparison to 1 */
            count++;
            last_prime = n;
            
            /* Nested conditional with another PHI */
            int should_log;
            if (count % 100 == 0) {
                should_log = 1;
            } else {
                should_log = hot_mode;
            }
            
            int tmp_log = should_log;
            if (tmp_log) {
                /* Simulate logging operation */
                count += 0;  /* No-op for profile */
            }
        } else if (check_prime != 0) {
            /* This path should be cold */
            count--;
        }
    }
    
    return count;
}

/* Main function with profile-generating runtime behavior */
int main(int argc, char** argv) {
    int mode = 1;  /* Default to hot mode */
    int result = 0;
    clock_t start, end;
    
    /* Parse command line for mode */
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    printf("Starting AutoFDO test with mode %d\n", mode);
    start = clock();
    
    /* Warm-up phase - establish baseline profile */
    printf("Phase 1: Warm-up\n");
    for (int phase = 0; phase < 3; phase++) {
        result += phi_conditional_chain(phase % 2, WARM_ITERATIONS / 10);
    }
    
    /* Main execution phase with mode-dependent behavior */
    printf("Phase 2: Main execution\n");
    switch (mode) {
        case 1:  /* HOT mode - heavily exercise PHI-to-conditional paths */
            result += phi_conditional_chain(1, HOT_ITERATIONS);
            result += nested_phi_pattern(42, HOT_ITERATIONS / 10);
            
            /* Create array for PHI loop condition */
            int hot_data[1000];
            for (int i = 0; i < 1000; i++) hot_data[i] = i;
            result += phi_loop_condition(hot_data, 1000, 800);
            
            result += complex_phi_merges(1, HOT_ITERATIONS / 100);
            result += compute_primes_with_phi(100000, 1);
            break;
            
        case 2:  /* MIXED mode - balanced hot/cold paths */
            result += phi_conditional_chain(0, HOT_ITERATIONS / 2);
            result += nested_phi_pattern(17, HOT_ITERATIONS / 20);
            
            int mixed_data[500];
            for (int i = 0; i < 500; i++) mixed_data[i] = i % 100;
            result += phi_loop_condition(mixed_data, 500, 250);
            
            result += complex_phi_merges(2, HOT_ITERATIONS / 200);
            result += compute_primes_with_phi(50000, 0);
            break;
            
        default: /* COLD mode - exercise rare paths */
            result += phi_conditional_chain(0, COLD_ITERATIONS);
            result += nested_phi_pattern(99, COLD_ITERATIONS * 2);
            
            int cold_data[100];
            for (int i = 0; i < 100; i++) cold_data[i] = -i;
            result += phi_loop_condition(cold_data, 100, 10);
            
            result += complex_phi_merges(0, COLD_ITERATIONS * 3);
            result += compute_primes_with_phi(1000, 0);
            break;
    }
    
    /* Cool-down phase - different pattern */
    printf("Phase 3: Cool-down\n");
    result += phi_conditional_chain((mode + 1) % 2, WARM_ITERATIONS / 20);
    
    end = clock();
    double cpu_time = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("Result: %d\n", result);
    printf("Execution time: %.2f seconds\n", cpu_time);
    printf("Mode %d completed\n", mode);
    
    return 0;
}
