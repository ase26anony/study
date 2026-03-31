#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Loop with PHI-derived exit condition */
int pattern1_loop_with_phi_flag(int iterations) {
    int flag = 0;  // This becomes a PHI node at loop header
    int count = 0;
    int i = 0;
    
    while (!flag) {  // Conditional on PHI-derived value (flag != 0)
        count++;
        i++;
        if (i >= iterations) {
            flag = 1;  // Sets flag inside loop
        }
    }
    return count;
}

/* Pattern 2: If-else chain assigning 0/1 to variable used later */
int pattern2_ifelse_phi(int a, int b) {
    int result;
    
    // PHI node created at merge point
    if (a > b) {
        result = 1;
    } else {
        result = 0;
    }
    
    // Introduce SSA copy chain
    int temp1 = result;
    int temp2 = temp1;
    
    // Conditional on copy of PHI result
    if (temp2 == 1) {  // Compares against constant 1
        return a * 2;
    } else {
        return b * 2;
    }
}

/* Pattern 3: Nested control with multiple PHI nodes */
int pattern3_nested_control(int n) {
    int sum = 0;
    int continue_loop = 1;  // PHI at loop header
    
    for (int i = 0; continue_loop; i++) {
        int inner_flag;
        
        // Another PHI node
        if (i % 2 == 0) {
            inner_flag = 1;
        } else {
            inner_flag = 0;
        }
        
        // Copy through temporary
        int check = inner_flag;
        
        // Conditional on copy
        if (check != 0) {  // Compares against constant 0
            sum += i;
        }
        
        if (i >= n) {
            continue_loop = 0;  // Modifies loop condition
        }
    }
    return sum;
}

/* Pattern 4: Switch-case setting 0/1 value */
int pattern4_switch_phi(int val) {
    int code = 0;  // Initial value
    
    switch (val % 3) {
        case 0:
            code = 1;
            break;
        case 1:
            code = 0;
            break;
        case 2:
            code = 1;
            break;
    }
    
    // Multiple SSA copies
    int x = code;
    int y = x;
    int z = y;
    
    // Final conditional
    if (z == 0) {  // Explicit comparison with 0
        return val * 10;
    } else {
        return val * 20;
    }
}

/* Pattern 5: Ternary operator creating PHI */
int pattern5_ternary_phi(int x, int y) {
    // Ternary creates PHI node
    int is_greater = (x > y) ? 1 : 0;
    
    // Pass through function-like macro (inlined)
    #define PASS(v) (v)
    int passed = PASS(is_greater);
    
    if (passed) {  // Implicit comparison with 0
        return x - y;
    }
    return y - x;
}

/* Pattern 6: Complex chain with multiple PHIs */
int pattern6_complex_chain(int limit) {
    int a = 0;
    int b = 0;
    int total = 0;
    
    while (a < limit) {
        int condition;
        
        // PHI for condition
        if (a % 2 == 0) {
            condition = 1;
        } else {
            condition = 0;
        }
        
        // Long copy chain
        int c1 = condition;
        int c2 = c1;
        int c3 = c2;
        int c4 = c3;
        
        // Nested if with conditional on copy chain
        if (c4 == 1) {  // Compare with 1
            total += a;
        }
        
        // Another conditional inside
        if (b > 10) {
            int temp = condition;
            if (temp != 0) {  // Compare with 0
                total -= 5;
            }
        }
        
        a++;
        b = (b + 1) % 15;
    }
    return total;
}

/* Pattern 7: Loop with break condition from PHI */
int pattern7_loop_break(int max) {
    int found = 0;  // PHI at loop header
    int value = 0;
    
    for (int i = 0; i < 100; i++) {
        if (i == max) {
            found = 1;
            value = i * 2;
        }
        
        // Copy through temporary
        int check_found = found;
        
        // Conditional on copy
        if (check_found) {  // Implicit != 0
            break;
        }
    }
    return value;
}

/* Main function with its own pattern */
int main() {
    int total = 0;
    int use_patterns = 1;  // PHI-derived flag for main's loop
    
    // Array to vary inputs
    int inputs[] = {5, 10, 15, 20, 25};
    int input_count = sizeof(inputs) / sizeof(inputs[0]);
    
    // Main loop with condition from PHI
    int i = 0;
    while (use_patterns) {
        // Call each pattern function
        total += pattern1_loop_with_phi_flag(inputs[i % input_count]);
        total += pattern2_ifelse_phi(inputs[i % input_count], inputs[(i + 1) % input_count]);
        total += pattern3_nested_control(inputs[i % input_count]);
        total += pattern4_switch_phi(inputs[i % input_count]);
        total += pattern5_ternary_phi(inputs[i % input_count], inputs[(i + 2) % input_count]);
        total += pattern6_complex_chain(inputs[i % input_count]);
        total += pattern7_loop_break(inputs[i % input_count]);
        
        i++;
        
        // Modify loop condition (creates PHI)
        if (i >= 3) {
            use_patterns = 0;  // Will be PHI at loop header
        }
    }
    
    printf("Total result: %d\n", total);
    printf("Pattern execution completed.\n");
    
    // Additional test with different branch directions
    int test_val = 7;
    
    // Force different paths
    if (pattern2_ifelse_phi(test_val, test_val * 2) > 0) {
        printf("Branch A taken\n");
    } else {
        printf("Branch B taken\n");
    }
    
    // Test with zero comparison
    int zero_test = pattern5_ternary_phi(0, 1);
    if (zero_test == 0) {
        printf("Zero comparison path\n");
    }
    
    return 0;
}
