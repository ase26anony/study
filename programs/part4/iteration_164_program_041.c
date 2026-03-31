/* autofdo_phi_conditional.c
 * Test program for GCC AutoFDO profile transformation of PHI-to-conditional patterns
 * Compile with: gcc -O2 -fauto-profile autofdo_phi_conditional.c -o autofdo_test
 * Run with: ./autofdo_test <mode> <iterations>
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HOT_ITERATIONS 1000000
#define COLD_ITERATIONS 100
#define ARRAY_SIZE 1000

/* Function 1: Hot path with PHI-to-conditional pattern through assignment chain */
int process_hot_path(int mode, int iterations) {
    int sum = 0;
    int i, j;
    
    for (i = 0; i < iterations; i++) {
        int phi_val;
        int tmp1, tmp2, tmp3, cmp_var;
        
        /* Create PHI node based on loop iteration */
        if (i % 2 == 0) {
            phi_val = 1;  /* Hot path value */
        } else {
            phi_val = 0;  /* Less frequent path */
        }
        
        /* Chain of SSA assignments to trigger while loop walking */
        tmp1 = phi_val;
        tmp2 = tmp1;
        tmp3 = tmp2 + 0;  /* Arithmetic that doesn't break single-assignment */
        cmp_var = tmp3;
        
        /* Conditional using PHI-derived value directly */
        if (cmp_var) {  /* This becomes: if (phi_val) after optimization */
            /* Hot path - executed frequently */
            for (j = 0; j < 10; j++) {
                sum += i * j;
            }
        } else {
            /* Cold path - executed less frequently */
            sum += i;
        }
        
        /* Another PHI pattern with explicit comparison */
        int phi_val2;
        if (mode == 1) {
            phi_val2 = 1;
        } else {
            phi_val2 = 0;
        }
        
        int chain1 = phi_val2;
        int chain2 = chain1;
        
        /* Explicit equality comparison */
        if (chain2 == 1) {
            sum += i * 2;
        }
    }
    
    return sum;
}

/* Function 2: Nested conditionals with PHI propagation */
int process_nested_phi(int base, int depth) {
    int result = base;
    int level = 0;
    
    while (level < depth) {
        int phi_select;
        
        /* PHI based on multiple conditions */
        if (level % 3 == 0) {
            phi_select = 1;
        } else if (level % 3 == 1) {
            phi_select = 0;
        } else {
            phi_select = (result % 2 == 0) ? 1 : 0;
        }
        
        /* Assignment chain */
        int a = phi_select;
        int b = a;
        int c = b;
        
        /* Conditional with PHI-derived value */
        if (c != 0) {  /* if (phi_select != 0) */
            result = result * 2 + 1;
            level++;
        } else {
            result = result / 2;
            level += 2;
        }
        
        /* Another PHI pattern inside the loop */
        int inner_phi;
        if (result > 1000) {
            inner_phi = 1;
        } else {
            inner_phi = 0;
        }
        
        int tmp = inner_phi;
        if (tmp) {  /* if (inner_phi) */
            result -= 100;
        }
    }
    
    return result;
}

/* Function 3: Array processing with PHI-based conditions */
void process_array_with_phi(int *array, int size, int threshold) {
    int i;
    int hot_count = 0;
    int cold_count = 0;
    
    for (i = 0; i < size; i++) {
        int use_fast_path;
        
        /* PHI-like selection based on array value */
        if (array[i] > threshold) {
            use_fast_path = 1;
        } else {
            use_fast_path = 0;
        }
        
        /* Multiple assignment chain */
        int stage1 = use_fast_path;
        int stage2 = stage1;
        int stage3 = stage2;
        int final_flag = stage3;
        
        /* Conditional using the PHI-derived value */
        if (final_flag == 1) {
            /* Hot path - optimized processing */
            array[i] = array[i] * 2;
            hot_count++;
            
            /* Nested conditional with another PHI */
            int nested_flag;
            if (array[i] > threshold * 2) {
                nested_flag = 1;
            } else {
                nested_flag = 0;
            }
            
            int n1 = nested_flag;
            int n2 = n1;
            if (n2) {
                array[i] += 100;
            }
        } else {
            /* Cold path - simple processing */
            array[i] = array[i] / 2;
            cold_count++;
        }
    }
    
    /* Final conditional based on counts */
    int summary_flag;
    if (hot_count > cold_count) {
        summary_flag = 1;
    } else {
        summary_flag = 0;
    }
    
    int s1 = summary_flag;
    int s2 = s1;
    if (s2) {
        printf("Hot path dominated: %d vs %d\n", hot_count, cold_count);
    }
}

/* Function 4: Complex control flow with multiple PHI merges */
int complex_phi_pattern(int seed, int iterations) {
    int value = seed;
    int i;
    
    for (i = 0; i < iterations; i++) {
        int branch_selector;
        
        /* Multiple predecessor-like conditions */
        if (value < 0) {
            branch_selector = 0;
            value = -value;
        } else if (value > 1000) {
            branch_selector = 1;
            value = value % 1000;
        } else if (value % 7 == 0) {
            branch_selector = 1;
            value = value * 3 + 1;
        } else {
            branch_selector = 0;
            value = value + i;
        }
        
        /* Long assignment chain */
        int v1 = branch_selector;
        int v2 = v1;
        int v3 = v2;
        int v4 = v3;
        int v5 = v4;
        int cond_var = v5;
        
        /* Loop condition based on PHI-derived value */
        while (cond_var) {  /* while (branch_selector) */
            value = value >> 1;
            cond_var = 0;  /* Break after one iteration */
        }
        
        /* Another PHI for early exit */
        int early_exit;
        if (value > 10000) {
            early_exit = 1;
        } else {
            early_exit = 0;
        }
        
        int ee1 = early_exit;
        int ee2 = ee1;
        if (ee2 == 1) {
            break;
        }
    }
    
    return value;
}

/* Function 5: Mixed hot/cold paths with call site variation */
int mixed_paths_with_calls(int mode, int data) {
    int result = data;
    
    /* PHI for call site selection */
    int call_hot_path;
    if (mode == 1) {
        call_hot_path = 1;  /* Hot call site */
    } else {
        call_hot_path = 0;  /* Cold call site */
    }
    
    int ch1 = call_hot_path;
    int ch2 = ch1;
    
    if (ch2) {
        /* Frequently called - hot path */
        for (int i = 0; i < 100; i++) {
            result = complex_phi_pattern(result, 10);
        }
    } else {
        /* Rarely called - cold path */
        result = process_nested_phi(result, 5);
    }
    
    return result;
}

/* Main function with profile-generating runtime behavior */
int main(int argc, char *argv[]) {
    int mode = 1;  /* Default to hot mode */
    int iterations = HOT_ITERATIONS;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    srand(time(NULL));
    
    printf("Running mode %d with %d iterations\n", mode, iterations);
    
    int total_result = 0;
    
    /* Warm-up phase with different behavior */
    if (mode == 0) {
        /* Cold mode - minimal execution */
        total_result = process_hot_path(0, COLD_ITERATIONS);
        total_result += process_nested_phi(total_result, 3);
    } else if (mode == 1) {
        /* Hot mode - dominant execution profile */
        total_result = process_hot_path(1, iterations);
        
        /* Process array with PHI conditions */
        int array[ARRAY_SIZE];
        for (int i = 0; i < ARRAY_SIZE; i++) {
            array[i] = rand() % 10000;
        }
        process_array_with_phi(array, ARRAY_SIZE, 5000);
        
        /* Mix in complex patterns */
        total_result += complex_phi_pattern(total_result, iterations / 100);
        
        /* Call from hot path */
        total_result = mixed_paths_with_calls(1, total_result);
    } else if (mode == 2) {
        /* Mixed mode - balanced profile */
        for (int phase = 0; phase < 10; phase++) {
            if (phase % 3 == 0) {
                total_result += process_hot_path(0, iterations / 10);
            } else {
                total_result += process_nested_phi(total_result, 5);
            }
        }
    }
    
    /* Final checksum calculation */
    int checksum = 0;
    for (int i = 0; i < 1000; i++) {
        int flag;
        if (total_result % (i + 1) == 0) {
            flag = 1;
        } else {
            flag = 0;
        }
        
        int f1 = flag;
        int f2 = f1;
        if (f2) {
            checksum += i;
        }
    }
    
    printf("Result: %d, Checksum: %d\n", total_result, checksum);
    
    return 0;
}
