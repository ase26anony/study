#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Loop with PHI-derived exit condition */
int pattern1_loop_phi_exit(int iterations) {
    int flag = 0;  // Initial value
    int count = 0;
    
    while (!flag) {  // Conditional on PHI-derived value (flag != 0)
        count++;
        // This creates a PHI node for 'flag' at loop header
        if (count >= iterations) {
            flag = 1;  // One branch sets flag to 1
        } else {
            flag = 0;  // Other branch sets flag to 0
        }
        
        // Introduce SSA copy chain
        int temp1 = flag;
        int temp2 = temp1;
        if (temp2 == 1) {  // Conditional on copy of PHI-derived value
            // Additional branch to create annotated block
            if (count % 2 == 0) {
                count += 1;
            }
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
    
    // SSA copy chain through multiple assignments
    int x = result;
    int y = x;
    int z = y;
    
    // Conditional on the copy chain end
    if (z == 0) {  // Compare against constant 0
        return a + b;
    } else {
        return a * b;
    }
}

/* Pattern 3: Nested control with ternary operator */
int pattern3_ternary_phi(int value) {
    // Ternary creates PHI-like assignment
    int is_even = (value % 2 == 0) ? 1 : 0;
    
    // Multiple SSA copies
    int copy1 = is_even;
    int copy2 = copy1;
    
    // Complex nested structure for annotation
    int total = 0;
    for (int i = 0; i < value; i++) {
        if (copy2 != 0) {  // Conditional on PHI-derived value
            total += i;
            if (i % 3 == 0) {
                total *= 2;
            }
        } else {
            total -= i;
            if (i % 4 == 0) {
                total /= 2;
            }
        }
        
        // Update copy chain inside loop
        copy2 = copy1;
    }
    
    return total;
}

/* Pattern 4: Switch-case with PHI assignment */
int pattern4_switch_phi(int option) {
    int flag;
    
    switch (option) {
        case 1:
            flag = 1;
            break;
        case 2:
            flag = 0;
            break;
        case 3:
            flag = 1;
            break;
        default:
            flag = 0;
            break;
    }
    
    // Long SSA copy chain
    int a = flag;
    int b = a;
    int c = b;
    int d = c;
    
    // Conditional with constant 1 comparison
    if (d == 1) {
        return option * 10;
    }
    return option;
}

/* Pattern 5: Function call propagation */
static inline int propagate(int val) {
    // Simple propagation function
    return val;
}

int pattern5_call_phi(int base) {
    int state = 0;
    int sum = 0;
    
    for (int i = 0; i < base; i++) {
        // PHI node for state at loop header
        if (i % 5 == 0) {
            state = 1;
        } else if (i % 3 == 0) {
            state = 0;
        }
        
        // Propagate through function call (inlined)
        int propagated = propagate(state);
        int final_val = propagated;
        
        // Multiple conditionals on the propagated value
        if (final_val) {  // Implicit comparison with 0
            sum += i * 2;
            if (i % 2 == 0) {
                sum += 5;
            }
        } else if (final_val == 0) {  // Explicit comparison with 0
            sum += i;
            if (i % 7 == 0) {
                sum -= 3;
            }
        }
    }
    
    return sum;
}

/* Pattern 6: Complex loop with multiple PHIs */
int pattern6_multi_phi(int limit) {
    int flag1 = 0, flag2 = 1;
    int result = 0;
    
    for (int i = 0; i < limit; i++) {
        // Multiple PHI nodes in loop header
        int temp = flag1;
        
        // Nested conditionals
        if (temp == 0) {
            result += i;
            if (i % 2 == 0) {
                flag1 = 1;
                flag2 = 0;
            }
        } else if (temp == 1) {
            result -= i;
            if (i % 3 == 0) {
                flag1 = 0;
                
                // Another SSA copy chain
                int chain1 = flag2;
                int chain2 = chain1;
                if (chain2 != 1) {  // Compare against constant 1
                    result *= 2;
                }
            }
        }
        
        // Additional branch for annotation
        if (flag2 == 0 && result > 100) {
            result %= 100;
        }
    }
    
    return result;
}

/* Main function with diverse execution paths */
int main() {
    int total = 0;
    
    // Array to control different execution paths
    int inputs[] = {5, 10, 15, 20, 25};
    int num_inputs = sizeof(inputs) / sizeof(inputs[0]);
    
    // Main loop with PHI-derived condition (mimics pattern at top level)
    int continue_flag = 1;
    int idx = 0;
    
    while (continue_flag) {  // Conditional on PHI-derived value
        int input = inputs[idx % num_inputs];
        
        // Call each pattern function
        total += pattern1_loop_phi_exit(input);
        total += pattern2_ifelse_phi(input, input / 2);
        total += pattern3_ternary_phi(input);
        total += pattern4_switch_phi(input % 4);
        total += pattern5_call_phi(input);
        total += pattern6_multi_phi(input);
        
        idx++;
        
        // Create PHI for continue_flag
        if (idx >= 10) {
            continue_flag = 0;  // Set to 0 to exit
        } else {
            continue_flag = 1;  // Set to 1 to continue
        }
        
        // SSA copy chain in main
        int check = continue_flag;
        int verify = check;
        
        // Additional conditional on the copy
        if (verify == 0) {
            total += 1000;  // Bonus when exiting
        }
    }
    
    // Final computation with conditional
    int final_flag = (total > 5000) ? 1 : 0;
    int final_copy = final_flag;
    
    if (final_copy == 1) {
        total *= 2;
    } else if (final_copy == 0) {
        total += 500;
    }
    
    printf("Final result: %d\n", total);
    
    // Additional verification runs
    for (int i = 0; i < 3; i++) {
        int test_val = pattern2_ifelse_phi(i * 10, i * 5);
        int test_copy = test_val;
        
        if (test_copy == 0) {
            printf("Iteration %d: zero path\n", i);
        } else {
            printf("Iteration %d: non-zero path\n", i);
        }
    }
    
    return 0;
}
