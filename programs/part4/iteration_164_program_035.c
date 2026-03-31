/* autofdo_phi_test.c
 * Test program to trigger GCC AutoFDO PHI-to-conditional analysis
 * Compile: gcc -O2 -fauto-profile autofdo_phi_test.c -o autofdo_phi_test
 * Run: ./autofdo_phi_test <mode> <iterations>
 * Modes: 1=hot path dominant, 2=cold path dominant, 3=mixed
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
volatile int global_counter = 0;

/* Function 1: Complex PHI pattern with SSA copy chain */
unsigned long long process_data_mode1(int *data, int size, int threshold) {
    unsigned long long sum = 0;
    int i, j;
    
    /* Outer hot loop - will be heavily annotated */
    for (i = 0; i < HOT_LOOP_ITERATIONS; i++) {
        int phi_value;
        int tmp1, tmp2, tmp3;
        
        /* Create branching that feeds into PHI */
        if (i % 100 < 95) {  /* Hot path - 95% probability */
            phi_value = 1;
        } else {              /* Cold path - 5% probability */
            phi_value = 0;
        }
        
        /* PHI node would be created here after merging */
        int phi_result = phi_value;
        
        /* Create SSA copy chain to trigger while loop in uncovered code */
        tmp1 = phi_result;      /* First assignment */
        tmp2 = tmp1;            /* Second assignment */
        tmp3 = tmp2 + 0;        /* Third assignment (arithmetic that preserves value) */
        int cmp_var = tmp3;     /* Fourth assignment */
        
        /* Conditional using PHI-derived value - triggers uncovered analysis */
        if (cmp_var) {  /* Direct use in if condition */
            /* Hot path - process array */
            for (j = 0; j < size; j++) {
                data[j] = data[j] * 2 + 1;
                sum += data[j];
            }
        } else {
            /* Cold path - minimal processing */
            sum += i;
        }
        
        /* Another PHI pattern with explicit comparison */
        int phi_value2;
        if (i % 1000 < 999) {  /* Very hot path */
            phi_value2 = 1;
        } else {
            phi_value2 = 0;
        }
        
        int phi_result2 = phi_value2;
        int chain1 = phi_result2;
        int chain2 = chain1;
        
        /* Explicit equality comparison */
        if (chain2 == 1) {  /* Explicit comparison with 1 */
            sum += 1000;
        }
        
        /* While loop with PHI-derived condition */
        int loop_control = phi_result;
        int loop_counter = 0;
        while (loop_control && loop_counter < 10) {
            sum += loop_counter;
            loop_counter++;
            /* Break early sometimes */
            if (loop_counter > 5) {
                loop_control = 0;  /* Break condition */
            }
        }
    }
    
    return sum;
}

/* Function 2: Nested PHI patterns with function calls */
unsigned long long process_data_mode2(int *data, int size, int threshold) {
    unsigned long long sum = 0;
    int i;
    
    /* Different loop structure for varied profile */
    for (i = 0; i < HOT_LOOP_ITERATIONS / 10; i++) {
        int phi_value_a, phi_value_b;
        
        /* Complex branching for PHI creation */
        if (i % 3 == 0) {
            phi_value_a = 1;
            if (i % 6 == 0) {
                phi_value_b = 1;
            } else {
                phi_value_b = 0;
            }
        } else {
            phi_value_a = 0;
            phi_value_b = (i % 5 == 0) ? 1 : 0;
        }
        
        /* PHI nodes */
        int phi_a = phi_value_a;
        int phi_b = phi_value_b;
        
        /* Longer SSA copy chain */
        int t1 = phi_a;
        int t2 = t1;
        int t3 = t2;
        int t4 = t3 + 0;
        int t5 = t4;
        
        /* Multiple conditionals from same PHI source */
        if (t5) {
            /* Call function from hot path */
            sum += hot_path_function(data, size);
        }
        
        if (phi_b != 0) {  /* Not-equal-zero comparison */
            sum += cold_path_function(i);
        }
        
        /* Switch-like pattern using PHI */
        int selector = phi_a;
        int chain_s1 = selector;
        int chain_s2 = chain_s1;
        
        if (chain_s2 == 1) {
            sum += process_subarray(data, size / 2);
        }
    }
    
    return sum;
}

/* Function 3: Mixed hot/cold paths with varying PHI patterns */
unsigned long long process_data_mode3(int *data, int size, int threshold) {
    unsigned long long sum = 0;
    int i;
    
    for (i = 0; i < HOT_LOOP_ITERATIONS; i++) {
        /* Varying PHI patterns based on multiple conditions */
        int phi_value;
        
        if (i < threshold) {
            if (i % 100 < 80) {
                phi_value = 1;
            } else {
                phi_value = 0;
            }
        } else {
            if (i % 100 < 20) {
                phi_value = 1;
            } else {
                phi_value = 0;
            }
        }
        
        int phi_result = phi_value;
        
        /* Complex SSA chain across basic blocks */
        int intermediate;
        {
            int local1 = phi_result;
            intermediate = local1;
        }
        
        int final_var;
        {
            int local2 = intermediate;
            final_var = local2;
        }
        
        /* Conditional in loop */
        for (int j = 0; j < 5; j++) {
            int loop_phi;
            if (j % 2 == 0) {
                loop_phi = final_var;
            } else {
                loop_phi = !final_var;
            }
            
            int loop_chain = loop_phi;
            
            if (loop_chain) {
                sum += data[j % size] * 3;
            }
        }
    }
    
    return sum;
}

/* Helper functions called from different paths */
int hot_path_function(int *data, int size) {
    int result = 0;
    for (int i = 0; i < size; i += 4) {
        result += data[i];
    }
    return result;
}

int cold_path_function(int value) {
    /* Cold function - minimal computation */
    return value % 100;
}

int process_subarray(int *data, int size) {
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += data[i];
        /* Create internal branching */
        if (i % 3 == 0) {
            sum += 1;
        }
    }
    return sum;
}

/* Warm-up function to establish profile patterns */
void warmup_execution(int *data, int size) {
    unsigned long long warmup_sum = 0;
    
    for (int i = 0; i < WARMUP_ITERATIONS; i++) {
        int phi_val = (i % 10 < 8) ? 1 : 0;
        int chain1 = phi_val;
        int chain2 = chain1;
        
        if (chain2) {
            for (int j = 0; j < size; j++) {
                data[j] = (data[j] + 1) % 1000;
                warmup_sum += data[j];
            }
        } else {
            warmup_sum += i;
        }
    }
    
    global_counter += (int)(warmup_sum % 1000);
}

int main(int argc, char *argv[]) {
    int mode = 1;  /* Default to hot path dominant */
    int custom_iterations = HOT_LOOP_ITERATIONS;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        custom_iterations = atoi(argv[2]);
    }
    
    /* Initialize data array */
    int *data = (int *)malloc(ARRAY_SIZE * sizeof(int));
    if (!data) {
        fprintf(stderr, "Memory allocation failed\n");
        return 1;
    }
    
    /* Seed with pseudo-random but deterministic values */
    srand(42);
    for (int i = 0; i < ARRAY_SIZE; i++) {
        data[i] = rand() % 1000;
    }
    
    /* Warm-up phase to establish profile patterns */
    printf("Starting warm-up...\n");
    warmup_execution(data, ARRAY_SIZE);
    
    /* Main processing based on mode */
    printf("Running mode %d with ~%d iterations...\n", mode, custom_iterations);
    
    unsigned long long total_sum = 0;
    clock_t start_time = clock();
    
    switch (mode) {
        case 1:
            /* Hot path dominant - triggers heavy annotation of hot blocks */
            total_sum = process_data_mode1(data, ARRAY_SIZE, custom_iterations / 2);
            break;
            
        case 2:
            /* Cold path dominant - triggers annotation of cold blocks */
            total_sum = process_data_mode2(data, ARRAY_SIZE, custom_iterations / 10);
            break;
            
        case 3:
            /* Mixed paths - balanced execution */
            total_sum = process_data_mode3(data, ARRAY_SIZE, custom_iterations / 2);
            break;
            
        default:
            /* Run all modes sequentially for comprehensive profile */
            total_sum += process_data_mode1(data, ARRAY_SIZE, custom_iterations / 3);
            total_sum += process_data_mode2(data, ARRAY_SIZE, custom_iterations / 3);
            total_sum += process_data_mode3(data, ARRAY_SIZE, custom_iterations / 3);
            break;
    }
    
    clock_t end_time = clock();
    double elapsed = (double)(end_time - start_time) / CLOCKS_PER_SEC;
    
    /* Final computation to ensure all code paths contribute to output */
    unsigned long long checksum = total_sum;
    for (int i = 0; i < ARRAY_SIZE; i++) {
        checksum = (checksum * 31 + data[i]) % 1000000007;
    }
    
    printf("Result: checksum = %llu\n", checksum);
    printf("Execution time: %.2f seconds\n", elapsed);
    printf("Global counter: %d\n", global_counter);
    
    free(data);
    return 0;
}
