/* autofdo_phi_test.c
 * Test program to trigger GCC AutoFDO profile analysis for PHI-to-conditional patterns
 * Compile with: gcc -O2 -fauto-profile autofdo_phi_test.c -o autofdo_phi_test
 * Run with: ./autofdo_phi_test <mode> <iterations>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define HOT_LOOP_ITERATIONS 1000000
#define WARM_LOOP_ITERATIONS 10000
#define COLD_LOOP_ITERATIONS 10

/* Function 1: Complex PHI pattern with SSA copy chains */
uint64_t process_with_phi_chains(int mode, int iterations) {
    uint64_t result = 0;
    int i, j;
    
    /* Outer loop to create hot/cold path annotations */
    for (i = 0; i < iterations; i++) {
        int phi_value;
        int tmp1, tmp2, tmp3;
        
        /* Create branching that feeds into PHI node */
        if (mode == 1) {
            /* Hot path - executed frequently */
            phi_value = 1;  /* This becomes PHI input from this edge */
        } else if (mode == 2) {
            /* Medium path */
            phi_value = (i % 100 == 0) ? 1 : 0;
        } else {
            /* Cold path - rarely executed */
            phi_value = 0;  /* This becomes PHI input from this edge */
        }
        
        /* PHI node is conceptually created here by compiler */
        int phi_result = phi_value;
        
        /* Create SSA copy chain to trigger while loop in uncovered code */
        tmp1 = phi_result;      /* First assignment */
        tmp2 = tmp1;            /* Second assignment */
        tmp3 = tmp2 + 0;        /* Third assignment with arithmetic that doesn't break pattern */
        int cmp_var = tmp3;     /* Fourth assignment */
        
        /* Conditional using PHI-derived value - triggers uncovered analysis */
        if (cmp_var) {  /* Direct use: if (phi_derived) */
            /* Hot path computation */
            for (j = 0; j < 100; j++) {
                result += (i * j) & 0xFF;
            }
        } else {
            /* Cold path computation */
            result += i & 0xF;
        }
        
        /* Another conditional with explicit comparison */
        int cmp_var2 = cmp_var;
        if (cmp_var2 == 1) {  /* Explicit equality: if (phi_derived == 1) */
            result ^= 0xABCD;
        }
    }
    
    return result;
}

/* Function 2: Nested loops with varying PHI patterns */
uint64_t nested_phi_patterns(int hot_level, int size) {
    uint64_t sum = 0;
    int i, j;
    
    for (i = 0; i < size; i++) {
        int outer_phi;
        
        /* Create PHI based on outer loop condition */
        if (hot_level > 5) {
            outer_phi = 1;
        } else if (hot_level > 2) {
            outer_phi = (i % 3 == 0) ? 1 : 0;
        } else {
            outer_phi = 0;
        }
        
        /* Chain of assignments */
        int chain1 = outer_phi;
        int chain2 = chain1;
        int chain3 = chain2;
        
        for (j = 0; j < 100; j++) {
            int inner_phi;
            
            /* Inner PHI node */
            if (chain3 && (j % 10 == 0)) {
                inner_phi = 1;
            } else {
                inner_phi = 0;
            }
            
            /* More SSA copies */
            int inner_tmp1 = inner_phi;
            int inner_tmp2 = inner_tmp1;
            
            /* Multiple conditionals using PHI values */
            if (inner_tmp2 != 0) {  /* if (phi_derived != 0) */
                sum += (i * j * hot_level);
            }
            
            if (inner_tmp1 == 1) {  /* Another comparison */
                sum ^= j;
            }
        }
        
        /* Loop condition using PHI-derived value */
        int loop_control = chain1;
        while (loop_control && i < size/2) {  /* while (phi_derived) */
            sum += i;
            loop_control = 0;  /* Break after one iteration */
        }
    }
    
    return sum;
}

/* Function 3: Array processing with data-dependent PHIs */
uint64_t array_based_phi_test(int* data, int length, int threshold) {
    uint64_t checksum = 0;
    int i;
    
    for (i = 0; i < length; i++) {
        int value = data[i];
        int phi_select;
        
        /* PHI inputs from different conditions */
        if (value > threshold) {
            phi_select = 1;
        } else if (value == threshold) {
            phi_select = (i % 2 == 0) ? 1 : 0;
        } else {
            phi_select = 0;
        }
        
        /* Complex SSA copy network */
        int a = phi_select;
        int b = a;
        int c = b + 0;  /* Arithmetic that preserves value */
        int d = c;
        int e = d;
        
        /* Multiple uses of PHI-derived value */
        if (a) {
            checksum += value * 2;
        }
        
        if (e == 1) {
            checksum ^= value;
        }
        
        /* Nested condition based on PHI */
        if (b) {
            for (int k = 0; k < 3; k++) {
                if (d) {  /* PHI-derived value used again */
                    checksum += k;
                }
            }
        }
    }
    
    return checksum;
}

/* Function 4: Recursive pattern with PHI propagation */
uint64_t recursive_phi_helper(int depth, int max_depth, int hot_path) {
    if (depth >= max_depth) {
        return 1;
    }
    
    int phi_val;
    
    /* PHI-like selection based on recursion depth */
    if (hot_path && depth < max_depth / 2) {
        phi_val = 1;
    } else {
        phi_val = (depth % 3 == 0) ? 1 : 0;
    }
    
    /* Assignment chain */
    int chain1 = phi_val;
    int chain2 = chain1;
    
    uint64_t result = 0;
    
    /* Conditional using PHI-derived value */
    if (chain2) {
        result += recursive_phi_helper(depth + 1, max_depth, hot_path) * 3;
    } else {
        result += recursive_phi_helper(depth + 1, max_depth, 0);
    }
    
    /* Another conditional */
    if (chain1 == 1) {
        result += depth * 7;
    }
    
    return result;
}

/* Main function with different execution modes */
int main(int argc, char** argv) {
    int mode = 1;  /* Default to hot mode */
    int iterations = HOT_LOOP_ITERATIONS;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    printf("Running AutoFDO PHI test with mode=%d, iterations=%d\n", mode, iterations);
    
    uint64_t total_result = 0;
    clock_t start = clock();
    
    /* Warm-up phase - different profile */
    if (mode == 3) {
        printf("Cold path warm-up\n");
        total_result += process_with_phi_chains(0, WARM_LOOP_ITERATIONS);
    }
    
    /* Main execution with selected mode */
    switch (mode) {
        case 1:  /* Hot path dominant */
            printf("Executing hot path...\n");
            total_result += process_with_phi_chains(1, iterations);
            total_result += nested_phi_patterns(10, iterations / 100);
            break;
            
        case 2:  /* Mixed hot/cold */
            printf("Executing mixed paths...\n");
            total_result += process_with_phi_chains(2, iterations);
            total_result += nested_phi_patterns(5, iterations / 50);
            break;
            
        case 3:  /* Cold path dominant */
            printf("Executing cold path...\n");
            total_result += process_with_phi_chains(3, COLD_LOOP_ITERATIONS);
            total_result += nested_phi_patterns(1, COLD_LOOP_ITERATIONS * 10);
            break;
            
        default:
            printf("Unknown mode, using default hot path\n");
            total_result += process_with_phi_chains(1, iterations);
    }
    
    /* Array-based test with data-dependent PHIs */
    int array_size = 10000;
    int* data_array = malloc(array_size * sizeof(int));
    if (data_array) {
        for (int i = 0; i < array_size; i++) {
            data_array[i] = (i * 17) % 100;
        }
        
        int threshold = (mode == 1) ? 30 : 70;
        total_result += array_based_phi_test(data_array, array_size, threshold);
        
        free(data_array);
    }
    
    /* Recursive test */
    total_result += recursive_phi_helper(0, (mode == 1) ? 15 : 8, mode == 1);
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Result: %lu\n", total_result);
    printf("Time elapsed: %.3f seconds\n", elapsed);
    printf("Mode %d completed\n", mode);
    
    return 0;
}
