#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Loop exit condition depends on PHI-assigned flag */
int pattern1_loop_flag(int iterations) {
    int flag = 0;
    int count = 0;
    
    while (!flag) {  // This creates a PHI for 'flag' at loop header
        count++;
        if (count >= iterations) {
            flag = 1;  // Sets flag inside loop
        }
    }
    
    // Create SSA copy chain
    int a = flag;
    int b = a;
    int c = b;
    
    // Conditional with constant RHS (0)
    if (c == 0) {
        return 1;
    }
    return 0;
}

/* Pattern 2: If-else chain setting boolean, then conditional check */
int pattern2_if_else_phi(int x) {
    int value;
    
    // This creates a PHI node at the merge point
    if (x > 0) {
        value = 1;
    } else {
        value = 0;
    }
    
    // Multiple SSA copies
    int tmp1 = value;
    int tmp2 = tmp1;
    
    // Conditional with constant RHS (1)
    if (tmp2 == 1) {
        return x * 2;
    }
    return x;
}

/* Pattern 3: Nested control with ternary operator */
int pattern3_ternary_phi(int a, int b) {
    // Ternary creates PHI
    int cmp_result = (a > b) ? 1 : 0;
    
    // Pass through function-like macro (inlined)
    #define PASS(v) (v)
    int passed = PASS(cmp_result);
    
    // Implicit boolean check (equivalent to != 0)
    if (passed) {
        return a - b;
    }
    return b - a;
}

/* Pattern 4: Switch-case setting 0/1 value */
int pattern4_switch_phi(char op) {
    int should_invert;
    
    switch (op) {
        case 'A':
        case 'B':
            should_invert = 1;
            break;
        case 'C':
        case 'D':
            should_invert = 0;
            break;
        default:
            should_invert = 0;
    }
    
    // Chain of assignments
    int x = should_invert;
    int y = x;
    
    // Conditional with != 1 check
    if (y != 1) {
        return 100;
    }
    return 200;
}

/* Pattern 5: Complex loop with inner condition */
int pattern5_nested_control(int n) {
    int total = 0;
    int continue_loop = 1;
    
    for (int i = 0; i < n && continue_loop; i++) {
        int inner_flag = 0;
        
        // Inner if-else creates PHI
        if (i % 3 == 0) {
            inner_flag = 1;
        } else {
            inner_flag = 0;
        }
        
        // Copy through temporary
        int check = inner_flag;
        
        // Conditional in inner block
        if (check == 1) {
            total += i * 2;
        } else {
            total += i;
        }
        
        // Update outer loop condition
        if (total > 1000) {
            continue_loop = 0;  // Creates PHI for continue_loop
        }
    }
    
    // Final check on PHI-derived value
    int final_check = continue_loop;
    if (final_check == 0) {
        total += 10000;
    }
    
    return total;
}

/* Pattern 6: Multiple PHIs in same basic block */
int pattern6_multiple_phis(int x, int y) {
    int a, b;
    
    if (x > y) {
        a = 1;
        b = 0;
    } else {
        a = 0;
        b = 1;
    }
    
    // Both a and b come from PHIs
    int sum = a + b;
    
    // Check on PHI-derived value
    int check_a = a;
    if (check_a == 1) {
        return x + y;
    }
    
    int check_b = b;
    if (check_b == 0) {
        return x - y;
    }
    
    return 0;
}

/* Helper function to create copy chain */
static inline int propagate(int val) {
    int temp = val;
    return temp;
}

/* Pattern 7: Using inline function for copy chain */
int pattern7_function_copy(int val) {
    int original;
    
    // Create PHI
    if (val % 2 == 0) {
        original = 1;
    } else {
        original = 0;
    }
    
    // Pass through inline function multiple times
    int p1 = propagate(original);
    int p2 = propagate(p1);
    int p3 = propagate(p2);
    
    // Final conditional
    if (p3 == 0) {
        return val * 3;
    }
    return val * 5;
}

int main() {
    int total = 0;
    
    // Pattern 1: Loop with flag
    total += pattern1_loop_flag(10);
    
    // Pattern 2: If-else phi
    for (int i = -5; i < 5; i++) {
        total += pattern2_if_else_phi(i);
    }
    
    // Pattern 3: Ternary
    total += pattern3_ternary_phi(100, 50);
    total += pattern3_ternary_phi(50, 100);
    
    // Pattern 4: Switch
    char ops[] = {'A', 'B', 'C', 'D', 'E'};
    for (int i = 0; i < 5; i++) {
        total += pattern4_switch_phi(ops[i]);
    }
    
    // Pattern 5: Nested control
    total += pattern5_nested_control(50);
    
    // Pattern 6: Multiple PHIs
    total += pattern6_multiple_phis(10, 20);
    total += pattern6_multiple_phis(20, 10);
    
    // Pattern 7: Function copy chain
    for (int i = 0; i < 20; i++) {
        total += pattern7_function_copy(i);
    }
    
    // Main loop with flag pattern (mimics target at top level)
    int main_flag = 0;
    int main_counter = 0;
    
    while (!main_flag) {
        main_counter++;
        
        // Create PHI for flag
        if (main_counter % 3 == 0) {
            main_flag = 1;
        } else {
            main_flag = 0;
        }
        
        // Copy chain
        int f1 = main_flag;
        int f2 = f1;
        
        // Conditional on PHI-derived value with constant 0
        if (f2 == 0) {
            total += main_counter;
        } else {
            total -= main_counter;
        }
        
        if (main_counter >= 100) break;
    }
    
    printf("Final result: %d\n", total);
    
    // Verify computation
    if (total != 10867) {
        printf("Unexpected result! Expected 10867, got %d\n", total);
        return 1;
    }
    
    return 0;
}
