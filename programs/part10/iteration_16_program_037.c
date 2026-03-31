#include <stdio.h>
#include <stdlib.h>

/* Helper function to create SSA copy chains */
static inline int pass_through(int x) { return x; }
static inline int copy_value(int x) { int y = x; return y; }

/* Pattern 1: Loop exit condition depends on PHI-assigned flag */
int pattern1_loop_exit(int iterations) {
    int flag = 0;  // Initial value
    int count = 0;
    int i = 0;
    
    while (i < iterations) {
        // PHI node for flag at loop header
        int current_flag = flag;
        
        // Create copy chain
        int temp1 = current_flag;
        int temp2 = pass_through(temp1);
        int temp3 = copy_value(temp2);
        
        // Conditional branch comparing PHI-derived value against 0
        if (temp3 == 0) {  // Line should trigger uncovered code
            count += i;
        }
        
        // Update flag based on condition (creates PHI for next iteration)
        if (i > iterations / 2) {
            flag = 1;
        } else {
            flag = 0;
        }
        
        i++;
    }
    
    // Another conditional using the final flag value
    int final_check = flag;
    if (final_check != 1) {  // Compare against 1
        count *= 2;
    }
    
    return count;
}

/* Pattern 2: If-else chain setting 0/1, then conditional */
int pattern2_if_else_merge(int a, int b) {
    int result;
    
    // Complex control flow to ensure annotation
    if (a > b) {
        if (a % 2 == 0) {
            result = 1;
        } else {
            result = 0;
        }
    } else {
        if (b % 3 == 0) {
            result = 1;
        } else {
            result = 0;
        }
    }
    
    // PHI node at merge point for 'result'
    int phi_result = result;
    
    // Create multiple copy levels
    int level1 = phi_result;
    int level2 = pass_through(level1);
    int level3 = copy_value(level2);
    int level4 = pass_through(level3);
    
    // Conditional branch on the copy chain
    if (level4) {  // Implicit comparison against 0
        return a + b;
    } else {
        return a - b;
    }
}

/* Pattern 3: Switch-case setting 0/1 with nested loops */
int pattern3_switch_nested(int x) {
    int flag = 0;
    
    switch (x % 4) {
        case 0:
            flag = 1;
            break;
        case 1:
            flag = 0;
            break;
        case 2:
            flag = 1;
            break;
        case 3:
            flag = 0;
            break;
    }
    
    // PHI from switch merge
    int switch_result = flag;
    
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        // Nested control flow
        int inner_flag = switch_result;
        
        for (int j = 0; j < 5; j++) {
            // Multiple copy operations
            int chain1 = inner_flag;
            int chain2 = chain1;
            int chain3 = pass_through(chain2);
            
            // Conditional in nested loop
            if (chain3 == 1) {  // Compare against 1
                sum += i * j;
            } else {
                sum -= i;
            }
        }
        
        // Modify flag for next iteration (creates loop PHI)
        if (i % 3 == 0) {
            switch_result = 1;
        }
    }
    
    return sum;
}

/* Pattern 4: Ternary operator producing 0/1 */
int pattern4_ternary_chain(int x, int y) {
    // Ternary creates value 0 or 1
    int ternary_result = (x > y) ? 1 : 0;
    
    // Multiple assignments creating SSA chain
    int a = ternary_result;
    int b = a;
    int c = pass_through(b);
    int d = copy_value(c);
    int e = pass_through(d);
    
    // Complex control structure
    int total = 0;
    for (int i = 0; i < x; i++) {
        // Use the copy chain in condition
        if (e != 0) {  // Compare against 0
            total += y;
        } else {
            total -= i;
        }
        
        // Nested if to create more basic blocks
        if (i % 2 == 0) {
            int f = e;
            if (f == 1) {  // Another comparison against 1
                total *= 2;
            }
        }
    }
    
    return total;
}

/* Pattern 5: Multiple PHIs in loop header */
int pattern5_multiple_phis(int n) {
    int flag1 = 0, flag2 = 1;
    int result = 0;
    
    for (int i = 0; i < n; i++) {
        // Both flags have PHIs at loop header
        int current_flag1 = flag1;
        int current_flag2 = flag2;
        
        // Combine flags
        int combined = current_flag1 & current_flag2;
        
        // Copy chain
        int chain_start = combined;
        int chain_mid = pass_through(chain_start);
        int chain_end = chain_mid;
        
        // Conditional with both 0 and 1 comparisons
        if (chain_end == 0) {
            result += i;
        }
        
        if (chain_start != 1) {
            result -= 1;
        }
        
        // Update flags for next iteration
        flag1 = (i % 3 == 0) ? 1 : 0;
        flag2 = (i % 5 == 0) ? 1 : 0;
    }
    
    return result;
}

/* Main function with varied control flow */
int main() {
    int total = 0;
    
    // Array to vary inputs
    int inputs[] = {10, 20, 30, 40, 50};
    
    // Main loop with flag-based exit (mimics pattern at top level)
    int done = 0;
    int idx = 0;
    
    while (!done) {  // Implicit comparison against 0
        int current_input = inputs[idx];
        
        // Call each pattern function
        total += pattern1_loop_exit(current_input);
        total += pattern2_if_else_merge(current_input, current_input / 2);
        total += pattern3_switch_nested(current_input);
        total += pattern4_ternary_chain(current_input, current_input + 5);
        total += pattern5_multiple_phis(current_input % 8);
        
        // Update flag for loop exit (creates PHI)
        idx++;
        if (idx >= 5) {
            done = 1;  // Will be compared against 0 in next iteration
        } else {
            done = 0;
        }
        
        // Additional copy chain for the flag
        int flag_copy = done;
        int flag_copy2 = pass_through(flag_copy);
        
        // Extra conditional in main loop
        if (flag_copy2 == 1) {
            total += 1000;
        }
    }
    
    // Final conditional with copy chain
    int final_val = total;
    int check1 = final_val;
    int check2 = copy_value(check1);
    
    if (check2 > 0) {  // Not 0/1 comparison, but ensures code is live
        printf("Result: %d\n", total);
    } else {
        printf("Zero result: %d\n", total);
    }
    
    // Additional test cases with different branch patterns
    for (int i = 0; i < 3; i++) {
        int test_flag = (i % 2 == 0) ? 1 : 0;
        int chain_a = test_flag;
        int chain_b = chain_a;
        
        if (chain_b) {  // Implicit != 0
            printf("Iteration %d: flag is true\n", i);
        }
    }
    
    return 0;
}
