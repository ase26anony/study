/* autofdo_phi_conditional.c
 * Test program for GCC AutoFDO profile reading and transformation
 * Specifically targets PHI-to-conditional constant propagation analysis
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HOT_ITERATIONS 10000000
#define WARM_ITERATIONS 1000000
#define COLD_ITERATIONS 100

/* Function 1: Complex PHI pattern with SSA copy chains */
int process_with_phi_chains(int mode, int iterations) {
    int result = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        int phi_val;
        int tmp1, tmp2, tmp3;
        int cmp_var;
        
        /* Create branching that feeds into PHI node */
        if (mode == 1) {
            /* Hot path - sets value to 1 */
            phi_val = 1;
        } else if (mode == 2) {
            /* Medium path - sets value based on iteration */
            phi_val = (i % 100 == 0) ? 1 : 0;
        } else {
            /* Cold path - always 0 */
            phi_val = 0;
        }
        
        /* Create SSA copy chain to trigger while loop in uncovered code */
        tmp1 = phi_val;      /* First assignment */
        tmp2 = tmp1;         /* Second assignment */
        tmp3 = tmp2 + 0;     /* Arithmetic that preserves value */
        cmp_var = tmp3;      /* Final copy before comparison */
        
        /* Multiple comparison types using PHI-derived value */
        if (cmp_var) {  /* Direct use in if condition */
            result += i * 2;
        }
        
        if (cmp_var == 1) {  /* Explicit equality comparison */
            result += i / 2;
        }
        
        /* Another PHI pattern with different structure */
        int phi_val2;
        if (i % 3 == 0) {
            phi_val2 = 1;
        } else {
            phi_val2 = 0;
        }
        
        int chain1 = phi_val2;
        int chain2 = chain1;
        
        /* Use in while condition */
        int counter = 3;
        while (chain2 && counter > 0) {
            result += counter;
            counter--;
        }
    }
    
    return result;
}

/* Function 2: Nested loops with varying PHI patterns */
int nested_phi_analysis(int depth, int width) {
    int total = 0;
    int i, j;
    
    for (i = 0; i < depth; i++) {
        int outer_phi;
        
        /* PHI based on outer loop */
        if (i % 10 == 0) {
            outer_phi = 1;
        } else {
            outer_phi = 0;
        }
        
        /* Chain of assignments */
        int a = outer_phi;
        int b = a;
        int c = b;
        
        for (j = 0; j < width; j++) {
            int inner_phi;
            
            /* PHI based on inner loop with complex condition */
            if ((i + j) % 7 == 0) {
                inner_phi = 1;
            } else {
                inner_phi = 0;
            }
            
            int x = inner_phi;
            int y = x;
            
            /* Multiple comparisons in hot inner loop */
            if (y != 0) {
                total += i * j;
            }
            
            if (c == 1) {
                total += j;
            }
            
            /* Another SSA chain */
            int z = y;
            if (z) {
                total += 1;
            }
        }
    }
    
    return total;
}

/* Function 3: Recursive PHI pattern with call site variations */
int recursive_phi_helper(int n, int *acc) {
    if (n <= 0) return 0;
    
    int phi_val;
    if (n % 5 == 0) {
        phi_val = 1;
    } else {
        phi_val = 0;
    }
    
    /* SSA copy chain */
    int t1 = phi_val;
    int t2 = t1;
    int t3 = t2;
    
    if (t3) {
        *acc += n * 2;
    }
    
    if (t3 == 1) {
        *acc += n / 2;
    }
    
    return recursive_phi_helper(n - 1, acc) + 1;
}

int recursive_phi_pattern(int iterations) {
    int accumulator = 0;
    int i;
    
    for (i = 0; i < iterations; i++) {
        int start = (i % 20) + 1;
        recursive_phi_helper(start, &accumulator);
    }
    
    return accumulator;
}

/* Function 4: Array processing with data-dependent PHI */
int array_based_phi(int *data, int size) {
    int sum = 0;
    int i;
    
    for (i = 0; i < size; i++) {
        int phi_val;
        
        /* PHI based on array data */
        if (data[i] > 1000) {
            phi_val = 1;
        } else {
            phi_val = 0;
        }
        
        /* Extended SSA chain */
        int v1 = phi_val;
        int v2 = v1;
        int v3 = v2 + 0;  /* Arithmetic that doesn't change value */
        int v4 = v3;
        int v5 = v4;
        
        /* Multiple conditional uses */
        if (v5) {
            sum += data[i];
        }
        
        if (v5 == 1) {
            sum += i;
        }
        
        /* Another PHI in same basic block */
        int phi_val2;
        if (data[i] % 2 == 0) {
            phi_val2 = 1;
        } else {
            phi_val2 = 0;
        }
        
        int w1 = phi_val2;
        int w2 = w1;
        
        if (w2 != 0) {
            sum -= data[i] / 2;
        }
    }
    
    return sum;
}

/* Main function with profile-generating runtime behavior */
int main(int argc, char *argv[]) {
    int mode = 1;  /* Default to hot mode */
    int iterations = HOT_ITERATIONS;
    int result = 0;
    
    /* Parse command line for mode selection */
    if (argc > 1) {
        mode = atoi(argv[1]);
        if (argc > 2) {
            iterations = atoi(argv[2]);
        }
    }
    
    printf("Running mode %d with %d iterations\n", mode, iterations);
    
    /* Warm-up phase with medium profile */
    printf("Warm-up phase...\n");
    result += process_with_phi_chains(2, WARM_ITERATIONS / 10);
    
    /* Main execution with mode-dependent behavior */
    printf("Main execution phase...\n");
    
    switch (mode) {
        case 1:  /* Hot path dominant */
            result += process_with_phi_chains(1, iterations);
            result += nested_phi_analysis(1000, 100);
            break;
            
        case 2:  /* Mixed hot/cold */
            result += process_with_phi_chains(2, iterations);
            result += nested_phi_analysis(100, 1000);
            break;
            
        case 3:  /* Cold path dominant */
            result += process_with_phi_chains(3, iterations);
            result += nested_phi_analysis(10, 10);
            break;
            
        case 4:  /* Recursive pattern */
            result += recursive_phi_pattern(iterations / 100);
            break;
            
        case 5:  /* Array-based pattern */
            {
                int size = iterations / 100;
                int *data = malloc(size * sizeof(int));
                if (data) {
                    for (int i = 0; i < size; i++) {
                        data[i] = (i * 17) % 2345;
                    }
                    result += array_based_phi(data, size);
                    free(data);
                }
            }
            break;
            
        default:
            printf("Unknown mode, using default\n");
            result += process_with_phi_chains(1, iterations / 10);
    }
    
    /* Additional mixed workload to create varied profile */
    printf("Mixed workload phase...\n");
    for (int phase = 0; phase < 3; phase++) {
        if (phase == mode % 3) {
            /* Hotter path for this phase */
            result += nested_phi_analysis(500, 200);
        } else {
            /* Colder path */
            result += process_with_phi_chains(3, COLD_ITERATIONS);
        }
    }
    
    printf("Result checksum: %d\n", result);
    return 0;
}
