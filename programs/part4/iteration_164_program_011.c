/* test_autofdo_phi_conditional.c
 * 
 * This program is designed to trigger the uncovered PHI-to-conditional
 * analysis in GCC's AutoFDO profile reader (auto-profile.cc lines 1312-1333).
 * It creates patterns where boolean values (0/1) flow through PHI nodes
 * into conditional comparisons with intermediate SSA copy chains.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

/* ========== Pattern 1: Direct PHI-to-Condition with Copy Chain ========== */
int pattern1_hot_path(int iterations) {
    int sum = 0;
    volatile int force_ssa = 0; /* Prevent optimization */
    
    for (int i = 0; i < iterations; i++) {
        int phi_val;
        int tmp1, tmp2, tmp3;
        
        /* Create branching that leads to PHI node */
        if (i % 17 == 0) {  /* Cold path - rarely taken */
            phi_val = 0;
            force_ssa += 1;
        } else {  /* Hot path - frequently taken */
            phi_val = 1;
            force_ssa += 2;
        }
        
        /* SSA copy chain to trigger while loop in uncovered code */
        tmp1 = phi_val;      /* First assignment */
        tmp2 = tmp1;         /* Second assignment */
        tmp3 = tmp2 + 0;     /* Third assignment (arithmetic that doesn't change value) */
        
        /* Conditional using PHI-derived value - triggers uncovered analysis */
        if (tmp3) {  /* Direct use in if condition */
            sum += i * 2;  /* Hot computation */
        } else {
            sum += i / 2;  /* Cold computation */
        }
        
        /* Another conditional with explicit comparison */
        if (tmp2 == 1) {  /* Explicit equality check */
            sum += 3;
        }
    }
    
    return sum + force_ssa;
}

/* ========== Pattern 2: Nested PHI with Multiple Predecessors ========== */
int pattern2_complex_phi(int mode, int limit) {
    int result = 0;
    int phi_base;
    
    /* Multiple predecessor blocks creating complex PHI */
    if (mode == 0) {
        phi_base = 1;  /* Hot path value */
        result += 100;
    } else if (mode == 1) {
        phi_base = 0;  /* Medium path value */
        result += 50;
    } else {
        phi_base = 1;  /* Another hot path */
        result += 75;
    }
    
    /* Intermediate SSA copies */
    int chain1 = phi_base;
    int chain2 = chain1;
    int chain3 = chain2;
    int final_val = chain3;
    
    /* Loop with PHI-derived condition */
    for (int i = 0; i < limit; i++) {
        /* PHI inside loop with copy chain */
        int loop_phi;
        if (i % 2 == 0) {
            loop_phi = final_val;
        } else {
            loop_phi = !final_val;
        }
        
        int loop_tmp1 = loop_phi;
        int loop_tmp2 = loop_tmp1;
        
        /* Multiple conditionals using PHI value */
        if (loop_tmp2) {
            result += i * 3;
        }
        
        if (loop_tmp1 != 0) {  /* Another comparison type */
            result += i;
        }
    }
    
    return result;
}

/* ========== Pattern 3: PHI in Loop Exit Condition ========== */
int pattern3_loop_exit(int size) {
    int data[1000];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < size && i < 1000; i++) {
        data[i] = i % 100;
    }
    
    int should_continue = 1;
    int iterations = 0;
    
    /* Loop with PHI-controlled exit */
    while (should_continue) {
        int exit_phi;
        
        /* Create PHI based on multiple conditions */
        if (iterations < size / 2) {
            exit_phi = 1;  /* Continue */
        } else if (iterations < size) {
            exit_phi = (iterations % 10 != 0);  /* Mixed */
        } else {
            exit_phi = 0;  /* Exit */
        }
        
        /* Copy chain */
        int exit_tmp1 = exit_phi;
        int exit_tmp2 = exit_tmp1;
        int exit_tmp3 = exit_tmp2;
        
        /* Use in loop condition through assignment */
        should_continue = exit_tmp3;
        
        if (iterations < 1000) {
            sum += data[iterations % 1000];
        }
        
        iterations++;
        
        /* Safety check */
        if (iterations > 1000000) break;
    }
    
    return sum + iterations;
}

/* ========== Pattern 4: Multi-Block PHI Network ========== */
int pattern4_phi_network(int depth) {
    int val = 0;
    
    for (int d = 0; d < depth; d++) {
        int phi_node;
        
        /* Complex predecessor structure */
        switch (d % 4) {
            case 0:
                phi_node = 1;
                val += d * 2;
                break;
            case 1:
                phi_node = 0;
                val += d * 3;
                break;
            case 2:
                phi_node = 1;
                val += d * 5;
                break;
            case 3:
                phi_node = (d % 3 == 0) ? 1 : 0;
                val += d * 7;
                break;
        }
        
        /* Extended copy chain */
        int c1 = phi_node;
        int c2 = c1;
        int c3 = c2 + 0;  /* Arithmetic that preserves value */
        int c4 = c3;
        int c5 = c4;
        
        /* Nested conditionals */
        if (c5) {
            if (d % 2 == 0) {
                val += 1000;
            } else {
                val += 2000;
            }
        }
        
        /* Comparison with constant 1 */
        if (c3 == 1) {
            val += 500;
        }
    }
    
    return val;
}

/* ========== Pattern 5: Real Computation with PHI Patterns ========== */
int pattern5_prime_check(int limit) {
    int prime_count = 0;
    
    for (int n = 2; n < limit; n++) {
        int is_prime = 1;  /* Start assuming prime */
        
        for (int i = 2; i * i <= n; i++) {
            if (n % i == 0) {
                is_prime = 0;  /* Not prime */
                break;
            }
        }
        
        /* PHI-like pattern created by the break */
        int phi_val = is_prime;
        
        /* Copy chain */
        int check1 = phi_val;
        int check2 = check1;
        int check3 = check2 + 0;
        
        /* Conditional using PHI-derived value */
        if (check3) {
            prime_count++;
            /* Additional computation to make block "hot" */
            for (int j = 0; j < 10; j++) {
                prime_count += (n % 2);
            }
        }
    }
    
    return prime_count;
}

/* ========== Main Function with Profile-Generating Behavior ========== */
int main(int argc, char *argv[]) {
    int mode = 1;  /* Default to hot mode */
    int total_result = 0;
    
    /* Parse command line for mode selection */
    if (argc > 1) {
        mode = atoi(argv[1]);
    }
    
    /* Seed for reproducible behavior */
    srand(42);
    
    clock_t start = clock();
    
    /* Mode 0: Cold paths only - rarely executed */
    /* Mode 1: Hot paths dominant - frequently executed */
    /* Mode 2: Mixed behavior */
    
    if (mode == 0) {
        /* COLD MODE - execute cold paths */
        printf("Running in COLD mode (rare paths)\n");
        
        /* Pattern 1 with few iterations */
        total_result += pattern1_hot_path(100);
        
        /* Pattern 2 with cold mode */
        total_result += pattern2_complex_phi(1, 500);
        
        /* Pattern 3 with small size */
        total_result += pattern3_loop_exit(100);
        
        /* Pattern 4 with shallow depth */
        total_result += pattern4_phi_network(100);
        
        /* Pattern 5 with small limit */
        total_result += pattern5_prime_check(1000);
        
    } else if (mode == 1) {
        /* HOT MODE - execute hot paths millions of times */
        printf("Running in HOT mode (dominant paths)\n");
        
        /* Execute hot patterns many times to generate strong profile */
        for (int repeat = 0; repeat < 10; repeat++) {
            /* Pattern 1 with many iterations */
            total_result += pattern1_hot_path(1000000);
            
            /* Pattern 2 with hot mode */
            total_result += pattern2_complex_phi(0, 50000);
            
            /* Pattern 3 with large size */
            total_result += pattern3_loop_exit(50000);
            
            /* Pattern 4 with deep recursion */
            total_result += pattern4_phi_network(10000);
            
            /* Pattern 5 with large limit */
            total_result += pattern5_prime_check(50000);
        }
        
    } else if (mode == 2) {
        /* MIXED MODE - balanced execution */
        printf("Running in MIXED mode\n");
        
        /* Alternate between hot and cold */
        for (int batch = 0; batch < 5; batch++) {
            if (batch % 2 == 0) {
                total_result += pattern1_hot_path(500000);
                total_result += pattern2_complex_phi(0, 25000);
            } else {
                total_result += pattern1_hot_path(10000);
                total_result += pattern2_complex_phi(1, 1000);
            }
            
            total_result += pattern3_loop_exit(20000);
            total_result += pattern4_phi_network(5000);
            total_result += pattern5_prime_check(20000);
        }
    }
    
    clock_t end = clock();
    double elapsed = (double)(end - start) / CLOCKS_PER_SEC;
    
    printf("Result: %d\n", total_result);
    printf("Time elapsed: %.2f seconds\n", elapsed);
    
    /* Additional verification computation */
    int verify = 0;
    for (int i = 0; i < 1000; i++) {
        verify += (total_result % (i + 1));
    }
    printf("Verification hash: %d\n", verify);
    
    return 0;
}
