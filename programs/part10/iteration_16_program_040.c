#include <stdio.h>
#include <stdlib.h>

/* Helper function to create SSA copy chains */
static inline int pass_through(int x) { return x; }
static inline int copy_value(int x) { int y = x; return y; }

/* Pattern 1: Loop exit condition depends on PHI-assigned flag */
int pattern1_loop_phi_flag(int iterations) {
    int flag = 0;  /* Initial value */
    int count = 0;
    int i = 0;
    
    /* Loop creates a PHI node for 'flag' at the header */
    while (i < iterations) {
        /* Complex control flow inside loop */
        if (i % 3 == 0) {
            flag = 1;  /* One assignment to flag */
        } else if (i % 3 == 1) {
            flag = 0;  /* Another assignment to flag */
        } else {
            flag = (i % 2);  /* Third assignment to flag */
        }
        
        /* Create SSA copy chain */
        int temp1 = flag;
        int temp2 = pass_through(temp1);
        int temp3 = copy_value(temp2);
        
        /* Conditional branch comparing against constant 0 */
        if (temp3 == 0) {  /* This should trigger the analysis */
            count += 1;
        }
        
        i++;
    }
    
    /* Another conditional at loop exit */
    int final_check = flag;
    if (final_check != 0) {  /* Compare against 0 */
        count *= 2;
    }
    
    return count;
}

/* Pattern 2: If-else chain setting 0/1, then conditional */
int pattern2_ifelse_phi(int x, int y) {
    int result;
    
    /* PHI node created at merge point */
    if (x > y) {
        result = 1;
    } else if (x < y) {
        result = 0;
    } else {
        result = (x % 2);  /* Either 0 or 1 */
    }
    
    /* Multiple SSA copies */
    int a = result;
    int b = pass_through(a);
    int c = copy_value(b);
    int d = pass_through(c);
    
    /* Conditional comparing against 1 */
    if (d == 1) {  /* Should trigger analysis */
        return x * 2;
    } else {
        return y * 3;
    }
}

/* Pattern 3: Switch-case setting 0/1, followed by conditional */
int pattern3_switch_phi(int option) {
    int value;
    
    switch (option % 4) {
        case 0:
            value = 1;
            break;
        case 1:
            value = 0;
            break;
        case 2:
            value = 1;
            break;
        default:
            value = 0;
            break;
    }
    
    /* Create copy chain through function calls */
    int v1 = value;
    int v2 = pass_through(v1);
    
    /* Implicit boolean check (compares against 0) */
    if (v2) {  /* Equivalent to if (v2 != 0) */
        return 100;
    }
    
    return -100;
}

/* Pattern 4: Ternary operator producing 0/1 */
int pattern4_ternary_phi(int a, int b, int c) {
    /* Ternary creates PHI-like value */
    int choice = (a > b) ? ((c > 0) ? 1 : 0) : 0;
    
    /* Multiple assignments creating SSA web */
    int x = choice;
    int y = x;
    int z = pass_through(y);
    
    /* Compare against 0 with not-equal operator */
    if (z != 0) {  /* Should trigger analysis */
        return a + b + c;
    }
    
    return a * b * c;
}

/* Pattern 5: Nested loops with PHI in outer loop header */
int pattern5_nested_loops(int n) {
    int total = 0;
    int outer_flag = 0;
    
    for (int i = 0; i < n; i++) {
        /* PHI for outer_flag at loop header */
        int inner_flag = 0;
        
        for (int j = 0; j < i; j++) {
            if ((i * j) % 5 == 0) {
                inner_flag = 1;
            } else {
                inner_flag = 0;
            }
            
            /* Copy chain inside inner loop */
            int f1 = inner_flag;
            int f2 = copy_value(f1);
            
            /* Conditional on copied value */
            if (f2 == 1) {  /* Compare against 1 */
                total += j;
            }
        }
        
        /* Update outer flag based on inner results */
        if (inner_flag == 0) {  /* Another compare against 0 */
            outer_flag = 1;
        }
        
        /* Use outer_flag in condition */
        int of_copy = outer_flag;
        if (of_copy != 0) {  /* Compare against 0 */
            total += i * 10;
        }
    }
    
    return total;
}

/* Pattern 6: Complex control flow with multiple merges */
int pattern6_complex_cfg(int x) {
    int val1, val2;
    
    if (x > 100) {
        val1 = 1;
        if (x > 200) {
            val2 = 1;
        } else {
            val2 = 0;
        }
    } else {
        val1 = 0;
        val2 = (x > 50) ? 1 : 0;
    }
    
    /* Both values come from PHI nodes */
    int combined = val1 & val2;  /* Results in 0 or 1 */
    
    /* Long copy chain */
    int t1 = combined;
    int t2 = t1;
    int t3 = pass_through(t2);
    int t4 = copy_value(t3);
    int t5 = pass_through(t4);
    
    /* Final conditional */
    if (t5 == 0) {  /* Compare against 0 */
        return x * 2;
    } else {
        return x / 2;
    }
}

/* Main function with rich control flow */
int main(int argc, char **argv) {
    int total = 0;
    
    /* Initialize with command line or defaults */
    int base = (argc > 1) ? atoi(argv[1]) : 100;
    
    /* Pattern 1: Loop with phi flag */
    total += pattern1_loop_phi_flag(base % 20 + 5);
    
    /* Pattern 2: If-else phi */
    total += pattern2_ifelse_phi(base, base / 2);
    
    /* Pattern 3: Switch phi */
    total += pattern3_switch_phi(base);
    
    /* Pattern 4: Ternary phi */
    total += pattern4_ternary_phi(base, base + 1, base - 1);
    
    /* Pattern 5: Nested loops */
    total += pattern5_nested_loops(base % 10 + 3);
    
    /* Pattern 6: Complex CFG */
    total += pattern6_complex_cfg(base);
    
    /* Main loop with flag-based exit (mimics target pattern) */
    int main_flag = 0;
    int main_counter = 0;
    
    while (main_counter < 10) {
        /* Update flag based on complex condition */
        if ((total + main_counter) % 7 == 0) {
            main_flag = 1;
        } else {
            main_flag = 0;
        }
        
        /* Create SSA copy chain */
        int mf1 = main_flag;
        int mf2 = pass_through(mf1);
        
        /* Conditional on copied value comparing to 1 */
        if (mf2 == 1) {  /* Should trigger the uncovered analysis */
            total += 1000;
        }
        
        main_counter++;
    }
    
    /* Final conditional with implicit 0 comparison */
    int final_val = (total > 5000) ? 1 : 0;
    if (final_val) {  /* if (final_val != 0) */
        total += 9999;
    }
    
    printf("Result: %d\n", total);
    return 0;
}
