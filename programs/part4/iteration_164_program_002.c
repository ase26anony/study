/* autofdo_phi_test.c - Test program for AutoFDO PHI-to-conditional analysis */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HOT_ITERATIONS 1000000
#define WARM_ITERATIONS 10000
#define COLD_ITERATIONS 10

/* Function 1: Complex PHI pattern with SSA copy chains */
int process_with_phi_chain(int mode, int limit) {
    int result = 0;
    
    for (int i = 0; i < limit; i++) {
        int phi_val;
        int tmp1, tmp2, tmp3;
        
        /* Different predecessors set different constant values */
        if (mode == 1) {
            phi_val = 1;  /* Hot path */
        } else if (mode == 2) {
            phi_val = 0;  /* Cold path */
        } else {
            /* Complex decision with runtime dependency */
            phi_val = (i % 100) == 0 ? 1 : 0;
        }
        
        /* Create SSA copy chain to trigger the while loop */
        tmp1 = phi_val;      /* First assignment */
        tmp2 = tmp1;         /* Second assignment */
        tmp3 = tmp2 + 0;     /* Third assignment (arithmetic that preserves value) */
        
        /* Conditional using PHI-derived value - triggers uncovered code */
        if (tmp3) {  /* Direct use of PHI-derived value */
            result += i * 2;  /* Hot computation */
        } else {
            result += i / 2;  /* Cold computation */
        }
        
        /* Another conditional with explicit comparison */
        int cmp_var = tmp2;
        if (cmp_var == 1) {  /* Explicit equality check */
            result ^= i;
        }
    }
    
    return result;
}

/* Function 2: Nested loops with PHI in loop condition */
int nested_phi_pattern(int base, int iterations) {
    int total = 0;
    int outer_flag;
    
    for (int j = 0; j < iterations; j++) {
        /* PHI value determined by complex condition */
        if (j < iterations / 2) {
            outer_flag = 1;
        } else {
            outer_flag = (base + j) % 3 == 0 ? 1 : 0;
        }
        
        /* Chain of assignments */
        int chain1 = outer_flag;
        int chain2 = chain1;
        int chain3 = chain2;
        
        /* Loop with PHI-derived condition */
        int k = 0;
        while (chain3) {  /* PHI value used in loop condition */
            total += k * j;
            k++;
            if (k > 10) break;
            
            /* Modify chain to eventually break loop */
            chain3 = k < 5 ? chain2 : 0;
        }
        
        /* Switch-like pattern with PHI */
        int selector;
        if (total % 1000 < 500) {
            selector = 1;
        } else {
            selector = 0;
        }
        
        int sel_copy = selector;
        if (sel_copy != 0) {  /* Another comparison type */
            total += j * j;
        }
    }
    
    return total;
}

/* Function 3: Multiple PHI nodes feeding into single conditional */
int multi_phi_convergence(int seed, int count) {
    int acc = seed;
    
    for (int i = 0; i < count; i++) {
        int flag_a, flag_b, flag_c;
        
        /* Three independent PHI sources */
        if (i % 3 == 0) {
            flag_a = 1;
        } else {
            flag_a = 0;
        }
        
        if (acc % 7 == 0) {
            flag_b = 1;
        } else {
            flag_b = 0;
        }
        
        flag_c = (i + seed) % 11 < 6 ? 1 : 0;
        
        /* Combine through assignments */
        int combined = flag_a;
        combined = flag_b;
        combined = flag_c;
        
        /* Multiple SSA copies */
        int copy1 = combined;
        int copy2 = copy1;
        int final_flag = copy2;
        
        /* Conditional that uses the final PHI-derived value */
        if (final_flag == 1) {  /* Explicit equality with 1 */
            acc = acc * 3 + 1;
        } else {
            acc = acc / 2;
        }
        
        /* Another use in different context */
        int tmp = final_flag;
        while (tmp > 0) {  /* Comparison with 0 */
            acc += i;
            tmp--;
        }
    }
    
    return acc;
}

/* Function 4: Recursive pattern with PHI propagation */
int recursive_phi(int depth, int max_depth, int toggle) {
    if (depth >= max_depth) return 1;
    
    int local_flag;
    
    /* PHI-like selection based on parameter */
    if (toggle) {
        local_flag = 1;
    } else {
        local_flag = (depth % 2) == 0 ? 1 : 0;
    }
    
    /* Assignment chain */
    int chain = local_flag;
    chain = chain;  /* Self-assignment creates SSA copy */
    int final = chain;
    
    int result = 0;
    if (final) {  /* Direct boolean use */
        result += recursive_phi(depth + 1, max_depth, 1);
        result += recursive_phi(depth + 1, max_depth, 0);
    } else {
        result += depth * 2;
    }
    
    return result;
}

/* Function 5: Array processing with PHI-dependent branches */
int array_phi_analysis(int* data, int size, int threshold) {
    int sum = 0;
    int hot_counter = 0;
    
    for (int i = 0; i < size; i++) {
        int is_hot;
        
        /* PHI determined by array content */
        if (data[i] > threshold) {
            is_hot = 1;
        } else {
            is_hot = 0;
        }
        
        /* Multiple SSA copies */
        int copy1 = is_hot;
        int copy2 = copy1;
        int copy3 = copy2 + 0;  /* Arithmetic that doesn't change value */
        
        /* Branch using PHI-derived value */
        if (copy3 == 1) {  /* Explicit comparison */
            sum += data[i] * 2;
            hot_counter++;
        } else {
            sum += data[i];
        }
        
        /* Another conditional in same block */
        int tmp = copy2;
        if (tmp) {  /* Implicit boolean test */
            sum ^= i;
        }
    }
    
    return sum + hot_counter;
}

/* Main function with profile-generating behavior */
int main(int argc, char** argv) {
    int mode = 1;  /* Default to hot mode */
    int total_result = 0;
    
    /* Parse command line for mode selection */
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    printf("Running AutoFDO PHI test in mode %d\n", mode);
    
    /* Warm-up phase - establish baseline profile */
    clock_t start = clock();
    
    /* Execute different patterns based on mode */
    switch (mode) {
        case 1:  /* Hot path dominant */
            printf("Executing hot path...\n");
            for (int phase = 0; phase < 10; phase++) {
                total_result += process_with_phi_chain(1, HOT_ITERATIONS);
                total_result += nested_phi_pattern(phase, HOT_ITERATIONS / 100);
            }
            break;
            
        case 2:  /* Cold path dominant */
            printf("Executing cold path...\n");
            total_result += process_with_phi_chain(2, COLD_ITERATIONS);
            total_result += nested_phi_pattern(42, COLD_ITERATIONS * 10);
            break;
            
        case 3:  /* Mixed behavior */
            printf("Executing mixed paths...\n");
            total_result += process_with_phi_chain(3, WARM_ITERATIONS);
            total_result += multi_phi_convergence(12345, WARM_ITERATIONS / 10);
            break;
            
        case 4:  /* Recursive pattern */
            printf("Executing recursive pattern...\n");
            total_result += recursive_phi(0, 15, 1);
            total_result += recursive_phi(0, 10, 0);
            break;
            
        case 5:  /* Array processing */
            printf("Executing array processing...\n");
            {
                int data[10000];
                for (int i = 0; i < 10000; i++) {
                    data[i] = rand() % 1000;
                }
                total_result += array_phi_analysis(data, 10000, 500);
            }
            break;
            
        default:  /* All patterns */
            printf("Executing all patterns...\n");
            total_result += process_with_phi_chain(1, HOT_ITERATIONS / 10);
            total_result += nested_phi_pattern(1, HOT_ITERATIONS / 100);
            total_result += multi_phi_convergence(54321, HOT_ITERATIONS / 50);
            total_result += recursive_phi(0, 12, 1);
            {
                int data[5000];
                for (int i = 0; i < 5000; i++) {
                    data[i] = rand() % 1000;
                }
                total_result += array_phi_analysis(data, 5000, 300);
            }
            break;
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Result checksum: %d\n", total_result);
    printf("Execution time: %.3f seconds\n", elapsed);
    
    return total_result % 256;  /* Return non-zero for verification */
}
