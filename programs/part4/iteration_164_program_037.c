/* autofdo_phi_conditional.c
 * 
 * This program generates execution patterns specifically designed to trigger
 * GCC's AutoFDO PHI-to-conditional analysis in auto-profile.cc lines 1312-1333.
 * 
 * Compilation and usage:
 * 1. First compilation (with existing profile or empty):
 *    gcc -O2 -fauto-profile autofdo_phi_conditional.c -o autofdo_test
 *    
 * 2. Run with dominant hot path:
 *    ./autofdo_test 1 > /dev/null
 *    
 * 3. Collect profile (using perf or similar):
 *    perf record -e cycles:u -b ./autofdo_test 1
 *    perf convert --to-ctf perf.data > perf.ctf
 *    create_gcov --binary=./autofdo_test --profile=perf.ctf --gcov=autofdo_test.gcov
 *    
 * 4. Recompile with profile:
 *    gcc -O2 -fauto-profile -Wauto-profile autofdo_phi_conditional.c -o autofdo_test_opt
 *    
 * 5. Verify optimization:
 *    gcc -O2 -fauto-profile -fdump-tree-afdo -fdump-tree-afdo-details autofdo_phi_conditional.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HOT_ITERATIONS 1000000
#define WARM_ITERATIONS 10000
#define COLD_ITERATIONS 10

/* Function 1: Complex PHI pattern with SSA copy chains */
int process_data_phi_chain(int mode, int size) {
    int result = 0;
    int i, j;
    
    /* Create varying execution profiles based on mode */
    int iterations = (mode == 1) ? HOT_ITERATIONS : 
                     (mode == 2) ? WARM_ITERATIONS : COLD_ITERATIONS;
    
    for (i = 0; i < iterations; i++) {
        int phi_value;
        int tmp1, tmp2, tmp3;
        
        /* Create control flow that generates PHI nodes */
        if (i % 3 == 0) {
            /* Path A: sets value to 1 */
            phi_value = 1;
        } else if (i % 3 == 1) {
            /* Path B: sets value to 0 */
            phi_value = 0;
        } else {
            /* Path C: sets value based on complex condition */
            phi_value = (i % 7 == 0) ? 1 : 0;
        }
        
        /* Create SSA copy chain to trigger the while loop in auto-profile.cc */
        tmp1 = phi_value;      /* First assignment */
        tmp2 = tmp1;           /* Copy through SSA */
        tmp3 = tmp2 + 0;       /* Arithmetic that preserves value */
        
        /* Multiple conditional comparisons using PHI-derived value */
        if (tmp3) {  /* Direct use in if condition - will be GIMPLE_COND with constant RHS */
            /* Hot path for mode 1 */
            for (j = 0; j < size; j++) {
                result += (i * j) % 100;
            }
        } else {
            /* Cold path */
            result -= 1;
        }
        
        /* Another conditional with explicit comparison */
        if (tmp2 == 1) {  /* Explicit equality with 1 */
            result += i % 100;
        }
        
        /* Yet another with != 0 */
        if (tmp1 != 0) {  /* Inequality with 0 */
            result += (i * 2) % 50;
        }
    }
    
    return result;
}

/* Function 2: Nested loops with PHI in loop condition */
int nested_phi_loop(int mode, int *data, int size) {
    int total = 0;
    int outer_iter = (mode == 1) ? 1000 : 100;
    
    for (int i = 0; i < outer_iter; i++) {
        int loop_control;
        int tmp_a, tmp_b, tmp_c;
        
        /* PHI pattern based on multiple predecessors */
        if (i % 4 == 0) {
            loop_control = 1;
        } else if (data[i % size] > 50) {
            loop_control = 1;
        } else {
            loop_control = 0;
        }
        
        /* Multi-step SSA copy chain */
        tmp_a = loop_control;
        tmp_b = tmp_a;
        tmp_c = tmp_b;
        
        /* Use PHI-derived value in loop condition */
        while (tmp_c) {  /* While loop with PHI-derived condition */
            for (int j = 0; j < size; j++) {
                total += data[j] * i;
            }
            tmp_c = 0;  /* Break after one iteration pattern */
        }
        
        /* Another conditional block */
        if (tmp_b == 1) {
            total += i * 100;
        }
    }
    
    return total;
}

/* Function 3: Switch-like pattern with PHI propagation */
int switch_phi_pattern(int mode, int value) {
    int result = 0;
    int flag;
    int chain1, chain2, chain3;
    
    /* Complex branching creates annotated basic blocks */
    switch (value % 5) {
        case 0:
            flag = 1;
            result += 100;
            break;
        case 1:
            flag = 0;
            result += 200;
            break;
        case 2:
            flag = (mode == 1) ? 1 : 0;
            result += 300;
            break;
        case 3:
            flag = (value > 1000) ? 1 : 0;
            result += 400;
            break;
        default:
            flag = 0;
            result += 500;
    }
    
    /* Extended SSA copy chain */
    chain1 = flag;
    chain2 = chain1;
    chain3 = chain2 + 0;  /* Preserves value but creates assignment */
    
    /* Multiple conditionals using the chain */
    if (chain3) {
        /* Hot path for profile */
        for (int i = 0; i < (mode == 1 ? 1000 : 10); i++) {
            result += i * chain3;
        }
    }
    
    if (chain2 == 1) {
        result *= 2;
    }
    
    return result;
}

/* Function 4: Recursive pattern with PHI across calls */
int recursive_phi_helper(int depth, int seed, int *flag) {
    if (depth <= 0) {
        *flag = (seed % 2 == 0) ? 1 : 0;
        return seed;
    }
    
    int left_flag, right_flag;
    int left_val = recursive_phi_helper(depth - 1, seed * 3 + 1, &left_flag);
    int right_val = recursive_phi_helper(depth - 1, seed * 5 + 2, &right_flag);
    
    /* PHI node created from multiple return paths */
    *flag = (left_flag || right_flag) ? 1 : 0;
    
    int tmp = *flag;
    int final_flag = tmp;
    
    /* Conditional using PHI result */
    if (final_flag) {
        return left_val + right_val + 1;
    } else {
        return left_val + right_val;
    }
}

int recursive_phi_driver(int mode) {
    int total = 0;
    int depth = (mode == 1) ? 8 : 3;  /* Different depths for different profiles */
    
    for (int i = 0; i < (mode == 1 ? 100 : 10); i++) {
        int flag;
        int result = recursive_phi_helper(depth, i, &flag);
        
        int tmp1 = flag;
        int tmp2 = tmp1;
        
        if (tmp2) {
            total += result * 2;
        } else {
            total += result;
        }
    }
    
    return total;
}

/* Main function with different execution modes */
int main(int argc, char **argv) {
    int mode = 1;  /* Default to hot path */
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    /* Initialize data array */
    int data_size = 1000;
    int *data = malloc(data_size * sizeof(int));
    for (int i = 0; i < data_size; i++) {
        data[i] = (i * 17) % 100;
    }
    
    int final_result = 0;
    
    /* Warm-up phase */
    clock_t start = clock();
    
    /* Execute different patterns based on mode */
    switch (mode) {
        case 1:  /* Hot path - executes millions of iterations */
            printf("Executing HOT path...\n");
            for (int phase = 0; phase < 10; phase++) {
                final_result += process_data_phi_chain(1, 100);
                final_result += nested_phi_loop(1, data, data_size);
                final_result += switch_phi_pattern(1, phase * 1000);
                final_result += recursive_phi_driver(1);
            }
            break;
            
        case 2:  /* Warm path - mixed execution */
            printf("Executing WARM path...\n");
            for (int phase = 0; phase < 3; phase++) {
                final_result += process_data_phi_chain(2, 50);
                final_result += nested_phi_loop(2, data, data_size / 2);
                final_result += switch_phi_pattern(2, phase * 500);
                final_result += recursive_phi_driver(2);
            }
            break;
            
        case 3:  /* Cold path - minimal execution */
            printf("Executing COLD path...\n");
            final_result += process_data_phi_chain(3, 10);
            final_result += nested_phi_loop(3, data, 10);
            final_result += switch_phi_pattern(3, 100);
            final_result += recursive_phi_driver(3);
            break;
            
        default:
            printf("Unknown mode. Using mixed profile.\n");
            /* Mix of all paths */
            final_result += process_data_phi_chain(1, 100);
            final_result += process_data_phi_chain(2, 50);
            final_result += process_data_phi_chain(3, 10);
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    /* Use the result to prevent optimization */
    volatile int checksum = final_result % 1000000;
    
    printf("Mode %d: Result checksum = %d, Time = %.3f seconds\n", 
           mode, checksum, elapsed);
    
    free(data);
    return checksum != 0 ? 0 : 1;
}
