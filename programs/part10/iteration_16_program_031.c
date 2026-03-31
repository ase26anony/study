#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Loop exit condition depends on PHI-assigned flag */
int pattern1_loop_exit_flag(int iterations) {
    int flag = 0;  // Initial value
    int count = 0;
    
    while (!flag) {  // Conditional on PHI-derived value (flag)
        count++;
        if (count >= iterations) {
            flag = 1;  // Sets flag inside loop - creates PHI at loop header
        }
    }
    return count;
}

/* Pattern 2: If-else chain sets boolean, later conditional uses it */
int pattern2_if_else_merge(int x, int y) {
    int result;
    
    // First if-else sets a boolean value
    if (x > y) {
        result = 1;  // Could be PHI at merge point
    } else {
        result = 0;  // Could be PHI at merge point
    }
    
    // Introduce SSA copy chain
    int temp1 = result;
    int temp2 = temp1;
    
    // Conditional on the PHI-derived value through copy chain
    if (temp2 == 1) {  // Compares against constant 1
        return x * 2;
    } else {
        return y * 2;
    }
}

/* Pattern 3: Nested control with ternary operator */
int pattern3_ternary_phi(int a, int b, int c) {
    // Ternary creates PHI-like assignment
    int selector = (a > b) ? 1 : 0;
    
    // Multiple SSA copies
    int copy1 = selector;
    int copy2 = copy1;
    int copy3 = copy2;
    
    // Conditional in nested loop
    int sum = 0;
    for (int i = 0; i < c; i++) {
        if (copy3 != 0) {  // Compares against constant 0
            sum += a;
        } else {
            sum += b;
        }
        
        // Modify selector inside loop to create PHI
        if (i > c/2) {
            selector = 0;
        }
    }
    return sum;
}

/* Pattern 4: Switch-case setting 0/1 value */
int pattern4_switch_phi(int option) {
    int flag;
    
    switch (option % 3) {
        case 0:
            flag = 1;
            break;
        case 1:
            flag = 0;
            break;
        case 2:
            flag = 1;
            break;
    }
    
    // Pass through function-like inline expansion
    int intermediate = flag;
    intermediate = intermediate;  // Simple assignment chain
    
    // Multiple conditionals with the PHI-derived value
    if (intermediate == 1) {
        return option * 10;
    }
    
    // Another conditional in else branch
    if (!intermediate) {  // Equivalent to intermediate == 0
        return option * 5;
    }
    
    return option;
}

/* Pattern 5: Complex loop with multiple PHI nodes */
int pattern5_complex_loop(int n) {
    int state = 0;
    int total = 0;
    
    for (int i = 0; i < n; i++) {
        // Inner if-else creates PHI for state
        if (i % 2 == 0) {
            state = 1;
        } else {
            state = 0;
        }
        
        // Copy through temporary
        int check = state;
        
        // Conditional on the copied PHI value
        if (check) {  // Implicit comparison against 0
            total += i * 2;
        } else {
            total += i;
        }
        
        // Nested conditional that might create annotated BB
        if (total > 100) {
            int inner_check = check;
            if (inner_check == 1) {
                total -= 10;
            }
        }
    }
    return total;
}

/* Pattern 6: Function with return value used as boolean */
static inline int returns_one_or_zero(int x) {
    return (x % 2 == 0) ? 1 : 0;
}

int pattern6_function_call(int x) {
    // Get value from function (could be inlined)
    int val = returns_one_or_zero(x);
    
    // Chain of assignments
    int a = val;
    int b = a;
    
    // Conditional with the value
    if (b == 0) {
        return x * 3;
    } else {
        return x * 7;
    }
}

/* Main function with rich control flow */
int main() {
    int total = 0;
    int array[10];
    
    // Initialize array
    for (int i = 0; i < 10; i++) {
        array[i] = i * 2;
    }
    
    // Pattern 1: Loop with exit flag
    total += pattern1_loop_exit_flag(5);
    
    // Pattern 2: If-else merge
    total += pattern2_if_else_merge(10, 5);
    total += pattern2_if_else_merge(5, 10);
    
    // Pattern 3: Ternary with nested loops
    total += pattern3_ternary_phi(3, 7, 4);
    
    // Pattern 4: Switch-case
    for (int i = 0; i < 4; i++) {
        total += pattern4_switch_phi(i);
    }
    
    // Pattern 5: Complex loop
    total += pattern5_complex_loop(8);
    
    // Pattern 6: Function call pattern
    for (int i = 0; i < 6; i++) {
        total += pattern6_function_call(i);
    }
    
    // Main loop with flag-based exit (mimics pattern at top level)
    int main_flag = 0;
    int main_counter = 0;
    while (main_flag == 0) {  // Explicit comparison against 0
        main_counter++;
        total += main_counter;
        
        // Create PHI for flag
        if (main_counter >= 3) {
            main_flag = 1;  // Will create PHI at loop header
        } else {
            main_flag = 0;  // Alternative assignment
        }
    }
    
    // Final conditional with SSA copy chain
    int final_check = main_flag;
    int another_copy = final_check;
    if (another_copy != 0) {  // Comparison against 0
        total += 1000;
    }
    
    printf("Result: %d\n", total);
    
    // Additional calls to ensure all code paths are reachable
    // with different inputs to exercise various branches
    pattern1_loop_exit_flag(1);
    pattern1_loop_exit_flag(10);
    pattern2_if_else_merge(0, 0);
    pattern3_ternary_phi(1, 1, 1);
    pattern4_switch_phi(100);
    pattern5_complex_loop(1);
    pattern5_complex_loop(20);
    pattern6_function_call(11);
    
    return 0;
}
