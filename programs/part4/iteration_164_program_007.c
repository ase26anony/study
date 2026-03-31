/* test_autofdo_phi_cond.c
 * Generates PHI-to-conditional patterns for AutoFDO coverage
 * Compile: gcc -O2 -fauto-profile -o test_autofdo test_autofdo_phi_cond.c
 * Run: ./test_autofdo mode=1 (for hot path) then recompile with profile
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HOT_ITERATIONS 10000000
#define WARM_ITERATIONS 1000000
#define COLD_ITERATIONS 100

/* Function 1: Complex PHI pattern with assignment chains */
int process_with_phi_chains(int mode, int iterations) {
    int result = 0;
    int i, j;
    
    for (i = 0; i < iterations; i++) {
        int phi_val;
        int tmp1, tmp2, tmp3, cmp_var;
        
        /* Create branching that feeds into PHI */
        if (mode == 1) {
            /* Hot path - sets phi_val to 1 */
            phi_val = 1;
        } else if (mode == 2) {
            /* Medium path - sets phi_val based on iteration */
            phi_val = (i % 100 == 0) ? 1 : 0;
        } else {
            /* Cold path - always 0 */
            phi_val = 0;
        }
        
        /* Create SSA assignment chain */
        tmp1 = phi_val;          /* First assignment */
        tmp2 = tmp1;             /* Second assignment */
        tmp3 = tmp2 + 0;         /* Arithmetic that preserves value */
        cmp_var = tmp3;          /* Final assignment before use */
        
        /* Multiple comparison types using PHI-derived value */
        if (cmp_var) {           /* Direct use in if condition */
            result += i * 2;
        }
        
        if (cmp_var == 1) {      /* Explicit equality comparison */
            result += i / 2;
        }
        
        /* Nested conditional with PHI-derived value */
        for (j = 0; j < 10; j++) {
            int nested_tmp = cmp_var;
            if (nested_tmp != 0) {  /* Inequality comparison */
                result += j;
            }
        }
        
        /* Loop condition using PHI-derived value */
        int loop_ctrl = cmp_var;
        while (loop_ctrl > 0) {   /* Loop with PHI-derived control */
            result += 1;
            loop_ctrl--;
        }
    }
    
    return result;
}

/* Function 2: PHI pattern across multiple predecessor blocks */
int phi_across_predecessors(int seed, int limit) {
    int result = 0;
    int i;
    
    for (i = 0; i < limit; i++) {
        int cond_val;
        int a, b, c;
        
        /* Create complex branching to generate PHI */
        if (seed % 3 == 0) {
            a = 1;
            b = 0;
            /* Multiple blocks feeding into PHI */
            if (i % 2 == 0) {
                cond_val = a;
            } else {
                cond_val = b;
            }
        } else if (seed % 3 == 1) {
            a = 0;
            b = 1;
            /* Different pattern for this path */
            cond_val = (i < limit / 2) ? a : b;
        } else {
            a = 0;
            b = 0;
            /* Cold path pattern */
            cond_val = (i == 0) ? 1 : 0;
        }
        
        /* Long assignment chain */
        int chain1 = cond_val;
        int chain2 = chain1;
        int chain3 = chain2;
        int chain4 = chain3 + 0;
        int chain5 = chain4;
        int final_val = chain5;
        
        /* Use in various conditional contexts */
        if (final_val) {
            result += i * i;
        }
        
        if (final_val == 1) {
            result += i % 100;
        }
        
        /* Switch based on PHI-derived value */
        switch (final_val) {
            case 0:
                result -= 1;
                break;
            case 1:
                result += 2;
                break;
            default:
                result += 0;
        }
    }
    
    return result;
}

/* Function 3: Recursive PHI pattern with varying profile */
int recursive_phi_pattern(int depth, int max_depth, int hot_path) {
    if (depth >= max_depth) return 1;
    
    int phi_val;
    int result = 0;
    
    /* PHI value depends on recursion depth and hot_path flag */
    if (hot_path) {
        phi_val = (depth % 2 == 0) ? 1 : 0;
    } else {
        phi_val = (depth < max_depth / 2) ? 1 : 0;
    }
    
    /* Assignment chain */
    int tmp = phi_val;
    int tmp2 = tmp;
    int tmp3 = tmp2 + 0;
    int cmp = tmp3;
    
    /* Conditional using PHI-derived value */
    if (cmp) {
        result += recursive_phi_pattern(depth + 1, max_depth, hot_path);
    }
    
    if (cmp == 1) {
        result += recursive_phi_pattern(depth + 2, max_depth, hot_path);
    }
    
    return result;
}

/* Function 4: Array processing with PHI-based conditions */
int array_phi_processing(int* arr, int size, int threshold) {
    int sum = 0;
    int i;
    
    for (i = 0; i < size; i++) {
        int phi_val;
        
        /* PHI value based on array content and position */
        if (i < size / 2) {
            phi_val = (arr[i] > threshold) ? 1 : 0;
        } else {
            phi_val = (arr[i] < threshold) ? 1 : 0;
        }
        
        /* Multi-step assignment chain */
        int step1 = phi_val;
        int step2 = step1;
        int step3 = step2 + 0;
        int step4 = step3;
        int final = step4;
        
        /* Hot loop with PHI-derived condition */
        for (int j = 0; j < 5; j++) {
            if (final) {
                sum += arr[i] * j;
            }
            
            if (final == 1) {
                sum += j;
            }
        }
        
        /* Nested condition */
        if (final != 0) {
            sum += i;
        }
    }
    
    return sum;
}

/* Main function with profile-generating runtime behavior */
int main(int argc, char** argv) {
    int mode = 1;  /* Default to hot path */
    int iterations = HOT_ITERATIONS;
    int result = 0;
    
    /* Parse command line arguments */
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "mode=", 5) == 0) {
            mode = atoi(argv[i] + 5);
        } else if (strncmp(argv[i], "iter=", 5) == 0) {
            iterations = atoi(argv[i] + 5);
        }
    }
    
    printf("Running mode=%d with iterations=%d\n", mode, iterations);
    
    /* Warm-up phase */
    printf("Warm-up phase...\n");
    for (int warm = 0; warm < WARM_ITERATIONS / 1000; warm++) {
        result += process_with_phi_chains(mode, 100);
    }
    
    /* Main measurement phase with varying behaviors */
    printf("Main computation phase...\n");
    
    /* Dominant hot path for mode=1 */
    if (mode == 1) {
        /* Execute hot PHI patterns millions of times */
        result += process_with_phi_chains(1, iterations);
        result += phi_across_predecessors(1, iterations / 10);
        
        /* Array processing with hot conditions */
        int arr_size = 10000;
        int* arr = malloc(arr_size * sizeof(int));
        for (int i = 0; i < arr_size; i++) {
            arr[i] = i % 100;
        }
        result += array_phi_processing(arr, arr_size, 50);
        free(arr);
        
        /* Recursive hot path */
        result += recursive_phi_pattern(0, 20, 1);
        
    } else if (mode == 2) {
        /* Mixed hot/cold paths */
        result += process_with_phi_chains(2, iterations / 2);
        result += phi_across_predecessors(2, iterations / 20);
        
        /* Array with mixed patterns */
        int arr_size = 5000;
        int* arr = malloc(arr_size * sizeof(int));
        for (int i = 0; i < arr_size; i++) {
            arr[i] = (i % 3 == 0) ? 100 : 10;
        }
        result += array_phi_processing(arr, arr_size, 50);
        free(arr);
        
    } else {
        /* Cold path execution */
        result += process_with_phi_chains(3, COLD_ITERATIONS);
        result += phi_across_predecessors(3, COLD_ITERATIONS / 2);
        
        /* Minimal array processing */
        int arr_size = 100;
        int* arr = malloc(arr_size * sizeof(int));
        for (int i = 0; i < arr_size; i++) {
            arr[i] = i;
        }
        result += array_phi_processing(arr, arr_size, 1000);
        free(arr);
    }
    
    /* Final phase with different pattern */
    printf("Final phase...\n");
    for (int phase = 0; phase < 3; phase++) {
        int phase_mode = (phase == 0) ? 1 : ((phase == 1) ? 2 : 3);
        result += process_with_phi_chains(phase_mode, iterations / 100);
    }
    
    printf("Result checksum: %d\n", result);
    printf("Execution completed.\n");
    
    return 0;
}
