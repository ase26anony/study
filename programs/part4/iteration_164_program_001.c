/* auto-profile-test.c - Test program for AutoFDO PHI-to-conditional analysis */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define HOT_ITERATIONS 10000000
#define WARM_ITERATIONS 1000000
#define COLD_ITERATIONS 100

/* Global checksum for verification */
static uint64_t global_checksum = 0;

/* Function 1: Complex PHI pattern with SSA copy chains */
__attribute__((noinline))
uint64_t process_with_phi_chains(int mode, int iterations) {
    uint64_t sum = 0;
    int phi_result, tmp1, tmp2, tmp3, cmp_var;
    
    for (int i = 0; i < iterations; i++) {
        /* Create branching that feeds into PHI node */
        if (i % 100 < 95) {  /* Hot path - 95% probability */
            phi_result = 1;  /* Value 1 in hot predecessor */
        } else {
            phi_result = 0;  /* Value 0 in cold predecessor */
        }
        
        /* PHI node would be created here after SSA */
        int phi_val = phi_result;
        
        /* Chain of SSA assignments to trigger while loop */
        tmp1 = phi_val;
        tmp2 = tmp1;
        tmp3 = tmp2 + 0;  /* Arithmetic that doesn't change value */
        cmp_var = tmp3;
        
        /* Multiple comparison types using PHI-derived value */
        if (cmp_var) {  /* Direct use in if condition */
            sum += i * 2;
        } else {
            sum += i;
        }
        
        /* Another comparison with explicit equality */
        if (cmp_var == 1) {
            sum += (i % 256);
        }
        
        /* Use in nested condition */
        if (mode == 1) {
            int tmp4 = cmp_var;
            while (tmp4) {  /* Use in loop condition */
                sum += 1;
                tmp4 = 0;  /* Break after one iteration */
            }
        }
    }
    
    return sum;
}

/* Function 2: Nested PHI patterns with varying profile */
__attribute__((noinline))
uint64_t nested_phi_patterns(int depth, int width) {
    uint64_t result = 0;
    int outer_phi, middle_phi, inner_phi;
    
    for (int d = 0; d < depth; d++) {
        /* Outer PHI pattern */
        if (d % 3 == 0) {
            outer_phi = 1;
        } else {
            outer_phi = 0;
        }
        
        int outer_val = outer_phi;
        int chain1 = outer_val;
        int chain2 = chain1;
        
        for (int w = 0; w < width; w++) {
            /* Middle PHI pattern */
            if (w < width / 2) {
                middle_phi = 1;
            } else {
                middle_phi = 0;
            }
            
            int middle_val = middle_phi;
            int chain3 = middle_val;
            int chain4 = chain3;
            
            /* Inner PHI pattern based on both outer and middle */
            if (chain2 && chain4) {
                inner_phi = 1;
            } else if (chain2 || chain4) {
                inner_phi = (d + w) % 2;
            } else {
                inner_phi = 0;
            }
            
            int inner_val = inner_phi;
            int final_cmp = inner_val;
            
            /* Conditional using PHI-derived value */
            if (final_cmp != 0) {
                result += d * w;
            } else {
                result += d + w;
            }
            
            /* Another comparison chain */
            int tmp = final_cmp;
            if (tmp == 1) {
                result += 1;
            }
        }
    }
    
    return result;
}

/* Function 3: Array processing with PHI-dependent loops */
__attribute__((noinline))
uint64_t array_process_with_phi(int* data, int size, int threshold) {
    uint64_t sum = 0;
    int should_process;
    
    for (int i = 0; i < size; i++) {
        /* PHI pattern based on array values */
        if (data[i] > threshold) {
            should_process = 1;
        } else {
            should_process = 0;
        }
        
        int process_flag = should_process;
        int flag_copy1 = process_flag;
        int flag_copy2 = flag_copy1;
        
        /* Loop with PHI-derived condition */
        int iterations = flag_copy2 ? 3 : 1;
        for (int j = 0; j < iterations; j++) {
            if (flag_copy2) {
                sum += data[i] * j;
            } else {
                sum += data[i];
            }
        }
        
        /* Nested condition with copy chain */
        int tmp = flag_copy2;
        if (tmp == 1) {
            sum += i;
        }
    }
    
    return sum;
}

/* Function 4: Complex control flow with multiple PHI merges */
__attribute__((noinline))
uint64_t complex_phi_merges(int mode, int limit) {
    uint64_t total = 0;
    int path_selector;
    
    for (int i = 0; i < limit; i++) {
        /* First level branch */
        if (i % 10 < 7) {
            path_selector = 1;
        } else {
            path_selector = 0;
        }
        
        int selector1 = path_selector;
        int selector2 = selector1;
        
        /* Second level based on first */
        int branch_val;
        if (selector2) {
            if (mode == 1) {
                branch_val = 1;
            } else {
                branch_val = (i % 2);
            }
        } else {
            branch_val = 0;
        }
        
        int val1 = branch_val;
        int val2 = val1;
        int val3 = val2 + 0;
        
        /* Multiple uses of PHI-derived value */
        if (val3) {
            total += i * i;
        }
        
        if (val3 == 1) {
            total += i;
        }
        
        /* Use in switch-like pattern */
        int switch_var = val3;
        switch (switch_var) {
            case 0:
                total += 1;
                break;
            case 1:
                total += 2;
                break;
            default:
                total += 3;
        }
    }
    
    return total;
}

/* Main execution with profile-varying modes */
int main(int argc, char** argv) {
    int mode = 1;  /* Default to hot mode */
    int warmup = 1;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
        if (argc > 2) {
            warmup = atoi(argv[2]);
        }
    }
    
    printf("Running mode %d with warmup %d\n", mode, warmup);
    
    /* Warm-up phase with different profile */
    if (warmup) {
        printf("Warm-up phase...\n");
        for (int w = 0; w < WARM_ITERATIONS / 1000; w++) {
            global_checksum += process_with_phi_chains(0, 100);
        }
    }
    
    uint64_t result = 0;
    
    switch (mode) {
        case 1:  /* Hot mode - heavily exercises PHI-to-conditional paths */
            printf("Hot mode execution...\n");
            result += process_with_phi_chains(1, HOT_ITERATIONS);
            result += nested_phi_patterns(1000, 100);
            
            /* Array processing with hot path */
            {
                int hot_data[1000];
                for (int i = 0; i < 1000; i++) {
                    hot_data[i] = i * 2;  /* Mostly > threshold */
                }
                result += array_process_with_phi(hot_data, 1000, 100);
            }
            
            result += complex_phi_merges(1, HOT_ITERATIONS / 100);
            break;
            
        case 2:  /* Mixed mode - balanced execution */
            printf("Mixed mode execution...\n");
            result += process_with_phi_chains(2, WARM_ITERATIONS);
            result += nested_phi_patterns(100, 50);
            
            {
                int mixed_data[500];
                for (int i = 0; i < 500; i++) {
                    mixed_data[i] = i;  /* Mixed vs threshold */
                }
                result += array_process_with_phi(mixed_data, 500, 250);
            }
            
            result += complex_phi_merges(2, WARM_ITERATIONS / 10);
            break;
            
        case 3:  /* Cold mode - rarely taken branches */
            printf("Cold mode execution...\n");
            result += process_with_phi_chains(3, COLD_ITERATIONS);
            result += nested_phi_patterns(10, 5);
            
            {
                int cold_data[100];
                for (int i = 0; i < 100; i++) {
                    cold_data[i] = i;  /* Mostly < threshold */
                }
                result += array_process_with_phi(cold_data, 100, 1000);
            }
            
            result += complex_phi_merges(3, COLD_ITERATIONS);
            break;
            
        default:
            printf("Unknown mode, using default\n");
            result += process_with_phi_chains(0, 1000);
    }
    
    global_checksum += result;
    
    /* Final computation to ensure no dead code elimination */
    uint64_t final = global_checksum;
    for (int i = 0; i < 100; i++) {
        final = (final * 1103515245 + 12345) & 0x7fffffff;
    }
    
    printf("Result checksum: %lu\n", (unsigned long)final);
    
    return (int)(final % 256);
}
