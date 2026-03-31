/* autofdo_phi_conditional_test.c
 * 
 * This program generates specific control flow patterns to trigger
 * GCC's AutoFDO profile analysis for PHI-to-conditional transformations.
 * It creates boolean values (0/1) that flow through PHI nodes into
 * conditional comparisons with intermediate SSA copy chains.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Global variables to prevent optimization */
volatile int global_counter = 0;
volatile int global_mode = 0;
volatile int global_seed = 42;

/* Function 1: Simple PHI-to-conditional with direct copy chain */
int process_mode1(int iterations, int threshold) {
    int result = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        int condition_value;
        int tmp1, tmp2, tmp3;
        
        /* Create PHI node with boolean values from different paths */
        if (i % 100 < threshold) {
            /* Hot path - sets value to 1 */
            condition_value = 1;
        } else {
            /* Cold path - sets value to 0 */
            condition_value = 0;
        }
        
        /* Create SSA copy chain to trigger the while loop in auto-profile.cc */
        tmp1 = condition_value;      /* First copy */
        tmp2 = tmp1;                 /* Second copy */
        tmp3 = tmp2 + 0;             /* Third copy with arithmetic that doesn't change value */
        
        /* Use in conditional - direct boolean test */
        if (tmp3) {  /* This becomes: if (tmp3 != 0) */
            /* Hot path - executed frequently */
            result += i * 2;
            global_counter++;
        } else {
            /* Cold path - rarely executed */
            result -= i;
        }
        
        /* Another PHI pattern with explicit comparison */
        int another_value;
        if ((i + global_seed) % 50 < 25) {
            another_value = 1;
        } else {
            another_value = 0;
        }
        
        int copy1 = another_value;
        int copy2 = copy1;
        
        /* Explicit equality comparison with constant 1 */
        if (copy2 == 1) {
            result += i % 100;
        }
    }
    
    return result;
}

/* Function 2: Nested loops with complex PHI patterns */
int process_mode2(int outer_iter, int inner_iter) {
    int total = 0;
    int i, j;
    
    for (i = 0; i < outer_iter; i++) {
        /* Outer loop PHI pattern */
        int outer_flag;
        if (i % 3 == 0) {
            outer_flag = 1;
        } else {
            outer_flag = 0;
        }
        
        /* Multi-step copy chain */
        int chain1 = outer_flag;
        int chain2 = chain1;
        int chain3 = chain2;
        int chain4 = chain3 + 0;  /* Preserves SSA copy pattern */
        
        for (j = 0; j < inner_iter; j++) {
            /* Inner loop PHI pattern */
            int inner_flag;
            if ((i + j) % 10 < 5) {
                inner_flag = 1;
            } else {
                inner_flag = 0;
            }
            
            /* Another copy chain */
            int inner_copy1 = inner_flag;
            int inner_copy2 = inner_copy1;
            
            /* Use in while-like condition */
            int loop_control = inner_copy2;
            int k = 0;
            while (loop_control && k < 5) {
                total += (i * j + k);
                k++;
                /* Break early sometimes */
                if (k > 2 && (j % 3 == 0)) {
                    loop_control = 0;  /* Changes control flow */
                }
            }
            
            /* Complex conditional with PHI-derived value */
            if (chain4 && inner_copy2) {
                total += 1000;  /* Hot path */
            } else if (chain4 || inner_copy2) {
                total += 100;   /* Warm path */
            } else {
                total += 1;     /* Cold path */
            }
        }
        
        /* Use outer flag in conditional */
        if (chain4 != 0) {  /* Another way to test boolean */
            total += i * 100;
        }
    }
    
    return total;
}

/* Function 3: Recursive pattern with PHI propagation */
int recursive_phi_helper(int depth, int max_depth, int toggle) {
    if (depth >= max_depth) {
        return 1;
    }
    
    int result = 0;
    int branch_flag;
    
    /* PHI-like decision based on toggle */
    if (toggle) {
        branch_flag = 1;
    } else {
        branch_flag = 0;
    }
    
    /* Copy chain that crosses basic block boundaries */
    int copy_a = branch_flag;
    int copy_b = copy_a;
    
    if (copy_b) {
        /* Hot recursive path */
        result += recursive_phi_helper(depth + 1, max_depth, toggle ^ 1);
        result += recursive_phi_helper(depth + 1, max_depth / 2, toggle);
    } else {
        /* Cold recursive path */
        result += recursive_phi_helper(depth + 1, 2, toggle);
    }
    
    /* Another conditional using the same PHI-derived value */
    int copy_c = copy_b;
    if (copy_c == 1) {
        result += depth * 10;
    }
    
    return result;
}

int process_mode3(int iterations) {
    int sum = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        int toggle = (i % 2 == 0) ? 1 : 0;
        sum += recursive_phi_helper(0, 8, toggle);
        
        /* Additional PHI pattern in loop */
        int loop_flag;
        if (sum % 1000 > 500) {
            loop_flag = 1;
        } else {
            loop_flag = 0;
        }
        
        int chain_x = loop_flag;
        int chain_y = chain_x;
        int chain_z = chain_y;
        
        if (chain_z) {
            sum += i * 100;
        } else {
            sum += i;
        }
    }
    
    return sum;
}

/* Function 4: Array processing with data-dependent PHI nodes */
int process_mode4(int size) {
    int* data = (int*)malloc(size * sizeof(int));
    int i, result = 0;
    
    /* Initialize with pattern */
    for (i = 0; i < size; i++) {
        data[i] = (i * 17) % 100;
    }
    
    /* Process array with PHI-to-conditional patterns */
    for (i = 1; i < size - 1; i++) {
        int edge_flag;
        
        /* PHI based on array values */
        if (data[i] > data[i-1] && data[i] > data[i+1]) {
            edge_flag = 1;  /* Local maximum */
        } else if (data[i] < data[i-1] && data[i] < data[i+1]) {
            edge_flag = 0;  /* Local minimum */
        } else {
            /* Neither - use previous flag with PHI */
            static int prev_flag = 0;
            edge_flag = prev_flag;
            prev_flag = edge_flag ^ 1;  /* Toggle for next iteration */
        }
        
        /* Multi-block copy chain */
        int tmp_a = edge_flag;
        int tmp_b = tmp_a;
        int tmp_c = tmp_b + 0;
        int tmp_d = tmp_c;
        
        /* Use in conditional with different comparison types */
        if (tmp_d) {
            result += data[i] * 3;  /* Hot for maxima */
        } else if (tmp_d == 0) {
            result += data[i];      /* Warm for minima */
        }
        
        /* Another PHI for direction */
        int dir_flag;
        if (data[i] > 50) {
            dir_flag = 1;
        } else {
            dir_flag = 0;
        }
        
        int dir_copy = dir_flag;
        if (dir_copy != 0) {
            result += i;
        }
    }
    
    free(data);
    return result;
}

/* Function 5: Mixed patterns for comprehensive coverage */
int process_mode5(int base_iterations) {
    int total = 0;
    int phase = 0;
    
    while (phase < 3) {
        int phase_flag;
        
        /* PHI based on phase */
        switch (phase) {
            case 0: phase_flag = 1; break;
            case 1: phase_flag = 0; break;
            case 2: phase_flag = (total % 2); break;
        }
        
        /* Extended copy chain */
        int v1 = phase_flag;
        int v2 = v1;
        int v3 = v2;
        int v4 = v3 + 0;
        int v5 = v4;
        
        for (int i = 0; i < base_iterations; i++) {
            /* Inner PHI pattern */
            int inner_flag;
            if ((i + phase) % 7 < 3) {
                inner_flag = 1;
            } else {
                inner_flag = 0;
            }
            
            int inner_copy = inner_flag;
            
            /* Complex conditional network */
            if (v5 && inner_copy) {
                total += i * 7;  /* Very hot */
            } else if (v5) {
                total += i * 3;  /* Hot */
            } else if (inner_copy) {
                total += i;      /* Warm */
            } else {
                total += 1;      /* Cold */
            }
            
            /* Nested conditionals with same PHI source */
            if (v5) {
                if (inner_copy == 1) {
                    total += 10000;
                }
            }
        }
        
        phase++;
    }
    
    return total;
}

/* Main function with profile-generating runtime behavior */
int main(int argc, char** argv) {
    int mode = 1;  /* Default to hot mode */
    int iterations = 1000000;
    int result = 0;
    
    /* Parse command line arguments */
    if (argc > 1) {
        mode = atoi(argv[1]);
        if (argc > 2) {
            iterations = atoi(argv[2]);
        }
    }
    
    printf("Running mode %d with %d iterations\n", mode, iterations);
    
    /* Different execution modes create different profile patterns */
    switch (mode) {
        case 1:  /* Hot mode - heavily exercises PHI-to-conditional hot paths */
            result = process_mode1(iterations, 90);  /* 90% hot path */
            result += process_mode2(100, iterations / 100);
            break;
            
        case 2:  /* Balanced mode - mixed hot/cold paths */
            result = process_mode1(iterations, 50);  /* 50% hot path */
            result += process_mode3(iterations / 10);
            break;
            
        case 3:  /* Cold mode - mostly cold paths */
            result = process_mode1(iterations, 10);  /* 10% hot path */
            result += process_mode4(iterations / 10);
            break;
            
        case 4:  /* Comprehensive mode - all patterns */
            result = process_mode1(iterations / 2, 70);
            result += process_mode2(50, iterations / 50);
            result += process_mode3(iterations / 20);
            result += process_mode4(iterations / 5);
            result += process_mode5(iterations / 10);
            break;
            
        default:
            printf("Invalid mode. Using mode 1.\n");
            result = process_mode1(iterations, 90);
            break;
    }
    
    /* Add some noise to prevent optimization */
    result ^= (global_counter * 31);
    result += (clock() % 1000);
    
    printf("Result: %d\n", result);
    printf("Global counter: %d\n", global_counter);
    
    return result != 0 ? 0 : 1;  /* Return 0 for success if result is non-zero */
}
