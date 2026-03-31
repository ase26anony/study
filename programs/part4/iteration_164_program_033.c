/* test_autofdo_phi_cond.c
 * 
 * This program generates patterns that trigger AutoFDO's PHI-to-conditional
 * analysis in auto-profile.cc lines 1312-1333.
 * 
 * Compilation and usage:
 * 1. First compilation (with empty or existing profile):
 *    gcc -O2 -fauto-profile -o test_autofdo test_autofdo_phi_cond.c
 * 
 * 2. Run with dominant mode to generate profile data:
 *    ./test_autofdo 1 > /dev/null
 * 
 * 3. Recompile with collected profile:
 *    gcc -O2 -fauto-profile -Wauto-profile -o test_autofdo_opt test_autofdo_phi_cond.c
 * 
 * 4. For debugging dumps:
 *    gcc -O2 -fauto-profile -fdump-tree-afdo -fdump-tree-afdo-details -o test_autofdo_debug test_autofdo_phi_cond.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HOT_ITERATIONS 1000000
#define WARM_ITERATIONS 10000
#define COLD_ITERATIONS 10

/* Function 1: Complex PHI pattern with SSA copy chains in hot loop */
unsigned long long process_hot_data(int mode, int size) {
    unsigned long long checksum = 0;
    int* data = (int*)malloc(size * sizeof(int));
    
    if (!data) return 0;
    
    // Initialize with pattern
    for (int i = 0; i < size; i++) {
        data[i] = (i * 17) % 101;
    }
    
    /* HOT LOOP with PHI-to-conditional pattern */
    for (int iter = 0; iter < HOT_ITERATIONS; iter++) {
        int phi_val = 0;  // Will flow through PHI nodes
        
        /* Pattern 1: PHI with constant 0/1 from different predecessors */
        for (int i = 0; i < size; i++) {
            int pred_condition = (data[i] % 3 == 0);
            int temp1, temp2, temp3;
            
            /* Different basic blocks setting phi_val to 0 or 1 */
            if (pred_condition) {
                // Basic block A: sets to 1
                phi_val = 1;
                temp1 = phi_val;  // SSA copy chain start
            } else {
                // Basic block B: sets to 0  
                phi_val = 0;
                temp1 = phi_val;  // SSA copy chain start
            }
            
            /* Merge point with implicit PHI node for phi_val */
            /* Create SSA copy chain to trigger while loop in uncovered code */
            temp2 = temp1;      // First copy
            int temp3 = temp2;  // Second copy
            int cmp_var = temp3 + 0;  // Third copy with arithmetic that doesn't change value
            
            /* Conditional using PHI-derived value - triggers uncovered logic */
            if (cmp_var) {  // Direct use: if (phi_derived)
                data[i] += 7;
                checksum += data[i] * 3;
            } else {
                data[i] -= 3;
                checksum += data[i];
            }
            
            /* Another pattern with explicit comparison */
            int tmp_copy = cmp_var;
            if (tmp_copy == 1) {  // Explicit equality: if (phi_derived == 1)
                checksum ^= data[i];
            }
        }
        
        /* Nested loop with different PHI pattern */
        for (int j = 0; j < size / 2; j++) {
            int selector = (j % 5 == 0) ? 1 : 0;
            int phi_select;
            
            if (selector) {
                phi_select = 1;
            } else {
                phi_select = 0;
            }
            
            /* Longer SSA copy chain */
            int chain1 = phi_select;
            int chain2 = chain1;
            int chain3 = chain2;
            int chain4 = chain3;
            
            /* Multiple conditionals with same PHI source */
            if (chain4 != 0) {  // if (phi_derived != 0)
                data[j] *= 2;
                checksum += data[j];
            }
            
            if (chain2 == 1) {  // Another comparison
                checksum ^= (j * 13);
            }
        }
    }
    
    free(data);
    return checksum;
}

/* Function 2: Cold path with similar patterns but rarely executed */
unsigned long long process_cold_data(int size) {
    unsigned long long checksum = 0;
    int* data = (int*)malloc(size * sizeof(int));
    
    if (!data) return 0;
    
    for (int i = 0; i < size; i++) {
        data[i] = i % 17;
    }
    
    /* COLD LOOP - rarely executed */
    for (int iter = 0; iter < COLD_ITERATIONS; iter++) {
        int flag = 0;
        
        for (int i = 0; i < size; i++) {
            /* PHI pattern in cold path */
            int cold_phi;
            if (data[i] > 10) {
                cold_phi = 1;
            } else {
                cold_phi = 0;
            }
            
            /* SSA copies */
            int c1 = cold_phi;
            int c2 = c1;
            
            /* Conditional - rarely taken */
            if (c2) {
                checksum += data[i] * 5;
            }
        }
    }
    
    free(data);
    return checksum;
}

/* Function 3: Mixed hot/cold paths with function calls */
unsigned long long mixed_processing(int mode, int size) {
    unsigned long long total = 0;
    
    /* Outer loop creates varying profile */
    for (int outer = 0; outer < WARM_ITERATIONS; outer++) {
        int path_selector = (outer % 100 == 0) ? 1 : 0;  // 1% cold path
        int phi_path;
        
        if (path_selector) {
            phi_path = 1;  // Cold
        } else {
            phi_path = 0;  // Hot
        }
        
        /* SSA copy chain */
        int p1 = phi_path;
        int p2 = p1;
        int p3 = p2 + 0;  // Arithmetic that preserves value
        
        /* Branch based on PHI-derived value */
        if (p3 == 0) {  // Hot path
            total += process_hot_data(mode, size / 10);
        } else {        // Cold path
            total += process_cold_data(size / 100);
        }
    }
    
    return total;
}

/* Function 4: Loop with PHI condition in loop control */
unsigned long long phi_in_loop_control(int iterations) {
    unsigned long long sum = 0;
    int continue_flag;
    
    /* Loop where continuation depends on PHI value */
    int i = 0;
    while (i < iterations) {
        /* Set continue_flag through PHI in different basic blocks */
        if (i % 3 == 0) {
            continue_flag = 1;
        } else if (i % 3 == 1) {
            continue_flag = 0;
        } else {
            continue_flag = 1;
        }
        
        /* SSA copies */
        int cf1 = continue_flag;
        int cf2 = cf1;
        
        /* Use in loop condition - while (phi_derived) pattern */
        if (cf2) {
            sum += i * i;
            i++;
        } else {
            sum -= i;
            i += 2;
        }
        
        /* Another conditional inside loop */
        int tmp = cf1;
        if (tmp == 1) {
            sum ^= i;
        }
    }
    
    return sum;
}

/* Function 5: Complex nested conditionals with PHI chains */
unsigned long long complex_phi_network(int size) {
    unsigned long long checksum = 0;
    int* counters = (int*)calloc(size, sizeof(int));
    
    for (int i = 0; i < size; i++) {
        /* Multiple PHI sources */
        int phi_a, phi_b, phi_final;
        
        /* First level PHI */
        if (i % 2 == 0) {
            phi_a = 1;
        } else {
            phi_a = 0;
        }
        
        /* Second level PHI */
        if (i % 3 == 0) {
            phi_b = 1;
        } else {
            phi_b = 0;
        }
        
        /* PHI that selects between other PHIs */
        if (phi_a) {
            phi_final = phi_b;
        } else {
            phi_final = phi_a;
        }
        
        /* Long SSA copy chain */
        int f1 = phi_final;
        int f2 = f1;
        int f3 = f2;
        int f4 = f3;
        int f5 = f4 + 0;
        
        /* Multiple uses of same PHI-derived value */
        if (f5) {
            counters[i]++;
        }
        
        if (f3 == 1) {
            checksum += counters[i] * 7;
        }
        
        /* Use in array indexing */
        checksum += i * (f2 ? 3 : 1);
    }
    
    free(counters);
    return checksum;
}

int main(int argc, char** argv) {
    int mode = 1;  // Default to hot mode
    int size = 1000;
    
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    if (argc > 2) {
        size = atoi(argv[2]);
        if (size < 100) size = 100;
        if (size > 10000) size = 10000;
    }
    
    srand(time(NULL));
    unsigned long long total_checksum = 0;
    
    /* Warm-up phase */
    printf("Starting AutoFDO test (mode=%d, size=%d)\n", mode, size);
    
    /* Execute based on mode to create distinct profiles */
    switch (mode) {
        case 1:  /* Dominant hot path */
            printf("Executing hot path...\n");
            total_checksum += process_hot_data(mode, size);
            total_checksum += mixed_processing(mode, size);
            total_checksum += phi_in_loop_control(HOT_ITERATIONS / 100);
            break;
            
        case 2:  /* Mixed hot/cold */
            printf("Executing mixed paths...\n");
            total_checksum += process_hot_data(mode, size / 2);
            total_checksum += process_cold_data(size);
            total_checksum += mixed_processing(mode, size);
            break;
            
        case 3:  /* Cold path dominant */
            printf("Executing cold path...\n");
            total_checksum += process_cold_data(size);
            total_checksum += phi_in_loop_control(COLD_ITERATIONS * 10);
            break;
            
        case 4:  /* Complex patterns */
            printf("Executing complex patterns...\n");
            total_checksum += complex_phi_network(size);
            total_checksum += mixed_processing(mode, size * 2);
            break;
            
        default:
            printf("Executing all patterns...\n");
            total_checksum += process_hot_data(1, size);
            total_checksum += process_cold_data(size);
            total_checksum += mixed_processing(1, size);
            total_checksum += phi_in_loop_control(WARM_ITERATIONS);
            total_checksum += complex_phi_network(size);
            break;
    }
    
    printf("Checksum: %llu\n", total_checksum);
    
    /* Additional computation to prevent optimization */
    volatile unsigned long long sink = total_checksum;
    (void)sink;
    
    return 0;
}
