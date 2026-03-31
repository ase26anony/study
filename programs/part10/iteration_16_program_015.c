#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Loop exit condition depends on PHI-assigned flag */
int pattern1_loop_exit(int iterations) {
    int flag = 0;  // Initial value
    int count = 0;
    int i = 0;
    
    while (!flag) {  // Branch on PHI-derived value (flag)
        count++;
        i++;
        // PHI node at loop header: flag = φ(0, new_flag)
        int new_flag = (i >= iterations) ? 1 : 0;
        
        // Create SSA copy chain
        int temp1 = new_flag;
        int temp2 = temp1;
        flag = temp2;  // Assignment creates PHI at next iteration
    }
    return count;
}

/* Pattern 2: If-else chain sets boolean, later conditional uses it */
int pattern2_if_else_merge(int a, int b) {
    int result;
    
    // First if-else sets a 0/1 value
    if (a > b) {
        result = 1;
    } else {
        result = 0;
    }
    
    // SSA copy chain
    int copy1 = result;
    int copy2 = copy1;
    int copy3 = copy2;
    
    // Conditional branch on PHI-derived value
    if (copy3 == 1) {  // Compare against constant 1
        return a * 2;
    } else {
        return b * 3;
    }
}

/* Pattern 3: Ternary operator with SSA copies */
int pattern3_ternary_chain(int x, int y) {
    // Ternary creates PHI-like value
    int choice = (x % 2 == 0) ? 1 : 0;
    
    // Multiple SSA copies
    int chain1 = choice;
    int chain2 = chain1;
    int chain3 = chain2;
    
    // Branch with constant comparison
    if (chain3 != 0) {  // Compare against constant 0
        return x + y;
    } else {
        return x - y;
    }
}

/* Pattern 4: Nested loops with flag propagation */
int pattern4_nested_control(int n) {
    int total = 0;
    int outer_flag = 0;
    
    for (int i = 0; i < n; i++) {
        int inner_flag = (i % 3 == 0) ? 1 : 0;
        
        // SSA propagation
        int f1 = inner_flag;
        int f2 = f1;
        
        if (f2 == 1) {  // Branch on PHI-derived value
            total += i * 2;
        } else {
            total += i;
        }
        
        // Update outer flag based on condition
        if (total > 100) {
            outer_flag = 1;
        } else {
            outer_flag = 0;
        }
    }
    
    // Final branch on outer flag (PHI from loop)
    int final_copy = outer_flag;
    if (final_copy) {  // Implicit comparison against 0
        return total * 2;
    }
    return total;
}

/* Pattern 5: Switch-case setting 0/1 value */
int pattern5_switch_phi(int val) {
    int indicator = 0;
    
    switch (val % 4) {
        case 0:
            indicator = 1;
            break;
        case 1:
            indicator = 0;
            break;
        case 2:
            indicator = 1;
            break;
        case 3:
            indicator = 0;
            break;
    }
    
    // Propagate through assignments
    int a = indicator;
    int b = a;
    
    // Conditional branch
    if (b == 0) {  // Compare against constant 0
        return val * 10;
    } else {
        return val * 20;
    }
}

/* Helper function to create SSA copy chain */
static inline int propagate_value(int v) {
    int t1 = v;
    int t2 = t1;
    return t2;
}

/* Pattern 6: Using inline function for SSA propagation */
int pattern6_function_prop(int a, int b) {
    int cmp_result = (a > b) ? 1 : 0;
    
    // Pass through function (creates SSA copies)
    int propagated = propagate_value(cmp_result);
    
    // Branch on the result
    if (propagated == 1) {  // Compare against constant 1
        return a * b;
    }
    return a + b;
}

/* Main function with varied control flow */
int main(int argc, char **argv) {
    int total = 0;
    
    // Initialize with command line or default
    int iterations = (argc > 1) ? atoi(argv[1]) : 10;
    int seed = (argc > 2) ? atoi(argv[2]) : 42;
    
    // Pattern 1: Loop with PHI-derived exit condition
    total += pattern1_loop_exit(iterations);
    
    // Pattern 2: If-else merge point
    total += pattern2_if_else_merge(seed, seed * 2);
    
    // Pattern 3: Ternary operator chain
    total += pattern3_ternary_chain(seed, iterations);
    
    // Pattern 4: Nested control flow
    total += pattern4_nested_control(iterations);
    
    // Pattern 5: Switch to 0/1
    total += pattern5_switch_phi(seed);
    
    // Pattern 6: Function propagation
    total += pattern6_function_prop(seed, iterations);
    
    // Additional top-level loop with flag condition (mimics pattern)
    int main_flag = 0;
    int loop_count = 0;
    
    while (main_flag == 0) {  // Explicit comparison against 0
        loop_count++;
        
        // PHI-like update
        int new_flag = (loop_count >= 5) ? 1 : 0;
        
        // SSA copy chain
        int chain1 = new_flag;
        int chain2 = chain1;
        main_flag = chain2;
        
        total += loop_count;
    }
    
    printf("Result: %d\n", total);
    printf("Loop iterations in main: %d\n", loop_count);
    
    return 0;
}
