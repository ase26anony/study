/* autofdo_phi_test.c - Test program for GCC AutoFDO PHI-to-conditional analysis
 * 
 * This program creates patterns where boolean values (0/1) flow through
 * PHI nodes into conditional comparisons, with SSA copy chains in between.
 * The runtime behavior generates distinct profiles for AutoFDO annotation.
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
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        int cond_value;
        
        /* Create PHI node scenario: different values from different predecessors */
        if (i % 100 < 95) {  /* Hot path - 95% probability */
            cond_value = 1;  /* Value 1 from this predecessor */
        } else {             /* Cold path - 5% probability */
            cond_value = 0;  /* Value 0 from this predecessor */
        }
        
        /* PHI node would be created here for cond_value */
        int phi_result = cond_value;
        
        /* Create SSA copy chain to trigger the while loop walking back */
        int tmp1 = phi_result;
        int tmp2 = tmp1;
        int tmp3 = tmp2 + 0;  /* Arithmetic that doesn't break single-assignment */
        int cmp_var = tmp3;
        
        /* Conditional comparison using PHI-derived value */
        if (cmp_var) {  /* Direct use in if condition */
            result += i * 2;  /* Hot path computation */
        } else {
            result += i / 2;  /* Cold path computation */
        }
        
        /* Another comparison type: explicit equality */
        if (cmp_var == 1) {
            result += i % 100;
        }
    }
    
    return result;
}

/* Function 2: Nested PHI patterns with complex control flow */
int process_mode2(int mode, int iterations) {
    int result = 0;
    int outer_cond = (mode == 2) ? 1 : 0;
    
    for (int i = 0; i < iterations; i++) {
        int inner_cond1, inner_cond2;
        
        /* Two-level PHI pattern */
        if (i % 3 == 0) {
            inner_cond1 = 1;
        } else {
            inner_cond1 = 0;
        }
        
        if (i % 7 == 0) {
            inner_cond2 = 1;
        } else {
            inner_cond2 = 0;
        }
        
        /* PHI node for combined condition */
        int combined = (inner_cond1 && outer_cond) ? 1 : 0;
        
        /* Longer SSA copy chain */
        int chain1 = combined;
        int chain2 = chain1;
        int chain3 = chain2;
        int chain4 = chain3 + 0;
        int final_cond = chain4;
        
        /* Multiple conditional blocks with varying hotness */
        if (final_cond) {
            /* Hot when mode == 2 */
            for (int j = 0; j < 10; j++) {
                if (j % 2 == 0) {
                    result += i * j;
                } else {
                    result += i + j;
                }
            }
        } else {
            /* Cold when mode == 2 */
            result -= i;
        }
        
        /* Another PHI-to-conditional in loop condition */
        int loop_cond;
        if (i % 1000 == 0) {
            loop_cond = 1;
        } else {
            loop_cond = 0;
        }
        
        int loop_tmp = loop_cond;
        while (loop_tmp) {  /* Rarely executed loop */
            result += 1;
            loop_tmp = 0;  /* Execute once */
        }
    }
    
    return result;
}

/* Function 3: Array processing with PHI-dependent branches */
int process_mode3(int mode, int iterations) {
    int result = 0;
    int* data = (int*)malloc(iterations * sizeof(int));
    
    /* Initialize array with pattern */
    for (int i = 0; i < iterations; i++) {
        data[i] = i % 100;
    }
    
    /* Process array with PHI-to-conditional in hot loop */
    for (int i = 0; i < iterations; i++) {
        int threshold_cond;
        
        /* PHI based on array value */
        if (data[i] > 50) {
            threshold_cond = 1;
        } else {
            threshold_cond = 0;
        }
        
        /* SSA copy chain */
        int t1 = threshold_cond;
        int t2 = t1;
        int t3 = t2 + 0;
        
        /* Conditional with != 0 comparison */
        if (t3 != 0) {
            /* Hot path for values > 50 */
            result += data[i] * 3;
            
            /* Nested condition with another PHI */
            int nested_cond;
            if (data[i] > 75) {
                nested_cond = 1;
            } else {
                nested_cond = 0;
            }
            
            int n1 = nested_cond;
            if (n1 == 1) {  /* Explicit equality comparison */
                result += 1000;
            }
        } else {
            /* Cold path for values <= 50 */
            result += data[i];
        }
        
        /* Additional PHI pattern based on iteration count */
        int iter_cond;
        if (i % 10000 == 0) {
            iter_cond = 1;
        } else {
            iter_cond = 0;
        }
        
        int ic1 = iter_cond;
        int ic2 = ic1;
        if (ic2) {
            result += 99999;  /* Rarely executed */
        }
    }
    
    free(data);
    return result;
}

/* Function 4: Complex control flow with multiple PHI merges */
int process_mode4(int mode, int iterations) {
    int result = 0;
    
    for (int i = 0; i < iterations; i++) {
        int a, b, c;
        
        /* Multiple independent conditions creating PHI opportunities */
        if (i % 2 == 0) a = 1; else a = 0;
        if (i % 3 == 0) b = 1; else b = 0;
        if (i % 5 == 0) c = 1; else c = 0;
        
        /* Complex PHI merge pattern */
        int merged;
        if (a) {
            merged = b ? 1 : 0;
        } else {
            merged = c ? 1 : 0;
        }
        
        /* Extended SSA copy chain */
        int m1 = merged;
        int m2 = m1;
        int m3 = m2 + 0;
        int m4 = m3;
        int m5 = m4;
        int final = m5;
        
        /* Switch-like behavior using PHI result */
        if (final) {
            /* Path A - frequency depends on complex condition */
            for (int j = 0; j < 5; j++) {
                result += (i + j) * (j + 1);
            }
        } else {
            /* Path B */
            result -= i * 2;
        }
        
        /* Another PHI pattern in nested loop */
        for (int j = 0; j < 10; j++) {
            int loop_phi;
            if (j < 5) {
                loop_phi = 1;
            } else {
                loop_phi = 0;
            }
            
            int lp1 = loop_phi;
            int lp2 = lp1;
            if (lp2 == 1) {
                result += j;
            } else {
                result -= j;
            }
        }
    }
    
    return result;
}

/* Helper function to create call site variety for profile annotation */
int dispatch_processor(int mode, int iterations) {
    switch (mode) {
        case 1:
            return process_mode1(mode, iterations);
        case 2:
            return process_mode2(mode, iterations);
        case 3:
            return process_mode3(mode, iterations);
        case 4:
            return process_mode4(mode, iterations);
        default:
            return process_mode1(1, iterations / 10);
    }
}

/* Warm-up function to establish profile patterns */
void warmup_profile() {
    printf("Starting warm-up phase...\n");
    
    /* Execute all modes briefly to establish call graph */
    for (int mode = 1; mode <= 4; mode++) {
        int warmup_result = dispatch_processor(mode, WARM_ITERATIONS);
        printf("  Mode %d warm-up: result = %d\n", mode, warmup_result);
    }
}

int main(int argc, char** argv) {
    int dominant_mode = 1;  /* Default dominant mode */
    int total_iterations = HOT_ITERATIONS;
    
    /* Parse command line arguments */
    if (argc > 1) {
        dominant_mode = atoi(argv[1]);
        if (dominant_mode < 1 || dominant_mode > 4) {
            dominant_mode = 1;
        }
    }
    
    if (argc > 2) {
        total_iterations = atoi(argv[2]);
        if (total_iterations < 1000) {
            total_iterations = HOT_ITERATIONS;
        }
    }
    
    printf("AutoFDO PHI Test Program\n");
    printf("Dominant mode: %d, Iterations: %d\n\n", dominant_mode, total_iterations);
    
    /* Phase 1: Warm-up to establish profile patterns */
    warmup_profile();
    
    /* Phase 2: Dominant execution with hot path */
    printf("\nStarting dominant execution (mode %d)...\n", dominant_mode);
    clock_t start = clock();
    
    int dominant_result = dispatch_processor(dominant_mode, total_iterations);
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Dominant execution complete:\n");
    printf("  Result: %d\n", dominant_result);
    printf("  Time: %.2f seconds\n", elapsed);
    
    /* Phase 3: Brief cold path execution */
    printf("\nExecuting cold paths...\n");
    int cold_result = 0;
    
    for (int mode = 1; mode <= 4; mode++) {
        if (mode != dominant_mode) {
            cold_result += dispatch_processor(mode, COLD_ITERATIONS);
        }
    }
    
    printf("Cold paths result: %d\n", cold_result);
    
    /* Phase 4: Mixed execution for edge case coverage */
    printf("\nMixed execution for edge cases...\n");
    int mixed_result = 0;
    
    /* Create varying profile across different call sites */
    for (int i = 0; i < total_iterations / 1000; i++) {
        int current_mode = (i % 10 == 0) ? dominant_mode : ((i % 10) % 4 + 1);
        mixed_result += dispatch_processor(current_mode, 100);
    }
    
    printf("Mixed execution result: %d\n", mixed_result);
    
    /* Final checksum */
    int final_checksum = dominant_result + cold_result + mixed_result;
    printf("\nFinal checksum: %d\n", final_checksum);
    
    return 0;
}
