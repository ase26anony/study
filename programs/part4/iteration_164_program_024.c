/* autofdo_phi_test.c
 * Test program to trigger GCC AutoFDO PHI-to-conditional analysis
 * Compile with: gcc -O3 -fauto-profile -funroll-loops -finline-functions autofdo_phi_test.c -o autofdo_phi_test
 * Run with: ./autofdo_phi_test <mode> <iterations>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define HOT_ITERATIONS 10000000
#define WARM_ITERATIONS 1000000
#define COLD_ITERATIONS 1000

/* Global volatile to prevent optimization */
volatile int global_seed = 42;

/* Function 1: Complex PHI pattern with SSA copy chains */
uint64_t process_with_phi_chains(int mode, int iterations) {
    uint64_t sum = 0;
    int i, j;
    
    for (i = 0; i < iterations; i++) {
        int phi_val_1, phi_val_2;
        int tmp1, tmp2, tmp3, tmp4;
        
        /* Create branching that leads to PHI nodes */
        if (mode == 1) {
            /* Hot path - executed frequently */
            phi_val_1 = 1;
            phi_val_2 = 1;
        } else if (mode == 2) {
            /* Warm path */
            phi_val_1 = (i % 2 == 0) ? 1 : 0;
            phi_val_2 = 0;
        } else {
            /* Cold path */
            phi_val_1 = 0;
            phi_val_2 = (i < 10) ? 1 : 0;
        }
        
        /* PHI node simulation through multiple basic blocks */
        int phi_result;
        if (i % 3 == 0) {
            phi_result = phi_val_1;
        } else {
            phi_result = phi_val_2;
        }
        
        /* Create SSA copy chain as required by uncovered code */
        tmp1 = phi_result;      /* First assignment */
        tmp2 = tmp1;            /* Second assignment - triggers while loop */
        tmp3 = tmp2 + 0;        /* Arithmetic that preserves value */
        tmp4 = tmp3;            /* Another copy */
        
        /* Multiple conditional comparisons using PHI-derived value */
        if (tmp4) {  /* Direct use in if condition */
            for (j = 0; j < 10; j++) {
                sum += (i * j) & 0xFF;
            }
        }
        
        if (tmp4 == 1) {  /* Explicit equality comparison */
            sum += i * 2;
        }
        
        if (tmp4 != 0) {  /* Inequality comparison */
            sum += i * 3;
        }
        
        /* Use in loop condition */
        int loop_ctrl = tmp4;
        while (loop_ctrl && j < 5) {
            sum += (i + j) * 7;
            j++;
            loop_ctrl = 0;  /* Ensure loop terminates */
        }
    }
    
    return sum;
}

/* Function 2: Nested loops with varying PHI patterns */
uint64_t nested_phi_patterns(int depth, int width, int hot_path) {
    uint64_t total = 0;
    int i, j, k;
    
    for (i = 0; i < depth; i++) {
        int outer_phi;
        
        /* PHI based on outer loop index */
        if (i % 2 == 0) {
            outer_phi = 1;
        } else {
            outer_phi = (hot_path > 0) ? 1 : 0;
        }
        
        /* Chain of assignments */
        int chain1 = outer_phi;
        int chain2 = chain1;
        int chain3 = chain2 + 0;
        int final_val = chain3;
        
        for (j = 0; j < width; j++) {
            int inner_phi;
            
            /* Another PHI inside inner loop */
            if (j < width / 2) {
                inner_phi = 1;
            } else {
                inner_phi = (final_val > 0) ? 1 : 0;
            }
            
            /* More SSA copies */
            int inner_tmp1 = inner_phi;
            int inner_tmp2 = inner_tmp1;
            int inner_tmp3 = inner_tmp2;
            
            /* Conditional using PHI-derived value */
            if (inner_tmp3 == 1) {
                for (k = 0; k < 3; k++) {
                    total += (i * j * k) & 0xFF;
                }
            } else {
                total += i + j;
            }
            
            /* Another conditional branch */
            if (inner_tmp3) {
                total += (i ^ j) * 11;
            }
        }
    }
    
    return total;
}

/* Function 3: Array processing with PHI-dependent conditions */
uint64_t array_process_with_phi(int size, int threshold) {
    uint64_t result = 0;
    int* data = malloc(size * sizeof(int));
    
    if (!data) return 0;
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        data[i] = (i * 7) % 100;
    }
    
    /* Process array with PHI-based conditions */
    for (int i = 0; i < size; i++) {
        int phi_val;
        
        /* PHI that depends on array value */
        if (data[i] > threshold) {
            phi_val = 1;
        } else if (i % 3 == 0) {
            phi_val = 0;
        } else {
            phi_val = (data[i] % 2 == 0) ? 1 : 0;
        }
        
        /* SSA copy chain */
        int copy1 = phi_val;
        int copy2 = copy1;
        int copy3 = copy2 + 0;
        int copy4 = copy3;
        
        /* Multiple uses of PHI-derived value */
        if (copy4) {
            result += data[i] * 2;
        }
        
        if (copy4 == 1) {
            result += i * 3;
        }
        
        /* Nested condition */
        if (i % 5 == 0) {
            int nested_phi = copy4;
            int nested_copy = nested_phi;
            
            if (nested_copy) {
                result += data[i] * data[i];
            }
        }
    }
    
    free(data);
    return result;
}

/* Function 4: Recursive PHI pattern */
uint64_t recursive_phi_helper(int n, int depth, int* chain_val) {
    if (n <= 0 || depth >= 10) {
        return 0;
    }
    
    uint64_t sum = 0;
    int phi_val;
    
    /* PHI based on recursion depth */
    if (depth % 2 == 0) {
        phi_val = 1;
    } else {
        phi_val = (n % 2 == 0) ? 1 : 0;
    }
    
    /* Chain through SSA copies */
    int tmp1 = phi_val;
    int tmp2 = tmp1;
    *chain_val = tmp2;
    
    if (*chain_val) {
        sum += n * depth;
    }
    
    if (*chain_val == 1) {
        sum += recursive_phi_helper(n - 1, depth + 1, chain_val);
    }
    
    return sum;
}

uint64_t recursive_phi_pattern(int iterations) {
    uint64_t total = 0;
    int chain_val = 0;
    
    for (int i = 0; i < iterations; i++) {
        total += recursive_phi_helper(i % 20, 0, &chain_val);
    }
    
    return total;
}

/* Main function with different execution modes */
int main(int argc, char** argv) {
    int mode = 1;  /* Default to hot mode */
    int custom_iterations = 0;
    
    /* Parse command line arguments */
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        custom_iterations = atoi(argv[2]);
    }
    
    uint64_t total_result = 0;
    clock_t start_time = clock();
    
    printf("Running AutoFDO PHI test in mode %d\n", mode);
    
    /* Select execution pattern based on mode */
    switch (mode) {
        case 1:  /* HOT mode - triggers heavily annotated basic blocks */
            printf("Executing HOT path (millions of iterations)\n");
            total_result += process_with_phi_chains(1, 
                custom_iterations ? custom_iterations : HOT_ITERATIONS);
            total_result += nested_phi_patterns(1000, 100, 1);
            total_result += array_process_with_phi(10000, 50);
            total_result += recursive_phi_pattern(10000);
            break;
            
        case 2:  /* WARM mode - mixed hot/cold paths */
            printf("Executing WARM path (mixed profile)\n");
            total_result += process_with_phi_chains(2,
                custom_iterations ? custom_iterations : WARM_ITERATIONS);
            total_result += nested_phi_patterns(500, 50, 0);
            total_result += array_process_with_phi(5000, 30);
            total_result += recursive_phi_pattern(5000);
            break;
            
        case 3:  /* COLD mode - mostly cold paths */
            printf("Executing COLD path (rarely executed)\n");
            total_result += process_with_phi_chains(3,
                custom_iterations ? custom_iterations : COLD_ITERATIONS);
            total_result += nested_phi_patterns(100, 10, 0);
            total_result += array_process_with_phi(1000, 10);
            total_result += recursive_phi_pattern(1000);
            break;
            
        default:  /* Mixed mode - varying patterns */
            printf("Executing MIXED mode\n");
            for (int i = 0; i < 3; i++) {
                total_result += process_with_phi_chains(i + 1, 100000);
                total_result += nested_phi_patterns(200, 20, i % 2);
                total_result += array_process_with_phi(2000, i * 20);
            }
            break;
    }
    
    clock_t end_time = clock();
    double elapsed = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    
    printf("Total result: %lu\n", total_result);
    printf("Execution time: %.2f seconds\n", elapsed);
    printf("Checksum: %08lx\n", total_result & 0xFFFFFFFF);
    
    /* Additional verification computation */
    uint64_t verify = 0;
    for (int i = 0; i < 1000; i++) {
        int phi_test = (i % 3 == 0) ? 1 : 0;
        int tmp = phi_test;
        if (tmp) {
            verify += i * 3;
        }
        if (tmp == 1) {
            verify += i * 7;
        }
    }
    
    printf("Verification checksum: %08lx\n", verify & 0xFFFFFFFF);
    
    return 0;
}
