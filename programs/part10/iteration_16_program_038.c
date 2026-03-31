#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Loop exit condition depends on PHI-assigned flag */
int pattern1_loop_exit_flag(int iterations) {
    int flag = 0;  // Initial value
    int count = 0;
    
    while (!flag) {  // Conditional on PHI-derived value (implicit != 0)
        count++;
        if (count >= iterations) {
            flag = 1;  // Sets flag in loop body
        }
        // Create SSA copy chain
        int temp1 = flag;
        int temp2 = temp1;
        if (temp2 == 0) {  // Explicit comparison with 0
            // Do some work
            count = count * 2 - count;  // Identity operation
        }
    }
    return count;
}

/* Pattern 2: If-else chain sets 0/1, later conditional uses it */
int pattern2_if_else_merge(int a, int b) {
    int result;
    
    // PHI node at control flow merge
    if (a > b) {
        result = 1;
    } else {
        result = 0;
    }
    
    // SSA copy chain
    int x = result;
    int y = x;
    int z = y;
    
    // Conditional on PHI-derived value
    if (z == 1) {  // Explicit comparison with 1
        return a + b;
    } else {
        return a - b;
    }
}

/* Pattern 3: Ternary operator sets 0/1, used in nested loop */
int pattern3_ternary_nested(int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        // Ternary creates PHI-like behavior
        int is_even = (i % 2 == 0) ? 1 : 0;
        
        // SSA copies through function-like inline
        int copy1 = is_even;
        int copy2 = copy1;
        
        // Nested if with comparison to 0
        if (copy2 != 0) {  // Explicit comparison with 0
            for (int j = 0; j < i; j++) {
                if (j % 3 == 0) {
                    sum += j;
                }
            }
        }
        
        // Another conditional in same basic block
        if (is_even) {  // Implicit comparison with 0
            sum -= i;
        }
    }
    return sum;
}

/* Pattern 4: Switch-case sets 0/1, used in conditional */
int pattern4_switch_case(int val) {
    int flag;
    
    switch (val % 4) {
        case 0: flag = 1; break;
        case 1: flag = 0; break;
        case 2: flag = 1; break;
        case 3: flag = 0; break;
        default: flag = 0; break;
    }
    
    // Multiple SSA copies
    int a = flag;
    int b = a;
    int c = b;
    int d = c;
    
    // Conditional with explicit 1 comparison
    if (d == 1) {
        return val * 2;
    }
    return val / 2;
}

/* Pattern 5: Complex PHI in loop header with multiple predecessors */
int pattern5_complex_phi(int limit) {
    int state = 0;
    int total = 0;
    int i = 0;
    
    while (i < limit) {
        // PHI node for 'state' at loop header
        int current_state = state;
        
        // SSA copy chain
        int s1 = current_state;
        int s2 = s1;
        
        // Conditional on copied value
        if (s2 == 0) {  // Compare with 0
            total += i;
            state = 1;  // Change state
        } else {
            total -= i;
            state = 0;  // Change state
        }
        
        // Another conditional in same block
        if (current_state) {  // Implicit comparison
            total += 100;
        }
        
        i++;
    }
    return total;
}

/* Helper function to create SSA copy chain */
static inline int propagate_value(int v) {
    int t1 = v;
    int t2 = t1;
    return t2;
}

/* Pattern 6: Using inline function for SSA copies */
int pattern6_inline_propagation(int a, int b) {
    int cmp_result = (a == b) ? 1 : 0;
    
    // Pass through inline function
    int propagated = propagate_value(cmp_result);
    
    // Multiple conditionals
    if (propagated == 1) {
        return a * b;
    }
    
    if (!propagated) {  // Implicit comparison with 0
        return a + b;
    }
    
    return 0;
}

/* Pattern 7: Nested control flow with multiple PHIs */
int pattern7_nested_control_flow(int x, int y, int z) {
    int flag1, flag2;
    
    if (x > 0) {
        if (y > 0) {
            flag1 = 1;
        } else {
            flag1 = 0;
        }
        flag2 = 1;
    } else {
        if (z > 0) {
            flag1 = 1;
        } else {
            flag1 = 0;
        }
        flag2 = 0;
    }
    
    // SSA copies of both flags
    int f1_copy = flag1;
    int f2_copy = flag2;
    
    // Conditional on first copied value
    if (f1_copy == 0) {
        x = -x;
    }
    
    // Conditional on second copied value  
    if (f2_copy != 1) {  // Compare with 1
        y = -y;
    }
    
    return x + y + z;
}

int main() {
    int total = 0;
    
    // Initialize with different values to exercise various paths
    int test_values[] = {5, 10, 15, 20, 25};
    int num_tests = sizeof(test_values) / sizeof(test_values[0]);
    
    // Main loop with flag-based control (mimics target pattern)
    int continue_processing = 1;
    int processed_count = 0;
    
    while (continue_processing) {  // Conditional on PHI-derived value
        for (int i = 0; i < num_tests; i++) {
            int val = test_values[i];
            
            // Call each pattern function
            total += pattern1_loop_exit_flag(val % 3 + 2);
            total += pattern2_if_else_merge(val, val * 2);
            total += pattern3_ternary_nested(val % 5 + 3);
            total += pattern4_switch_case(val);
            total += pattern5_complex_phi(val % 4 + 2);
            total += pattern6_inline_propagation(val, val + 1);
            total += pattern7_nested_control_flow(val, val - 1, val + 2);
            
            processed_count++;
            
            // SSA copy chain in main's loop
            int temp_flag = continue_processing;
            int flag_copy = temp_flag;
            
            // Conditional on copied flag value
            if (flag_copy == 1) {  // Explicit comparison with 1
                if (processed_count >= 3) {
                    continue_processing = 0;  // Will cause loop exit
                }
            }
        }
    }
    
    // Final conditional on result
    int result_copy = total;
    if (result_copy != 0) {  // Compare with 0
        printf("Result: %d\n", total);
    } else {
        printf("Zero result\n");
    }
    
    // Additional test with array processing
    int data[10];
    for (int i = 0; i < 10; i++) {
        data[i] = i;
    }
    
    int even_count = 0;
    for (int i = 0; i < 10; i++) {
        int is_even = (data[i] % 2 == 0) ? 1 : 0;
        int check = is_even;
        if (check) {  // Implicit comparison
            even_count++;
        }
    }
    
    printf("Even count: %d\n", even_count);
    
    return total != 0 ? 0 : 1;
}
