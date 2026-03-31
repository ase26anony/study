#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Loop exit condition depends on PHI-assigned flag */
int pattern1_loop_flag(int iterations) {
    int flag = 0;
    int count = 0;
    
    while (!flag) {  // This creates a PHI node for 'flag' at loop header
        count++;
        if (count >= iterations) {
            flag = 1;  // Sets flag inside loop
        }
        // Create SSA copy chain
        int temp1 = flag;
        int temp2 = temp1;
        if (temp2 == 0) {  // Conditional on PHI-derived value vs 0
            // Do some work
            count += 1;
        }
    }
    return count;
}

/* Pattern 2: If-else chain sets boolean, later conditional uses it */
int pattern2_if_else_merge(int x, int y) {
    int result;
    
    // First if-else sets a boolean value
    if (x > y) {
        result = 1;
    } else {
        result = 0;
    }
    
    // SSA copy chain
    int a = result;
    int b = a;
    int c = b;
    
    // Conditional on PHI-derived value
    if (c == 1) {  // Compares against constant 1
        return x * 2;
    } else {
        return y * 3;
    }
}

/* Pattern 3: Ternary operator creates PHI, followed by conditional */
int pattern3_ternary_phi(int a, int b) {
    // Ternary creates a PHI node
    int is_greater = (a > b) ? 1 : 0;
    
    // Multiple SSA copies
    int copy1 = is_greater;
    int copy2 = copy1;
    
    if (copy2) {  // Implicit comparison against 0
        return a - b;
    }
    return b - a;
}

/* Pattern 4: Nested loops with flag propagation */
int pattern4_nested_loops(int n) {
    int total = 0;
    int outer_flag = 0;
    
    for (int i = 0; i < n; i++) {
        int inner_flag = 0;
        int j = 0;
        
        while (!inner_flag) {  // PHI for inner_flag
            total += i + j;
            j++;
            
            if (j >= 3) {
                inner_flag = 1;  // Set inside loop
            }
            
            // SSA chain and conditional
            int f1 = inner_flag;
            int f2 = f1;
            if (f2 == 0) {  // Conditional on PHI-derived value
                total += 1;
            }
        }
        
        // Propagate to outer conditional
        outer_flag = (i >= n/2) ? 1 : 0;
        int of1 = outer_flag;
        if (of1) {  // Another conditional
            total *= 2;
        }
    }
    return total;
}

/* Pattern 5: Switch-case sets 0/1 value */
int pattern5_switch_phi(int option) {
    int value = 0;
    
    switch (option) {
        case 1:
            value = 1;
            break;
        case 2:
            value = 0;
            break;
        case 3:
            value = 1;
            break;
        default:
            value = 0;
    }
    
    // SSA copies
    int v1 = value;
    int v2 = v1;
    int v3 = v2;
    
    if (v3 != 0) {  // Compare against 0
        return 100 + option;
    }
    return 200 + option;
}

/* Pattern 6: Function call creates SSA copy chain */
static inline int pass_value(int v) {
    return v;  // Simple pass-through
}

int pattern6_function_call(int x) {
    int flag = (x % 2 == 0) ? 1 : 0;  // PHI from ternary
    
    // Multiple function calls create SSA copies
    int f1 = pass_value(flag);
    int f2 = pass_value(f1);
    
    if (f2 == 1) {  // Compare against 1
        return x * x;
    }
    return x + x;
}

/* Pattern 7: Complex control flow with multiple merges */
int pattern7_complex_flow(int a, int b, int c) {
    int result = 0;
    int condition;
    
    if (a > 0) {
        if (b > 0) {
            condition = 1;
        } else {
            condition = 0;
        }
    } else {
        if (c > 0) {
            condition = 1;
        } else {
            condition = 0;
        }
    }
    
    // Long SSA copy chain
    int c1 = condition;
    int c2 = c1;
    int c3 = c2;
    int c4 = c3;
    int c5 = c4;
    
    if (c5 == 0) {  // Conditional on deeply copied PHI value
        result = a + b + c;
    } else {
        result = a * b * c;
    }
    
    // Another conditional in same basic block
    int check = (result > 1000) ? 1 : 0;
    int ch1 = check;
    if (ch1) {  // Another conditional
        result /= 2;
    }
    
    return result;
}

/* Main function with varied control flow */
int main() {
    int total = 0;
    
    // Initialize test data
    int test_values[] = {5, 10, 15, 20, 25};
    int num_tests = sizeof(test_values) / sizeof(test_values[0]);
    
    // Pattern 1: Loop with flag
    for (int i = 0; i < num_tests; i++) {
        total += pattern1_loop_flag(test_values[i]);
    }
    
    // Pattern 2: If-else merge
    for (int i = 0; i < num_tests; i++) {
        for (int j = 0; j < num_tests; j++) {
            total += pattern2_if_else_merge(test_values[i], test_values[j]);
        }
    }
    
    // Pattern 3: Ternary
    for (int i = 0; i < num_tests; i++) {
        total += pattern3_ternary_phi(test_values[i], test_values[(i+1)%num_tests]);
    }
    
    // Pattern 4: Nested loops
    total += pattern4_nested_loops(10);
    
    // Pattern 5: Switch
    for (int i = 0; i < 5; i++) {
        total += pattern5_switch_phi(i);
    }
    
    // Pattern 6: Function call
    for (int i = 0; i < num_tests; i++) {
        total += pattern6_function_call(test_values[i]);
    }
    
    // Pattern 7: Complex flow
    for (int i = 0; i < num_tests; i++) {
        for (int j = 0; j < num_tests; j++) {
            for (int k = 0; k < num_tests; k++) {
                total += pattern7_complex_flow(test_values[i], test_values[j], test_values[k]);
            }
        }
    }
    
    // Main function also has a loop with flag pattern
    int main_flag = 0;
    int main_counter = 0;
    while (!main_flag) {
        main_counter++;
        if (main_counter >= 5) {
            main_flag = 1;
        }
        int mf1 = main_flag;
        if (mf1 == 0) {  // Conditional in main
            total += main_counter;
        }
    }
    
    printf("Total result: %d\n", total);
    
    // Verify computation
    int expected_min = 5000;  // Minimum expected based on patterns
    if (total > expected_min) {
        printf("Computation successful - large result indicates all patterns executed\n");
    } else {
        printf("Warning: Result smaller than expected\n");
    }
    
    return 0;
}
