/* autofdo_phi_conditional.c
 * 
 * This program generates control flow patterns where boolean values (0/1)
 * flow through PHI nodes into conditional comparisons, with SSA copy chains
 * in between. The runtime behavior creates distinct profile annotations
 * to trigger the uncovered AutoFDO analysis in auto-profile.cc lines 1312-1333.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HOT_ITERATIONS 10000000
#define WARM_ITERATIONS 1000000
#define COLD_ITERATIONS 100

/* Function 1: Simple PHI-to-conditional with direct copy chain */
int process_mode1(int mode, int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        int value_from_phi;
        int tmp1, tmp2, cmp_var;
        
        /* Create branching that feeds into PHI node */
        if (i % 3 == 0) {
            /* Hot path - taken 2/3 of iterations */
            value_from_phi = 1;  /* Becomes PHI operand from this edge */
        } else {
            /* Also hot but different value */
            value_from_phi = (i % 3 == 1) ? 1 : 0;  /* Mixed values */
        }
        
        /* PHI node would be created here by GCC SSA */
        int phi_result = value_from_phi;
        
        /* Create SSA copy chain to trigger while loop walking */
        tmp1 = phi_result;      /* First assignment copy */
        tmp2 = tmp1;           /* Second assignment copy */
        cmp_var = tmp2 + 0;    /* Third with arithmetic that doesn't break pattern */
        
        /* Conditional using the PHI-derived value - should trigger uncovered code */
        if (cmp_var) {  /* Direct use in if condition */
            sum += i * 2;  /* Hot computation */
        } else {
            sum += i;      /* Cold computation */
        }
        
        /* Another conditional with explicit comparison */
        if (cmp_var == 1) {  /* Explicit equality check */
            sum += 1;
        }
    }
    
    return sum;
}

/* Function 2: Nested PHI patterns with complex control flow */
int process_mode2(int mode, int iterations) {
    int sum = 0;
    int outer_loop = iterations / 100;
    
    for (int j = 0; j < outer_loop; j++) {
        int phi_val1, phi_val2;
        
        /* First branching structure */
        if (j % 10 < 7) {  /* 70% hot path */
            phi_val1 = 1;
        } else {
            phi_val1 = 0;
        }
        
        /* Second independent branching */
        if (j % 20 < 15) {  /* 75% hot path */
            phi_val2 = 1;
        } else {
            phi_val2 = 0;
        }
        
        /* PHI nodes would be created here */
        int combined = phi_val1 && phi_val2;
        
        /* Longer SSA copy chain */
        int chain1 = combined;
        int chain2 = chain1;
        int chain3 = chain2;
        int chain4 = chain3;
        int final_cmp = chain4;
        
        /* Inner loop with PHI-derived condition */
        for (int k = 0; k < 100; k++) {
            /* Conditional using the long chain */
            if (final_cmp != 0) {  /* Not-equal-zero comparison */
                sum += j + k;
            } else {
                sum -= j + k;
            }
            
            /* Another conditional in the same basic block */
            if (final_cmp == 1 && k % 2 == 0) {
                sum += 2;
            }
        }
    }
    
    return sum;
}

/* Function 3: PHI in loop condition with varying trip counts */
int process_mode3(int mode, int iterations) {
    int sum = 0;
    int should_continue;
    
    /* Create PHI that controls loop continuation */
    for (int i = 0; i < iterations; ) {
        int continue_value;
        
        /* Branching that determines if we continue */
        if (i < iterations * 9 / 10) {  /* 90% continue */
            continue_value = 1;
        } else {
            continue_value = 0;
        }
        
        /* PHI node for loop control */
        should_continue = continue_value;
        
        /* SSA copies */
        int tmp_continue = should_continue;
        int loop_check = tmp_continue;
        
        /* Use PHI-derived value in loop condition */
        if (!loop_check) {
            break;  /* Rare exit */
        }
        
        /* Loop body - always executed when continue_value is 1 */
        sum += i;
        i++;
        
        /* Nested conditional using same value */
        if (loop_check == 1) {
            sum += i % 7;
        }
    }
    
    return sum;
}

/* Function 4: Multiple PHIs feeding into same conditional */
int process_mode4(int mode, int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        int a, b, c;
        
        /* Three independent branches creating PHIs */
        if (i % 2 == 0) { a = 1; } else { a = 0; }
        if (i % 3 == 0) { b = 1; } else { b = 0; }
        if (i % 5 == 0) { c = 1; } else { c = 0; }
        
        /* Combine PHI results */
        int combined = (a && b) || c;
        
        /* Chain of assignments */
        int x1 = combined;
        int x2 = x1;
        int x3 = x2;
        
        /* Conditional with the PHI-derived value */
        if (x3) {
            /* Hot path - complex computation to prevent optimization */
            for (int j = 0; j < 10; j++) {
                sum += (i * j) % 13;
            }
        }
        
        /* Another use of same value */
        if (x3 == 0) {
            sum -= i;  /* Cold path */
        }
    }
    
    return sum;
}

/* Function 5: Cross-function PHI propagation */
static int global_selector = 0;

int helper_func(int selector, int input) {
    int phi_val;
    
    if (selector > 0) {
        phi_val = 1;
    } else {
        phi_val = 0;
    }
    
    /* PHI node here */
    int result = phi_val;
    
    /* Copy chain */
    int tmp = result;
    int final = tmp;
    
    return final;
}

int process_mode5(int mode, int iterations) {
    int sum = 0;
    
    for (int i = 0; i < iterations; i++) {
        /* Call helper which creates PHI */
        int bool_val = helper_func(i % 100, i);
        
        /* More copies */
        int copy1 = bool_val;
        int copy2 = copy1;
        
        /* Conditional using value from cross-function PHI */
        if (copy2) {
            sum += i * 3;
        } else {
            if (copy2 == 0) {  /* Explicit zero check */
                sum += i;
            }
        }
    }
    
    return sum;
}

/* Main function with profile-generating runtime behavior */
int main(int argc, char *argv[]) {
    int mode = 1;  /* Default to hot mode */
    int iterations = HOT_ITERATIONS;
    
    /* Parse command line for different profile modes */
    if (argc > 1) {
        mode = atoi(argv[1]);
        if (argc > 2) {
            iterations = atoi(argv[2]);
        }
    }
    
    int result = 0;
    clock_t start, end;
    
    start = clock();
    
    /* Warm-up phase - moderate iterations */
    if (mode == 0) {
        printf("Warm-up mode\n");
        result = process_mode1(0, WARM_ITERATIONS);
        result += process_mode2(0, WARM_ITERATIONS / 10);
    }
    /* Hot mode - maximum iterations, dominant path */
    else if (mode == 1) {
        printf("Hot mode - dominant path\n");
        for (int phase = 0; phase < 3; phase++) {
            result += process_mode1(1, iterations);
            result += process_mode2(1, iterations / 100);
            result += process_mode3(1, iterations);
            result += process_mode4(1, iterations / 10);
            result += process_mode5(1, iterations);
        }
    }
    /* Cold mode - rare paths */
    else if (mode == 2) {
        printf("Cold mode - rare paths\n");
        iterations = COLD_ITERATIONS;
        result = process_mode1(2, iterations);
        result += process_mode2(2, iterations);
        result += process_mode3(2, iterations);
    }
    /* Mixed mode - balanced */
    else if (mode == 3) {
        printf("Mixed mode\n");
        result = process_mode1(3, iterations / 2);
        result += process_mode2(3, iterations / 20);
        result += process_mode4(3, iterations / 5);
    }
    
    end = clock();
    
    double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    
    printf("Result: %d\n", result);
    printf("Time used: %.2f seconds\n", cpu_time_used);
    printf("Mode: %d, Iterations: %d\n", mode, iterations);
    
    /* Additional computation to ensure profile diversity */
    if (mode == 1) {
        /* Extra hot computation */
        int array[1000];
        for (int i = 0; i < 1000; i++) {
            array[i] = i;
        }
        for (int i = 0; i < 1000000; i++) {
            result += array[i % 1000] % 17;
        }
    }
    
    return result % 1000;  /* Return checksum for verification */
}
