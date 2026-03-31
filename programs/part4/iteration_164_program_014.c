/* autofdo_phi_analysis.c
 * 
 * This program generates specific control flow patterns to trigger
 * GCC's AutoFDO PHI-to-conditional analysis in auto-profile.cc
 * lines 1312-1333.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HOT_ITERATIONS 1000000
#define COLD_ITERATIONS 100
#define ARRAY_SIZE 10000

/* Function 1: Simple PHI-to-conditional with assignment chain */
int phi_chain_hot_path(int mode, int iterations) {
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        int phi_val;
        int tmp1, tmp2, cmp_var;
        
        /* Create PHI node with values from different predecessors */
        if (i % 2 == 0) {
            /* Hot path - executed frequently */
            phi_val = 1;  /* Constant 1 */
        } else {
            /* Less frequent path */
            phi_val = 0;  /* Constant 0 */
        }
        
        /* Create SSA assignment chain */
        tmp1 = phi_val;      /* First assignment */
        tmp2 = tmp1;         /* Second assignment */
        cmp_var = tmp2 + 0;  /* Third assignment with arithmetic */
        
        /* Conditional using PHI-derived value - triggers uncovered code */
        if (cmp_var) {  /* Direct use in if condition */
            result += i * 2;
        } else {
            result += i;
        }
        
        /* Another comparison type */
        if (cmp_var == 1) {  /* Explicit equality comparison */
            result += 1;
        }
    }
    
    return result;
}

/* Function 2: Nested loops with complex PHI patterns */
int nested_phi_analysis(int mode, int outer_iter) {
    int total = 0;
    int data[ARRAY_SIZE];
    
    /* Initialize array */
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = i % 100;
    }
    
    for (int outer = 0; outer < outer_iter; outer++) {
        int phi_select;
        
        /* PHI with multiple predecessors */
        if (mode == 1) {
            phi_select = 1;  /* Hot path value */
        } else if (mode == 2) {
            phi_select = 0;  /* Cold path value */
        } else {
            phi_select = (outer % 3 == 0) ? 1 : 0;
        }
        
        /* Long assignment chain */
        int chain1 = phi_select;
        int chain2 = chain1;
        int chain3 = chain2;
        int chain4 = chain3;
        int final_val = chain4;
        
        /* Process array with PHI-controlled loop */
        int i = 0;
        while (final_val && i < ARRAY_SIZE) {  /* PHI in loop condition */
            if (final_val == 1) {  /* Explicit comparison */
                data[i] = data[i] * 2 + 1;
                total += data[i];
            }
            i += (final_val != 0) ? 2 : 1;  /* Another comparison */
        }
        
        /* Conditional with copied value */
        int check_val = final_val;
        if (check_val) {
            total += outer * 100;
        }
    }
    
    return total;
}

/* Function 3: Multiple PHI nodes feeding into same conditional */
int multi_phi_convergence(int seed, int iterations) {
    int sum = 0;
    int phi_a, phi_b, combined;
    
    for (int i = 0; i < iterations; i++) {
        /* Two independent PHI nodes */
        if (i % 10 == 0) {
            phi_a = 1;
        } else {
            phi_a = 0;
        }
        
        if (i % 7 == 0) {
            phi_b = 1;
        } else {
            phi_b = 0;
        }
        
        /* Combine PHI results */
        combined = phi_a & phi_b;
        
        /* Assignment chain */
        int tmp = combined;
        int cmp = tmp;
        
        /* Multiple comparison types */
        if (cmp) {
            sum += i * 3;
        }
        
        if (cmp == 1) {
            sum += 7;
        }
        
        if (cmp != 0) {
            sum += 11;
        }
    }
    
    return sum;
}

/* Function 4: PHI in switch-like pattern */
int phi_switch_pattern(int mode, int size) {
    int result = 0;
    int values[100];
    
    for (int i = 0; i < 100; i++) {
        values[i] = i;
    }
    
    for (int i = 0; i < size; i++) {
        int selector;
        
        /* Complex PHI selection */
        switch (mode) {
            case 1:  /* Hot path */
                selector = 1;
                break;
            case 2:  /* Medium path */
                selector = (i % 2 == 0) ? 1 : 0;
                break;
            default: /* Cold path */
                selector = 0;
                break;
        }
        
        /* Assignment propagation */
        int s1 = selector;
        int s2 = s1;
        int s3 = s2 + 0;  /* Arithmetic that preserves value */
        
        /* PHI-derived conditional in nested loop */
        for (int j = 0; j < 10 && s3; j++) {
            result += values[(i + j) % 100];
        }
        
        /* Another conditional */
        if (s3 == 1) {
            result += i * i;
        }
    }
    
    return result;
}

/* Function 5: Recursive PHI pattern */
int recursive_phi_helper(int depth, int max_depth, int *counter) {
    if (depth >= max_depth) {
        return 1;
    }
    
    int phi_val;
    if (depth % 2 == 0) {
        phi_val = 1;
    } else {
        phi_val = 0;
    }
    
    /* Assignment chain */
    int v1 = phi_val;
    int v2 = v1;
    
    int result = 0;
    if (v2) {
        result += recursive_phi_helper(depth + 1, max_depth, counter);
    }
    
    if (v2 == 1) {
        (*counter)++;
        result += depth * 10;
    }
    
    return result;
}

int recursive_phi_driver(int iterations) {
    int total = 0;
    int counter = 0;
    
    for (int i = 0; i < iterations; i++) {
        total += recursive_phi_helper(0, 5, &counter);
    }
    
    return total + counter;
}

/* Main function with profile-generating behavior */
int main(int argc, char *argv[]) {
    int mode = 1;  /* Default to hot mode */
    int iterations = HOT_ITERATIONS;
    
    /* Parse command line for mode selection */
    if (argc > 1) {
        mode = atoi(argv[1]);
        if (mode < 1 || mode > 3) {
            mode = 1;
        }
    }
    
    if (argc > 2) {
        iterations = atoi(argv[2]);
        if (iterations < 1) {
            iterations = HOT_ITERATIONS;
        }
    }
    
    printf("Running AutoFDO PHI analysis test - Mode %d, Iterations %d\n", 
           mode, iterations);
    
    int result = 0;
    clock_t start = clock();
    
    /* Warm-up phase - establish baseline profile */
    for (int warm = 0; warm < 1000; warm++) {
        result += phi_chain_hot_path(mode, 100);
    }
    
    /* Main measurement phase with different behaviors per mode */
    switch (mode) {
        case 1:  /* Hot path dominant */
            result += phi_chain_hot_path(1, iterations);
            result += nested_phi_analysis(1, iterations / 100);
            result += multi_phi_convergence(42, iterations);
            result += phi_switch_pattern(1, iterations / 10);
            result += recursive_phi_driver(iterations / 1000);
            break;
            
        case 2:  /* Mixed hot/cold */
            result += phi_chain_hot_path(2, iterations / 2);
            result += nested_phi_analysis(2, iterations / 200);
            result += multi_phi_convergence(123, iterations / 2);
            result += phi_switch_pattern(2, iterations / 20);
            result += recursive_phi_driver(iterations / 2000);
            break;
            
        case 3:  /* Cold path dominant */
            result += phi_chain_hot_path(3, COLD_ITERATIONS);
            result += nested_phi_analysis(3, COLD_ITERATIONS / 10);
            result += multi_phi_convergence(999, COLD_ITERATIONS);
            result += phi_switch_pattern(3, COLD_ITERATIONS / 5);
            result += recursive_phi_driver(COLD_ITERATIONS / 50);
            break;
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Result checksum: %d\n", result);
    printf("Execution time: %.3f seconds\n", elapsed);
    
    /* Additional verification computation */
    int verify = 0;
    for (int i = 0; i < 1000; i++) {
        int phi_test = (i % 3 == 0) ? 1 : 0;
        int tmp = phi_test;
        if (tmp) {
            verify += i;
        }
        if (tmp == 1) {
            verify += 1;
        }
    }
    
    printf("Verification value: %d\n", verify);
    
    return 0;
}
