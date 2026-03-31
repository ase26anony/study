#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Loop with PHI-derived exit condition */
int pattern1_loop_with_phi_flag(int iterations) {
    int flag = 0;  // This becomes a PHI at loop header
    int count = 0;
    int i = 0;
    
    while (!flag) {  // Conditional on PHI-derived value
        count++;
        i++;
        if (i >= iterations) {
            flag = 1;  // Sets flag inside loop
        }
    }
    return count;
}

/* Pattern 2: If-else chain setting boolean, then conditional */
int pattern2_if_else_phi(int a, int b) {
    int result;
    
    // PHI node created at merge point
    if (a > b) {
        result = 1;
    } else {
        result = 0;
    }
    
    // SSA copy chain
    int temp1 = result;
    int temp2 = temp1;
    
    // Conditional on copy of PHI result
    if (temp2 == 1) {
        return a + b;
    } else {
        return a - b;
    }
}

/* Pattern 3: Nested control with SSA copies */
int pattern3_nested_with_copies(int n) {
    int sum = 0;
    int continue_flag = 1;  // Initial value
    
    for (int i = 0; i < n; i++) {
        int inner_flag = (i % 2 == 0) ? 1 : 0;  // Ternary creates PHI
        
        // Multiple SSA copies
        int copy1 = inner_flag;
        int copy2 = copy1;
        int copy3 = copy2;
        
        // Conditional on copy chain
        if (copy3 != 0) {
            sum += i * 2;
        } else {
            sum += i;
        }
        
        // Update outer flag based on condition
        if (sum > 100) {
            continue_flag = 0;
        }
    }
    
    // Outer conditional on PHI-derived flag
    int final_copy = continue_flag;
    if (final_copy == 0) {
        return sum - 50;
    }
    return sum;
}

/* Pattern 4: Switch-case setting 0/1 value */
int pattern4_switch_phi(int val) {
    int indicator = 0;
    
    switch (val % 3) {
        case 0:
            indicator = 1;
            break;
        case 1:
            indicator = 0;
            break;
        case 2:
            indicator = 1;
            break;
    }
    
    // Chain of assignments
    int a = indicator;
    int b = a;
    
    // Conditional on the chain
    if (b) {  // Implicit comparison to 0
        return val * 2;
    }
    return val / 2;
}

/* Pattern 5: Complex loop with multiple PHI nodes */
int pattern5_complex_loop(int limit) {
    int active = 1;
    int total = 0;
    int i = 0;
    
    while (active) {  // Conditional on PHI
        int should_continue;
        
        if (i % 3 == 0) {
            should_continue = 1;
        } else if (i % 3 == 1) {
            should_continue = 0;
        } else {
            should_continue = 1;
        }
        
        // Propagate through temporary
        int check = should_continue;
        
        if (check == 0) {
            total += i;
        } else {
            total += i * i;
        }
        
        i++;
        if (i >= limit) {
            active = 0;  // Update PHI operand
        }
        
        // Another conditional inside loop
        int temp = active;
        if (temp == 1) {
            total += 10;
        }
    }
    
    return total;
}

/* Helper function to create SSA copy chain */
static inline int propagate_value(int v) {
    int x = v;
    int y = x;
    return y;
}

/* Pattern 6: Using function call for SSA chain */
int pattern6_function_chain(int a, int b) {
    int decision;
    
    // PHI at merge point
    if (a == b) {
        decision = 1;
    } else {
        decision = 0;
    }
    
    // Pass through function (inlined)
    int propagated = propagate_value(decision);
    
    // Conditional on propagated value
    if (propagated != 1) {
        return a * b;
    }
    return a + b;
}

/* Pattern 7: Multiple basic blocks with annotations */
int pattern7_multi_bb(int n) {
    int state = 0;
    int result = 0;
    
    for (int i = 0; i < n; i++) {
        // Multiple conditionals creating rich CFG
        if (i % 2 == 0) {
            if (i % 3 == 0) {
                state = 1;
            } else {
                state = 0;
            }
        } else {
            state = (i % 5 == 0) ? 1 : 0;
        }
        
        // Copy and conditional
        int current = state;
        if (current == 1) {
            result += i * 3;
        } else {
            result += i;
        }
        
        // Another conditional to create more edges
        int check = (result > 100) ? 1 : 0;
        if (check) {
            result -= 50;
        }
    }
    
    return result;
}

int main() {
    int total = 0;
    
    // Initialize with different values to exercise various paths
    int test_values[] = {5, 10, 15, 20, 25};
    int num_tests = sizeof(test_values) / sizeof(test_values[0]);
    
    // Pattern 1
    for (int i = 0; i < num_tests; i++) {
        total += pattern1_loop_with_phi_flag(test_values[i]);
    }
    
    // Pattern 2
    for (int i = 0; i < num_tests; i++) {
        total += pattern2_if_else_phi(test_values[i], test_values[num_tests - i - 1]);
    }
    
    // Pattern 3
    for (int i = 0; i < num_tests; i++) {
        total += pattern3_nested_with_copies(test_values[i]);
    }
    
    // Pattern 4
    for (int i = 0; i < num_tests; i++) {
        total += pattern4_switch_phi(test_values[i]);
    }
    
    // Pattern 5
    for (int i = 0; i < num_tests; i++) {
        total += pattern5_complex_loop(test_values[i]);
    }
    
    // Pattern 6
    for (int i = 0; i < num_tests; i++) {
        total += pattern6_function_chain(test_values[i], i);
    }
    
    // Pattern 7
    for (int i = 0; i < num_tests; i++) {
        total += pattern7_multi_bb(test_values[i]);
    }
    
    // Main loop with PHI-derived flag (mimics target pattern)
    int main_flag = 0;
    int main_counter = 0;
    int main_total = 0;
    
    while (main_flag == 0) {  // Direct comparison to 0
        main_total += test_values[main_counter % num_tests];
        main_counter++;
        
        // Create SSA copy chain
        int flag_copy = main_flag;
        int flag_copy2 = flag_copy;
        
        if (main_counter >= 10) {
            main_flag = 1;  // Update PHI operand
        }
        
        // Another conditional using copy
        if (flag_copy2 != 1) {
            main_total += 5;
        }
    }
    
    total += main_total;
    
    printf("Final result: %d\n", total);
    printf("Test completed successfully\n");
    
    return 0;
}
