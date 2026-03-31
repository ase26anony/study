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
 *    perf convert --to-ctf perf.data > perf.prof
 *    
 * 4. Recompile with profile:
 *    gcc -O2 -fauto-profile -Wauto-profile autofdo_phi_conditional.c -o autofdo_opt
 *    
 * 5. Verify optimization with dumps:
 *    gcc -O2 -fauto-profile -fdump-tree-afdo -fdump-tree-afdo-details autofdo_phi_conditional.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define HOT_ITERATIONS 1000000
#define WARM_ITERATIONS 10000
#define COLD_ITERATIONS 10

/* Function 1: Complex PHI pattern with SSA copy chains in hot loop */
uint64_t process_hot_data(int mode, int size) {
    uint64_t sum = 0;
    int* data = (int*)malloc(size * sizeof(int));
    
    if (!data) return 0;
    
    /* Initialize with pattern */
    for (int i = 0; i < size; i++) {
        data[i] = i % 100;
    }
    
    /* Main hot processing loop - will be heavily annotated */
    for (int iter = 0; iter < HOT_ITERATIONS; iter++) {
        int use_fast_path = 0;
        int tmp1, tmp2, tmp3;
        
        /* Create branching that sets boolean values in different predecessors */
        if (mode == 1) {
            /* Hot path - sets value to 1 */
            use_fast_path = 1;  /* This becomes a constant 1 in predecessor */
        } else if (mode == 2) {
            /* Warm path - sets value based on condition */
            use_fast_path = (iter % 100 == 0);
        } else {
            /* Cold path - sets value to 0 */
            use_fast_path = 0;  /* This becomes a constant 0 in predecessor */
        }
        
        /* PHI NODE CREATION POINT */
        /* The variable 'phi_val' will become a PHI node merging values from
           different predecessor blocks */
        int phi_val = use_fast_path;
        
        /* SSA COPY CHAIN - triggers the while loop in uncovered code */
        /* These create single-assignment copies that the analysis walks through */
        tmp1 = phi_val;          /* First copy */
        tmp2 = tmp1;             /* Second copy */
        tmp3 = tmp2 + 0;         /* Arithmetic that preserves value (tmp3 = tmp2 + 0) */
        int cmp_var = tmp3;      /* Final copy before comparison */
        
        /* CONSTANT COMPARISON PATTERNS - exactly what the uncovered code looks for */
        /* Pattern 1: Direct use in if condition (if (cmp_var)) */
        if (cmp_var) {  /* This becomes: if (cmp_var != 0) */
            /* Hot path - process with fast algorithm */
            for (int i = 0; i < size; i += 2) {
                sum += data[i] * 3;
            }
        } else {
            /* Cold path - process with full algorithm */
            for (int i = 0; i < size; i++) {
                sum += data[i];
            }
        }
        
        /* Pattern 2: Explicit equality comparison (cmp_var == 1) */
        int check_again = tmp2;  /* Another SSA copy from the chain */
        if (check_again == 1) {  /* Explicit comparison with constant 1 */
            /* Additional hot processing */
            for (int i = 1; i < size; i += 2) {
                sum += data[i] * 2;
            }
        }
        
        /* Pattern 3: Inequality comparison (cmp_var != 0) */
        int final_check = cmp_var;
        if (final_check != 0) {  /* Explicit comparison with constant 0 */
            sum += size;
        }
    }
    
    free(data);
    return sum;
}

/* Function 2: Nested loops with PHI-dependent conditions */
uint64_t process_matrix(int rows, int cols, int threshold) {
    uint64_t total = 0;
    int use_optimized = 0;
    int tmp_a, tmp_b;
    
    /* Create matrix */
    int** matrix = (int**)malloc(rows * sizeof(int*));
    for (int i = 0; i < rows; i++) {
        matrix[i] = (int*)malloc(cols * sizeof(int));
        for (int j = 0; j < cols; j++) {
            matrix[i][j] = (i * cols + j) % 256;
        }
    }
    
    /* Outer loop with PHI pattern */
    for (int i = 0; i < rows; i++) {
        /* Set boolean based on complex condition */
        if (i < threshold) {
            use_optimized = 1;  /* Hot path value */
        } else if (i == threshold) {
            use_optimized = (matrix[i][0] > 128);  /* Variable path */
        } else {
            use_optimized = 0;  /* Cold path value */
        }
        
        /* PHI node */
        int row_optimize = use_optimized;
        
        /* SSA copy chain */
        tmp_a = row_optimize;
        tmp_b = tmp_a;
        int should_optimize = tmp_b;
        
        /* Loop with PHI-derived condition */
        for (int j = 0; j < cols; j++) {
            /* Pattern 4: Condition in inner loop */
            if (should_optimize) {
                /* Optimized computation */
                total += matrix[i][j] * matrix[i][j];
            } else {
                /* Standard computation */
                total += matrix[i][j];
            }
            
            /* Additional check with explicit constant */
            if (should_optimize == 1 && j % 16 == 0) {
                total += 1000;
            }
        }
        
        /* Pattern 5: While loop with PHI condition */
        int cleanup_flag = should_optimize;
        int cleanup_count = 0;
        while (cleanup_flag && cleanup_count < 5) {
            total += cleanup_count * 100;
            cleanup_count++;
            /* This creates a loop condition that depends on PHI */
        }
    }
    
    /* Cleanup */
    for (int i = 0; i < rows; i++) {
        free(matrix[i]);
    }
    free(matrix);
    
    return total;
}

/* Function 3: Recursive pattern with PHI propagation */
uint64_t process_tree(int depth, int branch, int* use_fast) {
    if (depth <= 0) return 1;
    
    uint64_t result = 0;
    int local_fast;
    
    /* Create PHI pattern in recursive calls */
    if (depth > 5) {
        local_fast = *use_fast;  /* Inherit from parent */
    } else if (depth == 5) {
        local_fast = (branch % 3 == 0);  /* Variable at boundary */
    } else {
        local_fast = 0;  /* Cold leaf */
    }
    
    /* SSA copies */
    int tmp1 = local_fast;
    int tmp2 = tmp1;
    int current_fast = tmp2;
    
    /* Process with condition */
    for (int i = 0; i < branch; i++) {
        if (current_fast) {
            result += depth * 100 + i;
        } else {
            result += depth * 10 + i;
        }
        
        /* Check with explicit constant */
        if (current_fast == 1 && i % 2 == 0) {
            result += 500;
        }
    }
    
    /* Recursive calls with propagated value */
    for (int i = 0; i < 2; i++) {
        int child_fast = current_fast;
        result += process_tree(depth - 1, branch + i, &child_fast);
    }
    
    return result;
}

/* Function 4: Mixed hot/cold paths with function calls */
uint64_t process_mixed(int mode, int iterations) {
    uint64_t checksum = 0;
    int path_selector;
    
    for (int i = 0; i < iterations; i++) {
        /* Complex branching to create annotated basic blocks */
        if (mode == 1) {
            if (i % 1000 < 950) {  /* 95% hot */
                path_selector = 1;
            } else {                /* 5% warm */
                path_selector = (i % 2);
            }
        } else if (mode == 2) {
            path_selector = (i % 3 == 0);  /* 33% hot */
        } else {
            path_selector = 0;  /* Always cold */
        }
        
        /* PHI node */
        int current_path = path_selector;
        
        /* Extended SSA chain */
        int chain1 = current_path;
        int chain2 = chain1;
        int chain3 = chain2 + 0;  /* Arithmetic that doesn't change value */
        int chain4 = chain3;
        int final_decision = chain4;
        
        /* Multiple comparison patterns */
        if (final_decision) {
            /* Hot path function call */
            checksum += process_hot_data(1, 10);
        } else if (final_decision == 0) {
            /* Cold path function call */
            checksum += process_hot_data(3, 5);
        }
        
        /* Additional check */
        if (final_decision == 1 && i % 100 == 0) {
            checksum += 9999;
        }
    }
    
    return checksum;
}

/* Main function with different execution modes */
int main(int argc, char** argv) {
    int mode = 1;  /* Default to hot mode */
    uint64_t total_result = 0;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    printf("Starting AutoFDO test with mode %d\n", mode);
    
    /* Warm-up phase - creates initial profile annotations */
    printf("Warm-up phase...\n");
    for (int warm = 0; warm < WARM_ITERATIONS; warm++) {
        total_result += process_hot_data(mode, 50);
    }
    
    /* Main processing phase - heavily exercises hot paths */
    printf("Main processing phase...\n");
    
    /* Execute different patterns based on mode */
    switch (mode) {
        case 1:  /* HOT MODE - triggers heavy annotation of hot paths */
            printf("Executing hot path patterns...\n");
            total_result += process_hot_data(1, 100);
            total_result += process_matrix(100, 100, 80);  /* 80% hot rows */
            
            {
                int fast_flag = 1;
                total_result += process_tree(8, 3, &fast_flag);
            }
            
            total_result += process_mixed(1, HOT_ITERATIONS / 10);
            break;
            
        case 2:  /* WARM MODE - mixed hot/cold */
            printf("Executing warm path patterns...\n");
            total_result += process_hot_data(2, 50);
            total_result += process_matrix(50, 50, 25);  /* 50% hot rows */
            
            {
                int fast_flag = 0;
                total_result += process_tree(6, 2, &fast_flag);
            }
            
            total_result += process_mixed(2, WARM_ITERATIONS);
            break;
            
        case 3:  /* COLD MODE - mostly cold paths */
            printf("Executing cold path patterns...\n");
            total_result += process_hot_data(3, 20);
            total_result += process_matrix(20, 20, 5);  /* 25% hot rows */
            
            {
                int fast_flag = 0;
                total_result += process_tree(4, 1, &fast_flag);
            }
            
            total_result += process_mixed(3, COLD_ITERATIONS);
            break;
            
        default:
            printf("Unknown mode, using mixed...\n");
            total_result += process_hot_data(2, 30);
            break;
    }
    
    /* Final aggregation */
    printf("Final checksum: %lu\n", total_result);
    
    /* Ensure result is used to prevent optimization */
    volatile uint64_t sink = total_result;
    
    return 0;
}
