/* auto_profile_test.c
 * 
 * This program generates execution patterns to trigger GCC's AutoFDO
 * PHI-to-conditional analysis in auto-profile.cc lines 1312-1333.
 * 
 * Compilation and usage:
 * 1. First compilation: gcc -O2 -fauto-profile auto_profile_test.c -o auto_profile_test
 * 2. Run with dominant mode: ./auto_profile_test 1
 * 3. Recompile with profile: gcc -O2 -fauto-profile -Wauto-profile auto_profile_test.c -o auto_profile_test_opt
 * 4. Run optimized version: ./auto_profile_test_opt 1
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* Global variables to prevent optimization */
volatile int global_seed = 42;
volatile int global_result = 0;

/* Function 1: Hot path with PHI-to-conditional pattern through assignment chain */
int process_hot_path(int iterations, int mode) {
    int sum = 0;
    
    /* Outer loop with many iterations to create hot basic blocks */
    for (int i = 0; i < iterations; i++) {
        int phi_value;
        int tmp1, tmp2, tmp3;
        
        /* Create branching that leads to PHI node */
        if (i % 3 == 0) {
            /* Path A: sets value to 1 */
            phi_value = 1;
        } else if (i % 3 == 1) {
            /* Path B: sets value to 0 */
            phi_value = 0;
        } else {
            /* Path C: sets value based on mode */
            phi_value = (mode == 1) ? 1 : 0;
        }
        
        /* PHI node is implicitly created here by compiler */
        int phi_result = phi_value;
        
        /* Create SSA assignment chain to trigger the while loop in uncovered code */
        tmp1 = phi_result;          /* First assignment */
        tmp2 = tmp1;                /* Second assignment - SSA copy */
        tmp3 = tmp2 + 0;            /* Third assignment - arithmetic that doesn't break pattern */
        int cmp_var = tmp3;         /* Fourth assignment - final variable for comparison */
        
        /* Conditional branch with constant comparison (0/1) */
        if (cmp_var == 1) {         /* This triggers gimple_cond with constant rhs */
            /* Hot path - executed frequently */
            sum += i * 2;
            
            /* Nested condition with same pattern */
            int nested_tmp = cmp_var;
            if (nested_tmp) {       /* Direct use in if condition */
                sum += i % 7;
            }
        } else if (cmp_var == 0) {  /* Another constant comparison */
            /* Less frequent path */
            sum += i % 5;
            
            /* Chain of assignments for else path */
            int else_tmp1 = cmp_var;
            int else_tmp2 = else_tmp1;
            if (!else_tmp2) {       /* Negation of constant */
                sum += 1;
            }
        }
        
        /* Additional complexity to prevent optimization */
        if (sum > 1000000) {
            sum = sum % 1000000;
        }
    }
    
    return sum;
}

/* Function 2: Cold path with different PHI patterns */
int process_cold_path(int iterations, int threshold) {
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        int selector;
        
        /* Complex branching for PHI creation */
        if (i < threshold) {
            selector = 1;
        } else if (i < threshold * 2) {
            selector = 0;
        } else {
            selector = (i % 2 == 0) ? 1 : 0;
        }
        
        /* PHI node */
        int phi_val = selector;
        
        /* Longer assignment chain */
        int chain1 = phi_val;
        int chain2 = chain1;
        int chain3 = chain2 * 1;  /* Multiplication by 1 preserves value */
        int chain4 = chain3;
        int final_val = chain4;
        
        /* Multiple comparison types */
        switch (final_val) {
            case 0:
                result += i * 3;
                break;
            case 1:
                result += i * 7;
                /* Nested condition */
                if (final_val == 1) {
                    result += 11;
                }
                break;
            default:
                result += 1;
        }
        
        /* Loop with PHI-derived condition */
        int loop_ctrl = final_val;
        int j = 0;
        while (loop_ctrl && j < 3) {  /* while(phi_derived) pattern */
            result += j;
            j++;
            /* Modify loop_ctrl to eventually exit */
            if (j == 2) loop_ctrl = 0;
        }
    }
    
    return result;
}

/* Function 3: Mixed hot/cold paths with function calls */
int mixed_path_processor(int* data, int size, int hot_factor) {
    int total = 0;
    
    for (int i = 0; i < size; i++) {
        int flag;
        
        /* Create PHI based on array values */
        if (data[i] > 100) {
            flag = 1;
        } else if (data[i] < 0) {
            flag = 0;
        } else {
            flag = (i % hot_factor == 0) ? 1 : 0;
        }
        
        /* Assignment chain crossing basic block boundaries */
        int intermediate = flag;
        if (i % 10 == 0) {
            intermediate = intermediate;  /* Dummy assignment in different block */
        }
        
        int compare_val = intermediate;
        
        /* Function call from both hot and cold paths */
        if (compare_val) {
            /* Hot path function call */
            total += process_hot_path(10, 1);
        } else {
            /* Cold path function call */
            total += process_cold_path(5, 2);
        }
        
        /* Additional comparison */
        if (compare_val == 0) {
            total -= data[i] % 3;
        }
    }
    
    return total;
}

/* Function 4: Complex nested loops with PHI patterns */
int nested_loop_pattern(int depth) {
    int accumulator = 0;
    
    for (int a = 0; a < depth; a++) {
        for (int b = 0; b < 100; b++) {
            int phi_input;
            
            /* PHI with multiple predecessors */
            if (a % 2 == 0) {
                if (b % 3 == 0) {
                    phi_input = 1;
                } else {
                    phi_input = 0;
                }
            } else {
                phi_input = (a + b) % 2;
            }
            
            /* PHI node */
            int phi_out = phi_input;
            
            /* Multi-step assignment chain */
            int step1 = phi_out;
            int step2 = step1;
            int step3 = step2 + (a * 0);  /* Arithmetic that doesn't change value */
            int step4 = step3;
            
            /* Multiple conditional uses */
            if (step4) {
                accumulator += a * b;
                
                if (step4 == 1) {
                    accumulator += b;
                }
            }
            
            if (!step4) {
                accumulator -= a;
            }
            
            /* Use in loop condition */
            int loop_flag = step4;
            int counter = 0;
            do {
                accumulator += counter;
                counter++;
                if (counter > 2) loop_flag = 0;
            } while (loop_flag);
        }
    }
    
    return accumulator;
}

/* Function 5: Recursive pattern with PHI propagation */
int recursive_phi_pattern(int n, int depth) {
    if (n <= 0 || depth <= 0) {
        return 0;
    }
    
    int branch_selector;
    
    /* PHI pattern in recursive function */
    if (n % 2 == 0) {
        branch_selector = 1;
    } else {
        branch_selector = 0;
    }
    
    int phi_val = branch_selector;
    int tmp_copy = phi_val;
    int final_compare = tmp_copy;
    
    int result = 0;
    
    if (final_compare == 1) {
        /* Hot recursive path */
        result = n + recursive_phi_pattern(n - 1, depth - 1);
        
        /* Additional condition */
        if (final_compare) {
            result += 5;
        }
    } else {
        /* Cold recursive path */
        result = n * 2 + recursive_phi_pattern(n - 2, depth - 1);
    }
    
    return result;
}

/* Main function with profile-generating runtime behavior */
int main(int argc, char** argv) {
    int mode = 1;  /* Default to hot mode */
    int iterations = 1000000;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations < 1000) iterations = 1000;
        if (iterations > 10000000) iterations = 10000000;
    }
    
    srand(global_seed);
    clock_t start = clock();
    
    int final_result = 0;
    
    /* Select execution mode to create different profile patterns */
    switch (mode) {
        case 1:  /* Dominant hot mode - triggers hot path annotations */
            printf("Running HOT mode with %d iterations\n", iterations);
            
            /* Execute hot path many times */
            final_result += process_hot_path(iterations, 1);
            
            /* Mix with some cold execution */
            final_result += process_cold_path(iterations / 100, 10);
            
            /* Process array with mixed paths */
            {
                int data_size = 10000;
                int* data = malloc(data_size * sizeof(int));
                for (int i = 0; i < data_size; i++) {
                    data[i] = rand() % 200;
                }
                final_result += mixed_path_processor(data, data_size, 7);
                free(data);
            }
            
            /* Nested loops */
            final_result += nested_loop_pattern(50);
            
            break;
            
        case 2:  /* Balanced mode - mixed hot/cold */
            printf("Running BALANCED mode with %d iterations\n", iterations);
            
            final_result += process_hot_path(iterations / 2, 2);
            final_result += process_cold_path(iterations / 2, 5);
            
            {
                int data_size = 5000;
                int* data = malloc(data_size * sizeof(int));
                for (int i = 0; i < data_size; i++) {
                    data[i] = rand() % 300 - 50;
                }
                final_result += mixed_path_processor(data, data_size, 3);
                free(data);
            }
            
            final_result += recursive_phi_pattern(100, 10);
            
            break;
            
        case 3:  /* Cold mode - mostly cold paths */
            printf("Running COLD mode with %d iterations\n", iterations);
            
            final_result += process_hot_path(iterations / 10, 3);
            final_result += process_cold_path(iterations, 20);
            
            final_result += nested_loop_pattern(20);
            final_result += recursive_phi_pattern(50, 5);
            
            break;
            
        default:
            printf("Running DEFAULT mode\n");
            final_result += process_hot_path(10000, 1);
            final_result += process_cold_path(1000, 2);
    }
    
    /* Add some noise to prevent optimization */
    final_result ^= (rand() % 256);
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Result: %d\n", final_result);
    printf("Time elapsed: %.3f seconds\n", elapsed);
    
    /* Store in global to prevent dead code elimination */
    global_result = final_result;
    
    return 0;
}
