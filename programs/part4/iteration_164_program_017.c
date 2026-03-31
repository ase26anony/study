/* autofdo-phi-test.c
 * Test program to trigger GCC AutoFDO profile analysis for PHI-to-conditional patterns
 * Compile with: gcc -O2 -fauto-profile autofdo-phi-test.c -o autofdo-phi-test
 * Run with: ./autofdo-phi-test mode [iterations]
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define HOT_ITERATIONS 10000000
#define WARM_ITERATIONS 1000000
#define COLD_ITERATIONS 1000

/* Function 1: Simple PHI with direct conditional */
unsigned long long process_phi_direct(int mode, int iterations) {
    unsigned long long sum = 0;
    int i, j;
    
    for (i = 0; i < iterations; i++) {
        int phi_val;
        int tmp1, tmp2, cmp_var;
        
        /* Create branching that feeds into PHI */
        if (mode == 1) {
            phi_val = 1;  /* Hot path */
        } else if (mode == 2) {
            phi_val = 0;  /* Cold path */
        } else {
            phi_val = (i & 1);  /* Mixed path */
        }
        
        /* SSA copy chain to trigger while loop walking */
        tmp1 = phi_val;
        tmp2 = tmp1;
        cmp_var = tmp2;
        
        /* Additional copy operations */
        int tmp3 = cmp_var + 0;  /* Arithmetic that doesn't change value */
        int tmp4 = tmp3;
        int final_val = tmp4;
        
        /* Conditional using PHI-derived value - triggers uncovered code */
        if (final_val) {  /* Direct use in if condition */
            /* Hot computation path */
            for (j = 0; j < 100; j++) {
                sum += i * j;
            }
        } else {
            /* Cold computation path */
            sum += i;
        }
        
        /* Another conditional with explicit comparison */
        if (final_val == 1) {  /* Explicit equality check */
            sum += i * 2;
        }
    }
    
    return sum;
}

/* Function 2: Complex PHI with nested conditionals */
unsigned long long process_phi_complex(int mode, int iterations) {
    unsigned long long sum = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        int phi_val1, phi_val2;
        
        /* Two independent branches feeding into PHI */
        if (i % 3 == 0) {
            phi_val1 = 1;
        } else {
            phi_val1 = 0;
        }
        
        if (mode > 0) {
            phi_val2 = 1;
        } else {
            phi_val2 = 0;
        }
        
        /* PHI-like selection based on another condition */
        int selected_val;
        if (i % 5 == 0) {
            selected_val = phi_val1;
        } else {
            selected_val = phi_val2;
        }
        
        /* Multiple SSA copies */
        int chain1 = selected_val;
        int chain2 = chain1;
        int chain3 = chain2;
        int chain4 = chain3 + 0;  /* Preserve SSA chain */
        
        /* Nested conditionals with PHI-derived values */
        if (chain4) {
            /* Hot inner loop */
            int j;
            for (j = 0; j < 50; j++) {
                if (chain4 == 1) {  /* Explicit comparison */
                    sum += i + j;
                } else {
                    sum += i - j;
                }
            }
            
            /* Another conditional */
            if (chain4 != 0) {  /* Inequality check */
                sum += i * 3;
            }
        } else {
            /* Cold path with its own PHI */
            int cold_phi;
            if (mode == 2) {
                cold_phi = 1;
            } else {
                cold_phi = 0;
            }
            
            int cold_tmp = cold_phi;
            if (cold_tmp) {
                sum += i * 4;
            }
        }
    }
    
    return sum;
}

/* Function 3: PHI in loop condition */
unsigned long long process_phi_loop(int mode, int iterations) {
    unsigned long long sum = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        int continue_flag;
        
        /* Determine loop continuation flag */
        if (mode == 1) {
            continue_flag = (i < iterations / 2) ? 1 : 0;
        } else {
            continue_flag = (i < iterations / 10) ? 1 : 0;
        }
        
        /* SSA copy chain */
        int flag_copy1 = continue_flag;
        int flag_copy2 = flag_copy1;
        int loop_flag = flag_copy2;
        
        /* Loop controlled by PHI-derived value */
        int inner_count = 0;
        while (loop_flag && inner_count < 10) {  /* while(phi_derived) */
            sum += i * inner_count;
            inner_count++;
            
            /* Modify loop flag through another PHI */
            if (inner_count > 5) {
                loop_flag = 0;  /* Break condition */
            }
            /* SSA copy to maintain chain */
            int tmp_flag = loop_flag;
            loop_flag = tmp_flag;
        }
        
        /* Post-loop conditional */
        if (continue_flag == 1) {  /* Explicit equality */
            sum += i * 100;
        }
    }
    
    return sum;
}

/* Function 4: Multiple PHIs feeding into single conditional */
unsigned long long process_multi_phi(int mode, int iterations) {
    unsigned long long sum = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        int phi_a, phi_b, phi_c;
        
        /* Three independent PHI sources */
        if (i % 2 == 0) {
            phi_a = 1;
        } else {
            phi_a = 0;
        }
        
        if (mode % 2 == 0) {
            phi_b = 1;
        } else {
            phi_b = 0;
        }
        
        if (i % 3 == 0) {
            phi_c = 1;
        } else {
            phi_c = 0;
        }
        
        /* Combine PHIs */
        int combined;
        if (phi_a && phi_b) {
            combined = 1;
        } else if (phi_c) {
            combined = 1;
        } else {
            combined = 0;
        }
        
        /* Long SSA copy chain */
        int c1 = combined;
        int c2 = c1;
        int c3 = c2 + 0;
        int c4 = c3;
        int c5 = c4;
        int final_cond = c5;
        
        /* Conditional with multiple blocks */
        if (final_cond) {
            /* Very hot block */
            int j;
            for (j = 0; j < 20; j++) {
                sum += (i * j) / (j + 1);
            }
        } else {
            /* Cold block with nested condition */
            if (phi_a == 0 && phi_b == 0) {  /* Multiple comparisons */
                sum += i;
            }
        }
        
        /* Another use of the same PHI chain */
        if (final_cond == 1) {
            sum += i * 2;
        }
    }
    
    return sum;
}

/* Function 5: Cross-function PHI propagation */
static int global_mode = 0;

static int get_phi_value(int idx, int local_mode) {
    int val;
    
    /* Different paths based on global and local state */
    if (global_mode == 1) {
        val = (idx % 10 == 0) ? 1 : 0;
    } else if (local_mode == 1) {
        val = (idx % 5 == 0) ? 1 : 0;
    } else {
        val = (idx % 3 == 0) ? 1 : 0;
    }
    
    /* SSA copies before return */
    int tmp = val;
    return tmp;
}

unsigned long long process_cross_function(int mode, int iterations) {
    unsigned long long sum = 0;
    int i;
    
    global_mode = mode;
    
    for (i = 0; i < iterations; i++) {
        /* Get PHI value from function call */
        int phi_val = get_phi_value(i, mode);
        
        /* Copy chain */
        int local_copy1 = phi_val;
        int local_copy2 = local_copy1;
        int cond_var = local_copy2 + 0;
        
        /* Conditional using cross-function PHI */
        if (cond_var) {
            /* Hot: array processing */
            int arr[10];
            int j;
            for (j = 0; j < 10; j++) {
                arr[j] = i * j;
                sum += arr[j];
            }
            
            /* Nested conditional */
            if (cond_var == 1) {
                sum += arr[5];
            }
        } else {
            /* Cold: simple increment */
            sum += i;
        }
    }
    
    return sum;
}

/* Main function with different profile modes */
int main(int argc, char *argv[]) {
    int mode = 1;  /* Default to hot mode */
    int iterations = HOT_ITERATIONS;
    unsigned long long total_sum = 0;
    clock_t start, end;
    
    /* Parse command line */
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    printf("Running mode %d with %d iterations\n", mode, iterations);
    start = clock();
    
    /* Execute different patterns based on mode */
    switch (mode) {
        case 1:  /* Hot mode - all hot paths */
            printf("Mode 1: Hot path dominant\n");
            total_sum += process_phi_direct(1, iterations);
            total_sum += process_phi_complex(1, iterations / 10);
            total_sum += process_phi_loop(1, iterations / 5);
            total_sum += process_multi_phi(1, iterations / 2);
            total_sum += process_cross_function(1, iterations / 3);
            break;
            
        case 2:  /* Cold mode - mostly cold paths */
            printf("Mode 2: Cold path dominant\n");
            total_sum += process_phi_direct(2, iterations / 100);
            total_sum += process_phi_complex(0, iterations / 1000);
            total_sum += process_phi_loop(0, iterations / 500);
            total_sum += process_multi_phi(0, iterations / 200);
            total_sum += process_cross_function(0, iterations / 300);
            break;
            
        case 3:  /* Mixed mode - balanced */
            printf("Mode 3: Mixed hot/cold paths\n");
            total_sum += process_phi_direct(3, iterations);
            total_sum += process_phi_complex(3, iterations / 2);
            total_sum += process_phi_loop(3, iterations / 3);
            total_sum += process_multi_phi(3, iterations / 4);
            total_sum += process_cross_function(3, iterations / 5);
            break;
            
        case 4:  /* Warm-up then hot */
            printf("Mode 4: Warm-up phase\n");
            total_sum += process_phi_direct(2, WARM_ITERATIONS);
            total_sum += process_phi_complex(2, WARM_ITERATIONS / 10);
            printf("Mode 4: Hot phase\n");
            total_sum += process_phi_direct(1, iterations);
            total_sum += process_phi_complex(1, iterations / 2);
            total_sum += process_phi_loop(1, iterations / 3);
            break;
            
        default:  /* Variable mode based on input */
            printf("Mode %d: Variable behavior\n", mode);
            for (int phase = 0; phase < 3; phase++) {
                int phase_mode = (mode + phase) % 3;
                total_sum += process_phi_direct(phase_mode, iterations / 3);
                total_sum += process_phi_complex(phase_mode, iterations / 6);
            }
            break;
    }
    
    end = clock();
    double cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    
    printf("Total checksum: %llu\n", total_sum);
    printf("Time used: %.2f seconds\n", cpu_time_used);
    printf("Checksum per second: %.0f\n", total_sum / (cpu_time_used + 0.0001));
    
    return 0;
}
