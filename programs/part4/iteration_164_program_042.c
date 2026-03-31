/* test_autofdo_phi_cond.c
 * 
 * This program generates patterns to trigger AutoFDO's PHI-to-conditional
 * analysis in auto-profile.cc lines 1312-1333.
 * 
 * Compilation and usage:
 * 1. First compilation (with empty or existing profile):
 *    gcc -O2 -fauto-profile -o test_autofdo test_autofdo_phi_cond.c
 * 
 * 2. Run with dominant hot path:
 *    ./test_autofdo mode=1 > /dev/null
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

/* Function 1: Complex PHI pattern with SSA copy chains */
unsigned long long process_data_mode1(int *data, int size, int threshold) {
    unsigned long long sum = 0;
    int i, j;
    
    /* Outer hot loop - will be heavily annotated */
    for (i = 0; i < HOT_ITERATIONS; i++) {
        int use_fast_path = 0;
        int tmp1, tmp2, tmp3;
        
        /* Create branching that sets boolean in different predecessors */
        if (i % 100 < 95) {  /* Hot path - 95% probability */
            use_fast_path = 1;  /* Set to 1 in hot predecessor */
        } else {  /* Cold path - 5% probability */
            use_fast_path = 0;  /* Set to 0 in cold predecessor */
        }
        
        /* PHI node is implicitly created here - use_fast_path gets value
         * from different incoming edges */
        
        /* Create SSA copy chain to trigger the while loop walking back */
        tmp1 = use_fast_path;      /* First copy */
        tmp2 = tmp1;               /* Second copy */
        tmp3 = tmp2 + 0;           /* Arithmetic that preserves value */
        int cmp_var = tmp3;        /* Final copy before comparison */
        
        /* Conditional using PHI-derived value - triggers uncovered code */
        if (cmp_var) {  /* Direct use of boolean from PHI */
            /* Hot path - simple addition */
            sum += i * 2;
        } else {
            /* Cold path - more complex computation */
            sum += i / 2;
            for (j = 0; j < 10; j++) {
                sum += j;
            }
        }
        
        /* Another PHI pattern with explicit comparison to 1 */
        int another_flag;
        if (i % 1000 == 0) {
            another_flag = 1;
        } else {
            another_flag = 0;
        }
        
        /* Longer SSA copy chain */
        int chain1 = another_flag;
        int chain2 = chain1;
        int chain3 = chain2;
        int chain4 = chain3;
        
        if (chain4 == 1) {  /* Explicit comparison to 1 */
            sum += 1000;
        }
    }
    
    return sum;
}

/* Function 2: Nested loops with varying PHI patterns */
unsigned long long process_data_mode2(int *data, int size) {
    unsigned long long sum = 0;
    int i, j;
    
    for (i = 0; i < WARM_ITERATIONS; i++) {
        int flag_from_phi = 0;
        
        /* Complex branching creating multiple predecessor blocks */
        if (data[i % size] > 100) {
            if (i % 3 == 0) {
                flag_from_phi = 1;
            } else {
                flag_from_phi = 0;
            }
        } else {
            if (i % 7 == 0) {
                flag_from_phi = 1;
            } else {
                flag_from_phi = 0;
            }
        }
        
        /* Multi-step SSA propagation */
        int intermediate1 = flag_from_phi;
        int intermediate2 = intermediate1;
        /* Insert dummy arithmetic that doesn't change value */
        intermediate2 = intermediate2 + (i & 0);  
        int final_flag = intermediate2;
        
        /* Loop with PHI-derived condition */
        j = 0;
        while (final_flag && j < 5) {  /* Use in loop condition */
            sum += data[(i + j) % size];
            j++;
        }
        
        /* Another comparison pattern */
        if (final_flag != 0) {  /* Comparison to 0 */
            sum += i * 3;
        }
    }
    
    return sum;
}

/* Function 3: Cold path with rare PHI patterns */
unsigned long long process_data_mode3(int *data, int size) {
    unsigned long long sum = 0;
    int i;
    
    for (i = 0; i < COLD_ITERATIONS; i++) {
        int rare_flag;
        
        /* This path is rarely taken */
        if (i == 0) {
            rare_flag = 1;
        } else {
            rare_flag = 0;
        }
        
        /* Chain of assignments */
        int a = rare_flag;
        int b = a;
        int c = b;
        int d = c;
        int e = d;
        
        /* Multiple conditionals using the PHI-derived value */
        if (e) {
            sum += 99999;
        }
        
        if (e == 1) {
            sum += 88888;
        }
        
        if (e != 0) {
            sum += 77777;
        }
    }
    
    return sum;
}

/* Function 4: Mixed hot/cold PHI patterns with function calls */
static int helper_transform(int x, int flag) {
    /* This function creates additional control flow */
    int result;
    
    /* PHI pattern inside helper */
    int internal_flag;
    if (flag) {
        internal_flag = 1;
    } else {
        internal_flag = 0;
    }
    
    /* SSA copies */
    int t1 = internal_flag;
    int t2 = t1;
    
    if (t2) {
        result = x * 2;
    } else {
        result = x / 2;
    }
    
    return result;
}

unsigned long long process_data_mode4(int *data, int size) {
    unsigned long long sum = 0;
    int i;
    
    for (i = 0; i < HOT_ITERATIONS / 10; i++) {
        int call_flag = 0;
        
        /* Determine which helper path to use */
        if (i % 100 < 80) {  /* 80% hot path */
            call_flag = 1;
        } else {  /* 20% warm path */
            call_flag = 0;
        }
        
        /* Propagate through SSA */
        int f1 = call_flag;
        int f2 = f1;
        int f3 = f2;
        
        /* Call helper from both hot and cold paths */
        sum += helper_transform(data[i % size], f3);
        
        /* Additional conditional */
        if (f3 == 1) {
            sum += helper_transform(i, 1);
        }
    }
    
    return sum;
}

/* Main function with input-dependent branching */
int main(int argc, char *argv[]) {
    int mode = 1;  /* Default to hot mode */
    int data_size = 1000;
    int *data = malloc(data_size * sizeof(int));
    unsigned long long result = 0;
    
    /* Initialize data */
    srand(time(NULL));
    for (int i = 0; i < data_size; i++) {
        data[i] = rand() % 1000;
    }
    
    /* Parse command line for mode */
    for (int i = 1; i < argc; i++) {
        if (strncmp(argv[i], "mode=", 5) == 0) {
            mode = atoi(argv[i] + 5);
        }
    }
    
    /* Warm-up phase - creates initial profile annotations */
    for (int warm = 0; warm < 100; warm++) {
        result += process_data_mode2(data, data_size);
    }
    
    /* Main execution based on mode */
    switch (mode) {
        case 1:  /* Hot dominant path */
            printf("Running mode 1 (hot path dominant)\n");
            result += process_data_mode1(data, data_size, 500);
            result += process_data_mode4(data, data_size);
            break;
            
        case 2:  /* Balanced warm path */
            printf("Running mode 2 (balanced paths)\n");
            result += process_data_mode2(data, data_size);
            result += process_data_mode4(data, data_size);
            break;
            
        case 3:  /* Cold path emphasis */
            printf("Running mode 3 (cold paths)\n");
            result += process_data_mode3(data, data_size);
            for (int i = 0; i < 10; i++) {
                result += process_data_mode2(data, data_size);
            }
            break;
            
        default:
            printf("Running all modes\n");
            result += process_data_mode1(data, data_size, 500);
            result += process_data_mode2(data, data_size);
            result += process_data_mode3(data, data_size);
            result += process_data_mode4(data, data_size);
            break;
    }
    
    /* Output checksum to prevent optimization */
    printf("Result checksum: %llu\n", result);
    
    free(data);
    return 0;
}
