#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Loop exit condition depends on PHI-assigned flag */
int pattern1_loop_flag(int iterations) {
    int flag = 0;  // Initial value
    int count = 0;
    
    while (!flag) {  // Conditional on PHI-derived value (flag != 0)
        count++;
        if (count >= iterations) {
            flag = 1;  // Sets flag in loop body
        }
        
        // Create SSA copy chain
        int temp1 = flag;
        int temp2 = temp1;
        if (temp2 == 1) {  // Conditional with constant RHS 1
            // Additional control flow
            int inner = 0;
            for (int i = 0; i < 5; i++) {
                inner += i;
            }
        }
    }
    return count;
}

/* Pattern 2: If-else chain assigns 0/1, then conditional on that value */
int pattern2_phi_merge(int a, int b) {
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
    
    // Conditional with constant RHS 0
    if (z == 0) {
        return a + b;
    } else {
        return a * b;
    }
}

/* Pattern 3: Ternary operator creating PHI, followed by conditional */
int pattern3_ternary_phi(int value) {
    // Ternary creates PHI node
    int is_even = (value % 2 == 0) ? 1 : 0;
    
    // Multiple SSA copies
    int copy1 = is_even;
    int copy2 = copy1;
    
    // Conditional with implicit check (if (x) == if (x != 0))
    if (copy2) {
        return value * 2;
    }
    
    // Another conditional with explicit 1
    int temp = copy1;
    if (temp != 1) {
        return value / 2;
    }
    
    return value;
}

/* Pattern 4: Nested loops with PHI-derived condition */
int pattern4_nested_loops(int n) {
    int total = 0;
    int continue_outer = 1;
    
    for (int i = 0; i < n && continue_outer; i++) {
        int flag = 0;
        
        for (int j = 0; j < n; j++) {
            total += i * j;
            
            if (i * j > 100) {
                flag = 1;
                break;
            }
        }
        
        // SSA copy and conditional
        int check = flag;
        if (check == 0) {  // Constant RHS 0
            continue_outer = 1;
        } else {
            continue_outer = 0;
        }
    }
    
    return total;
}

/* Pattern 5: Switch-case setting 0/1 value */
int pattern5_switch_phi(int option) {
    int status;
    
    switch (option) {
        case 1:
            status = 1;
            break;
        case 2:
            status = 0;
            break;
        case 3:
            status = 1;
            break;
        default:
            status = 0;
    }
    
    // Chain of assignments
    int a = status;
    int b = a;
    
    // Conditional on the PHI-derived value
    if (b == 1) {
        return option * 10;
    } else if (b == 0) {
        return option * 5;
    }
    
    return option;
}

/* Helper function to create SSA copy chain through function call */
static inline int pass_value(int v) {
    // Simple function creates SSA copies
    int local = v;
    return local;
}

/* Pattern 6: Function call in SSA chain */
int pattern6_function_chain(int x) {
    int base = (x > 0) ? 1 : 0;  // PHI
    
    // Multiple levels of copying
    int level1 = pass_value(base);
    int level2 = pass_value(level1);
    int level3 = level2;
    
    // Conditional with constant RHS
    if (level3 == 0) {
        return -x;
    }
    
    return x * x;
}

/* Pattern 7: Complex control flow with multiple PHIs */
int pattern7_complex_flow(int a, int b, int c) {
    int flag1, flag2;
    
    // First conditional creates PHI
    if (a > 10) {
        flag1 = 1;
    } else {
        flag1 = 0;
    }
    
    // Second conditional
    if (b < 20) {
        flag2 = 1;
    } else {
        flag2 = 0;
    }
    
    // Combine flags
    int combined = flag1 && flag2 ? 1 : 0;
    
    // SSA copies
    int check1 = combined;
    int check2 = check1;
    
    // Nested conditionals
    if (c > 0) {
        if (check2 == 1) {  // Constant RHS 1
            return a + b + c;
        }
    } else {
        if (check2 != 0) {  // Constant RHS 0
            return a * b * c;
        }
    }
    
    return 0;
}

int main() {
    int total = 0;
    
    // Main loop with flag-based exit (mimics pattern at top level)
    int done = 0;
    int iteration = 0;
    
    while (!done) {  // Conditional on PHI-derived value
        // Call each pattern function with different inputs
        total += pattern1_loop_flag(iteration + 1);
        total += pattern2_phi_merge(iteration, iteration * 2);
        total += pattern3_ternary_phi(iteration);
        total += pattern4_nested_loops(iteration % 5 + 1);
        total += pattern5_switch_phi(iteration % 4);
        total += pattern6_function_chain(iteration - 5);
        total += pattern7_complex_flow(iteration, iteration + 1, iteration + 2);
        
        iteration++;
        
        // Set exit condition based on computation
        int exit_flag = (total > 1000) ? 1 : 0;
        
        // Create SSA copy chain for the flag
        int temp_flag = exit_flag;
        int final_flag = temp_flag;
        
        // Conditional with constant RHS 1
        if (final_flag == 1) {
            done = 1;
        }
        
        // Additional control flow to create annotated blocks
        if (iteration % 3 == 0) {
            for (int i = 0; i < 2; i++) {
                total += i;
            }
        } else if (iteration % 3 == 1) {
            int j = 0;
            while (j < 2) {
                total -= j;
                j++;
            }
        }
    }
    
    printf("Result: %d\n", total);
    printf("Iterations: %d\n", iteration);
    
    // Additional test cases with arrays
    int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = i;
    }
    
    // Process array with conditional
    int sum = 0;
    int found = 0;
    for (int i = 0; i < 10 && !found; i++) {
        sum += arr[i];
        
        // Conditional on PHI-derived value
        int threshold_check = (sum > 20) ? 1 : 0;
        int copy = threshold_check;
        
        if (copy == 1) {  // Constant RHS 1
            found = 1;
        }
    }
    
    printf("Array sum: %d\n", sum);
    
    return 0;
}
