#include <stdio.h>
#include <stdlib.h>

/* Helper function to create SSA copy chains */
static inline int pass_value(int v) { return v; }
static inline int copy_once(int v) { int t = v; return t; }
static inline int copy_twice(int v) { int a = v; int b = a; return b; }

/* Pattern 1: Loop exit condition depends on PHI-assigned flag */
int pattern1_loop_exit(int iterations) {
    int flag = 0;  // Initial definition
    int count = 0;
    
    while (!flag) {  // Conditional on PHI-derived value (flag != 0)
        count++;
        if (count >= iterations) {
            flag = 1;  // One assignment to flag
        } else {
            flag = 0;  // Another assignment to flag - creates PHI at loop header
        }
        
        // Create copy chain
        int flag_copy = flag;
        int flag_copy2 = flag_copy;
        if (flag_copy2 == 1) {  // Conditional with constant RHS (1)
            break;
        }
    }
    return count;
}

/* Pattern 2: If-else chain sets 0/1, later conditional uses it */
int pattern2_if_else_merge(int a, int b) {
    int result;
    
    if (a > b) {
        result = 1;  // Assignment 1
    } else if (a < b) {
        result = 0;  // Assignment 2
    } else {
        result = 1;  // Assignment 3 - creates PHI at merge point
    }
    
    // Pass through copy chain
    int tmp1 = result;
    int tmp2 = pass_value(tmp1);
    int tmp3 = copy_twice(tmp2);
    
    if (tmp3 == 0) {  // Conditional with constant RHS (0)
        return a + b;
    } else {
        return a - b;
    }
}

/* Pattern 3: Nested control with ternary operator */
int pattern3_ternary_phi(int x, int y) {
    int flag = (x > y) ? 1 : 0;  // Creates PHI
    
    // Multiple copy operations
    int a = flag;
    int b = copy_once(a);
    int c = pass_value(b);
    
    if (c) {  // Implicit comparison with 0
        for (int i = 0; i < x; i++) {
            y += i;
            // Nested condition to create annotated BB
            if (y % 2 == 0) {
                y *= 2;
            }
        }
    }
    return y;
}

/* Pattern 4: Switch-case setting 0/1 value */
int pattern4_switch_phi(int op, int val) {
    int should_process;
    
    switch (op) {
        case 1: should_process = 1; break;
        case 2: should_process = 0; break;
        case 3: should_process = 1; break;
        default: should_process = 0; break;  // Creates PHI
    }
    
    // Long copy chain
    int v1 = should_process;
    int v2 = v1;
    int v3 = copy_once(v2);
    int v4 = pass_value(v3);
    int v5 = copy_twice(v4);
    
    if (v5 != 1) {  // Conditional with constant RHS (1)
        return val * 2;
    }
    
    // Complex loop to ensure BB annotation
    int sum = 0;
    for (int i = 0; i < val; i++) {
        if (i % 3 == 0) {
            sum += i * 2;
        } else if (i % 3 == 1) {
            sum += i;
        } else {
            sum += i / 2;
        }
    }
    return sum;
}

/* Pattern 5: Loop with multiple exit flags */
int pattern5_multi_exit(int limit) {
    int done = 0;  // Initial
    int count = 0;
    int total = 0;
    
    while (count < limit) {
        int inner_flag = 0;
        
        // Inner loop with its own PHI
        for (int i = 0; i < 10; i++) {
            if ((count + i) % 5 == 0) {
                inner_flag = 1;
            } else {
                inner_flag = 0;
            }
            
            // Use inner_flag in condition
            if (inner_flag == 0) {  // Constant RHS (0)
                total += i;
            }
        }
        
        count++;
        if (count >= limit / 2) {
            done = 1;
        } else {
            done = 0;
        }
        
        // Copy and check
        int done_copy = done;
        if (done_copy) {  // Implicit comparison with 0
            total += 1000;
        }
    }
    return total;
}

/* Pattern 6: Complex PHI network */
int pattern6_complex_phi(int a, int b, int c) {
    int flag1, flag2, final_flag;
    
    if (a > 0) {
        flag1 = 1;
    } else {
        flag1 = 0;
    }
    
    if (b > 0) {
        flag2 = 1;
    } else {
        flag2 = 0;
    }
    
    // PHI from multiple sources
    if (c > 0) {
        final_flag = flag1;
    } else {
        final_flag = flag2;
    }
    
    // Multiple copy operations
    int t1 = final_flag;
    int t2 = t1;
    int t3 = pass_value(t2);
    int t4 = copy_once(t3);
    int t5 = copy_twice(t4);
    
    if (t5 == 1) {  // Explicit comparison with 1
        return a + b + c;
    } else {
        return a * b * c;
    }
}

/* Main function with its own pattern */
int main() {
    int total = 0;
    int data[6] = {5, 10, 15, 20, 25, 30};
    
    // Main loop with flag-based exit (mimics target pattern)
    int main_flag = 0;
    int idx = 0;
    
    while (!main_flag) {  // Conditional on PHI-derived value
        int val = data[idx % 6];
        
        // Call each pattern function
        total += pattern1_loop_exit(val % 5 + 1);
        total += pattern2_if_else_merge(val, idx);
        total += pattern3_ternary_phi(val, idx);
        total += pattern4_switch_phi(val % 4, val);
        total += pattern5_multi_exit(val % 3 + 2);
        total += pattern6_complex_phi(val, idx, total % 10);
        
        idx++;
        
        // Set flag through PHI
        if (idx >= 100 || total > 100000) {
            main_flag = 1;
        } else {
            main_flag = 0;
        }
        
        // Copy chain in main
        int flag_copy = main_flag;
        if (flag_copy == 0) {  // Constant RHS (0)
            total += 1;
        }
    }
    
    printf("Result: %d\n", total);
    printf("Pattern executions completed.\n");
    
    // Additional runs with different inputs
    for (int i = 0; i < 10; i++) {
        pattern1_loop_exit(i + 1);
        pattern2_if_else_merge(i, i * 2);
        pattern3_ternary_phi(i * 3, i * 4);
    }
    
    return 0;
}
