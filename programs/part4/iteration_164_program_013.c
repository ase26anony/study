/* autofdo_phi_conditional.c
 * Test program for GCC AutoFDO profile analysis of PHI-to-conditional patterns
 * Compile with: gcc -O2 -fauto-profile -o autofdo_test autofdo_phi_conditional.c
 * Run with: ./autofdo_test 1 (for hot path) or ./autofdo_test 2 (for cold path)
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HOT_ITERATIONS 10000000
#define COLD_ITERATIONS 100
#define ARRAY_SIZE 1000

/* Function 1: Complex PHI pattern with assignment chains */
int process_with_phi_chain(int mode, int iterations) {
    int result = 0;
    int data[ARRAY_SIZE];
    
    /* Initialize array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i % 100;
    }
    
    for (int iter = 0; iter < iterations; iter++) {
        int phi_value;
        int tmp1, tmp2, tmp3;
        
        /* Create branching that leads to PHI node */
        if (mode == 1) {
            /* Hot path - sets phi_value to 1 */
            phi_value = 1;
        } else if (mode == 2) {
            /* Cold path - sets phi_value to 0 */
            phi_value = 0;
        } else {
            /* Rare path - sets based on iteration */
            phi_value = (iter % 100 == 0) ? 1 : 0;
        }
        
        /* Chain of SSA assignments to trigger while loop in uncovered code */
        tmp1 = phi_value;      /* First assignment */
        tmp2 = tmp1;           /* Second assignment */
        tmp3 = tmp2 + 0;       /* Third assignment (arithmetic that doesn't change value) */
        
        /* Multiple conditional comparisons using the PHI-derived value */
        
        /* Direct use in if condition (phi_derived) */
        if (tmp3) {  /* This becomes if (phi_value) after optimization */
            /* Hot computation path */
            for (int i = 0; i < ARRAY_SIZE; i++) {
                data[i] = (data[i] * 3 + 7) % 100;
            }
        } else {
            /* Cold computation path */
            for (int i = 0; i < ARRAY_SIZE; i++) {
                data[i] = (data[i] + 1) % 100;
            }
        }
        
        /* Explicit equality comparison (phi_derived == 1) */
        int cmp_var = tmp2;  /* Another SSA copy */
        if (cmp_var == 1) {
            /* Another hot path */
            int sum = 0;
            for (int i = 0; i < ARRAY_SIZE; i += 2) {
                sum += data[i];
            }
            result += sum % 1000;
        }
        
        /* Use in loop condition */
        int loop_flag = tmp1;  /* Yet another SSA copy */
        while (loop_flag) {
            /* This loop only executes when phi_value == 1 */
            for (int i = 0; i < 10; i++) {
                data[i] = (data[i] * 2) % 100;
            }
            loop_flag = 0;  /* Break after one iteration */
        }
        
        /* Complex nested conditional with PHI-derived value */
        int final_check = tmp3;  /* Final SSA copy */
        if (final_check != 0) {
            if (iter % 1000 == 0) {
                /* Rare nested path */
                for (int i = 0; i < ARRAY_SIZE; i++) {
                    data[i] = 99 - data[i];
                }
            }
        }
        
        /* Accumulate result to prevent optimization */
        for (int i = 0; i < ARRAY_SIZE; i++) {
            result = (result + data[i]) % 1000000;
        }
    }
    
    return result;
}

/* Function 2: Multiple PHI nodes with different profile characteristics */
int nested_phi_patterns(int depth, int width) {
    int total = 0;
    
    for (int i = 0; i < width; i++) {
        int phi_select;
        
        /* PHI node selection based on multiple conditions */
        if (i < width / 2) {
            phi_select = 1;  /* Hotter path */
        } else if (i < width * 3 / 4) {
            phi_select = 0;  /* Medium path */
        } else {
            phi_select = (i % 10 == 0) ? 1 : 0;  /* Mixed path */
        }
        
        /* SSA assignment chain */
        int chain1 = phi_select;
        int chain2 = chain1;
        int chain3 = chain2 + 0;
        
        /* Conditional using PHI-derived value */
        if (chain3) {
            /* Recursive call for deeper nesting */
            if (depth > 0) {
                total += nested_phi_patterns(depth - 1, width / 2);
            } else {
                total += i * 3;
            }
        } else {
            total += i;
        }
        
        /* Another conditional with explicit comparison */
        int check = chain2;
        if (check == 1) {
            total += width * 2;
        }
    }
    
    return total;
}

/* Function 3: Loop-based PHI patterns with varying trip counts */
int loop_phi_analysis(int base_iterations) {
    int accumulator = 0;
    int loop_control;
    
    /* Outer loop with PHI-controlled inner loops */
    for (int outer = 0; outer < base_iterations / 1000; outer++) {
        /* PHI value based on outer iteration */
        if (outer % 3 == 0) {
            loop_control = 1;  /* Frequent case */
        } else if (outer % 3 == 1) {
            loop_control = 0;  /* Less frequent */
        } else {
            loop_control = (outer % 10 == 0) ? 1 : 0;  /* Rare */
        }
        
        /* SSA propagation */
        int lc_copy1 = loop_control;
        int lc_copy2 = lc_copy1;
        
        /* Inner loop condition based on PHI */
        int inner_iters = lc_copy2 ? 100 : 10;
        for (int inner = 0; inner < inner_iters; inner++) {
            /* Nested conditional with another PHI-like pattern */
            int inner_flag;
            if (inner < inner_iters / 2) {
                inner_flag = 1;
            } else {
                inner_flag = 0;
            }
            
            int flag_copy = inner_flag;
            if (flag_copy) {
                accumulator += (outer * inner) % 100;
            } else {
                accumulator += inner;
            }
        }
        
        /* Post-loop conditional */
        if (lc_copy1) {
            accumulator = (accumulator * 7) % 10000;
        }
    }
    
    return accumulator;
}

/* Function 4: Array processing with PHI-dependent branching */
int array_phi_processing(int* array, int size, int mode) {
    int sum = 0;
    int processing_flag;
    
    /* Set PHI source value based on mode */
    if (mode == 1) {
        processing_flag = 1;  /* Aggressive processing */
    } else {
        processing_flag = 0;  /* Conservative processing */
    }
    
    /* Multiple SSA copies */
    int flag1 = processing_flag;
    int flag2 = flag1;
    int flag3 = flag2 + 0;
    
    for (int i = 0; i < size; i++) {
        int element_flag;
        
        /* Another PHI-like decision per element */
        if (array[i] > 50) {
            element_flag = 1;
        } else {
            element_flag = 0;
        }
        
        int elem_copy = element_flag;
        
        /* Conditional based on outer PHI-derived value */
        if (flag3) {
            /* Hot path processing */
            if (elem_copy) {
                array[i] = (array[i] * 3) % 100;
            } else {
                array[i] = (array[i] + 10) % 100;
            }
        } else {
            /* Cold path processing */
            if (elem_copy == 1) {
                array[i] = array[i] / 2;
            }
        }
        
        /* Another conditional check */
        int check = flag2;
        if (check != 0) {
            sum += array[i] * 2;
        } else {
            sum += array[i];
        }
    }
    
    return sum;
}

/* Main function with profile-generating runtime behavior */
int main(int argc, char** argv) {
    int mode = 1;  /* Default to hot path */
    int iterations = HOT_ITERATIONS;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
        if (mode == 2) {
            iterations = COLD_ITERATIONS;
        }
    }
    
    printf("Running mode %d with %d iterations\n", mode, iterations);
    
    clock_t start = clock();
    
    /* Execute functions with different PHI patterns */
    int result1 = process_with_phi_chain(mode, iterations);
    printf("Result 1: %d\n", result1);
    
    int result2 = nested_phi_patterns(3, 1000);
    printf("Result 2: %d\n", result2);
    
    int result3 = loop_phi_analysis(iterations);
    printf("Result 3: %d\n", result3);
    
    int array[ARRAY_SIZE];
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = i % 100;
    }
    int result4 = array_phi_processing(array, ARRAY_SIZE, mode);
    printf("Result 4: %d\n", result4);
    
    /* Final checksum */
    int final_result = result1 + result2 + result3 + result4;
    printf("Final checksum: %d\n", final_result);
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    printf("Execution time: %.2f seconds\n", elapsed);
    
    return 0;
}
