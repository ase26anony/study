/* test_autofdo_phi_cond.c
 * 
 * This program generates patterns to trigger GCC's AutoFDO PHI-to-conditional
 * analysis in auto-profile.cc lines 1312-1333.
 * 
 * Compilation and usage:
 * 1. First compilation: gcc -O2 -fauto-profile test_autofdo_phi_cond.c -o test_autofdo
 * 2. Run with dominant mode: ./test_autofdo 1 > /dev/null
 * 3. Collect profile (using perf or similar): perf record -e cycles:u -b ./test_autofdo 1
 * 4. Create AutoFDO profile: create_gcov --binary=./test_autofdo --profile=perf.data --gcov=test_autofdo.afdo
 * 5. Recompile with profile: gcc -O2 -fauto-profile=test_autofdo.afdo test_autofdo_phi_cond.c -o test_autofdo_opt
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <time.h>

#define HOT_ITERATIONS 1000000
#define WARM_ITERATIONS 10000
#define COLD_ITERATIONS 10

/* Function 1: Direct PHI-to-conditional with SSA copy chain */
unsigned long long process_array_hot(int *arr, int size, int threshold) {
    unsigned long long sum = 0;
    int use_fast_path = 0;  /* Will become PHI node */
    
    /* Create PHI node condition - different values in different predecessors */
    if (size > threshold) {
        use_fast_path = 1;  /* Hot path value */
    } else {
        use_fast_path = 0;  /* Cold path value */
    }
    
    /* SSA copy chain to trigger while loop walking */
    int tmp1 = use_fast_path;
    int tmp2 = tmp1;
    int tmp3 = tmp2 + 0;  /* Arithmetic that doesn't break single-assignment */
    int cmp_var = tmp3;
    
    /* Multiple comparison types using PHI-derived value */
    if (cmp_var) {  /* Direct use in if condition - line 1312-1333 logic */
        /* Hot path - heavily executed */
        for (int i = 0; i < size; i++) {
            sum += arr[i] * 2;
        }
    } else {
        /* Cold path - rarely executed */
        for (int i = 0; i < size; i++) {
            sum += arr[i];
        }
    }
    
    /* Another conditional with explicit comparison */
    if (cmp_var == 1) {  /* Explicit equality comparison */
        sum += 1000;
    }
    
    return sum;
}

/* Function 2: Nested loops with PHI in loop condition */
unsigned long long nested_phi_pattern(int mode, int iterations) {
    unsigned long long total = 0;
    int outer_control = 0;
    
    /* Different PHI values based on mode */
    if (mode == 1) {
        outer_control = 1;  /* Hot */
    } else {
        outer_control = 0;  /* Cold */
    }
    
    /* SSA copy chain */
    int chain1 = outer_control;
    int chain2 = chain1;
    int loop_cond = chain2;
    
    /* Use PHI-derived value in loop condition */
    while (loop_cond) {  /* While loop with PHI condition */
        for (int i = 0; i < iterations; i++) {
            /* Create inner PHI pattern */
            int inner_flag = 0;
            if (i % 3 == 0) {
                inner_flag = 1;
            } else {
                inner_flag = 0;
            }
            
            /* Another SSA chain */
            int inner_tmp = inner_flag;
            int inner_cmp = inner_tmp;
            
            if (inner_cmp != 0) {  /* Not-equal comparison */
                total += i * 3;
            } else {
                total += i;
            }
        }
        
        /* Break after one iteration for mode 1, continue for mode 2 */
        if (mode == 1) {
            loop_cond = 0;
        }
    }
    
    return total;
}

/* Function 3: Complex control flow with multiple PHI nodes */
unsigned long long complex_phi_network(int selector, int *data, int len) {
    unsigned long long result = 0;
    int phi_val1 = 0, phi_val2 = 0;
    
    /* Multiple predecessor blocks creating PHI nodes */
    if (selector > 100) {
        phi_val1 = 1;
        phi_val2 = 0;
    } else if (selector > 50) {
        phi_val1 = 0;
        phi_val2 = 1;
    } else {
        phi_val1 = 0;
        phi_val2 = 0;
    }
    
    /* Cross-copy between PHI results */
    int mix1 = phi_val1;
    int mix2 = phi_val2;
    int final_cond = mix1 | mix2;  /* Still SSA name */
    
    /* Chain of assignments */
    int a = final_cond;
    int b = a;
    int c = b + 0;
    
    /* Conditional using the complex PHI result */
    for (int i = 0; i < len; i++) {
        if (c) {  /* PHI-derived condition */
            result += data[i] * data[i];
        } else {
            result += data[i];
        }
        
        /* Nested condition with another PHI */
        int nested_flag = (i % 2 == 0) ? 1 : 0;
        int nested_copy = nested_flag;
        if (nested_copy == 1) {  /* Explicit comparison */
            result += 1;
        }
    }
    
    return result;
}

/* Function 4: Switch-like pattern implemented with PHI */
unsigned long long switch_phi_pattern(int case_id, int iterations) {
    unsigned long long acc = 0;
    int case_flag = 0;
    
    /* Different basic blocks set different values */
    switch (case_id) {
        case 1:  /* Hot case */
            case_flag = 1;
            break;
        case 2:  /* Warm case */
            case_flag = 0;
            break;
        case 3:  /* Cold case */
            case_flag = 1;
            break;
        default: /* Very cold */
            case_flag = 0;
            break;
    }
    
    /* Multiple SSA copies */
    int f1 = case_flag;
    int f2 = f1;
    int f3 = f2;
    
    /* Loop with PHI-derived condition */
    for (int i = 0; i < iterations; i++) {
        if (f3) {  /* Direct use */
            acc += i * i;
        }
        
        /* Create another PHI inside loop */
        int loop_phi = (i % 10 == 0) ? 1 : 0;
        int loop_copy = loop_phi;
        if (loop_copy) {
            acc += 100;
        }
    }
    
    return acc;
}

/* Helper to generate array data */
void fill_array(int *arr, int size, int seed) {
    srand(seed);
    for (int i = 0; i < size; i++) {
        arr[i] = rand() % 100;
    }
}

/* Main function with profile-generating runtime behavior */
int main(int argc, char **argv) {
    int mode = 1;  /* Default to hot mode */
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    unsigned long long total_result = 0;
    int array_size = 1000;
    int *data = malloc(array_size * sizeof(int));
    
    /* Warm-up phase */
    printf("Starting warm-up...\n");
    for (int warm = 0; warm < WARM_ITERATIONS; warm++) {
        fill_array(data, array_size, warm);
        total_result += process_array_hot(data, array_size, 500);
    }
    
    /* Main execution with mode-dependent behavior */
    if (mode == 1) {
        /* HOT DOMINANT PATH - executes millions of times */
        printf("Running hot mode...\n");
        for (int iter = 0; iter < HOT_ITERATIONS; iter++) {
            fill_array(data, array_size, iter);
            
            /* Mix of different PHI patterns */
            total_result += process_array_hot(data, array_size, 800);  /* Mostly takes hot path */
            total_result += nested_phi_pattern(1, 100);  /* Hot nested pattern */
            total_result += complex_phi_network(iter % 200, data, 100);
            
            if (iter % 100 == 0) {
                total_result += switch_phi_pattern(1, 50);  /* Hot case */
            }
        }
    } else if (mode == 2) {
        /* WARM PATH - mixed behavior */
        printf("Running warm mode...\n");
        for (int iter = 0; iter < WARM_ITERATIONS; iter++) {
            fill_array(data, array_size, iter + 1000);
            total_result += process_array_hot(data, array_size, 200);  /* Mixed paths */
            total_result += nested_phi_pattern(2, 50);
            total_result += complex_phi_network(iter % 100, data, 200);
            total_result += switch_phi_pattern(2, 30);
        }
    } else {
        /* COLD PATH - rarely executed */
        printf("Running cold mode...\n");
        for (int iter = 0; iter < COLD_ITERATIONS; iter++) {
            fill_array(data, array_size, iter + 2000);
            total_result += process_array_hot(data, array_size, 1000);  /* Takes cold path */
            total_result += nested_phi_pattern(3, 10);
            total_result += complex_phi_network(iter % 20, data, 50);
            total_result += switch_phi_pattern(3, 5);
        }
    }
    
    /* Verification output */
    printf("Result checksum: %llu\n", total_result);
    
    free(data);
    return 0;
}
