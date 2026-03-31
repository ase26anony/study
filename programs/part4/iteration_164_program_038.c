/* autofdo_phi_condition_test.c
 * 
 * This program generates specific control flow patterns to trigger
 * GCC's AutoFDO PHI-to-conditional analysis in auto-profile.cc
 * lines 1312-1333.
 *
 * Compilation for coverage:
 * 1. First compilation (with dummy profile):
 *    g++ -O2 -fauto-profile autofdo_phi_condition_test.c -o test_prog
 * 2. Run to generate profile data:
 *    ./test_prog 1 > /dev/null
 *    perf record -e cycles:u -b -o perf.data -- ./test_prog 1
 *    perf convert --to-ctf=ctf.data perf.data
 *    create_gcov --binary=./test_prog --profile=ctf.data --gcov=test_prog.gcov
 * 3. Recompile with actual profile:
 *    g++ -O2 -fauto-profile -Wauto-profile autofdo_phi_condition_test.c -o test_prog_opt
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HOT_ITERATIONS 1000000
#define WARM_ITERATIONS 10000
#define COLD_ITERATIONS 10

/* Function 1: Complex PHI pattern with SSA copy chains in hot loop */
unsigned long long process_hot_path(int mode, int iterations) {
    unsigned long long sum = 0;
    int i, j;
    
    /* Outer hot loop - will be heavily annotated */
    for (i = 0; i < iterations; i++) {
        int phi_val;
        int tmp1, tmp2, tmp3;
        
        /* Create branching that feeds into PHI node */
        if (i % 3 == 0) {
            /* Path A: sets value to 1 */
            phi_val = 1;
        } else if (i % 3 == 1) {
            /* Path B: sets value to 0 */
            phi_val = 0;
        } else {
            /* Path C: sets value based on mode */
            phi_val = (mode > 0) ? 1 : 0;
        }
        
        /* PHI node would be created here after SSA */
        int phi_result = phi_val;
        
        /* Create SSA copy chain to trigger while loop walking */
        tmp1 = phi_result;      /* First assignment copy */
        tmp2 = tmp1;            /* Second assignment copy */
        tmp3 = tmp2 + 0;        /* Arithmetic that preserves value */
        int cmp_var = tmp3;     /* Final copy before comparison */
        
        /* Conditional using PHI-derived value - triggers uncovered code */
        if (cmp_var) {          /* Direct use in if condition */
            /* Hot path - executed frequently */
            for (j = 0; j < 10; j++) {
                sum += i * j;
            }
        } else {
            /* Cold path - rarely executed */
            sum += i;
        }
        
        /* Another PHI pattern with explicit comparison */
        int another_phi;
        if (i % 100 == 0) {
            another_phi = 1;
        } else {
            another_phi = 0;
        }
        
        /* More SSA copies */
        int chain1 = another_phi;
        int chain2 = chain1;
        int chain3 = chain2;
        
        /* Explicit equality comparison */
        if (chain3 == 1) {      /* Explicit comparison to 1 */
            sum += i * 2;
        }
    }
    
    return sum;
}

/* Function 2: Nested conditionals with PHI patterns */
unsigned long long process_nested_phi(int depth, int width) {
    unsigned long long result = 0;
    int i, d;
    
    for (i = 0; i < width; i++) {
        int phi_base = (i % 2 == 0) ? 1 : 0;
        
        /* Create deep SSA copy chain */
        int level1 = phi_base;
        int level2 = level1;
        int level3 = level2;
        int final_val = level3;
        
        /* Use in loop condition */
        int counter = depth;
        while (final_val && counter > 0) {  /* PHI-derived in loop condition */
            result += i * counter;
            counter--;
            
            /* Nested conditional with its own PHI */
            int inner_phi;
            if (counter % 3 == 0) {
                inner_phi = 1;
            } else {
                inner_phi = 0;
            }
            
            int inner_copy = inner_phi;
            if (inner_copy != 0) {  /* Inequality comparison */
                result += 1;
            }
        }
    }
    
    return result;
}

/* Function 3: Array processing with data-dependent PHI */
unsigned long long process_array_with_phi(int *data, int size, int threshold) {
    unsigned long long sum = 0;
    int i;
    
    for (i = 0; i < size; i++) {
        /* PHI value depends on array data */
        int data_phi;
        if (data[i] > threshold) {
            data_phi = 1;
        } else {
            data_phi = 0;
        }
        
        /* Multi-step SSA propagation */
        int stage1 = data_phi;
        int stage2 = stage1;
        int stage3 = stage2 * 1;  /* Multiplication by 1 preserves value */
        int cond_var = stage3;
        
        /* Multiple uses of PHI-derived value */
        if (cond_var) {
            sum += data[i] * 2;
        } else if (cond_var == 0) {  /* Explicit zero comparison */
            sum += data[i];
        }
        
        /* Another PHI in the same basic block */
        int secondary_phi;
        if (i % 5 == 0) {
            secondary_phi = 1;
        } else {
            secondary_phi = 0;
        }
        
        int copy_a = secondary_phi;
        int copy_b = copy_a;
        if (copy_b == 1) {
            sum += i;
        }
    }
    
    return sum;
}

/* Function 4: Mixed hot/cold paths with function calls */
unsigned long long mixed_path_processing(int selector, int iterations) {
    unsigned long long total = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        /* Complex PHI with multiple predecessors */
        int complex_phi;
        
        if (selector == 1) {
            /* Hot path predecessor */
            complex_phi = 1;
        } else if (selector == 2) {
            /* Medium path predecessor */
            complex_phi = (i % 2 == 0) ? 1 : 0;
        } else {
            /* Cold path predecessor */
            complex_phi = 0;
        }
        
        /* Extended SSA copy chain */
        int v1 = complex_phi;
        int v2 = v1;
        int v3 = v2 + 0;
        int v4 = v3;
        int v5 = v4;
        
        /* Branch using the heavily copied PHI value */
        if (v5) {
            /* Call function from hot path */
            total += process_hot_path(1, 10);
        } else {
            /* Call function from cold path */
            total += process_nested_phi(3, 5);
        }
    }
    
    return total;
}

/* Function 5: Switch-based PHI pattern */
unsigned long long switch_phi_pattern(int mode, int count) {
    unsigned long long result = 0;
    int i;
    
    for (i = 0; i < count; i++) {
        int switch_phi;
        
        switch (mode) {
            case 1:  /* Hot case */
                switch_phi = 1;
                break;
            case 2:  /* Warm case */
                switch_phi = (i % 3 == 0) ? 1 : 0;
                break;
            default: /* Cold case */
                switch_phi = 0;
                break;
        }
        
        /* SSA copies through multiple variables */
        int a = switch_phi;
        int b = a;
        int c = b;
        int d = c;
        
        /* Multiple conditionals using the same PHI chain */
        if (d) {
            result += i * 100;
        }
        
        if (d == 1) {
            result += i * 50;
        }
        
        if (d != 0) {
            result += i * 25;
        }
    }
    
    return result;
}

int main(int argc, char *argv[]) {
    int mode = 1;  /* Default to hot mode */
    int iterations = HOT_ITERATIONS;
    unsigned long long final_result = 0;
    
    /* Parse command line for mode selection */
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    /* Seed for reproducible behavior */
    srand(42);
    
    printf("Starting AutoFDO PHI pattern test (mode=%d)\n", mode);
    
    /* Warm-up phase with moderate iterations */
    printf("Warm-up phase...\n");
    final_result += process_hot_path(mode, WARM_ITERATIONS);
    
    /* Main processing with mode-dependent behavior */
    printf("Main processing phase...\n");
    
    switch (mode) {
        case 1:  /* Hot mode - triggers heavily annotated paths */
            printf("Executing HOT path patterns...\n");
            final_result += process_hot_path(1, HOT_ITERATIONS);
            final_result += mixed_path_processing(1, HOT_ITERATIONS / 10);
            break;
            
        case 2:  /* Mixed mode - balanced hot/cold */
            printf("Executing MIXED path patterns...\n");
            final_result += process_hot_path(2, HOT_ITERATIONS / 2);
            final_result += mixed_path_processing(2, HOT_ITERATIONS / 20);
            break;
            
        case 3:  /* Cold mode - triggers rarely executed paths */
            printf("Executing COLD path patterns...\n");
            final_result += process_hot_path(0, COLD_ITERATIONS);
            final_result += mixed_path_processing(3, COLD_ITERATIONS);
            break;
            
        default:
            printf("Executing DEFAULT patterns...\n");
            final_result += process_hot_path(mode, HOT_ITERATIONS / 4);
            break;
    }
    
    /* Array processing with data-dependent PHI */
    printf("Array processing phase...\n");
    int array_size = 10000;
    int *data_array = malloc(array_size * sizeof(int));
    
    for (int i = 0; i < array_size; i++) {
        data_array[i] = rand() % 1000;
    }
    
    final_result += process_array_with_phi(data_array, array_size, 500);
    free(data_array);
    
    /* Switch-based pattern */
    printf("Switch pattern phase...\n");
    final_result += switch_phi_pattern(mode, iterations / 100);
    
    /* Nested PHI patterns */
    printf("Nested pattern phase...\n");
    final_result += process_nested_phi(5, 1000);
    
    /* Verification checksum */
    printf("Final result checksum: %llu\n", final_result);
    printf("Test completed successfully.\n");
    
    return 0;
}
