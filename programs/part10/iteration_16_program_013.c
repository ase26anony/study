#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Loop exit condition depends on PHI-assigned flag */
int pattern1_loop_flag(int iterations) {
    int flag = 0;
    int count = 0;
    
    while (!flag) {  // Conditional on PHI-derived value (flag)
        count++;
        if (count >= iterations) {
            flag = 1;  // Sets flag in loop body
        }
    }
    return count;
}

/* Pattern 2: If-else chain assigns 0/1, then conditional on that value */
int pattern2_if_else_phi(int a, int b) {
    int result;
    
    // PHI node at control flow merge
    if (a > b) {
        result = 1;
    } else {
        result = 0;
    }
    
    // Introduce SSA copy chain
    int temp1 = result;
    int temp2 = temp1;
    
    // Conditional on PHI-derived value with copy chain
    if (temp2 == 1) {  // Compares against constant 1
        return a + b;
    } else {
        return a - b;
    }
}

/* Pattern 3: Nested control with ternary operator */
int pattern3_ternary_phi(int x, int y) {
    // Ternary creates PHI-like assignment
    int is_positive = (x + y) > 0 ? 1 : 0;
    
    // Multiple SSA copies
    int copy1 = is_positive;
    int copy2 = copy1;
    int copy3 = copy2;
    
    // Conditional with constant 0 comparison
    if (copy3 != 0) {  // Compares against constant 0
        return x * y;
    }
    return x / (y ? y : 1);
}

/* Pattern 4: Switch-case setting 0/1 value */
int pattern4_switch_phi(int op, int a, int b) {
    int flag;
    
    switch (op) {
        case 1: flag = 1; break;
        case 2: flag = 0; break;
        case 3: flag = 1; break;
        default: flag = 0; break;
    }
    
    // SSA copy through function-like inline expansion
    int tmp = flag;
    tmp = tmp;  // Identity assignment
    int final = tmp;
    
    // Conditional branch
    if (final == 1) {
        return a * b;
    } else {
        return a + b;
    }
}

/* Pattern 5: Complex loop with inner condition */
int pattern5_nested_control(int n) {
    int total = 0;
    int continue_loop = 1;
    int i = 0;
    
    while (continue_loop) {  // Conditional on PHI-derived value
        for (int j = 0; j < 5; j++) {
            if ((i + j) % 3 == 0) {
                total += i;
            } else {
                total += j;
            }
        }
        
        i++;
        
        // Set loop control variable based on condition
        if (i >= n) {
            continue_loop = 0;  // Will become PHI at loop header
        } else if (total > 1000) {
            continue_loop = 0;
        } else {
            continue_loop = 1;
        }
    }
    return total;
}

/* Pattern 6: Multiple PHI nodes in loop header */
int pattern6_multi_phi(int limit) {
    int a = 0, b = 1;
    int sum = 0;
    
    for (int i = 0; i < limit; i++) {
        // Both a and b are PHI nodes in loop header
        int temp = a;
        a = b;
        b = temp + b;
        
        // Conditional on PHI-derived value with copy
        int check = a;
        if (check % 2 == 0) {  // Not 0/1 comparison, but creates rich CFG
            sum += a;
        }
        
        // Additional conditional with 0/1 comparison
        int even_flag = (a % 2 == 0) ? 1 : 0;
        int flag_copy = even_flag;
        if (flag_copy == 1) {
            sum += 10;
        }
    }
    return sum;
}

/* Helper function to create SSA copy chain */
static inline int propagate_value(int v) {
    int x = v;
    int y = x;
    return y;
}

/* Pattern 7: Using inline function for SSA copies */
int pattern7_inline_copy(int x) {
    int state;
    
    if (x > 100) {
        state = 1;
    } else if (x > 50) {
        state = 0;
    } else {
        state = 1;
    }
    
    // Multiple levels of propagation
    int level1 = propagate_value(state);
    int level2 = propagate_value(level1);
    
    // Final conditional
    if (level2 == 0) {
        return x * 2;
    }
    return x / 2;
}

/* Main function with varied control flow */
int main() {
    int total = 0;
    int data[] = {10, 20, 30, 40, 50};
    int n = sizeof(data) / sizeof(data[0]);
    
    // Pattern 1: Loop with flag
    total += pattern1_loop_flag(5);
    printf("Pattern1 result: %d\n", pattern1_loop_flag(5));
    
    // Pattern 2: If-else phi
    for (int i = 0; i < n; i++) {
        total += pattern2_if_else_phi(data[i], data[(i + 1) % n]);
    }
    printf("Pattern2 accumulated: %d\n", total);
    
    // Pattern 3: Ternary phi
    total += pattern3_ternary_phi(100, -50);
    printf("Pattern3 result: %d\n", pattern3_ternary_phi(100, -50));
    
    // Pattern 4: Switch phi
    for (int op = 0; op < 4; op++) {
        total += pattern4_switch_phi(op, 15, 25);
    }
    printf("Pattern4 accumulated: %d\n", total);
    
    // Pattern 5: Nested control
    total += pattern5_nested_control(8);
    printf("Pattern5 result: %d\n", pattern5_nested_control(8));
    
    // Pattern 6: Multiple PHI nodes
    total += pattern6_multi_phi(10);
    printf("Pattern6 result: %d\n", pattern6_multi_phi(10));
    
    // Pattern 7: Inline copies
    for (int i = 0; i < n; i++) {
        total += pattern7_inline_copy(data[i] * 3);
    }
    printf("Pattern7 accumulated: %d\n", total);
    
    // Additional main loop with flag-based control (mimics target pattern)
    int main_flag = 0;
    int main_counter = 0;
    int threshold = 1000;
    
    while (main_flag == 0) {  // Direct comparison with constant 0
        main_counter += total % 17;
        
        // Create SSA copy chain
        int flag_copy = main_flag;
        int flag_copy2 = flag_copy;
        
        // Condition check through copies
        if (flag_copy2 != 0) {  // Comparison with constant 0
            break;
        }
        
        if (main_counter > threshold) {
            main_flag = 1;  // Will flow through PHI at loop header
        }
        
        // Additional branching for rich CFG
        if (main_counter % 7 == 0) {
            total += 7;
        } else if (main_counter % 5 == 0) {
            total += 5;
        } else {
            total += 1;
        }
    }
    
    printf("Final result: %d (counter: %d)\n", total, main_counter);
    
    return 0;
}
