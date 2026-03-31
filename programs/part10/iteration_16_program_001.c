#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Loop exit condition depends on PHI-assigned flag */
int pattern1_loop_flag(int iterations) {
    int flag = 0;  // Initial value
    int count = 0;
    
    while (!flag) {  // Conditional on PHI-derived value (flag != 0)
        count++;
        if (count >= iterations) {
            flag = 1;  // One branch sets flag to 1
        } else {
            flag = 0;  // Other branch sets flag to 0
        }
        // PHI node at loop header merges flag values
    }
    return count;
}

/* Pattern 2: If-else chain sets boolean, later conditional uses it */
int pattern2_if_else_merge(int x, int y) {
    int result;
    
    // First if-else sets result to 0 or 1
    if (x > y) {
        result = 1;
    } else {
        result = 0;
    }
    
    // SSA copy chain
    int temp1 = result;
    int temp2 = temp1;
    
    // Conditional on the PHI-derived value
    if (temp2 == 1) {  // Explicit comparison with 1
        return x * 2;
    } else {
        return y * 3;
    }
}

/* Pattern 3: Nested control with ternary operator */
int pattern3_ternary_phi(int a, int b, int c) {
    int flag;
    
    // Complex control flow that creates PHI
    if (a > 0) {
        if (b > 0) {
            flag = 1;
        } else {
            flag = (c != 0) ? 1 : 0;  // Ternary creates PHI
        }
    } else {
        flag = 0;
    }
    
    // Multiple SSA copies
    int f1 = flag;
    int f2 = f1;
    int f3 = f2;
    
    // Conditional with implicit boolean check (compares with 0)
    if (f3) {  // Equivalent to f3 != 0
        return a + b + c;
    }
    return a - b - c;
}

/* Pattern 4: Switch-case setting 0/1 value */
int pattern4_switch_phi(int option) {
    int value = 0;  // Default
    
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
            value = 0;  // PHI merges all these assignments
    }
    
    // Chain of assignments
    int v1 = value;
    int v2 = v1;
    
    // Conditional comparing with 0
    if (v2 == 0) {
        return option * 10;
    } else {
        return option * 20;
    }
}

/* Pattern 5: Loop with multiple exit conditions */
int pattern5_complex_loop(int limit) {
    int done = 0;
    int i = 0;
    int sum = 0;
    
    while (i < limit) {
        sum += i;
        i++;
        
        // Complex condition setting done flag
        if (sum > 100) {
            done = 1;
        } else if (i % 7 == 0) {
            done = 0;
        } else {
            done = (sum % 3 == 0) ? 1 : 0;
        }
        
        // Conditional on PHI-derived done flag
        int d1 = done;
        int d2 = d1;
        if (d2 != 0) {  // Comparison with 0
            break;
        }
    }
    return sum;
}

/* Pattern 6: Function call propagation through SSA copies */
static inline int propagate(int x) {
    return x;  // Simple propagation
}

int pattern6_function_prop(int x, int y) {
    int flag;
    
    if (x > y) {
        flag = 1;
    } else {
        flag = 0;
    }
    
    // Propagate through function call (inlined)
    int f1 = propagate(flag);
    int f2 = propagate(f1);
    
    // Conditional with explicit 1 comparison
    if (f2 == 1) {
        return x - y;
    } else {
        return y - x;
    }
}

/* Pattern 7: Multiple nested loops with PHI flags */
int pattern7_nested_loops(int n) {
    int total = 0;
    int outer_flag = 0;
    
    for (int i = 0; i < n; i++) {
        int inner_flag = 0;
        
        for (int j = 0; j < n; j++) {
            total += i * j;
            
            if (total > 1000) {
                inner_flag = 1;
            } else {
                inner_flag = 0;
            }
            
            // SSA copy chain
            int f1 = inner_flag;
            int f2 = f1;
            
            // Conditional in inner loop
            if (f2 != 0) {
                break;
            }
        }
        
        // Update outer flag based on inner results
        if (total > 5000) {
            outer_flag = 1;
        } else {
            outer_flag = 0;
        }
        
        int of1 = outer_flag;
        if (of1 == 1) {
            break;
        }
    }
    return total;
}

/* Main function with diverse control flow */
int main() {
    int total = 0;
    
    // Array to provide varied inputs
    int inputs[] = {5, 10, 3, 8, 12, 7};
    int n = sizeof(inputs) / sizeof(inputs[0]);
    
    // Main loop with flag-based exit condition (mimics target pattern)
    int main_flag = 0;
    int idx = 0;
    
    while (!main_flag) {  // Conditional on PHI-derived value
        int val = inputs[idx];
        
        // Call each pattern function
        total += pattern1_loop_flag(val % 5 + 1);
        total += pattern2_if_else_merge(val, val * 2);
        total += pattern3_ternary_phi(val, val + 1, val - 1);
        total += pattern4_switch_phi(val % 4);
        total += pattern5_complex_loop(val % 10 + 5);
        total += pattern6_function_prop(val, val / 2 + 1);
        total += pattern7_nested_loops(val % 3 + 2);
        
        idx++;
        
        // Set flag based on condition (creates PHI)
        if (idx >= n || total > 10000) {
            main_flag = 1;
        } else {
            main_flag = 0;
        }
        
        // SSA copy chain in main
        int mf1 = main_flag;
        int mf2 = mf1;
        // Conditional will be analyzed
    }
    
    printf("Final result: %d\n", total);
    
    // Additional test cases to ensure coverage
    for (int i = 0; i < 3; i++) {
        int test_flag = (i % 2 == 0) ? 1 : 0;
        int tf1 = test_flag;
        if (tf1 == 0) {
            total += i;
        } else {
            total -= i;
        }
    }
    
    printf("Adjusted result: %d\n", total);
    return 0;
}
