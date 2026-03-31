/* autofdo_phi_conditional_test.c
 * 
 * This program generates specific control flow patterns to trigger GCC's
 * AutoFDO PHI-to-conditional analysis in auto-profile.cc lines 1312-1333.
 * 
 * Compilation and usage:
 * 1. First compilation (with existing profile or empty):
 *    gcc -O2 -fauto-profile autofdo_phi_conditional_test.c -o test_prog
 *    
 * 2. Run with dominant mode to generate profile data:
 *    ./test_prog 1 > /dev/null
 *    
 * 3. Recompile with collected profile:
 *    gcc -O2 -fauto-profile -Wauto-profile autofdo_phi_conditional_test.c -o test_prog_opt
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

/* ========== Pattern 1: Simple PHI-to-conditional with copy chain ========== */
int process_hot_loop(int mode, int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        int base_value;
        
        /* Create two predecessor blocks with different values */
        if (i % 17 == 0) {
            /* Cold path - executed rarely */
            base_value = 0;
        } else {
            /* Hot path - executed frequently */
            base_value = 1;
        }
        
        /* PHI node is conceptually created here (compiler will generate it) */
        int phi_result = base_value;
        
        /* Create SSA copy chain to trigger the while loop in auto-profile.cc */
        int tmp1 = phi_result;      /* First copy */
        int tmp2 = tmp1;            /* Second copy */
        int cmp_var = tmp2;         /* Third copy */
        int final_cmp = cmp_var + 0; /* Arithmetic that preserves value */
        
        /* Conditional using the PHI-derived value - will be analyzed */
        if (final_cmp) {  /* if (phi_derived) pattern */
            /* Hot path - heavily executed */
            sum += i * 2;
        } else {
            /* Cold path - rarely executed */
            sum += i / 2;
        }
        
        /* Another conditional with explicit comparison */
        if (cmp_var == 1) {  /* if (phi_derived == 1) pattern */
            sum += i % 100;
        }
    }
    
    return sum;
}

/* ========== Pattern 2: Nested PHI with complex control flow ========== */
int process_nested_phi(int mode, int outer_iter, int inner_iter) {
    int total = 0;
    
    for (int i = 0; i < outer_iter; i++) {
        int outer_flag;
        
        /* Outer conditional creates PHI */
        if (mode == 1) {
            outer_flag = 1;  /* Hot in mode 1 */
        } else {
            outer_flag = 0;  /* Cold in other modes */
        }
        
        /* Copy chain across basic block boundary */
        int chain1 = outer_flag;
        int chain2 = chain1;
        
        for (int j = 0; j < inner_iter; j++) {
            int inner_flag;
            
            /* Inner conditional creates another PHI */
            if (j % 7 == 0) {
                inner_flag = 0;
            } else {
                inner_flag = chain2;  /* Use outer PHI result */
            }
            
            /* More SSA copies */
            int tmp_a = inner_flag;
            int tmp_b = tmp_a;
            int tmp_c = tmp_b;
            
            /* Multiple conditionals using PHI-derived values */
            if (tmp_c != 0) {  /* if (phi_derived != 0) pattern */
                total += i * j;
                
                if (tmp_b == 1) {  /* Nested conditional */
                    total += j * 3;
                }
            }
            
            /* While loop using PHI-derived value */
            int while_ctr = 3;
            while (tmp_a && while_ctr > 0) {  /* while (phi_derived) pattern */
                total += while_ctr;
                while_ctr--;
            }
        }
    }
    
    return total;
}

/* ========== Pattern 3: Switch-like PHI propagation ========== */
int process_switch_pattern(int mode, int size) {
    int result = 0;
    int* data = malloc(size * sizeof(int));
    
    if (!data) return 0;
    
    /* Initialize array with pattern */
    for (int i = 0; i < size; i++) {
        data[i] = i % 100;
    }
    
    /* Process array with PHI-based conditionals */
    for (int i = 0; i < size; i++) {
        int selector;
        
        /* Complex condition creating PHI */
        if (mode == 1) {
            if (i < size * 0.9) {
                selector = 1;  /* Very hot path in mode 1 */
            } else {
                selector = 0;  /* Warm path */
            }
        } else if (mode == 2) {
            selector = (i % 3 == 0) ? 1 : 0;  /* Mixed in mode 2 */
        } else {
            selector = 0;  /* Mostly cold in mode 3 */
        }
        
        /* Long copy chain */
        int v1 = selector;
        int v2 = v1;
        int v3 = v2 + 0;  /* Arithmetic that doesn't change value */
        int v4 = v3;
        int v5 = v4;
        
        /* Conditional with the PHI-derived value */
        if (v5) {
            /* Hot computation path */
            result += data[i] * 2;
            
            /* Nested conditional with another PHI */
            int nested_flag = (data[i] > 50) ? v4 : 0;
            int n1 = nested_flag;
            int n2 = n1;
            
            if (n2 == 1) {
                result += data[i] / 2;
            }
        } else {
            /* Cold computation path */
            result -= data[i];
        }
        
        /* Another use of the PHI-derived value */
        if (v3 == 1) {
            result += i % 10;
        }
    }
    
    free(data);
    return result;
}

/* ========== Pattern 4: Function calls with PHI propagation ========== */
int helper_function(int flag, int value) {
    /* This function will be inlined, creating more complex SSA */
    int local_flag = flag;
    int tmp1 = local_flag;
    int tmp2 = tmp1;
    
    if (tmp2) {
        return value * 3;
    } else {
        return value / 3;
    }
}

int process_with_calls(int mode, int iterations) {
    int accumulator = 0;
    
    for (int i = 0; i < iterations; i++) {
        int call_flag;
        
        /* Create PHI based on mode and iteration */
        if (mode == 1) {
            call_flag = (i % 100 < 95) ? 1 : 0;  /* Mostly hot */
        } else {
            call_flag = (i % 100 < 10) ? 1 : 0;  /* Mostly cold */
        }
        
        /* Copy before call */
        int pre_call = call_flag;
        int pre_call2 = pre_call;
        
        /* Function call with PHI-derived value */
        accumulator += helper_function(pre_call2, i);
        
        /* More copies after call */
        int post_call = pre_call;
        int final_flag = post_call;
        
        /* Conditional using the flag */
        if (final_flag == 1) {
            accumulator += i * i;
        }
    }
    
    return accumulator;
}

/* ========== Pattern 5: Loop-carried PHI dependencies ========== */
int process_loop_carried_phi(int mode, int iterations) {
    int state = 0;
    int total = 0;
    
    for (int i = 0; i < iterations; i++) {
        int loop_flag;
        
        /* PHI that depends on previous iteration */
        if (state > 100) {
            loop_flag = 0;
        } else {
            loop_flag = 1;
        }
        
        /* Copy chain within loop */
        int chain1 = loop_flag;
        int chain2 = chain1;
        int chain3 = chain2 + 0;
        
        /* Update state based on PHI-derived value */
        if (chain3) {
            state += i % 50;
        } else {
            state -= i % 20;
        }
        
        /* Another conditional */
        if (chain2 == 1) {
            total += state;
        }
        
        /* Nested loop with PHI propagation */
        for (int j = 0; j < 5; j++) {
            int inner_flag = (j < chain1) ? 1 : 0;
            int inner_copy = inner_flag;
            
            if (inner_copy) {
                total += j;
            }
        }
    }
    
    return total;
}

/* ========== Main driver with profile generation modes ========== */
int main(int argc, char** argv) {
    int mode = 1;  /* Default to hot mode */
    int result = 0;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    /* Seed for reproducible but varied behavior */
    srand(mode * 12345);
    
    clock_t start = clock();
    
    /* Execute different patterns based on mode to generate varied profiles */
    switch (mode) {
        case 1:  /* HOT MODE - Dominant execution path */
            printf("Executing HOT mode (dominant profile)...\n");
            
            /* Pattern 1: Simple hot loop */
            result += process_hot_loop(mode, HOT_ITERATIONS);
            
            /* Pattern 2: Nested with hot outer */
            result += process_nested_phi(mode, 1000, 100);
            
            /* Pattern 3: Switch-like with hot path */
            result += process_switch_pattern(mode, 50000);
            
            /* Pattern 4: Function calls hot */
            result += process_with_calls(mode, HOT_ITERATIONS / 10);
            
            /* Pattern 5: Loop-carried hot */
            result += process_loop_carried_phi(mode, HOT_ITERATIONS / 5);
            break;
            
        case 2:  /* WARM MODE - Mixed execution */
            printf("Executing WARM mode (mixed profile)...\n");
            
            result += process_hot_loop(mode, WARM_ITERATIONS);
            result += process_nested_phi(mode, 100, 50);
            result += process_switch_pattern(mode, 10000);
            result += process_with_calls(mode, WARM_ITERATIONS / 5);
            result += process_loop_carried_phi(mode, WARM_ITERATIONS / 2);
            break;
            
        case 3:  /* COLD MODE - Rare execution */
        default:
            printf("Executing COLD mode (rare profile)...\n");
            
            result += process_hot_loop(mode, COLD_ITERATIONS);
            result += process_nested_phi(mode, 10, 5);
            result += process_switch_pattern(mode, 1000);
            result += process_with_calls(mode, COLD_ITERATIONS);
            result += process_loop_carried_phi(mode, COLD_ITERATIONS);
            break;
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    /* Output checksum and timing for verification */
    printf("Result checksum: %d\n", result);
    printf("Execution time: %.3f seconds\n", elapsed);
    
    return 0;
}
