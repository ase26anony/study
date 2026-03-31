#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Loop exit condition depends on PHI-assigned flag */
int pattern1_loop_with_phi_flag(int iterations) {
    int flag = 0;  // Initial value
    int count = 0;
    int i = 0;
    
    // Loop header creates PHI node for 'flag'
    while (!flag) {  // if (flag == 0) - compares against constant 0
        count++;
        i++;
        
        // This creates PHI for 'flag' at loop header
        if (i >= iterations) {
            flag = 1;  // Set to 1 to exit
        } else {
            flag = 0;  // Keep as 0 to continue
        }
    }
    return count;
}

/* Pattern 2: If-else chain sets 0/1, then conditional on that value */
int pattern2_ifelse_phi_conditional(int x, int y) {
    int result;
    
    // This creates a PHI node at the merge point
    if (x > y) {
        result = 1;
    } else {
        result = 0;
    }
    
    // Introduce SSA copy chain
    int temp1 = result;
    int temp2 = temp1;
    
    // Conditional on PHI-derived value (compares against 1)
    if (temp2 == 1) {
        return x * 2;
    } else {
        return y * 3;
    }
}

/* Pattern 3: Nested control with multiple PHI nodes */
int pattern3_nested_control(int n) {
    int sum = 0;
    int i = 0;
    int continue_loop = 1;  // Initial value
    
    while (continue_loop) {  // if (continue_loop != 0)
        // Inner if-else creates PHI for 'mod'
        int mod;
        if (i % 2 == 0) {
            mod = 1;
        } else {
            mod = 0;
        }
        
        // Copy through temporary
        int check = mod;
        
        // Conditional on PHI-derived value (compares against 0)
        if (check == 0) {
            sum += i * 2;
        } else {
            sum += i;
        }
        
        i++;
        
        // Update loop condition through PHI
        if (i >= n) {
            continue_loop = 0;
        } else {
            continue_loop = 1;
        }
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
    
    // Multiple SSA copies
    int a = indicator;
    int b = a;
    int c = b;
    
    // Conditional on the final copy
    if (c != 0) {  // Compares against 0
        return val * 10;
    }
    return val;
}

/* Pattern 5: Ternary operator creating PHI */
int pattern5_ternary_phi(int a, int b) {
    // Ternary creates PHI node
    int is_greater = (a > b) ? 1 : 0;
    
    // Pass through function-like macro (inlined)
    #define PASS(v) (v)
    int checked = PASS(is_greater);
    
    // Conditional on PHI-derived value
    if (checked == 1) {
        return a - b;
    } else {
        return b - a;
    }
}

/* Pattern 6: Complex chain with multiple PHIs */
int pattern6_complex_chain(int limit) {
    int total = 0;
    int active = 1;  // Start active
    
    for (int i = 0; i < limit; i++) {
        // First PHI: determine if we process this iteration
        int process;
        if (active && (i % 4 != 0)) {
            process = 1;
        } else {
            process = 0;
        }
        
        // Copy chain
        int p1 = process;
        int p2 = p1;
        
        // Conditional on copy (compares against 1)
        if (p2 == 1) {
            total += i;
            
            // Second PHI: update active state
            if (total > 100) {
                active = 0;
            } else {
                active = 1;
            }
        }
        
        // Third conditional on active (compares against 0)
        int a1 = active;
        if (a1 == 0) {
            break;
        }
    }
    return total;
}

/* Pattern 7: Multiple loops with PHI-based conditions */
int pattern7_multi_loop(int n) {
    int result = 0;
    int outer_flag = 1;
    
    // Outer loop with PHI condition
    while (outer_flag) {  // if (outer_flag != 0)
        int inner_flag = 1;
        int j = 0;
        
        // Inner loop with its own PHI
        while (inner_flag && j < 5) {  // if (inner_flag != 0)
            // PHI for computation
            int add_value;
            if ((result + j) % 3 == 0) {
                add_value = 1;
            } else {
                add_value = 0;
            }
            
            // Copy and check
            int av_copy = add_value;
            if (av_copy == 1) {  // Compare against 1
                result += j * 2;
            } else {
                result += j;
            }
            
            j++;
            
            // Update inner flag
            if (j >= 3 && result > 20) {
                inner_flag = 0;
            }
        }
        
        n--;
        if (n <= 0) {
            outer_flag = 0;
        }
    }
    return result;
}

/* Main function with its own pattern */
int main() {
    int total = 0;
    
    // Array to control branch directions
    int test_cases[] = {5, 10, 3, 8, 12, 7};
    int num_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    
    // Main loop with PHI-like flag
    int continue_main = 1;
    int idx = 0;
    
    while (continue_main) {  // if (continue_main != 0)
        // Call each pattern function
        total += pattern1_loop_with_phi_flag(test_cases[idx % num_cases]);
        total += pattern2_ifelse_phi_conditional(test_cases[idx % num_cases], 
                                                test_cases[(idx + 1) % num_cases]);
        total += pattern3_nested_control(test_cases[idx % num_cases]);
        total += pattern4_switch_phi(test_cases[idx % num_cases]);
        total += pattern5_ternary_phi(test_cases[idx % num_cases], 
                                     test_cases[(idx + 2) % num_cases]);
        total += pattern6_complex_chain(test_cases[idx % num_cases]);
        total += pattern7_multi_loop(test_cases[idx % num_cases] % 4 + 1);
        
        idx++;
        
        // Update continue flag (creates PHI)
        if (idx >= num_cases * 2) {
            continue_main = 0;
        } else {
            continue_main = 1;
        }
    }
    
    printf("Final result: %d\n", total);
    printf("Pattern executions completed.\n");
    
    return 0;
}
