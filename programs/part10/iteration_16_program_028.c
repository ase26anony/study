#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Loop with PHI-derived exit condition */
int pattern1_loop_with_phi_flag(int iterations) {
    int flag = 0;  // This becomes a PHI at loop header
    int count = 0;
    int i = 0;
    
    while (!flag) {  // Conditional on PHI-derived value (flag != 0)
        count++;
        i++;
        if (i >= iterations) {
            flag = 1;  // Sets flag inside loop - creates PHI
        }
        
        // Create SSA copy chain
        int temp1 = flag;
        int temp2 = temp1;
        if (temp2 == 0) {  // Conditional on copy of PHI value
            // Do some work
            count += 2;
        }
    }
    return count;
}

/* Pattern 2: If-else chain assigning 0/1 to variable */
int pattern2_ifelse_phi(int a, int b) {
    int result;
    
    // This creates a PHI node at the merge point
    if (a > b) {
        result = 1;
    } else {
        result = 0;
    }
    
    // SSA copy chain
    int x = result;
    int y = x;
    int z = y;
    
    // Conditional on PHI-derived value through copy chain
    if (z == 1) {  // Compares against constant 1
        return a * 2;
    } else {
        return b * 3;
    }
}

/* Pattern 3: Nested control with ternary operator */
int pattern3_ternary_phi(int x) {
    // Ternary creates PHI
    int is_even = (x % 2 == 0) ? 1 : 0;
    
    // Multiple SSA copies
    int copy1 = is_even;
    int copy2 = copy1;
    
    // Conditional on copy chain
    if (copy2 != 0) {  // Compares against constant 0
        return x / 2;
    }
    return x * 3 + 1;
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
            flag = 1;  // Creates PHI with multiple incoming values
    }
    
    // Chain of assignments
    int a = flag;
    int b = a;
    
    // Conditional check
    if (b) {  // Implicit comparison against 0
        return 100;
    }
    return 200;
}

/* Pattern 5: Complex loop with inner condition */
int pattern5_nested_phi(int n) {
    int total = 0;
    int continue_loop = 1;  // Initial value
    
    for (int i = 0; continue_loop; i++) {
        if (i >= n) {
            continue_loop = 0;  // Modifies PHI operand
        }
        
        // Inner conditional that uses PHI-derived value
        int local_flag = continue_loop;
        int flag_copy = local_flag;
        
        if (flag_copy == 1) {  // Explicit comparison to 1
            total += i;
        }
        
        // Another conditional with different constant
        if (!local_flag) {  // Implicit comparison to 0
            total += 1000;
        }
    }
    return total;
}

/* Helper function to create SSA copy chain */
static inline int pass_through(int v) {
    int t1 = v;
    int t2 = t1;
    return t2;
}

/* Pattern 6: Function call in copy chain */
int pattern6_function_phi(int a, int b) {
    int cmp_result;
    
    // Create PHI
    if (a == b) {
        cmp_result = 1;
    } else if (a > b) {
        cmp_result = 0;
    } else {
        cmp_result = 1;  // Another incoming edge to PHI
    }
    
    // Pass through function (inlined)
    int passed = pass_through(cmp_result);
    
    // Conditional on result
    if (passed == 0) {  // Compare against 0
        return a - b;
    }
    return b - a;
}

/* Pattern 7: Loop with multiple exit conditions */
int pattern7_multi_exit_phi(int limit) {
    int done = 0;
    int count = 0;
    
    while (1) {
        count++;
        
        // Multiple ways to set done flag
        if (count >= limit) {
            done = 1;
        }
        
        if (count % 7 == 0) {
            done = 1;
        }
        
        // Copy chain
        int d1 = done;
        int d2 = d1;
        
        // Conditional on copy
        if (d2 != 0) {  // Compare against 0
            break;
        }
    }
    return count;
}

/* Main function with rich control flow */
int main(int argc, char **argv) {
    int total = 0;
    
    // Initialize with command line or default
    int base = (argc > 1) ? atoi(argv[1]) : 100;
    
    // Pattern 1
    total += pattern1_loop_with_phi_flag(base % 10 + 5);
    
    // Pattern 2
    total += pattern2_ifelse_phi(base, base / 2);
    
    // Pattern 3
    total += pattern3_ternary_phi(base);
    
    // Pattern 4
    total += pattern4_switch_phi('+');
    total += pattern4_switch_phi('*');
    
    // Pattern 5
    total += pattern5_nested_phi(base % 8 + 3);
    
    // Pattern 6
    total += pattern6_function_phi(base, base / 3);
    
    // Pattern 7
    total += pattern7_multi_exit_phi(base % 6 + 4);
    
    // Additional pattern in main itself
    int main_flag = 0;
    int iterations = 0;
    
    while (main_flag == 0) {  // Direct comparison to 0
        iterations++;
        
        // Create SSA copy
        int flag_copy = main_flag;
        
        if (iterations >= 5) {
            main_flag = 1;  // Creates PHI
        }
        
        // Use copy in condition
        if (flag_copy == 0) {  // Compare against 0
            total += iterations;
        }
    }
    
    printf("Result: %d\n", total);
    return 0;
}
