#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Loop exit condition depends on PHI-assigned flag */
int pattern1_loop_flag(int iterations) {
    int flag = 0;  // Initial value
    int count = 0;
    
    while (!flag) {  // Conditional on PHI-derived value (flag)
        count++;
        if (count >= iterations) {
            flag = 1;  // One assignment to flag
        } else {
            flag = 0;  // Another assignment to flag - creates PHI at loop header
        }
    }
    return count;
}

/* Pattern 2: If-else chain sets boolean, later conditional uses it */
int pattern2_if_else_merge(int a, int b) {
    int result;
    
    // First if-else creates PHI for result
    if (a > b) {
        result = 1;
    } else {
        result = 0;
    }
    
    // Copy chain through temporary variables
    int temp1 = result;
    int temp2 = temp1;
    
    // Conditional branch comparing against constant 0
    if (temp2 == 0) {
        return a + b;
    } else {
        return a - b;
    }
}

/* Pattern 3: Nested control with ternary operator */
int pattern3_ternary_phi(int x, int y) {
    // Ternary creates PHI node
    int flag = (x % 2 == 0) ? 1 : 0;
    
    // Multiple copy assignments
    int a = flag;
    int b = a;
    int c = b;
    
    // Conditional with implicit comparison to 0
    if (c) {  // Equivalent to if (c != 0)
        return x * y;
    }
    return x + y;
}

/* Pattern 4: Switch-case setting 0/1 value */
int pattern4_switch_phi(int option) {
    int value;
    
    switch (option % 3) {
        case 0:
            value = 1;
            break;
        case 1:
            value = 0;
            break;
        case 2:
            value = 1;
            break;
        default:
            value = 0;
    }
    
    // Copy through function-like inline expansion
    int tmp = value;
    tmp = tmp;  // Simple assignment chain
    
    // Explicit comparison to 1
    if (tmp == 1) {
        return option * 2;
    } else {
        return option / 2;
    }
}

/* Pattern 5: Complex loop with inner condition affecting outer flag */
int pattern5_nested_control(int n) {
    int done = 0;
    int sum = 0;
    int i = 0;
    
    while (!done) {  // Conditional on PHI-derived 'done'
        for (int j = 0; j < 3; j++) {
            sum += i + j;
            
            if (sum > 100) {
                done = 1;  // One assignment
            } else if (i >= n) {
                done = 1;  // Another assignment
            } else {
                done = 0;  // Third assignment - creates PHI
            }
        }
        i++;
    }
    
    // Another conditional using a different PHI-derived value
    int check = (sum % 2 == 0) ? 1 : 0;
    int verify = check;
    
    if (verify != 0) {  // Comparison to constant 0
        sum += 100;
    }
    
    return sum;
}

/* Pattern 6: Multiple PHI nodes in loop header */
int pattern6_multiple_phis(int limit) {
    int continue_loop = 1;
    int use_special = 0;
    int total = 0;
    
    for (int i = 0; continue_loop; i++) {
        // Multiple variables with PHI nodes at loop header
        if (i % 2 == 0) {
            use_special = 1;
        } else {
            use_special = 0;
        }
        
        if (use_special) {  // Conditional on PHI-derived value
            total += i * 2;
        } else {
            total += i;
        }
        
        // Chain of copies
        int flag_copy = continue_loop;
        int flag_copy2 = flag_copy;
        
        if (flag_copy2 == 1) {  // Explicit comparison to 1
            if (i >= limit) {
                continue_loop = 0;
            }
        }
    }
    return total;
}

/* Helper function to create copy chains */
static inline int pass_value(int v) {
    int local = v;
    return local;  // Creates assignment chain
}

/* Pattern 7: Using inline function for copy chain */
int pattern7_function_call(int x) {
    int flag;
    
    // PHI from if-else
    if (x > 0) {
        flag = 1;
    } else {
        flag = 0;
    }
    
    // Multiple levels of copy through function
    int a = pass_value(flag);
    int b = pass_value(a);
    
    // Conditional comparing to 0
    if (b == 0) {
        return -x;
    }
    return x * x;
}

/* Main function with varied control flow */
int main() {
    int total = 0;
    
    // Array to vary inputs
    int inputs[] = {5, 10, 3, 7, 12, 8};
    int n = sizeof(inputs) / sizeof(inputs[0]);
    
    // Main loop with flag-based exit condition
    int processed_all = 0;
    int index = 0;
    
    while (!processed_all) {  // Pattern in main function
        int val = inputs[index];
        
        // Call different pattern functions
        total += pattern1_loop_flag(val % 5 + 1);
        total += pattern2_if_else_merge(val, val * 2);
        total += pattern3_ternary_phi(val, val + 1);
        total += pattern4_switch_phi(val);
        total += pattern5_nested_control(val % 4 + 2);
        total += pattern6_multiple_phis(val % 3 + 3);
        total += pattern7_function_call(val - 5);
        
        index++;
        
        // PHI assignment for loop control
        if (index >= n) {
            processed_all = 1;
        } else {
            processed_all = 0;
        }
    }
    
    // Final conditional with copy chain
    int final_flag = (total > 1000) ? 1 : 0;
    int check1 = final_flag;
    int check2 = check1;
    
    if (check2 == 1) {  // Comparison to constant 1
        printf("Large result: %d\n", total);
    } else {
        printf("Result: %d\n", total);
    }
    
    // Additional verification computation
    int verify_total = 0;
    for (int i = 0; i < 10; i++) {
        int flag = (i % 3 == 0) ? 1 : 0;
        int tmp = flag;
        
        if (tmp != 0) {  // Another comparison to 0
            verify_total += i * 10;
        } else {
            verify_total += i;
        }
    }
    
    printf("Verification: %d\n", verify_total);
    
    return 0;
}
