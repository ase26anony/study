#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Loop exit condition depends on PHI-assigned flag */
int pattern1_loop_with_phi_flag(int iterations) {
    int flag = 0;  // Initial value
    int count = 0;
    int i = 0;
    
    while (!flag) {  // Branch on PHI-derived value (flag)
        count++;
        i++;
        
        // This creates a PHI node for 'flag' at loop header
        if (i >= iterations) {
            flag = 1;  // Set to 1
        } else {
            flag = 0;  // Set to 0
        }
        
        // Copy chain: flag -> tmp1 -> tmp2
        int tmp1 = flag;
        int tmp2 = tmp1;
        
        // Branch comparing against constant 0
        if (tmp2 == 0) {
            // Continue loop
        }
    }
    return count;
}

/* Pattern 2: If-else chain setting 0/1, then conditional branch */
int pattern2_if_else_phi(int a, int b) {
    int result;
    
    // PHI node will be created here
    if (a > b) {
        result = 1;
    } else {
        result = 0;
    }
    
    // SSA copy chain
    int x = result;
    int y = x;
    int z = y;
    
    // Branch comparing against constant 1
    if (z == 1) {
        return a + b;
    } else {
        return a - b;
    }
}

/* Pattern 3: Nested control with ternary operator */
int pattern3_ternary_phi(int value) {
    // Ternary creates PHI-like assignment
    int is_even = (value % 2 == 0) ? 1 : 0;
    
    // Multiple copy assignments
    int copy1 = is_even;
    int copy2 = copy1;
    
    // Branch with implicit boolean check (compares against 0)
    if (copy2) {  // Equivalent to if (copy2 != 0)
        return value * 2;
    }
    
    // Another explicit comparison
    int copy3 = copy2;
    if (copy3 == 0) {
        return value / 2;
    }
    
    return value;
}

/* Pattern 4: Switch-case setting 0/1 */
int pattern4_switch_phi(char op) {
    int should_add;
    
    switch (op) {
        case '+':
        case '-':
            should_add = 1;  // Set to 1
            break;
        case '*':
        case '/':
            should_add = 0;  // Set to 0
            break;
        default:
            should_add = 0;
    }
    
    // Chain of assignments
    int a = should_add;
    int b = a;
    
    // Branch comparing against 1
    if (b != 1) {
        return 0;
    }
    return 1;
}

/* Pattern 5: Complex loop with multiple PHI nodes */
int pattern5_complex_phi_chain(int n) {
    int sum = 0;
    int continue_loop = 1;  // Initial value
    
    for (int i = 0; continue_loop; i++) {
        // Multiple conditions affecting the flag
        int condition1 = (i < n) ? 1 : 0;
        int condition2 = (i % 10 != 0) ? 1 : 0;
        
        // PHI for continue_loop
        if (condition1 && condition2) {
            continue_loop = 1;
            sum += i;
        } else {
            continue_loop = 0;
        }
        
        // Copy through temporary
        int tmp = continue_loop;
        int tmp2 = tmp;
        
        // Branch on the copied value
        if (tmp2 == 1) {
            // Continue
        }
        
        if (i > 1000) {  // Safety break
            continue_loop = 0;
        }
    }
    return sum;
}

/* Helper function to create SSA copy chain */
static inline int pass_value(int v) {
    int local = v;
    return local;  // Creates assignment copy
}

/* Pattern 6: Function call in copy chain */
int pattern6_function_phi(int x) {
    int flag;
    
    // PHI node
    if (x > 100) {
        flag = 1;
    } else if (x < 0) {
        flag = 1;
    } else {
        flag = 0;
    }
    
    // Pass through function (inline creates assignments)
    int passed = pass_value(flag);
    int passed2 = pass_value(passed);
    
    // Branch on the result
    if (passed2 == 0) {
        return -x;
    }
    return x;
}

/* Pattern 7: Multiple basic blocks with annotations */
int pattern7_multi_block_phi(int mode) {
    int state;
    
    // First conditional block
    if (mode & 1) {
        state = 1;
    } else {
        state = 0;
    }
    
    // Intermediate block with another conditional
    int intermediate;
    if (mode & 2) {
        intermediate = state;
    } else {
        intermediate = !state;
    }
    
    // Copy chain
    int final_check = intermediate;
    
    // Target conditional branch
    if (final_check == 1) {
        return 100;
    }
    
    // Another branch in same BB
    int another_copy = final_check;
    if (another_copy == 0) {
        return 200;
    }
    
    return 300;
}

/* Main function with its own pattern */
int main() {
    int total = 0;
    
    // Array to vary inputs
    int test_values[] = {5, 10, 15, 20, 25};
    int num_tests = sizeof(test_values) / sizeof(test_values[0]);
    
    // Main loop with PHI-derived flag (mimics pattern at top level)
    int all_done = 0;
    int test_index = 0;
    
    while (!all_done) {  // Branch on PHI-derived value
        int current_value = test_values[test_index];
        
        // Call each pattern function
        total += pattern1_loop_with_phi_flag(current_value);
        total += pattern2_if_else_phi(current_value, current_value * 2);
        total += pattern3_ternary_phi(current_value);
        total += pattern4_switch_phi('+');
        total += pattern5_complex_phi_chain(current_value);
        total += pattern6_function_phi(current_value);
        total += pattern7_multi_block_phi(current_value);
        
        test_index++;
        
        // PHI for loop control
        if (test_index >= num_tests) {
            all_done = 1;  // Set to 1
        } else {
            all_done = 0;  // Set to 0
        }
        
        // Copy chain in main
        int done_copy = all_done;
        int done_copy2 = done_copy;
        
        // Branch comparing against 0
        if (done_copy2 == 0) {
            // Continue loop
        }
    }
    
    // Additional test with different branch directions
    for (int i = 0; i < 10; i++) {
        int flag = (i % 3 == 0) ? 1 : 0;
        int flag_copy = flag;
        
        if (flag_copy == 1) {
            total += i * 10;
        } else if (flag_copy == 0) {
            total += i;
        }
    }
    
    printf("Result: %d\n", total);
    
    // Verify with simple calculation
    int expected_min = 0;
    for (int i = 0; i < num_tests; i++) {
        expected_min += test_values[i];  // Minimum from pattern1
    }
    
    if (total > expected_min) {
        printf("Test passed - computed result: %d\n", total);
    } else {
        printf("Test failed - suspiciously low result\n");
    }
    
    return 0;
}
