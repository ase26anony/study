#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Loop with PHI-derived exit condition */
int pattern1_loop_phi_exit(int iterations) {
    int flag = 0;  // Initial value
    int count = 0;
    
    while (!flag) {  // Conditional on PHI-derived value (flag)
        count++;
        if (count >= iterations) {
            flag = 1;  // Sets flag in loop body
        }
        // Create SSA copy chain
        int temp1 = flag;
        int temp2 = temp1;
        if (temp2 == 1) {  // Conditional with copy chain
            break;
        }
    }
    return count;
}

/* Pattern 2: If-else chain with PHI merge */
int pattern2_ifelse_phi(int a, int b) {
    int result;
    
    // Creates PHI node at merge point
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
    if (z == 1) {  // Compares against constant 1
        return a * 2;
    } else {
        return b * 2;
    }
}

/* Helper function to create copy chain */
static inline int pass_through(int v) {
    int t1 = v;
    int t2 = t1;
    return t2;  // Creates SSA copy chain
}

/* Pattern 3: Nested control with function call */
int pattern3_nested_with_call(int n) {
    int sum = 0;
    int done = 0;  // PHI candidate
    
    for (int i = 0; i < n; i++) {
        if (i % 3 == 0) {
            done = 1;
        } else if (i % 5 == 0) {
            done = 0;
        } else {
            done = (i % 2);
        }
        
        // Use function to create copy chain
        int check = pass_through(done);
        
        // Nested conditional
        if (check != 0) {  // Implicit comparison with 0
            sum += i;
        } else {
            sum -= i;
        }
        
        // Another conditional in same BB
        if (i > n/2) {
            int flag_copy = done;
            if (flag_copy == 1) {  // Explicit comparison with 1
                sum *= 2;
            }
        }
    }
    return sum;
}

/* Pattern 4: Switch-case setting 0/1 */
int pattern4_switch_phi(int val) {
    int indicator;
    
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
        default:
            indicator = 0;
            break;
    }
    
    // Multiple copy operations
    int a = indicator;
    int b = a;
    int c = b;
    
    // Conditional with copy chain
    if (c) {  // Implicit c != 0
        return val * 10;
    }
    return val * 5;
}

/* Pattern 5: Ternary operator creating PHI */
int pattern5_ternary_phi(int x, int y) {
    // Ternary creates PHI
    int is_greater = (x > y) ? 1 : 0;
    
    // Chain of assignments
    int v1 = is_greater;
    int v2 = v1;
    
    // Conditional on the end of chain
    if (v2 == 0) {  // Compare against 0
        return y - x;
    } else {
        return x - y;
    }
}

/* Pattern 6: Complex loop with multiple PHIs */
int pattern6_complex_loop(int limit) {
    int state = 0;
    int total = 0;
    int i = 0;
    
    while (i < limit) {
        // Multiple assignments create PHI web
        int old_state = state;
        
        if (i % 7 == 0) {
            state = 1;
        } else if (i % 3 == 0) {
            state = 0;
        }
        
        // Copy through temporary
        int check_state = state;
        int verify = check_state;
        
        // Conditional in annotated block
        if (verify == 1) {
            total += i * 2;
        } else if (old_state == 0) {
            total += i;
        }
        
        // Another conditional in same block
        if (i > limit / 2) {
            int temp = state;
            if (temp != 1) {  // Compare against 1
                total -= 5;
            }
        }
        
        i++;
    }
    return total;
}

/* Main function with its own pattern */
int main() {
    int total_result = 0;
    int loop_flag = 0;  // For main's own PHI pattern
    
    // Initialize test data
    int test_values[] = {5, 10, 15, 20, 25};
    int num_tests = sizeof(test_values) / sizeof(test_values[0]);
    
    for (int i = 0; i < num_tests; i++) {
        // Set flag based on condition (creates PHI)
        if (i % 2 == 0) {
            loop_flag = 1;
        } else {
            loop_flag = 0;
        }
        
        // Copy chain in main
        int flag_copy = loop_flag;
        int flag_check = flag_copy;
        
        // Conditional on PHI-derived value in main
        if (flag_check == 1) {
            // Call pattern functions with different inputs
            total_result += pattern1_loop_phi_exit(test_values[i]);
            total_result += pattern2_ifelse_phi(test_values[i], test_values[i] / 2);
        } else {
            total_result += pattern3_nested_with_call(test_values[i]);
            total_result += pattern4_switch_phi(test_values[i]);
        }
        
        // Always execute these
        total_result += pattern5_ternary_phi(test_values[i], i);
        total_result += pattern6_complex_loop(test_values[i] % 10 + 5);
    }
    
    // Final conditional with copy chain
    int final_check = total_result;
    int verify_result = final_check;
    
    if (verify_result > 1000) {  // Compare against constant
        printf("Large result: %d\n", total_result);
    } else {
        printf("Result: %d\n", total_result);
    }
    
    return 0;
}
