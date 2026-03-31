#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Loop exit condition depends on PHI-assigned flag */
int pattern1_loop_phi_flag(int iterations) {
    int flag = 0;  // Initial value
    int count = 0;
    
    // Loop where exit condition depends on flag
    while (!flag) {  // Compares flag against 0
        count++;
        // Set flag based on condition - creates PHI node
        if (count >= iterations) {
            flag = 1;  // Becomes 1 in one branch
        } else {
            flag = 0;  // Stays 0 in other branch
        }
        
        // Create SSA copy chain
        int temp1 = flag;
        int temp2 = temp1;
        
        // Use the copy in a conditional (target for analysis)
        if (temp2 == 0) {  // Compares against constant 0
            // Do some work
            count += 1;
        }
    }
    return count;
}

/* Pattern 2: If-else chain sets 0/1, later used in conditional */
int pattern2_ifelse_phi(int a, int b) {
    int result;
    
    // PHI node created here
    if (a > b) {
        result = 1;
    } else {
        result = 0;
    }
    
    // SSA copy chain
    int x = result;
    int y = x;
    int z = y;
    
    // Target conditional with constant 1
    if (z == 1) {  // Compares against constant 1
        return a * 2;
    } else {
        return b * 2;
    }
}

/* Pattern 3: Ternary operator creating PHI */
int pattern3_ternary_phi(int value) {
    // Ternary creates PHI node
    int is_even = (value % 2 == 0) ? 1 : 0;
    
    // Multiple SSA copies
    int copy1 = is_even;
    int copy2 = copy1;
    
    // Nested control flow to ensure annotation
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        // Inner conditional using the PHI-derived value
        if (copy2 != 0) {  // Compares against constant 0
            sum += value + i;
        } else {
            sum += value - i;
        }
    }
    
    // Another conditional with the same variable
    if (copy1 == 1) {  // Compares against constant 1
        sum *= 2;
    }
    
    return sum;
}

/* Pattern 4: Switch-case setting 0/1 */
int pattern4_switch_phi(char op) {
    int flag;
    
    // Switch creates PHI node
    switch (op) {
        case 'A':
        case 'B':
            flag = 1;
            break;
        case 'C':
        case 'D':
            flag = 0;
            break;
        default:
            flag = 1;
    }
    
    // Propagate through function-like inline
    static inline int propagate(int v) { return v; }
    int propagated = propagate(flag);
    
    // Complex nested structure for annotation
    int result = 0;
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            // Target conditional inside nested loops
            if (propagated == 0) {  // Compares against constant 0
                result += i * j;
            } else {
                result += i + j;
            }
        }
    }
    
    return result;
}

/* Pattern 5: Loop with PHI-derived condition variable */
int pattern5_loop_complex(int n) {
    int continue_loop = 1;  // Initial value
    int total = 0;
    int i = 0;
    
    while (continue_loop) {  // Implicit comparison with 0
        total += i;
        i++;
        
        // PHI node for continue_loop
        if (i >= n) {
            continue_loop = 0;
        } else if (total > 1000) {
            continue_loop = 0;
        } else {
            continue_loop = 1;
        }
        
        // Create copy chain
        int check = continue_loop;
        int verify = check;
        
        // Multiple conditionals with the same variable
        if (verify != 1) {  // Compares against constant 1
            // Early exit path
            break;
        }
        
        if (check == 0) {  // Compares against constant 0
            // Another path
            total += 10;
        }
    }
    
    return total;
}

/* Pattern 6: Multiple PHIs in same basic block */
int pattern6_multiple_phis(int a, int b, int c) {
    int flag1, flag2;
    
    // First PHI
    if (a > 0) {
        flag1 = 1;
    } else {
        flag1 = 0;
    }
    
    // Second PHI
    if (b > 0) {
        flag2 = 1;
    } else {
        flag2 = 0;
    }
    
    // Combine through operations
    int combined = flag1 && flag2 ? 1 : 0;
    
    // Copy chain
    int tmp = combined;
    int final_check = tmp;
    
    // Complex control flow
    int result = 0;
    for (int i = 0; i < c; i++) {
        // Nested if-else
        if (i % 3 == 0) {
            if (final_check == 1) {  // Compares against constant 1
                result += i * 2;
            }
        } else if (i % 3 == 1) {
            if (final_check != 0) {  // Compares against constant 0
                result += i * 3;
            }
        } else {
            // Another conditional
            if (tmp == 0) {  // Compares against constant 0
                result += i;
            }
        }
    }
    
    return result;
}

/* Main function with rich control flow */
int main() {
    int total = 0;
    
    // Initialize with different values to exercise various paths
    int values[] = {5, 10, 15, 20, 25};
    char ops[] = {'A', 'B', 'C', 'D', 'E'};
    
    // Main loop with PHI-derived flag (mimics target pattern)
    int should_continue = 1;
    int iteration = 0;
    
    while (should_continue) {  // Implicit comparison with 0
        // Call each pattern function
        total += pattern1_loop_phi_flag(values[iteration % 5]);
        total += pattern2_ifelse_phi(iteration, values[iteration % 5]);
        total += pattern3_ternary_phi(iteration);
        total += pattern4_switch_phi(ops[iteration % 5]);
        total += pattern5_loop_complex(values[iteration % 5]);
        total += pattern6_multiple_phis(iteration, values[iteration % 5], 3);
        
        iteration++;
        
        // PHI for should_continue
        if (iteration >= 10) {
            should_continue = 0;
        } else if (total > 10000) {
            should_continue = 0;
        } else {
            should_continue = 1;
        }
        
        // Create copy chain for the main loop condition
        int continue_check = should_continue;
        int verify_continue = continue_check;
        
        // Additional conditional using the copied value
        if (verify_continue == 0) {  // Compares against constant 0
            // Cleanup or final calculation
            total += 1000;
        }
    }
    
    printf("Final result: %d\n", total);
    
    // Additional test cases with different inputs
    printf("Pattern1 test: %d\n", pattern1_loop_phi_flag(3));
    printf("Pattern2 test: %d\n", pattern2_ifelse_phi(10, 5));
    printf("Pattern3 test: %d\n", pattern3_ternary_phi(7));
    printf("Pattern4 test: %d\n", pattern4_switch_phi('C'));
    printf("Pattern5 test: %d\n", pattern5_loop_complex(8));
    printf("Pattern6 test: %d\n", pattern6_multiple_phis(1, 0, 5));
    
    return 0;
}
