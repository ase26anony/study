/* autofdo_phi_conditional_test.c
 * 
 * This program generates specific control flow patterns to trigger
 * GCC's AutoFDO PHI-to-conditional analysis in auto-profile.cc.
 * It creates boolean values (0/1) flowing through PHI nodes into
 * conditional comparisons with SSA copy chains in between.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Function 1: Simple PHI-to-conditional with direct copy chain */
int phi_conditional_simple(int mode, int iterations) {
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        int flag;
        
        /* Create two predecessor blocks with different flag values */
        if (mode == 1) {
            /* Hot path - executed frequently */
            flag = 1;
        } else {
            /* Cold path - executed rarely */
            flag = 0;
        }
        
        /* PHI node conceptually created here (compiler will generate) */
        int phi_result = flag;
        
        /* Create SSA copy chain to trigger the while loop in uncovered code */
        int tmp1 = phi_result;      /* First copy */
        int tmp2 = tmp1;            /* Second copy */
        int cmp_var = tmp2;         /* Third copy */
        int final_var = cmp_var + 0; /* Arithmetic that preserves value */
        
        /* Conditional using the PHI-derived value - triggers uncovered code */
        if (final_var) {  /* if (phi_derived) - direct use */
            result += i * 2;  /* Hot computation */
        } else {
            result += i;      /* Cold computation */
        }
    }
    
    return result;
}

/* Function 2: Nested PHI with multiple comparison types */
int phi_conditional_complex(int mode, int iterations) {
    int result = 0;
    int outer_flag = (mode > 0) ? 1 : 0;
    
    for (int i = 0; i < iterations; i++) {
        int inner_flag;
        
        /* Two different predecessor paths */
        if (i % 100 == 0) {
            inner_flag = 1;
        } else {
            inner_flag = outer_flag;
        }
        
        /* PHI node for inner_flag */
        int phi_val = inner_flag;
        
        /* Longer SSA copy chain */
        int chain1 = phi_val;
        int chain2 = chain1;
        int chain3 = chain2;
        int chain4 = chain3;
        int compare_val = chain4;
        
        /* Multiple comparison types in different basic blocks */
        if (compare_val == 1) {  /* Explicit equality comparison */
            result += i * 3;
            
            /* Nested conditional with same value */
            int tmp = compare_val;
            if (tmp != 0) {  /* Inequality comparison */
                result += 7;
            }
        } else if (compare_val == 0) {
            result += i / 2;
        }
        
        /* Use in loop-like condition */
        int loop_flag = compare_val;
        int counter = 0;
        while (loop_flag && counter < 5) {  /* While condition */
            result += counter;
            counter++;
            /* Break the loop flag */
            if (counter > 2) {
                int break_flag = loop_flag;
                if (break_flag) {
                    loop_flag = 0;  /* Change to break loop */
                }
            }
        }
    }
    
    return result;
}

/* Function 3: PHI across function boundaries */
int helper_function(int base, int selector) {
    int local_flag;
    
    /* Different call sites provide different values */
    if (selector == 1) {
        local_flag = 1;
    } else if (selector == 2) {
        local_flag = 0;
    } else {
        local_flag = (base % 2);
    }
    
    /* PHI node here */
    int phi_result = local_flag;
    
    /* Copy chain */
    int a = phi_result;
    int b = a;
    int c = b;
    
    return c;
}

int phi_cross_function(int mode, int iterations) {
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        int flag_from_helper;
        
        /* Call from hot path */
        if (mode == 1 && i < iterations * 9 / 10) {
            flag_from_helper = helper_function(i, 1);  /* Usually returns 1 */
        }
        /* Call from cold path */
        else if (mode == 2) {
            flag_from_helper = helper_function(i, 2);  /* Usually returns 0 */
        }
        /* Mixed calls */
        else {
            flag_from_helper = helper_function(i, 3);  /* Mixed returns */
        }
        
        /* PHI for the return value */
        int phi_flag = flag_from_helper;
        
        /* Multi-step copy chain */
        int x1 = phi_flag;
        int x2 = x1;
        int x3 = x2;
        int x4 = x3;
        int cond_var = x4;
        
        /* Conditional with the PHI-derived value */
        if (cond_var) {
            result += i * i;
        } else {
            result += i;
        }
        
        /* Another use with different comparison */
        if (cond_var == 1) {
            result += 100;
        }
    }
    
    return result;
}

/* Function 4: Array processing with PHI-based conditions */
int phi_array_processing(int mode, int size) {
    int* data = (int*)malloc(size * sizeof(int));
    int result = 0;
    
    /* Initialize array with pattern */
    for (int i = 0; i < size; i++) {
        data[i] = i % 100;
    }
    
    int threshold = (mode == 1) ? 50 : 80;
    
    for (int i = 0; i < size; i++) {
        int is_high;
        
        /* Two different conditions creating different predecessor blocks */
        if (data[i] > threshold) {
            is_high = 1;
        } else {
            is_high = 0;
        }
        
        /* PHI node */
        int phi_high = is_high;
        
        /* Copy chain that crosses basic block boundaries */
        int copy1 = phi_high;
        if (i % 3 == 0) {
            int copy2 = copy1;
            int copy3 = copy2;
            int final_flag = copy3;
            
            if (final_flag) {
                result += data[i] * 2;
            }
        } else {
            int copy2 = copy1;
            int final_flag = copy2;
            
            if (final_flag == 1) {
                result += data[i] * 3;
            }
        }
        
        /* Another PHI in the same loop */
        int needs_process;
        if (i % 7 == 0) {
            needs_process = 1;
        } else {
            needs_process = phi_high;  /* Depends on previous PHI */
        }
        
        int phi_process = needs_process;
        int chain_a = phi_process;
        int chain_b = chain_a;
        
        if (chain_b != 0) {
            result += 1;
        }
    }
    
    free(data);
    return result;
}

/* Function 5: Complex control flow with multiple PHIs */
int complex_phi_network(int mode, int iterations) {
    int result = 0;
    int state = 0;
    
    for (int i = 0; i < iterations; i++) {
        int flag_a, flag_b;
        
        /* Multiple predecessor blocks with different assignments */
        switch (state) {
            case 0:
                flag_a = 1;
                flag_b = 0;
                state = (mode == 1) ? 1 : 2;
                break;
            case 1:
                flag_a = 0;
                flag_b = 1;
                state = (i % 3 == 0) ? 0 : 2;
                break;
            case 2:
                flag_a = 1;
                flag_b = 1;
                state = 0;
                break;
        }
        
        /* Multiple PHI nodes */
        int phi_a = flag_a;
        int phi_b = flag_b;
        
        /* Interdependent copy chains */
        int tmp1 = phi_a;
        int tmp2 = phi_b;
        int tmp3 = tmp1;
        int tmp4 = tmp2;
        int combined = tmp3 && tmp4;
        
        /* Conditional using combined PHI values */
        if (combined) {
            result += i * 5;
        }
        
        /* Separate use of individual PHI values */
        int check_a = tmp3;
        if (check_a == 1) {
            result += 10;
        }
        
        int check_b = tmp4;
        if (check_b) {
            result += 20;
        }
    }
    
    return result;
}

/* Main function with profile-generating runtime behavior */
int main(int argc, char** argv) {
    int mode = 1;  /* Default to hot mode */
    int iterations = 1000000;
    
    /* Parse command line arguments */
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    printf("Running mode %d with %d iterations\n", mode, iterations);
    
    int result = 0;
    clock_t start = clock();
    
    /* Warm-up phase with mixed behavior */
    if (mode == 0) {
        /* Mixed mode - exercises all paths */
        for (int phase = 0; phase < 3; phase++) {
            result += phi_conditional_simple(phase % 2, iterations / 10);
            result += phi_conditional_complex(phase, iterations / 20);
        }
    }
    
    /* Main execution with mode-specific behavior */
    if (mode == 1) {
        /* HOT MODE - Dominant hot paths */
        printf("Executing hot paths...\n");
        
        /* Heavily executed hot path */
        result += phi_conditional_simple(1, iterations);
        
        /* Still hot but with some variation */
        result += phi_conditional_complex(1, iterations / 2);
        
        /* Hot function calls */
        result += phi_cross_function(1, iterations);
        
        /* Array processing with hot threshold */
        result += phi_array_processing(1, iterations / 10);
        
        /* Complex network mostly taking hot branches */
        result += complex_phi_network(1, iterations / 5);
        
    } else if (mode == 2) {
        /* COLD MODE - Mostly cold paths */
        printf("Executing cold paths...\n");
        
        /* Mostly cold path */
        result += phi_conditional_simple(0, iterations / 100);
        
        /* Mixed but leaning cold */
        result += phi_conditional_complex(0, iterations / 50);
        
        /* Cold function calls */
        result += phi_cross_function(2, iterations / 100);
        
        /* Array processing with cold threshold */
        result += phi_array_processing(2, iterations / 20);
        
        /* Complex network taking cold branches */
        result += complex_phi_network(0, iterations / 10);
        
    } else if (mode == 3) {
        /* BALANCED MODE - Equal mix */
        printf("Executing balanced paths...\n");
        
        /* Alternate between hot and cold */
        for (int i = 0; i < 10; i++) {
            result += phi_conditional_simple(i % 2, iterations / 10);
        }
        
        result += phi_conditional_complex(3, iterations / 2);
        result += phi_cross_function(3, iterations / 2);
        result += phi_array_processing(3, iterations / 10);
        result += complex_phi_network(3, iterations / 5);
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Result: %d\n", result);
    printf("Time elapsed: %.2f seconds\n", elapsed);
    
    /* Output checksum for verification */
    unsigned int checksum = 0;
    unsigned char* bytes = (unsigned char*)&result;
    for (size_t i = 0; i < sizeof(result); i++) {
        checksum += bytes[i];
    }
    printf("Checksum: %u\n", checksum);
    
    return 0;
}
