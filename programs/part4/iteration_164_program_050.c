/* autofdo_phi_cond_test.c
 * Test program to trigger GCC AutoFDO PHI-to-conditional analysis
 * Compile with: gcc -O2 -fauto-profile -o autofdo_test autofdo_phi_cond_test.c
 * Run with: ./autofdo_test 1 (for hot path) then recompile with profile
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HOT_ITERATIONS 10000000
#define COLD_ITERATIONS 100
#define ARRAY_SIZE 1000

/* Function 1: Complex PHI pattern with SSA copy chains */
int process_with_phi_chain(int mode, int limit) {
    int result = 0;
    int i;
    
    for (i = 0; i < limit; i++) {
        int phi_val;
        int tmp1, tmp2, tmp3;
        int final_cmp;
        
        /* Create different predecessor paths that merge at PHI */
        if (mode == 1) {
            /* Hot path - sets phi_val to 1 */
            phi_val = 1;
        } else if (mode == 2) {
            /* Medium path - sets phi_val based on i parity */
            phi_val = (i % 2 == 0) ? 1 : 0;
        } else {
            /* Cold path - always 0 */
            phi_val = 0;
        }
        
        /* SSA copy chain to trigger while loop walking */
        tmp1 = phi_val;      /* First assignment */
        tmp2 = tmp1;         /* Second assignment */
        tmp3 = tmp2 + 0;     /* Arithmetic that preserves value */
        final_cmp = tmp3;    /* Final copy before comparison */
        
        /* Multiple comparison types using PHI-derived value */
        if (final_cmp) {  /* Direct use in if condition */
            result += i * 2;
        }
        
        if (final_cmp == 1) {  /* Explicit equality comparison */
            result += i;
        }
        
        /* Nested conditional with PHI-derived value */
        while (final_cmp && (i % 10) < 5) {
            result += 1;
            break;
        }
    }
    
    return result;
}

/* Function 2: Multiple PHI nodes feeding into different conditionals */
int nested_phi_patterns(int seed, int iterations) {
    int total = 0;
    int data[ARRAY_SIZE];
    int i, j;
    
    /* Initialize array */
    for (i = 0; i < ARRAY_SIZE; i++) {
        data[i] = (i * seed) % 100;
    }
    
    for (i = 0; i < iterations; i++) {
        int phi_select;
        int chain1, chain2, chain3;
        
        /* PHI selection based on complex condition */
        if (i < iterations / 2) {
            phi_select = 1;
        } else {
            phi_select = (data[i % ARRAY_SIZE] > 50) ? 1 : 0;
        }
        
        /* Longer SSA copy chain */
        chain1 = phi_select;
        chain2 = chain1;
        int intermediate = chain2;
        chain3 = intermediate;
        
        /* Use in loop condition */
        int k = 0;
        while (chain3 && k < 5) {
            total += data[(i + k) % ARRAY_SIZE];
            k++;
        }
        
        /* Another conditional using the same PHI-derived value */
        if (chain3 == 1) {
            for (j = 0; j < 3; j++) {
                total -= data[(i + j) % ARRAY_SIZE] / 2;
            }
        }
    }
    
    return total;
}

/* Function 3: PHI from function call results */
int phi_from_calls(int toggle, int count) {
    int sum = 0;
    int call_result;
    int copy1, copy2;
    
    for (int i = 0; i < count; i++) {
        /* PHI-like selection from different function call patterns */
        if (toggle) {
            call_result = (i % 100 == 0) ? 1 : 0;
        } else {
            call_result = (i < count / 10) ? 1 : 0;
        }
        
        /* SSA copies */
        copy1 = call_result;
        copy2 = copy1;
        
        /* Multiple conditionals with the same PHI value */
        if (copy2) {
            sum += i * i;
        }
        
        if (copy2 != 0) {
            sum += i;
        }
        
        /* Nested with arithmetic */
        if (copy2 == 1 && (sum % 2) == 0) {
            sum += 100;
        }
    }
    
    return sum;
}

/* Function 4: Complex control flow with multiple merging points */
int complex_control_flow(int mode, int size) {
    int matrix[100][100];
    int result = 0;
    int i, j;
    
    /* Initialize */
    for (i = 0; i < 100; i++) {
        for (j = 0; j < 100; j++) {
            matrix[i][j] = (i * j + mode) % 100;
        }
    }
    
    for (i = 0; i < size; i++) {
        int phi_val_a, phi_val_b;
        int chain_a1, chain_a2, chain_b1, chain_b2;
        
        /* Two independent PHI patterns */
        if (mode == 1) {
            phi_val_a = 1;
            phi_val_b = (i % 3 == 0) ? 1 : 0;
        } else {
            phi_val_a = (matrix[i % 100][0] > 30) ? 1 : 0;
            phi_val_b = 0;
        }
        
        /* Copy chains for both PHI values */
        chain_a1 = phi_val_a;
        chain_a2 = chain_a1;
        chain_b1 = phi_val_b;
        chain_b2 = chain_b1;
        
        /* Interdependent conditionals */
        if (chain_a2) {
            for (j = 0; j < 10; j++) {
                result += matrix[i % 100][j % 100];
            }
        }
        
        if (chain_b2 == 1) {
            result -= matrix[i % 100][99];
        }
        
        /* Combined condition */
        if (chain_a2 && chain_b2) {
            result *= 2;
        }
    }
    
    return result;
}

/* Main function with profile-generating runtime behavior */
int main(int argc, char *argv[]) {
    int mode = 1;  /* Default to hot path */
    int result1, result2, result3, result4;
    clock_t start, end;
    double cpu_time_used;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    printf("Running mode %d\n", mode);
    
    start = clock();
    
    /* Execute different paths based on mode to create varied profile */
    switch (mode) {
        case 1:  /* Hot path - executes millions of times */
            result1 = process_with_phi_chain(1, HOT_ITERATIONS);
            result2 = nested_phi_patterns(42, HOT_ITERATIONS / 10);
            result3 = phi_from_calls(1, HOT_ITERATIONS);
            result4 = complex_control_flow(1, HOT_ITERATIONS / 100);
            break;
            
        case 2:  /* Medium path - mixed behavior */
            result1 = process_with_phi_chain(2, HOT_ITERATIONS / 2);
            result2 = nested_phi_patterns(17, HOT_ITERATIONS / 20);
            result3 = phi_from_calls(0, HOT_ITERATIONS / 2);
            result4 = complex_control_flow(2, HOT_ITERATIONS / 200);
            break;
            
        case 3:  /* Cold path - minimal execution */
            result1 = process_with_phi_chain(3, COLD_ITERATIONS);
            result2 = nested_phi_patterns(99, COLD_ITERATIONS);
            result3 = phi_from_calls(0, COLD_ITERATIONS);
            result4 = complex_control_flow(3, COLD_ITERATIONS);
            break;
            
        default:
            printf("Invalid mode. Using mode 1.\n");
            result1 = process_with_phi_chain(1, HOT_ITERATIONS);
            result2 = nested_phi_patterns(42, HOT_ITERATIONS / 10);
            result3 = phi_from_calls(1, HOT_ITERATIONS);
            result4 = complex_control_flow(1, HOT_ITERATIONS / 100);
            break;
    }
    
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    
    /* Calculate checksum for verification */
    int checksum = result1 ^ result2 ^ result3 ^ result4;
    
    printf("Results: %d, %d, %d, %d\n", result1, result2, result3, result4);
    printf("Checksum: %d\n", checksum);
    printf("Time used: %f seconds\n", cpu_time_used);
    
    return 0;
}
