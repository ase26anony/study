/* autofdo_phi_conditional.c - Test program for GCC AutoFDO PHI-to-conditional analysis */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define HOT_ITERATIONS 1000000
#define COLD_ITERATIONS 100
#define ARRAY_SIZE 10000

/* Function 1: Complex PHI pattern with SSA copy chains in hot loop */
int process_hot_path(int mode, int limit) {
    int result = 0;
    int i, j;
    
    /* Outer hot loop - will be heavily annotated */
    for (i = 0; i < HOT_ITERATIONS; i++) {
        int phi_val;
        int tmp1, tmp2, tmp3;
        int cmp_var;
        
        /* Create branching that feeds into PHI */
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
        
        /* PHI-like behavior simulated through variable reuse */
        int phi_result = phi_val;
        
        /* Chain of SSA copies to trigger the while loop walking back */
        tmp1 = phi_result;          /* First copy */
        tmp2 = tmp1;                /* Second copy */
        tmp3 = tmp2 + 0;            /* Arithmetic that preserves value */
        cmp_var = tmp3;             /* Final copy before comparison */
        
        /* Conditional using the PHI-derived value - direct use */
        if (cmp_var) {              /* Line 1312-1333 analysis starts here */
            /* Hot path - frequently taken */
            result += i * 2;
        } else {
            /* Cold path - rarely taken */
            result -= i;
        }
        
        /* Another PHI pattern with explicit comparison */
        int phi_val2;
        if (i % 5 == 0) {
            phi_val2 = 1;
        } else {
            phi_val2 = 0;
        }
        
        int chain1 = phi_val2;
        int chain2 = chain1;
        int chain3 = chain2;
        
        /* Explicit equality comparison with constant 1 */
        if (chain3 == 1) {          /* Should trigger integer_onep check */
            result += 3;
        }
        
        /* Nested conditional with PHI-derived value */
        if (i % 100 == 0) {
            int nested_phi;
            if (mode == 1) {
                nested_phi = 1;
            } else {
                nested_phi = 0;
            }
            
            int n1 = nested_phi;
            int n2 = n1;
            
            /* Comparison with constant 0 */
            if (n2 != 0) {          /* Should trigger integer_zerop check */
                result += 7;
            }
        }
    }
    
    return result;
}

/* Function 2: Cold path with different PHI patterns */
int process_cold_path(int mode, int limit) {
    int result = 0;
    
    for (int i = 0; i < COLD_ITERATIONS; i++) {
        int phi_val;
        
        /* Different branching pattern for cold path */
        if (i % 7 == 0) {
            phi_val = 1;
        } else {
            phi_val = 0;
        }
        
        /* Longer copy chain */
        int a = phi_val;
        int b = a;
        int c = b;
        int d = c;
        int e = d + 0;
        int f = e;
        
        /* While loop using PHI-derived value */
        while (f) {                 /* Loop condition analysis */
            result += i;
            f = 0;  /* Break after first iteration */
        }
        
        /* Switch-like pattern using PHI */
        int switch_phi;
        if (mode == 2) {
            switch_phi = 1;
        } else {
            switch_phi = 0;
        }
        
        int s1 = switch_phi;
        int s2 = s1;
        
        if (s2 == 1) {
            result += 11;
        }
    }
    
    return result;
}

/* Function 3: Array processing with PHI-dependent loops */
int process_array(int* data, int size, int threshold) {
    int sum = 0;
    int use_fast_path;
    
    /* PHI based on array characteristics */
    if (size > 1000) {
        use_fast_path = 1;
    } else {
        use_fast_path = 0;
    }
    
    /* Copy chain */
    int u1 = use_fast_path;
    int u2 = u1;
    int u3 = u2;
    
    /* Loop with PHI-derived condition */
    if (u3) {
        /* Fast path - vectorizable */
        for (int i = 0; i < size; i += 4) {
            sum += data[i] + data[i+1] + data[i+2] + data[i+3];
        }
    } else {
        /* Slow path - scalar */
        for (int i = 0; i < size; i++) {
            sum += data[i];
        }
    }
    
    /* Another PHI pattern inside loop */
    for (int i = 0; i < size; i++) {
        int element_phi;
        if (data[i] > threshold) {
            element_phi = 1;
        } else {
            element_phi = 0;
        }
        
        int e1 = element_phi;
        int e2 = e1;
        
        if (e2) {
            sum += 1000;
        }
    }
    
    return sum;
}

/* Function 4: Complex nested control flow with multiple PHIs */
int complex_nested_logic(int depth, int width) {
    int total = 0;
    
    for (int d = 0; d < depth; d++) {
        for (int w = 0; w < width; w++) {
            int phi1, phi2;
            
            /* First PHI */
            if (d % 2 == 0) {
                phi1 = 1;
            } else {
                phi1 = 0;
            }
            
            /* Second PHI */
            if (w % 3 == 0) {
                phi2 = 1;
            } else {
                phi2 = 0;
            }
            
            /* Copy chains for both PHIs */
            int p1a = phi1;
            int p1b = p1a;
            int p2a = phi2;
            int p2b = p2a;
            
            /* Combined condition */
            if (p1b && p2b) {
                total += d * w * 2;
            } else if (p1b || p2b) {
                total += d + w;
            } else {
                total += 1;
            }
            
            /* Nested if with PHI reuse */
            if (p1b) {
                int nested_phi;
                if (total % 2 == 0) {
                    nested_phi = 1;
                } else {
                    nested_phi = 0;
                }
                
                int n1 = nested_phi;
                int n2 = n1;
                
                if (n2 == 1) {
                    total += 5;
                }
            }
        }
    }
    
    return total;
}

/* Helper function to create call site diversity */
int helper_function(int x, int y, int use_alt) {
    int phi_val;
    
    if (use_alt) {
        phi_val = (x > y) ? 1 : 0;
    } else {
        phi_val = (x < y) ? 1 : 0;
    }
    
    int h1 = phi_val;
    int h2 = h1;
    int h3 = h2 + 0;
    
    if (h3) {
        return x * y;
    } else {
        return x + y;
    }
}

int main(int argc, char** argv) {
    int mode = 1;  /* Default to hot mode */
    int result = 0;
    clock_t start, end;
    
    /* Parse command line for mode selection */
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    printf("Running AutoFDO PHI test in mode %d\n", mode);
    start = clock();
    
    /* Phase 1: Warm-up with mixed behavior */
    printf("Phase 1: Warm-up\n");
    for (int warm = 0; warm < 1000; warm++) {
        result += process_cold_path(mode, 100);
    }
    
    /* Phase 2: Main processing with mode-dependent behavior */
    printf("Phase 2: Main processing\n");
    if (mode == 1) {
        /* Hot mode - execute hot path heavily */
        result += process_hot_path(mode, HOT_ITERATIONS);
        
        /* Process large array */
        int* data = malloc(ARRAY_SIZE * sizeof(int));
        for (int i = 0; i < ARRAY_SIZE; i++) {
            data[i] = i % 100;
        }
        result += process_array(data, ARRAY_SIZE, 50);
        free(data);
        
        /* Complex nested logic */
        result += complex_nested_logic(100, 100);
        
    } else if (mode == 2) {
        /* Cold mode - mostly cold paths */
        result += process_cold_path(mode, COLD_ITERATIONS);
        
        /* Small array */
        int data[100];
        for (int i = 0; i < 100; i++) {
            data[i] = i;
        }
        result += process_array(data, 100, 75);
        
        result += complex_nested_logic(10, 10);
        
    } else {
        /* Mixed mode */
        result += process_hot_path(mode, HOT_ITERATIONS / 10);
        result += process_cold_path(mode, COLD_ITERATIONS * 2);
        
        int data[500];
        for (int i = 0; i < 500; i++) {
            data[i] = i % 50;
        }
        result += process_array(data, 500, 25);
    }
    
    /* Phase 3: Final computations with helper calls */
    printf("Phase 3: Final computations\n");
    for (int i = 0; i < 10000; i++) {
        result += helper_function(i, i % 100, mode == 1);
    }
    
    end = clock();
    double cpu_time_used = ((double)(end - start)) / CLOCKS_PER_SEC;
    
    printf("Result checksum: %d\n", result);
    printf("Time elapsed: %.2f seconds\n", cpu_time_used);
    
    return 0;
}
