#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Loop with PHI-derived exit condition */
int pattern1_loop_with_phi_flag(int iterations) {
    int flag = 0;  // Initial value
    int count = 0;
    
    while (!flag) {  // Conditional on PHI-derived value (flag != 0)
        count++;
        if (count >= iterations) {
            flag = 1;  // Sets flag in loop body
        }
        
        // Create SSA copy chain
        int temp1 = flag;
        int temp2 = temp1;
        if (temp2 == 1) {  // Conditional with RHS = 1
            // Additional nested control flow
            for (int i = 0; i < 3; i++) {
                if (i % 2 == 0) {
                    count += i;
                }
            }
        }
    }
    return count;
}

/* Pattern 2: If-else chain assigning 0/1 to variable */
int pattern2_if_else_phi(int a, int b) {
    int result;
    
    // PHI node at merge point
    if (a > b) {
        result = 1;
    } else if (a < b) {
        result = 0;
    } else {
        result = 1;  // Equal case
    }
    
    // Propagate through SSA copies
    int x = result;
    int y = x;
    int z = y;
    
    // Conditional on the PHI-derived value
    if (z == 0) {  // RHS = 0
        return a * 2;
    } else {
        return b * 3;
    }
}

/* Pattern 3: Ternary operator creating PHI */
int pattern3_ternary_phi(int value) {
    // Ternary creates PHI node
    int is_even = (value % 2 == 0) ? 1 : 0;
    
    // Multiple SSA copies
    int copy1 = is_even;
    int copy2 = copy1;
    
    // Nested control flow with conditional
    int sum = 0;
    for (int i = 0; i < value; i++) {
        if (copy2) {  // Implicit comparison with 0
            sum += i;
        } else {
            sum -= i;
        }
        
        // Modify copy2 to create more complex flow
        if (i % 5 == 0) {
            int temp = copy2;
            copy2 = temp;
        }
    }
    
    // Final conditional with explicit comparison
    if (copy2 == 1) {  // RHS = 1
        return sum * 2;
    }
    return sum;
}

/* Pattern 4: Switch-case setting 0/1 */
int pattern4_switch_phi(char op) {
    int flag;
    
    switch (op) {
        case '+':
        case '-':
            flag = 1;
            break;
        case '*':
        case '/':
            flag = 0;
            break;
        default:
            flag = 1;
    }
    
    // Chain of assignments
    int a = flag;
    int b = a;
    
    // Conditional in loop
    int result = 0;
    for (int i = 0; i < 10; i++) {
        if (b != 0) {  // Comparison with 0
            result += i * 2;
        } else {
            result += i;
        }
        
        // Nested if to create more basic blocks
        if (i % 3 == 0) {
            int c = b;
            if (c == 1) {  // Another conditional
                result += 100;
            }
        }
    }
    
    return result;
}

/* Pattern 5: Complex nested loops with PHI */
int pattern5_nested_loops(int n) {
    int continue_outer = 1;
    int total = 0;
    
    for (int i = 0; i < n && continue_outer; i++) {
        int inner_flag = 0;
        
        for (int j = 0; j < n; j++) {
            // PHI node for inner_flag
            if (j % 2 == 0) {
                inner_flag = 1;
            } else {
                inner_flag = 0;
            }
            
            // Copy and use in conditional
            int f1 = inner_flag;
            int f2 = f1;
            
            if (f2 == 0) {  // RHS = 0
                total += i + j;
            } else {
                total += i * j;
            }
            
            // Break condition based on flag
            if (f2) {  // Implicit comparison
                if (total > 1000) {
                    continue_outer = 0;
                    break;
                }
            }
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

/* Pattern 6: Using function calls in copy chain */
int pattern6_function_phi(int x, int y) {
    int cmp_result;
    
    // PHI from if-else
    if (x > y) {
        cmp_result = 1;
    } else {
        cmp_result = 0;
    }
    
    // Propagate through function call (inlined)
    int propagated = propagate_value(cmp_result);
    
    // Multiple conditionals
    int sum = 0;
    for (int i = 0; i < 5; i++) {
        if (propagated == 1) {  // RHS = 1
            sum += x + i;
        } else {
            sum += y + i;
        }
        
        // Nested conditional
        if (i % 2 == 0) {
            int temp = propagated;
            if (temp != 0) {  // Comparison with 0
                sum += 10;
            }
        }
    }
    
    return sum;
}

/* Main function with varied control flow */
int main() {
    int total = 0;
    
    // Array to control branch directions
    int inputs[] = {5, 10, 3, 7, 2, 8};
    int num_inputs = sizeof(inputs) / sizeof(inputs[0]);
    
    // Main loop with PHI-derived condition
    int continue_main = 1;
    int idx = 0;
    
    while (continue_main) {  // Conditional on PHI-derived value
        int val = inputs[idx % num_inputs];
        
        // Call each pattern function
        total += pattern1_loop_with_phi_flag(val);
        total += pattern2_if_else_phi(val, val * 2);
        total += pattern3_ternary_phi(val);
        total += pattern4_switch_phi(val % 2 ? '+' : '*');
        total += pattern5_nested_loops(val % 3 + 2);
        total += pattern6_function_phi(val, val + 1);
        
        idx++;
        
        // PHI-based exit condition
        if (idx >= 10 || total > 10000) {
            continue_main = 0;  // Sets the PHI variable
        }
        
        // Copy and check
        int check = continue_main;
        if (check == 0) {  // RHS = 0
            total += 1000;  // Bonus for exiting
        }
    }
    
    // Final conditional with PHI-derived value
    int final_flag = (total % 2 == 0) ? 1 : 0;
    int f1 = final_flag;
    int f2 = f1;
    
    if (f2) {  // Implicit comparison with 0
        total *= 2;
    } else {
        total += 500;
    }
    
    printf("Final result: %d\n", total);
    
    // Verify with simple calculation
    int expected_min = 1000;  // Minimum expected from our patterns
    if (total < expected_min) {
        printf("Warning: Result seems low\n");
    } else {
        printf("Result validated\n");
    }
    
    return 0;
}
