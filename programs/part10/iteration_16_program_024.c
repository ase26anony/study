#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Loop exit condition depends on PHI-assigned flag */
int pattern1_loop_exit_flag(int iterations) {
    int flag = 0;  // Initial value
    int count = 0;
    
    while (!flag) {  // Conditional on PHI-derived value (flag != 0)
        count++;
        if (count >= iterations) {
            flag = 1;  // One assignment to flag
        } else {
            flag = 0;  // Another assignment to flag - creates PHI at loop header
        }
        
        // Create SSA copy chain
        int temp1 = flag;
        int temp2 = temp1;
        if (temp2 == 1) {  // Conditional on copy of PHI value
            // Additional computation
            count *= 2;
        }
    }
    return count;
}

/* Pattern 2: If-else chain sets boolean, later conditional uses it */
int pattern2_if_else_merge(int a, int b) {
    int result;
    
    // PHI node will be created at merge point
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
    if (z == 0) {  // Compares against constant 0
        return a + b;
    } else {
        return a * b;
    }
}

/* Pattern 3: Ternary operator creates PHI, used in later conditional */
int pattern3_ternary_phi(int value) {
    // Ternary creates PHI node
    int is_even = (value % 2 == 0) ? 1 : 0;
    
    // Multiple SSA copies
    int copy1 = is_even;
    int copy2 = copy1;
    
    // Conditional with implicit boolean check (equivalent to != 0)
    if (copy2) {  // This becomes (copy2 != 0)
        return value / 2;
    }
    return value * 3 + 1;
}

/* Pattern 4: Nested loops with PHI in inner loop */
int pattern4_nested_loops(int n) {
    int total = 0;
    
    for (int i = 0; i < n; i++) {
        int inner_flag = 0;
        int j = 0;
        
        while (j < 10) {
            // PHI for inner_flag at loop header
            if (j % 2 == 0) {
                inner_flag = 1;
            } else {
                inner_flag = 0;
            }
            
            // SSA copy and conditional
            int tmp = inner_flag;
            if (tmp == 1) {  // Compare against constant 1
                total += i + j;
            } else {
                total += i - j;
            }
            j++;
        }
        
        // Another conditional using loop variable
        int check = (i > n/2) ? 1 : 0;
        if (check != 0) {  // Explicit comparison
            total *= 2;
        }
    }
    
    return total;
}

/* Pattern 5: Switch-case sets 0/1 value */
int pattern5_switch_phi(char op) {
    int should_double;
    
    switch (op) {
        case '+':
        case '-':
            should_double = 0;  // One assignment
            break;
        case '*':
        case '/':
            should_double = 1;  // Another assignment - creates PHI
            break;
        default:
            should_double = 0;
    }
    
    // Multiple SSA copies
    int a = should_double;
    int b = a;
    int c = b;
    
    // Conditional on the final copy
    if (c == 1) {
        return 100;
    } else {
        return 50;
    }
}

/* Helper function to create SSA copy chain through function call */
static inline int pass_value(int v) {
    // Simple function creates SSA copies
    int local = v;
    return local;
}

/* Pattern 6: Function call in SSA chain */
int pattern6_function_chain(int x) {
    int base = (x > 0) ? 1 : 0;  // PHI here
    
    // Chain through function calls
    int v1 = pass_value(base);
    int v2 = pass_value(v1);
    
    // Conditional with != 1 comparison
    if (v2 != 1) {  // Compare against constant 1
        return x * -1;
    }
    return x;
}

/* Pattern 7: Complex control flow with multiple merges */
int pattern7_complex_flow(int a, int b, int c) {
    int flag1, flag2;
    
    // First conditional structure
    if (a > 0) {
        flag1 = 1;
        if (b > 0) {
            flag2 = 1;
        } else {
            flag2 = 0;  // Creates PHI for flag2
        }
    } else {
        flag1 = 0;
        flag2 = (c > 0) ? 1 : 0;  // Creates PHI for flag2
    }
    // PHI nodes for flag1 and flag2 exist here
    
    // Use both flags in conditionals
    int result = 0;
    
    // SSA copy for flag1
    int f1_copy = flag1;
    if (f1_copy == 0) {  // Compare against 0
        result += a;
    }
    
    // SSA copy for flag2
    int f2_copy = flag2;
    if (f2_copy) {  // Implicit != 0
        result += b + c;
    }
    
    return result;
}

/* Main function with varied control flow */
int main() {
    int total = 0;
    
    // Array to vary inputs
    int inputs[] = {5, 10, 3, 7, 12, 8};
    int num_inputs = sizeof(inputs) / sizeof(inputs[0]);
    
    // Main loop with flag-based exit - mimics pattern at top level
    int done = 0;
    int idx = 0;
    
    while (!done) {  // Conditional on PHI-derived value
        int val = inputs[idx % num_inputs];
        
        // Call different pattern functions
        total += pattern1_loop_exit_flag(val % 5 + 1);
        total += pattern2_if_else_merge(val, val * 2);
        total += pattern3_ternary_phi(val);
        total += pattern4_nested_loops(val % 4 + 1);
        total += pattern5_switch_phi((val % 2) ? '+' : '*');
        total += pattern6_function_chain(val);
        total += pattern7_complex_flow(val, val + 1, val - 1);
        
        idx++;
        
        // PHI for done flag
        if (idx >= 20) {
            done = 1;  // One assignment
        } else {
            done = 0;  // Another assignment - creates PHI
        }
        
        // SSA copy and conditional in main loop
        int done_copy = done;
        if (done_copy == 1) {  // Compare against constant 1
            total += 1000;  // Bonus when done
        }
    }
    
    // Final conditional with SSA chain
    int final_check = (total > 10000) ? 1 : 0;
    int check_copy = final_check;
    int check_copy2 = check_copy;
    
    if (check_copy2 == 0) {  // Compare against constant 0
        total += 500;
    }
    
    printf("Result: %d\n", total);
    
    // Additional verification computation
    int verify = 0;
    for (int i = 0; i < total % 100; i++) {
        int flag = (i % 3 == 0) ? 1 : 0;  // PHI in loop
        int f = flag;
        if (f) {  // Conditional on PHI-derived value
            verify += i;
        }
    }
    printf("Verification: %d\n", verify);
    
    return 0;
}
