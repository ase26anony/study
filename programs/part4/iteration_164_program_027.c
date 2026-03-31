/* autofdo_phi_conditional.c
 * 
 * This program generates specific control flow patterns to trigger
 * GCC's AutoFDO profile analysis for PHI-to-conditional propagation.
 * It creates boolean values (0/1) that flow through PHI nodes into
 * conditional comparisons with SSA copy chains in between.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HOT_ITERATIONS 10000000
#define WARM_ITERATIONS 1000000
#define COLD_ITERATIONS 100

/* Function 1: Simple PHI-to-conditional with direct copy chain */
int process_with_phi_chain(int mode, int iterations) {
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        int flag;
        
        /* Create two predecessor blocks with different flag values */
        if (i % 100 < 95) {  /* Hot path - 95% of iterations */
            flag = 1;  /* Set to 1 in hot predecessor */
        } else {
            flag = 0;  /* Set to 0 in cold predecessor */
        }
        
        /* PHI node conceptually merges the two values */
        int phi_result = flag;
        
        /* Create SSA copy chain to trigger the while loop walking */
        int tmp1 = phi_result;
        int tmp2 = tmp1;
        int tmp3 = tmp2 + 0;  /* Arithmetic that doesn't break single-assignment */
        int cmp_var = tmp3;
        
        /* Conditional using PHI-derived value directly */
        if (cmp_var) {  /* This becomes: if (cmp_var != 0) */
            /* Hot path - heavily executed */
            result += i * 2;
        } else {
            /* Cold path - rarely executed */
            result -= i;
        }
        
        /* Another conditional with explicit comparison */
        if (cmp_var == 1) {
            result += i % 100;
        }
    }
    
    return result;
}

/* Function 2: Nested PHI patterns with multiple branches */
int nested_phi_patterns(int mode, int iterations) {
    int total = 0;
    int outer_flag, inner_flag;
    
    for (int i = 0; i < iterations; i++) {
        /* Outer PHI pattern */
        if (mode == 1) {
            outer_flag = (i % 10 == 0) ? 1 : 0;
        } else {
            outer_flag = (i % 1000 == 0) ? 1 : 0;
        }
        
        /* SSA copy chain for outer flag */
        int outer_tmp1 = outer_flag;
        int outer_tmp2 = outer_tmp1;
        int outer_cmp = outer_tmp2;
        
        /* Inner PHI pattern - depends on outer condition */
        if (outer_cmp) {
            if (i % 3 == 0) {
                inner_flag = 1;
            } else {
                inner_flag = 0;
            }
            
            /* Another SSA chain for inner flag */
            int inner_tmp = inner_flag;
            int inner_cmp = inner_tmp;
            
            /* Conditional using inner PHI-derived value */
            while (inner_cmp && total < 1000000) {
                total += i % 17;
                inner_cmp = 0;  /* Break after one iteration */
            }
            
            if (inner_flag != 0) {
                total += i * 3;
            }
        } else {
            total += i % 23;
        }
        
        /* Multiple comparison types */
        if (outer_flag == 1) {
            total += 7;
        } else if (outer_flag != 0) {
            total += 3;
        }
    }
    
    return total;
}

/* Function 3: Complex PHI network with array processing */
int array_based_phi_analysis(int* data, int size, int threshold) {
    int sum = 0;
    int hot_count = 0;
    int cold_count = 0;
    
    for (int i = 0; i < size; i++) {
        int is_hot;
        
        /* PHI predecessor 1: data-based condition */
        if (data[i] > threshold) {
            is_hot = 1;
            hot_count++;
        } 
        /* PHI predecessor 2: index-based condition */
        else if (i % 100 == 0) {
            is_hot = 1;
            hot_count++;
        }
        /* PHI predecessor 3: default cold path */
        else {
            is_hot = 0;
            cold_count++;
        }
        
        /* Multi-step SSA copy chain */
        int chain1 = is_hot;
        int chain2 = chain1;
        int chain3 = chain2 + 0;
        int chain4 = chain3;
        int final_flag = chain4;
        
        /* Conditional branch using the PHI-derived flag */
        if (final_flag) {
            /* Hot path - process intensively */
            for (int j = 0; j < 10; j++) {
                sum += data[i] * j;
            }
        } else {
            /* Cold path - minimal processing */
            sum += data[i];
        }
        
        /* Another conditional to create multiple annotated blocks */
        if (final_flag == 1 && i % 2 == 0) {
            sum += i;
        }
    }
    
    return sum + hot_count - cold_count;
}

/* Function 4: Recursive PHI pattern with function calls */
int recursive_phi_helper(int depth, int value, int* toggle) {
    if (depth <= 0) {
        return value;
    }
    
    int flag;
    
    /* Different PHI predecessors based on toggle */
    if (*toggle) {
        flag = 1;
        *toggle = 0;
    } else {
        flag = 0;
        *toggle = 1;
    }
    
    /* SSA copies */
    int tmp_flag = flag;
    int cmp_flag = tmp_flag;
    
    int result = value;
    
    if (cmp_flag) {
        /* Hot recursive path */
        result += recursive_phi_helper(depth - 1, value * 2, toggle);
    } else {
        /* Cold recursive path */
        result += recursive_phi_helper(depth - 1, value / 2, toggle);
    }
    
    /* Additional conditional */
    if (flag == 1 && depth > 2) {
        result += 100;
    }
    
    return result;
}

int recursive_phi_pattern(int iterations) {
    int total = 0;
    int toggle = 1;
    
    for (int i = 0; i < iterations; i++) {
        total += recursive_phi_helper(5, i, &toggle);
        
        /* Create varying profile for different call sites */
        if (i % 1000 == 0) {
            total += recursive_phi_helper(3, i * 2, &toggle);
        }
    }
    
    return total;
}

/* Main function with profile-generating runtime behavior */
int main(int argc, char** argv) {
    int mode = 1;  /* Default to hot mode */
    int iterations = HOT_ITERATIONS;
    
    /* Parse command line arguments */
    if (argc > 1) {
        mode = atoi(argv[1]);
        if (argc > 2) {
            iterations = atoi(argv[2]);
        }
    }
    
    printf("Running AutoFDO PHI pattern test in mode %d with %d iterations\n", 
           mode, iterations);
    
    int result = 0;
    clock_t start = clock();
    
    /* Select execution mode to create different profile patterns */
    switch (mode) {
        case 1:  /* Hot mode - heavily exercises hot paths */
            printf("Mode 1: Dominant hot path execution\n");
            result += process_with_phi_chain(1, iterations);
            result += nested_phi_patterns(1, iterations / 10);
            break;
            
        case 2:  /* Warm mode - balanced execution */
            printf("Mode 2: Balanced path execution\n");
            result += process_with_phi_chain(2, iterations / 10);
            result += nested_phi_patterns(2, iterations / 100);
            break;
            
        case 3:  /* Cold mode - exercises cold paths */
            printf("Mode 3: Cold path emphasis\n");
            result += process_with_phi_chain(3, COLD_ITERATIONS);
            result += nested_phi_patterns(3, COLD_ITERATIONS);
            break;
            
        case 4:  /* Array processing mode */
            printf("Mode 4: Array-based PHI analysis\n");
            {
                int size = iterations / 100;
                int* data = malloc(size * sizeof(int));
                for (int i = 0; i < size; i++) {
                    data[i] = rand() % 1000;
                }
                result += array_based_phi_analysis(data, size, 500);
                free(data);
            }
            break;
            
        case 5:  /* Recursive mode */
            printf("Mode 5: Recursive PHI patterns\n");
            result += recursive_phi_pattern(iterations / 1000);
            break;
            
        default:
            printf("Unknown mode, using default\n");
            result += process_with_phi_chain(1, WARM_ITERATIONS);
    }
    
    /* Mix all patterns for comprehensive coverage */
    if (mode != 4) {
        int small_array[1000];
        for (int i = 0; i < 1000; i++) {
            small_array[i] = i;
        }
        result += array_based_phi_analysis(small_array, 1000, 500);
    }
    
    if (mode != 5) {
        result += recursive_phi_pattern(1000);
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Result checksum: %d\n", result);
    printf("Execution time: %.2f seconds\n", elapsed);
    
    return 0;
}
