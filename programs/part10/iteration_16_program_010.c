#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Loop exit condition depends on PHI-assigned flag */
int pattern1_loop_flag(int iterations) {
    int flag = 0;  // Initial value
    int count = 0;
    
    while (!flag) {  // Conditional branch on PHI-derived value
        count++;
        if (count >= iterations) {
            flag = 1;  // Sets flag in loop body
        }
        // Create SSA copy chain
        int temp1 = flag;
        int temp2 = temp1;
        if (temp2 == 0) {  // Branch with constant RHS 0
            // Do some work
            count += 1;
        }
    }
    return count;
}

/* Pattern 2: If-else chain assigning boolean, then conditional */
int pattern2_if_else_merge(int a, int b) {
    int result;
    
    // PHI node will be created at merge point
    if (a > b) {
        result = 1;  // One arm sets to 1
    } else {
        result = 0;  // Other arm sets to 0
    }
    
    // SSA copy chain
    int copy1 = result;
    int copy2 = copy1;
    int copy3 = copy2;
    
    // Conditional branch on PHI-derived value
    if (copy3 == 1) {  // Branch with constant RHS 1
        return a * 2;
    } else {
        return b * 2;
    }
}

/* Pattern 3: Nested control structures with ternary operator */
int pattern3_ternary_nested(int x, int y, int z) {
    int flag;
    
    // Outer loop
    for (int i = 0; i < x; i++) {
        // Inner if-else
        if (y > z) {
            // Ternary creates PHI
            flag = (i % 2 == 0) ? 1 : 0;
        } else {
            flag = (i % 3 == 0) ? 1 : 0;
        }
        
        // SSA copies through function-like macro
        #define PASS(v) (v)
        int passed = PASS(flag);
        
        // Conditional in nested context
        if (passed != 0) {  // Implicit: if (passed) with RHS 0
            z += i;
        }
        
        // Another conditional with explicit comparison
        int another_copy = passed;
        if (another_copy == 0) {  // Branch with constant RHS 0
            y += 1;
        }
    }
    return y + z;
}

/* Pattern 4: Switch-case setting 0/1 value */
int pattern4_switch_phi(int val) {
    int indicator = 0;
    
    switch (val % 4) {
        case 0:
            indicator = 1;
            break;
        case 1:
            indicator = 0;
            break;
        case 2:
            indicator = 1;
            break;
        case 3:
            indicator = 0;
            break;
    }
    
    // Multiple SSA copies
    int chain1 = indicator;
    int chain2 = chain1;
    
    // Complex nested structure
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            int inner_copy = chain2;
            if (inner_copy) {  // if (inner_copy != 0)
                val += i;
            }
        } else {
            int another_copy = chain2;
            if (another_copy == 1) {  // Branch with constant RHS 1
                val -= i;
            }
        }
    }
    
    return val;
}

/* Pattern 5: Multiple PHIs in loop header */
int pattern5_multiple_phis(int n) {
    int sum = 0;
    int continue_flag = 1;
    int odd_flag = 0;
    
    // Loop with multiple PHI nodes
    for (int i = 0; continue_flag && i < n; i++) {
        // Update flags in loop body
        odd_flag = (i % 2 == 1) ? 1 : 0;
        
        if (i > n / 2) {
            continue_flag = 0;
        }
        
        // Use flags in conditionals
        int temp_flag = odd_flag;
        if (temp_flag == 1) {  // Branch with constant RHS 1
            sum += i * 3;
        } else {
            sum += i;
        }
        
        // Another conditional chain
        int another_temp = continue_flag;
        if (another_temp != 0) {  // Branch with constant RHS 0
            sum += 1;
        }
    }
    
    return sum;
}

/* Helper function to create SSA copy chain */
static inline int propagate_value(int v) {
    int local = v;
    return local;
}

/* Pattern 6: Using inline function for SSA copies */
int pattern6_function_propagation(int a, int b) {
    int decision;
    
    // PHI at merge point
    if (a > 100) {
        decision = 1;
    } else if (b > 100) {
        decision = 0;
    } else {
        decision = (a > b) ? 1 : 0;
    }
    
    // Multiple propagations
    int p1 = propagate_value(decision);
    int p2 = propagate_value(p1);
    int p3 = propagate_value(p2);
    
    // Nested loops with conditional
    int result = 0;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            int current = p3;
            if (current == 0) {  // Branch with constant RHS 0
                result += i + j;
            } else {
                result += i * j;
            }
        }
    }
    
    return result;
}

/* Main function with varied control flow */
int main() {
    int total = 0;
    
    // Initialize with different values to exercise various paths
    int test_values[] = {5, 10, 15, 20, 25};
    int num_tests = sizeof(test_values) / sizeof(test_values[0]);
    
    // Main loop with flag-based condition (mimics target pattern)
    int process_flag = 1;
    int idx = 0;
    
    while (process_flag) {  // Conditional on PHI-derived value
        int val = test_values[idx % num_tests];
        
        // Call each pattern function
        total += pattern1_loop_flag(val % 5 + 1);
        total += pattern2_if_else_merge(val, val * 2);
        total += pattern3_ternary_nested(val % 3, val % 4, val % 5);
        total += pattern4_switch_phi(val);
        total += pattern5_multiple_phis(val % 10 + 1);
        total += pattern6_function_propagation(val, val + 10);
        
        idx++;
        
        // Update flag - creates PHI for loop condition
        if (idx >= 10) {
            process_flag = 0;  // Will be used in next iteration's PHI
        }
        
        // SSA copy chain for the flag
        int flag_copy = process_flag;
        if (flag_copy == 1) {  // Branch with constant RHS 1
            total += 100;  // Bonus for continuing
        }
    }
    
    // Additional complex control flow
    for (int i = 0; i < 5; i++) {
        int inner_flag = (i % 2 == 0) ? 1 : 0;
        int chain1 = inner_flag;
        int chain2 = chain1;
        
        if (chain2) {  // if (chain2 != 0)
            total += pattern1_loop_flag(i + 1);
        }
        
        if (chain2 == 0) {  // Branch with constant RHS 0
            total += pattern2_if_else_merge(i, i * 2);
        }
    }
    
    printf("Total result: %d\n", total);
    printf("All patterns executed with varied control flow\n");
    
    return 0;
}
