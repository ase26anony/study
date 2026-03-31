/* test_autofdo_phi_cond.c
 * Generates PHI-to-conditional patterns for AutoFDO coverage
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HOT_ITERATIONS 1000000
#define WARM_ITERATIONS 10000
#define COLD_ITERATIONS 10

/* Function 1: Simple PHI with direct conditional */
int process_with_phi_direct(int mode, int iterations) {
    int result = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        int flag;
        
        /* Create PHI node with different values from different predecessors */
        if (mode == 1) {
            /* Hot path - sets flag to 1 */
            flag = 1;
        } else {
            /* Cold path - sets flag to 0 */
            flag = 0;
        }
        
        /* PHI node effectively created here in SSA */
        int tmp1 = flag;      /* First SSA copy */
        int tmp2 = tmp1;      /* Second SSA copy */
        int cmp_var = tmp2;   /* Third SSA copy - triggers while loop */
        
        /* Direct conditional use of PHI-derived value */
        if (cmp_var) {        /* Should become: if (1) for mode=1, if (0) for mode=2 */
            result += i * 2;  /* Hot computation */
        } else {
            result += i;      /* Cold computation */
        }
    }
    
    return result;
}

/* Function 2: Complex PHI chain with equality comparison */
int process_with_phi_equality(int mode, int iterations) {
    int result = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        int base_flag;
        
        /* Different values from different paths */
        if (i % 100 == 0) {
            base_flag = (mode == 1) ? 1 : 0;
        } else {
            base_flag = (mode == 2) ? 0 : 1;
        }
        
        /* Create longer SSA copy chain */
        int chain1 = base_flag;
        int chain2 = chain1 + 0;  /* Arithmetic that preserves value */
        int chain3 = chain2;
        int chain4 = chain3;
        int final_flag = chain4;
        
        /* Explicit equality comparison with constant 1 */
        if (final_flag == 1) {
            /* Hot path for mode=1 */
            result += (i * 3) / 2;
        } else if (final_flag != 0) {
            /* Should never execute with our 0/1 values */
            result -= i;
        } else {
            /* Cold path for mode=2 */
            result += i / 2;
        }
    }
    
    return result;
}

/* Function 3: Nested PHI patterns with loop conditions */
int process_nested_phi(int mode, int iterations) {
    int result = 0;
    int outer_i;
    
    for (outer_i = 0; outer_i < iterations / 100; outer_i++) {
        int inner_flag;
        
        /* PHI from multiple conditions */
        if (mode == 1) {
            if (outer_i % 3 == 0) {
                inner_flag = 1;
            } else {
                inner_flag = 1;  /* Still hot in mode 1 */
            }
        } else if (mode == 2) {
            if (outer_i % 10 == 0) {
                inner_flag = 0;
            } else {
                inner_flag = 0;  /* Still cold in mode 2 */
            }
        } else {
            inner_flag = (outer_i % 2 == 0) ? 1 : 0;
        }
        
        /* Multi-step SSA propagation */
        int propagate1 = inner_flag;
        int propagate2 = propagate1;
        int propagate3 = propagate2 + 0;  /* Preserves value */
        int loop_flag = propagate3;
        
        /* Use in loop condition */
        int inner_iter = 100;
        while (inner_iter-- > 0 && loop_flag) {
            result += outer_i * inner_iter;
            
            /* Nested conditional with same flag */
            int tmp_flag = loop_flag;
            if (tmp_flag == 1) {
                result += 1;
            }
        }
        
        /* Another use of the same PHI-derived value */
        int check_flag = loop_flag;
        if (check_flag != 0) {
            result += outer_i;
        }
    }
    
    return result;
}

/* Function 4: PHI across function boundaries */
static int helper_phi_source(int selector, int mode) {
    int value;
    
    /* Different return values create PHI at call site */
    if (selector > 0) {
        value = (mode == 1) ? 1 : 0;
    } else {
        value = (mode == 2) ? 0 : 1;
    }
    
    /* SSA copy chain within helper */
    int local_copy = value;
    return local_copy + 0;  /* Return with arithmetic */
}

int process_with_call_phi(int mode, int iterations) {
    int result = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Call creates PHI at call site merging different values */
        int flag = helper_phi_source(i % 5, mode);
        
        /* Propagate through SSA copies */
        int copy1 = flag;
        int copy2 = copy1;
        
        /* Multiple conditional uses */
        if (copy2) {
            result += i * i;
        }
        
        if (copy2 == 1) {
            result += i;
        }
    }
    
    return result;
}

/* Function 5: Array processing with PHI-dependent paths */
int process_array_with_phi(int mode, int size) {
    int* data = (int*)malloc(size * sizeof(int));
    int result = 0;
    int i;
    
    /* Initialize array */
    for (i = 0; i < size; i++) {
        data[i] = i % 100;
    }
    
    /* Process with PHI-dependent branching */
    for (i = 0; i < size; i++) {
        int threshold_flag;
        
        /* PHI based on mode and array value */
        if (mode == 1) {
            threshold_flag = (data[i] > 50) ? 1 : 1;  /* Always 1 in hot mode */
        } else {
            threshold_flag = (data[i] > 50) ? 0 : 0;  /* Always 0 in cold mode */
        }
        
        /* SSA propagation chain */
        int t1 = threshold_flag;
        int t2 = t1;
        int t3 = t2 + 0;
        int final_flag = t3;
        
        /* Branch on PHI-derived value */
        if (final_flag) {
            /* Hot path for mode=1 */
            result += data[i] * 3;
        } else {
            /* Cold path for mode=2 */
            result += data[i];
        }
        
        /* Another use in switch-like pattern */
        int check = final_flag;
        if (check == 1) {
            result += 1;
        } else if (check == 0) {
            result -= 1;
        }
    }
    
    free(data);
    return result;
}

/* Main function with profile-generating behavior */
int main(int argc, char** argv) {
    int mode = 1;  /* Default to hot mode */
    int total_result = 0;
    
    /* Parse command line for mode */
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    printf("Running in mode %d\n", mode);
    
    /* Warm-up phase - establish baseline profile */
    clock_t start = clock();
    
    /* Phase 1: Simple PHI patterns */
    total_result += process_with_phi_direct(mode, WARM_ITERATIONS);
    
    /* Phase 2: Complex PHI chains */
    total_result += process_with_phi_equality(mode, WARM_ITERATIONS / 2);
    
    /* Main measurement phase - heavily biased by mode */
    int main_iterations = (mode == 1) ? HOT_ITERATIONS : COLD_ITERATIONS;
    
    /* Execute hot/cold paths based on mode */
    total_result += process_with_phi_direct(mode, main_iterations);
    total_result += process_nested_phi(mode, main_iterations / 10);
    total_result += process_with_call_phi(mode, main_iterations / 5);
    total_result += process_array_with_phi(mode, main_iterations / 100);
    
    /* Mixed mode phase - some of both */
    if (mode == 3) {
        /* Mixed profile - triggers both paths */
        total_result += process_with_phi_direct(1, HOT_ITERATIONS / 2);
        total_result += process_with_phi_direct(2, HOT_ITERATIONS / 10);
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Result: %d\n", total_result);
    printf("Time: %.2f seconds\n", elapsed);
    
    /* Verification checksum */
    int checksum = total_result % 1000000;
    printf("Checksum: %06d\n", checksum);
    
    return 0;
}
