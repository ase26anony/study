/* autofdo_phi_conditional.c
 * Test program to trigger GCC AutoFDO PHI-to-conditional analysis
 * Compile with: gcc -O2 -fauto-profile autofdo_phi_conditional.c -o autofdo_test
 * Run with: ./autofdo_test mode=1
 * Then recompile with profile: gcc -O2 -fauto-profile -Wauto-profile autofdo_phi_conditional.c -o autofdo_optimized
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HOT_ITERATIONS 1000000
#define WARM_ITERATIONS 10000
#define COLD_ITERATIONS 10

/* Function 1: Complex PHI pattern with SSA copy chains in hot loop */
unsigned long process_hot_path(int mode, int iterations) {
    unsigned long sum = 0;
    int i, j;
    
    for (i = 0; i < iterations; i++) {
        int phi_val;
        int tmp1, tmp2, tmp3, cmp_var;
        
        /* Create branching that feeds into PHI node */
        if (mode == 1) {
            /* Hot path - executed most of the time */
            phi_val = 1;  /* This becomes PHI input from this edge */
        } else {
            /* Cold path */
            phi_val = 0;  /* This becomes PHI input from this edge */
        }
        
        /* PHI node is implicitly created here at start of loop body */
        /* Create SSA copy chain to trigger while loop walking */
        tmp1 = phi_val;      /* First assignment copy */
        tmp2 = tmp1;         /* Second copy */
        tmp3 = tmp2 + 0;     /* Arithmetic that doesn't change value */
        cmp_var = tmp3;      /* Final copy before comparison */
        
        /* Conditional using PHI-derived value - triggers uncovered code */
        if (cmp_var) {       /* Direct use in if condition */
            /* Hot inner loop */
            for (j = 0; j < 100; j++) {
                sum += (i * j) & 0xFF;
            }
        } else {
            /* Cold path - rarely executed */
            sum += i;
        }
        
        /* Another PHI pattern with explicit comparison */
        int phi_val2;
        if (i % 3 == 0) {
            phi_val2 = 1;
        } else {
            phi_val2 = 0;
        }
        
        int chain1 = phi_val2;
        int chain2 = chain1;
        
        /* Explicit equality comparison */
        if (chain2 == 1) {   /* integer_onep comparison */
            sum += i * 2;
        }
    }
    
    return sum;
}

/* Function 2: Nested conditionals with PHI propagation */
unsigned long process_nested_phi(int seed, int limit) {
    unsigned long result = 0;
    int i = 0;
    
    while (i < limit) {
        int base_val;
        
        /* Create different incoming values to PHI */
        if (seed % 2 == 0) {
            base_val = 1;
        } else {
            base_val = 0;
        }
        
        /* Multiple SSA copies */
        int copy1 = base_val;
        int copy2 = copy1;
        int copy3 = copy2;
        
        /* Use in while condition - creates another PHI opportunity */
        int loop_control = copy3;
        int inner_count = 0;
        
        while (loop_control && inner_count < 5) {
            result += (seed + i + inner_count);
            inner_count++;
            
            /* Modify control variable to eventually exit */
            if (inner_count >= 3) {
                int tmp = loop_control;
                loop_control = tmp - 1;  /* Breaks single-assignment but creates interesting pattern */
            }
        }
        
        /* Another PHI pattern with != 0 comparison */
        int phi_cond;
        if (i % 7 == 0) {
            phi_cond = 0;
        } else {
            phi_cond = 1;
        }
        
        int chain_a = phi_cond;
        int chain_b = chain_a;
        int chain_c = chain_b;
        
        if (chain_c != 0) {  /* integer_zerop with negation */
            result += i * 3;
        }
        
        i++;
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
    }
    
    return result;
}

/* Function 3: Array processing with PHI-based branching */
unsigned long process_array_with_phi(int *array, int size, int threshold) {
    unsigned long sum = 0;
    int i;
    
    for (i = 0; i < size; i++) {
        int use_fast_path;
        
        /* PHI input based on array value */
        if (array[i] > threshold) {
            use_fast_path = 1;
        } else {
            use_fast_path = 0;
        }
        
        /* Long SSA copy chain */
        int v1 = use_fast_path;
        int v2 = v1;
        int v3 = v2 + 0;  /* Arithmetic that preserves value */
        int v4 = v3;
        int v5 = v4;
        int final_cond = v5;
        
        /* Multiple comparison types */
        if (final_cond) {
            /* Fast path - simple processing */
            sum += array[i];
        } else if (final_cond == 1) {  /* Explicit equality check */
            /* This path should be rarely taken due to condition above */
            sum += array[i] * 2;
        } else {
            /* Slow path - complex processing */
            sum += (array[i] % 17) + (array[i] / 3);
        }
        
        /* Another PHI pattern inside the loop */
        int secondary_cond;
        if (sum % 2 == 0) {
            secondary_cond = 1;
        } else {
            secondary_cond = 0;
        }
        
        int s1 = secondary_cond;
        int s2 = s1;
        
        if (s2 == 1) {  /* integer_onep comparison */
            sum += i;
        }
    }
    
    return sum;
}

/* Function 4: Complex control flow with multiple PHI merges */
unsigned long complex_phi_merging(int mode, int depth) {
    unsigned long total = 0;
    int level = 0;
    
    /* Recursive-like structure implemented iteratively */
    while (level < depth) {
        int phi_value;
        
        /* Multiple predecessor blocks with different values */
        switch (mode) {
            case 1:
                phi_value = 1;
                break;
            case 2:
                phi_value = 0;
                break;
            case 3:
                phi_value = (level % 2);
                break;
            default:
                phi_value = 1;
        }
        
        /* Chain of assignments */
        int a1 = phi_value;
        int a2 = a1;
        int a3 = a2 + 0;
        int a4 = a3;
        
        /* Used in switch-like conditional */
        if (a4) {
            total += level * 100;
            mode = (mode * 3) % 7;
        } else {
            total += level;
            mode = (mode + 1) % 7;
        }
        
        /* Another PHI at loop back edge */
        int loop_phi;
        if (total % 1000 > 500) {
            loop_phi = 1;
        } else {
            loop_phi = 0;
        }
        
        int b1 = loop_phi;
        int b2 = b1;
        
        if (b2 != 0) {  /* != 0 comparison */
            level += 2;
        } else {
            level += 1;
        }
    }
    
    return total;
}

/* Helper to generate array data */
void generate_array(int *arr, int size, int seed) {
    for (int i = 0; i < size; i++) {
        arr[i] = (i * seed + 123) % 1000;
    }
}

int main(int argc, char *argv[]) {
    int mode = 1;  /* Default to hot path */
    int iterations = HOT_ITERATIONS;
    
    /* Parse command line for mode */
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "mode=", 5) == 0) {
            mode = atoi(argv[i] + 5);
        }
    }
    
    printf("Running mode %d\n", mode);
    
    unsigned long total_result = 0;
    clock_t start = clock();
    
    /* Warm-up phase - creates initial profile */
    if (mode == 0) {
        /* Warm-up with mixed paths */
        total_result += process_hot_path(1, WARM_ITERATIONS);
        total_result += process_hot_path(2, WARM_ITERATIONS / 10);
    } else if (mode == 1) {
        /* Hot path dominant */
        total_result += process_hot_path(1, HOT_ITERATIONS);
        total_result += process_nested_phi(42, HOT_ITERATIONS / 100);
        
        int array_size = 10000;
        int *array = malloc(array_size * sizeof(int));
        generate_array(array, array_size, mode);
        total_result += process_array_with_phi(array, array_size, 500);
        free(array);
        
        total_result += complex_phi_merging(1, 1000);
    } else if (mode == 2) {
        /* Cold path dominant */
        total_result += process_hot_path(2, COLD_ITERATIONS);
        total_result += process_nested_phi(99, COLD_ITERATIONS * 2);
        
        int array_size = 1000;
        int *array = malloc(array_size * sizeof(int));
        generate_array(array, array_size, mode);
        total_result += process_array_with_phi(array, array_size, 800);
        free(array);
        
        total_result += complex_phi_merging(2, 100);
    } else if (mode == 3) {
        /* Mixed path */
        total_result += process_hot_path(1, HOT_ITERATIONS / 2);
        total_result += process_hot_path(2, HOT_ITERATIONS / 20);
        total_result += process_nested_phi(123, HOT_ITERATIONS / 50);
        
        int array_size = 5000;
        int *array = malloc(array_size * sizeof(int));
        generate_array(array, array_size, mode);
        total_result += process_array_with_phi(array, array_size, 300);
        free(array);
        
        total_result += complex_phi_merging(3, 500);
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Result: %lu\n", total_result);
    printf("Time elapsed: %.3f seconds\n", elapsed);
    
    return 0;
}
