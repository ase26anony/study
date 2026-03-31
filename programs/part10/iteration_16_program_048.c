#include <stdio.h>
#include <stdlib.h>

/* Helper function to create SSA copy chains */
static inline int pass_through(int x) { return x; }
static inline int copy_value(int x) { int y = x; return y; }

/* Pattern 1: Loop exit condition depends on PHI-assigned flag */
int pattern1_loop_exit(int iterations) {
    int flag = 0;  /* Initial value */
    int count = 0;
    int i = 0;
    
    while (i < iterations) {
        /* Complex control flow that sets flag */
        if (i % 3 == 0) {
            flag = 1;
        } else if (i % 5 == 0) {
            flag = 0;
        } else {
            flag = (i % 2 == 0) ? 1 : 0;
        }
        
        /* Create SSA copy chain */
        int tmp1 = flag;
        int tmp2 = pass_through(tmp1);
        int tmp3 = copy_value(tmp2);
        
        /* Conditional branch comparing against 0 */
        if (tmp3 == 0) {  /* This should trigger the uncovered code */
            count += i;
        }
        
        i++;
    }
    
    /* Another conditional using the final flag value */
    int final_check = flag;
    if (final_check != 1) {  /* Compare against 1 */
        count *= 2;
    }
    
    return count;
}

/* Pattern 2: If-else chain setting 0/1, then conditional */
int pattern2_if_else_merge(int a, int b) {
    int result;
    
    /* PHI node will be created at merge point */
    if (a > b) {
        result = 1;
    } else if (a < b) {
        result = 0;
    } else {
        result = (a % 2 == 0) ? 1 : 0;
    }
    
    /* Multiple SSA copies */
    int x = result;
    int y = pass_through(x);
    int z = copy_value(y);
    
    /* Conditional branch - compare against constant */
    if (z == 1) {  /* Should trigger uncovered code */
        return a * b;
    } else {
        return a + b;
    }
}

/* Pattern 3: Switch-case setting 0/1 */
int pattern3_switch_case(int val) {
    int flag;
    
    switch (val % 4) {
        case 0: flag = 1; break;
        case 1: flag = 0; break;
        case 2: flag = 1; break;
        default: flag = 0; break;
    }
    
    /* Chain of assignments */
    int a = flag;
    int b = a;
    int c = pass_through(b);
    
    /* Implicit boolean check (compares against 0) */
    if (c) {  /* Equivalent to if (c != 0) */
        return val * 2;
    }
    
    return val / 2;
}

/* Pattern 4: Nested loops with PHI in inner loop */
int pattern4_nested_loops(int n) {
    int total = 0;
    
    for (int i = 0; i < n; i++) {
        int inner_flag = 0;
        
        for (int j = 0; j < i; j++) {
            /* PHI node for inner_flag at loop header */
            if (j % 2 == 0) {
                inner_flag = 1;
            } else {
                inner_flag = 0;
            }
            
            /* Multiple uses with copy chains */
            int t1 = inner_flag;
            int t2 = t1;
            
            /* Conditional comparing against 0 */
            if (t2 == 0) {  /* Should trigger */
                total += j;
            } else {
                total -= j;
            }
        }
        
        /* Another conditional using the flag */
        int check = inner_flag;
        if (check != 1) {  /* Compare against 1 */
            total += i;
        }
    }
    
    return total;
}

/* Pattern 5: Ternary operator producing 0/1 */
int pattern5_ternary(int x, int y) {
    /* Ternary creates PHI node */
    int is_greater = (x > y) ? 1 : 0;
    
    /* Pass through function to create SSA chain */
    int v1 = is_greater;
    int v2 = pass_through(v1);
    int v3 = copy_value(v2);
    int v4 = pass_through(v3);
    
    /* Conditional branch */
    if (v4 == 0) {  /* Should trigger */
        return x - y;
    } else {
        return y - x;
    }
}

/* Pattern 6: Complex control flow with multiple merges */
int pattern6_complex_flow(int limit) {
    int state = 0;
    int sum = 0;
    
    for (int i = 0; i < limit; i++) {
        /* Multiple branches setting state to 0 or 1 */
        if (i % 7 == 0) {
            state = 1;
        } else if (i % 11 == 0) {
            state = 0;
        } else {
            /* Nested condition */
            if (i % 3 == 0) {
                state = 1;
            } else {
                state = 0;
            }
        }
        
        /* Long copy chain */
        int a = state;
        int b = a;
        int c = b;
        int d = pass_through(c);
        int e = copy_value(d);
        
        /* Multiple conditionals in same basic block */
        if (e == 1) {
            sum += i * 2;
        }
        
        if (a == 0) {  /* Another conditional */
            sum -= i;
        }
    }
    
    return sum;
}

/* Main function with its own pattern */
int main() {
    int total = 0;
    int data[6] = {10, 5, 8, 12, 7, 15};
    
    /* Main loop with flag-based exit (mimics target pattern) */
    int done = 0;
    int idx = 0;
    
    while (!done) {  /* Implicit comparison against 0 */
        /* Call each pattern function */
        total += pattern1_loop_exit(data[idx % 6]);
        total += pattern2_if_else_merge(idx, data[idx % 6]);
        total += pattern3_switch_case(data[idx % 6]);
        total += pattern4_nested_loops(data[idx % 6] % 5 + 1);
        total += pattern5_ternary(idx, data[idx % 6]);
        total += pattern6_complex_flow(data[idx % 6] % 3 + 2);
        
        idx++;
        
        /* Set done flag based on condition (creates PHI) */
        if (idx >= 20 || total > 10000) {
            done = 1;  /* Will be used in while(!done) */
        } else {
            done = 0;
        }
        
        /* Create copy chain for the flag */
        int check_done = done;
        int verify = pass_through(check_done);
        
        /* Extra conditional on the flag */
        if (verify == 1) {  /* Compare against 1 */
            total += 1000;
        }
    }
    
    printf("Final result: %d\n", total);
    
    /* Additional test cases to exercise different paths */
    printf("Pattern1: %d\n", pattern1_loop_exit(5));
    printf("Pattern2: %d\n", pattern2_if_else_merge(7, 3));
    printf("Pattern3: %d\n", pattern3_switch_case(9));
    printf("Pattern4: %d\n", pattern4_nested_loops(4));
    printf("Pattern5: %d\n", pattern5_ternary(12, 8));
    printf("Pattern6: %d\n", pattern6_complex_flow(6));
    
    return 0;
}
