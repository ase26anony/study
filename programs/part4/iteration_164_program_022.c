/* autofdo_phi_conditional.c
 * Test program for GCC AutoFDO profile analysis of PHI-to-conditional patterns
 * Compile with: gcc -O3 -fauto-profile -funroll-loops -finline-functions autofdo_phi_conditional.c -o autofdo_test
 * Run with: ./autofdo_test <mode> <iterations>
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define HOT_LOOP_ITERATIONS 1000000
#define WARMUP_ITERATIONS 10000
#define ARRAY_SIZE 1000

/* Global to prevent optimization */
volatile int global_sink;

/* Function 1: Simple PHI-to-conditional with direct copy chain */
int process_phi_direct(int mode, int iterations) {
    int result = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        int phi_val;
        int tmp1, tmp2, tmp3;
        
        /* Create PHI node with different values from different predecessors */
        if (mode == 1) {
            /* Hot path - executed most frequently */
            phi_val = 1;  /* Basic block A */
        } else {
            /* Cold path - rarely executed */
            phi_val = 0;  /* Basic block B */
        }
        
        /* Merge point - PHI node implicitly created here */
        
        /* Chain of single SSA assignments to trigger while loop walkback */
        tmp1 = phi_val;      /* First assignment */
        tmp2 = tmp1;         /* Second assignment */
        tmp3 = tmp2 + 0;     /* Third assignment (arithmetic that doesn't change value) */
        
        /* Conditional using the PHI-derived value - triggers uncovered code */
        if (tmp3) {          /* Should be if (phi_val) after optimization */
            /* Hot path when mode == 1 */
            result += i * 2;
        } else {
            /* Cold path when mode != 1 */
            result += i / 2;
        }
        
        /* Another conditional with explicit comparison */
        int cmp_var = tmp3;
        if (cmp_var == 1) {  /* Explicit equality check */
            result += 3;
        }
    }
    
    return result;
}

/* Function 2: Nested PHI patterns with multiple merge points */
int process_nested_phi(int threshold, int size) {
    int sum = 0;
    int data[ARRAY_SIZE];
    
    /* Initialize array */
    for (int i = 0; i < size; i++) {
        data[i] = i % 100;
    }
    
    for (int i = 0; i < size; i++) {
        int flag1, flag2;
        int tmp_a, tmp_b, tmp_c;
        
        /* First branching point */
        if (data[i] > threshold) {
            flag1 = 1;
        } else {
            flag1 = 0;
        }
        
        /* First merge point with PHI for flag1 */
        
        /* Chain of assignments for flag1 */
        tmp_a = flag1;
        tmp_b = tmp_a;
        
        /* Second branching point based on flag1 */
        if (tmp_b) {
            flag2 = 1;
        } else {
            flag2 = 0;
        }
        
        /* Second merge point with PHI for flag2 */
        
        /* More assignment chains */
        tmp_c = flag2;
        int final_flag = tmp_c;
        
        /* Conditional using doubly-nested PHI result */
        if (final_flag) {
            /* Hot path for high values */
            sum += data[i] * 3;
        } else {
            /* Cold path for low values */
            sum += data[i];
        }
        
        /* Another comparison type */
        if (final_flag != 0) {
            sum += 5;
        }
    }
    
    return sum;
}

/* Function 3: Complex PHI network with loop-carried dependencies */
int process_loop_carried_phi(int mode, int limit) {
    int state = 0;
    int prev_state = 0;
    int result = 0;
    
    for (int i = 0; i < limit; i++) {
        int next_state;
        int tmp1, tmp2, tmp3;
        
        /* Loop-carried PHI: state depends on previous iteration */
        if (i == 0) {
            next_state = mode;
        } else {
            /* PHI node here: next_state = (prev_state > 50) ? 1 : 0 */
            if (prev_state > 50) {
                next_state = 1;
            } else {
                next_state = 0;
            }
        }
        
        /* Assignment chain */
        tmp1 = next_state;
        tmp2 = tmp1;
        tmp3 = tmp2;
        
        /* Conditional using loop-carried PHI value */
        while (tmp3) {  /* Loop condition from PHI */
            result += i;
            tmp3 = 0;  /* Break after one iteration */
        }
        
        /* Update for next iteration */
        prev_state = state;
        state = next_state + i % 10;
    }
    
    return result;
}

/* Function 4: Multiple PHI sources with varying profile counts */
int process_multi_source_phi(int* data, int size, int hot_cutoff) {
    int count = 0;
    int tmp_chain[5];
    
    for (int i = 0; i < size; i++) {
        int selector;
        int copy1, copy2, final;
        
        /* Multiple predecessor blocks with different frequencies */
        if (i < hot_cutoff) {
            /* Hot block - executed frequently */
            selector = 1;
        } else if (i < hot_cutoff + 100) {
            /* Warm block - executed moderately */
            selector = 0;
        } else {
            /* Cold block - rarely executed */
            selector = (data[i] % 2);
        }
        
        /* PHI node merges all three paths */
        
        /* Extended assignment chain */
        copy1 = selector;
        tmp_chain[0] = copy1;
        for (int j = 1; j < 5; j++) {
            tmp_chain[j] = tmp_chain[j-1];
        }
        copy2 = tmp_chain[4];
        final = copy2;
        
        /* Multiple conditionals using the same PHI-derived value */
        if (final) {
            count += data[i];
        }
        
        if (final == 1) {
            count += 2;
        }
        
        if (final != 0) {
            count += 3;
        }
    }
    
    return count;
}

/* Function 5: Recursive PHI pattern with function calls */
int recursive_phi_helper(int depth, int max_depth, int* call_count) {
    (*call_count)++;
    
    if (depth >= max_depth) {
        return 1;
    }
    
    int left_val, right_val;
    int tmp1, tmp2;
    
    /* Different values from recursive calls */
    left_val = recursive_phi_helper(depth + 1, max_depth, call_count);
    right_val = recursive_phi_helper(depth + 1, max_depth, call_count);
    
    /* PHI-like selection based on comparison */
    int selected = (left_val > right_val) ? 1 : 0;
    
    /* Assignment chain */
    tmp1 = selected;
    tmp2 = tmp1;
    
    /* Conditional using recursive PHI result */
    if (tmp2) {
        return left_val + depth;
    } else {
        return right_val + depth;
    }
}

/* Main driver with profile-generating behavior */
int main(int argc, char** argv) {
    int mode = 1;  /* Default to hot mode */
    int iterations = HOT_LOOP_ITERATIONS;
    clock_t start, end;
    double cpu_time_used;
    uint64_t total_result = 0;
    
    /* Parse command line */
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        iterations = atoi(argv[2]);
    }
    
    printf("Running AutoFDO test with mode=%d, iterations=%d\n", mode, iterations);
    
    /* Warm-up phase - establish baseline profile */
    start = clock();
    for (int warmup = 0; warmup < WARMUP_ITERATIONS; warmup++) {
        total_result += process_phi_direct(warmup % 3, 100);
    }
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    printf("Warm-up completed in %.4f seconds\n", cpu_time_used);
    
    /* Main execution with mode-dependent behavior */
    start = clock();
    
    switch (mode) {
        case 1:  /* Hot mode - dominantly executes hot paths */
            printf("Mode 1: Dominant hot path execution\n");
            for (int phase = 0; phase < 10; phase++) {
                total_result += process_phi_direct(1, iterations / 10);
                total_result += process_nested_phi(70, ARRAY_SIZE);
                
                int data[ARRAY_SIZE];
                for (int i = 0; i < ARRAY_SIZE; i++) {
                    data[i] = rand() % 100;
                }
                total_result += process_multi_source_phi(data, ARRAY_SIZE, 800);
            }
            break;
            
        case 2:  /* Balanced mode - mixed hot/cold paths */
            printf("Mode 2: Balanced hot/cold execution\n");
            for (int phase = 0; phase < 5; phase++) {
                total_result += process_phi_direct(phase % 2, iterations / 5);
                total_result += process_nested_phi(50, ARRAY_SIZE);
                total_result += process_loop_carried_phi(phase % 2, 5000);
            }
            break;
            
        case 3:  /* Cold mode - mostly cold paths */
            printf("Mode 3: Cold path execution\n");
            total_result += process_phi_direct(0, iterations);
            total_result += process_nested_phi(30, ARRAY_SIZE);
            
            int call_count = 0;
            total_result += recursive_phi_helper(0, 5, &call_count);
            printf("Recursive calls: %d\n", call_count);
            break;
            
        default:
            printf("Unknown mode, using mixed execution\n");
            for (int phase = 0; phase < 3; phase++) {
                total_result += process_phi_direct(phase, iterations / 3);
            }
    }
    
    end = clock();
    cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    /* Final computation to ensure all code is used */
    int final_data[100];
    for (int i = 0; i < 100; i++) {
        final_data[i] = i;
    }
    total_result += process_multi_source_phi(final_data, 100, 50);
    
    printf("Total result: %lu\n", total_result);
    printf("Execution time: %.4f seconds\n", cpu_time_used);
    printf("Checksum: %08lx\n", total_result & 0xFFFFFFFF);
    
    return 0;
}
