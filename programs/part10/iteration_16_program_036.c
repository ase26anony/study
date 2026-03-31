#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Loop exit condition depends on PHI-assigned flag */
int pattern1_loop_exit_flag(int iterations) {
    int flag = 0;
    int count = 0;
    
    while (!flag) {  // This creates a PHI for 'flag' at loop header
        count++;
        if (count >= iterations) {
            flag = 1;  // Sets flag inside loop - creates PHI
        }
    }
    
    // Temporary variable chain
    int a = flag;
    int b = a;
    int c = b;
    
    // Conditional on PHI-derived value with constant RHS
    if (c == 1) {  // Should trigger: cmp against constant 1
        return count * 2;
    }
    return count;
}

/* Pattern 2: If-else chain setting boolean, used later */
int pattern2_if_else_merge(int x, int y) {
    int result;
    
    // First conditional sets a value
    if (x > 0) {
        result = 1;
    } else {
        result = 0;
    }
    
    // Copy chain
    int tmp1 = result;
    int tmp2 = tmp1;
    
    // Nested if with the PHI-derived value
    if (y > 0) {
        // Another copy in nested scope
        int tmp3 = tmp2;
        if (tmp3 != 0) {  // cmp against constant 0
            return x + y;
        }
    }
    
    return x - y;
}

/* Pattern 3: Switch-case setting 0/1 value */
int pattern3_switch_to_bool(char op) {
    int flag;
    
    switch (op) {
        case 'A':
        case 'B':
            flag = 1;
            break;
        case 'C':
            flag = 0;
            break;
        default:
            flag = 1;
    }
    
    // Multiple copy chain
    int a = flag;
    int b = a;
    
    // Conditional with implicit boolean check (cmp against 0)
    if (b) {  // Equivalent to if (b != 0)
        return 100;
    }
    return 200;
}

/* Pattern 4: Ternary operator setting 0/1 */
int pattern4_ternary_phi(int a, int b) {
    // Ternary creates PHI
    int is_equal = (a == b) ? 1 : 0;
    
    // Function call to create copy through return
    int check1 = is_equal;
    
    // Complex conditional structure
    if (check1 == 0) {
        return a * b;
    } else {
        // Nested condition
        if (a > 10) {
            int check2 = check1;
            if (check2 == 1) {  // cmp against constant 1
                return a + b;
            }
        }
        return a - b;
    }
}

/* Pattern 5: Loop with multiple exit conditions */
int pattern5_complex_loop(int limit) {
    int done = 0;
    int counter = 0;
    int total = 0;
    
    while (!done) {
        counter++;
        
        // Multiple conditions that could set done
        if (counter >= limit) {
            done = 1;
        } else if (total > 1000) {
            done = 1;
        } else {
            total += counter;
        }
        
        // Intermediate computation
        if (counter % 2 == 0) {
            total += 5;
        }
    }
    
    // Copy chain from PHI
    int status = done;
    int verify = status;
    
    // Final conditional
    if (verify == 1) {
        return total * 2;
    }
    return total;
}

/* Pattern 6: Nested loops with flag propagation */
int pattern6_nested_loops(int n) {
    int outer_flag = 0;
    int sum = 0;
    
    for (int i = 0; i < n && !outer_flag; i++) {
        int inner_flag = 0;
        
        for (int j = 0; j < n && !inner_flag; j++) {
            sum += i * j;
            
            if (j > i * 2) {
                inner_flag = 1;  // Creates PHI in inner loop
            }
        }
        
        // Copy inner flag
        int check_inner = inner_flag;
        
        // Conditional on copied PHI value
        if (check_inner == 0) {  // cmp against constant 0
            sum += 100;
        }
        
        if (i > n / 2) {
            outer_flag = 1;  // Creates PHI in outer loop
        }
    }
    
    // Final check on outer flag
    int final_check = outer_flag;
    if (final_check) {  // implicit cmp against 0
        return sum + 1000;
    }
    return sum;
}

/* Helper function to create copy chain through function call */
static inline int propagate_value(int v) {
    // Simple function that returns its argument
    // Creates SSA copy through function boundary
    return v;
}

/* Pattern 7: Using function calls in copy chain */
int pattern7_function_copy(int x) {
    int condition = (x % 3 == 0) ? 1 : 0;
    
    // Multiple function calls create copy chain
    int v1 = propagate_value(condition);
    int v2 = propagate_value(v1);
    int v3 = propagate_value(v2);
    
    // Complex control flow
    if (x > 0) {
        if (v3 == 1) {  // cmp against constant 1
            return x * 10;
        }
    } else {
        if (v3 == 0) {  // cmp against constant 0
            return x * -10;
        }
    }
    
    return x;
}

int main() {
    int total = 0;
    
    // Initialize with different values to exercise various paths
    int test_values[] = {5, 10, 15, 20, 25};
    int num_tests = sizeof(test_values) / sizeof(test_values[0]);
    
    // Main loop with flag-based exit condition (mimics pattern at top level)
    int main_flag = 0;
    int main_counter = 0;
    
    while (!main_flag) {
        // Call each pattern function with different inputs
        for (int i = 0; i < num_tests; i++) {
            total += pattern1_loop_exit_flag(test_values[i]);
            total += pattern2_if_else_merge(test_values[i], test_values[(i + 1) % num_tests]);
            total += pattern3_switch_to_bool('A' + (i % 4));
            total += pattern4_ternary_phi(test_values[i], test_values[num_tests - i - 1]);
            total += pattern5_complex_loop(test_values[i]);
            total += pattern6_nested_loops(test_values[i] % 5 + 2);
            total += pattern7_function_copy(test_values[i]);
        }
        
        main_counter++;
        if (main_counter >= 3) {
            main_flag = 1;  // Creates PHI for main_flag
        }
    }
    
    // Final conditional on main_flag (PHI-derived)
    int final_check = main_flag;
    if (final_check == 1) {  // cmp against constant 1
        total += 10000;
    }
    
    printf("Result: %d\n", total);
    
    // Additional verification runs
    printf("Pattern1: %d\n", pattern1_loop_exit_flag(3));
    printf("Pattern2: %d\n", pattern2_if_else_merge(5, -3));
    printf("Pattern3: %d\n", pattern3_switch_to_bool('C'));
    printf("Pattern4: %d\n", pattern4_ternary_phi(10, 10));
    printf("Pattern5: %d\n", pattern5_complex_loop(7));
    printf("Pattern6: %d\n", pattern6_nested_loops(4));
    printf("Pattern7: %d\n", pattern7_function_copy(9));
    
    return 0;
}
