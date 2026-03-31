#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Loop exit condition depends on PHI-assigned flag */
int pattern1_loop_exit_flag(int iterations) {
    int flag = 0;  // Initial value
    int count = 0;
    
    // Loop header creates PHI node for 'flag'
    while (!flag) {  // Conditional on PHI-derived value
        count++;
        if (count >= iterations) {
            flag = 1;  // One assignment to flag
        } else {
            flag = 0;  // Another assignment to flag
        }
        // flag now has PHI at loop header
    }
    return count;
}

/* Pattern 2: If-else chain sets boolean, later conditional uses it */
int pattern2_if_else_merge(int a, int b) {
    int result;
    
    // This creates PHI at merge point
    if (a > b) {
        result = 1;
    } else {
        result = 0;
    }
    
    // SSA copy chain
    int temp1 = result;
    int temp2 = temp1;
    
    // Conditional on copy of PHI value
    if (temp2 == 1) {  // Compares against constant 1
        return a;
    } else {
        return b;
    }
}

/* Pattern 3: Nested control with multiple PHI nodes */
int pattern3_nested_control(int n) {
    int sum = 0;
    int i = 0;
    int continue_flag = 1;  // Initial value
    
    while (continue_flag) {  // Outer loop condition
        int inner_flag = 0;  // Reset each iteration
        
        for (int j = 0; j < n; j++) {
            if ((i + j) % 3 == 0) {
                inner_flag = 1;
            } else {
                inner_flag = 0;
            }
            
            // Conditional on inner PHI-derived value
            if (inner_flag) {  // Implicit comparison with 0
                sum += i * j;
            }
        }
        
        i++;
        if (i >= 10) {
            continue_flag = 0;  // Set exit condition
        } else {
            continue_flag = 1;  // Continue looping
        }
        // continue_flag has PHI at loop header
    }
    return sum;
}

/* Pattern 4: Switch-case sets 0/1 value */
int pattern4_switch_case(int val) {
    int indicator = 0;
    
    switch (val % 4) {
        case 0: indicator = 1; break;
        case 1: indicator = 0; break;
        case 2: indicator = 1; break;
        case 3: indicator = 0; break;
    }
    
    // Pass through function to create SSA copies
    int copy1 = indicator;
    int copy2 = copy1;
    int copy3 = copy2;
    
    // Conditional on the final copy
    if (copy3 != 0) {  // Compares against constant 0
        return val * 2;
    }
    return val;
}

/* Pattern 5: Ternary operator creating PHI */
int pattern5_ternary_phi(int x, int y) {
    // Ternary creates PHI node
    int is_greater = (x > y) ? 1 : 0;
    
    // Multiple assignments to create copy chain
    int a = is_greater;
    int b = a;
    
    // Conditional with explicit 0 comparison
    if (b == 0) {
        return y - x;
    } else {
        return x - y;
    }
}

/* Pattern 6: Complex chain with arithmetic */
int pattern6_complex_chain(int limit) {
    int state = 0;
    int total = 0;
    
    for (int i = 0; i < limit; i++) {
        // Multiple basic blocks create rich control flow
        if (i % 2 == 0) {
            state = 1;
        } else {
            state = 0;
        }
        
        // Intermediate computation
        int intermediate = state;
        intermediate += 5;
        intermediate -= 5;  // Back to original value
        
        // Final conditional
        if (intermediate == 1) {  // Compare against constant 1
            total += i;
        }
    }
    return total;
}

/* Helper to create SSA copy chains */
static inline int pass_value(int v) {
    int temp = v;
    return temp;  // Creates SSA copy
}

/* Pattern 7: Using inline function for copy chain */
int pattern7_function_copy(int a, int b) {
    int cmp_result;
    
    if (a == b) {
        cmp_result = 1;
    } else {
        cmp_result = 0;
    }
    
    // Multiple passes through function
    int passed1 = pass_value(cmp_result);
    int passed2 = pass_value(passed1);
    
    // Conditional on the passed value
    if (passed2) {  // Implicit comparison with 0
        return 1;
    }
    return 0;
}

int main() {
    int total = 0;
    int loop_flag = 1;  // PHI will be created for this
    
    // Main loop with PHI-derived exit condition
    for (int iteration = 0; loop_flag; iteration++) {
        // Exercise all patterns
        total += pattern1_loop_exit_flag(iteration % 5 + 1);
        total += pattern2_if_else_merge(iteration, iteration * 2);
        total += pattern3_nested_control(iteration % 3 + 2);
        total += pattern4_switch_case(iteration);
        total += pattern5_ternary_phi(iteration, 10);
        total += pattern6_complex_chain(iteration % 4 + 1);
        total += pattern7_function_copy(iteration % 2, (iteration + 1) % 2);
        
        // Update loop flag based on condition (creates PHI)
        if (iteration >= 7) {
            loop_flag = 0;  // One assignment
        } else {
            loop_flag = 1;  // Another assignment
        }
        // loop_flag has PHI at loop header
    }
    
    // Final conditional on a PHI-derived value
    int final_check = (total > 100) ? 1 : 0;
    int final_copy = final_check;
    
    if (final_copy == 1) {
        printf("Result: %d (above threshold)\n", total);
    } else {
        printf("Result: %d (below threshold)\n", total);
    }
    
    return 0;
}
