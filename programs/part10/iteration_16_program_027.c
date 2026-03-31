#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Loop exit condition depends on PHI-assigned flag */
int pattern1_loop_flag(int iterations) {
    int flag = 0;
    int count = 0;
    
    while (!flag) {  // Branch compares flag (PHI result) against 0
        count++;
        if (count >= iterations) {
            flag = 1;  // Sets flag in loop body
        }
    }
    return count;
}

/* Pattern 2: If-else chain sets boolean, later conditional uses it */
int pattern2_if_else_merge(int a, int b) {
    int result;
    
    // PHI node at control flow merge
    if (a > b) {
        result = 1;
    } else {
        result = 0;
    }
    
    // SSA copy chain
    int temp1 = result;
    int temp2 = temp1;
    
    // Conditional branch comparing PHI-derived value against 0
    if (temp2 == 0) {  // Should trigger the uncovered logic
        return b - a;
    } else {
        return a - b;
    }
}

/* Pattern 3: Nested control with SSA copies through function */
static inline int pass_through(int v) {
    int copy = v;  // Creates SSA copy
    return copy;
}

int pattern3_nested_control(int n) {
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        int flag;
        
        // PHI node in inner control flow
        if (i % 2 == 0) {
            flag = 1;
        } else {
            flag = 0;
        }
        
        // Multiple SSA copies
        int a = flag;
        int b = pass_through(a);
        
        // Conditional with constant RHS (1)
        if (b == 1) {  // Should trigger uncovered logic
            sum += i * 2;
        } else {
            sum += i;
        }
    }
    return sum;
}

/* Pattern 4: Switch-case sets 0/1 value */
int pattern4_switch_phi(int x) {
    int code;
    
    switch (x % 3) {
        case 0: code = 1; break;
        case 1: code = 0; break;
        case 2: code = 1; break;
        default: code = 0; break;
    }
    
    // SSA copy chain
    int y = code;
    int z = y;
    
    // Implicit boolean check (compares against 0)
    if (z) {  // Equivalent to if (z != 0)
        return x * 2;
    }
    return x / 2;
}

/* Pattern 5: Ternary operator creates PHI */
int pattern5_ternary_phi(int a, int b, int c) {
    // Ternary creates PHI node
    int is_max = (a > b && a > c) ? 1 : 0;
    
    // Chain of assignments
    int val1 = is_max;
    int val2 = val1;
    int val3 = val2;
    
    // Conditional with != 1 comparison
    if (val3 != 1) {  // Compares against constant 1
        return b + c;
    }
    return a;
}

/* Pattern 6: Complex nested loops with flag propagation */
int pattern6_complex_nested(int size) {
    int total = 0;
    int outer_flag = 0;
    
    for (int i = 0; i < size; i++) {
        int inner_flag = 0;
        
        for (int j = 0; j < size; j++) {
            // PHI node in inner loop
            if ((i + j) % 3 == 0) {
                inner_flag = 1;
            } else {
                inner_flag = 0;
            }
            
            // SSA propagation
            int temp = inner_flag;
            
            // Multiple conditionals in nested structure
            if (temp == 0) {
                total += i;
            } else {
                total += j;
            }
            
            if (j > size / 2) {
                outer_flag = 1;  // Affects outer loop
            }
        }
        
        // Use outer_flag in condition
        int check = outer_flag;
        if (check == 1) {  // PHI-derived comparison
            total += 1000;
        }
    }
    return total;
}

/* Main function with varied control flow */
int main() {
    int result = 0;
    
    // Pattern 1: Loop with flag
    result += pattern1_loop_flag(10);
    
    // Pattern 2: If-else merge
    result += pattern2_if_else_merge(20, 15);
    
    // Pattern 3: Nested with function call
    result += pattern3_nested_control(8);
    
    // Pattern 4: Switch-case
    result += pattern4_switch_phi(25);
    
    // Pattern 5: Ternary
    result += pattern5_ternary_phi(10, 20, 15);
    
    // Pattern 6: Complex nested
    result += pattern6_complex_nested(5);
    
    // Additional pattern in main itself
    int main_flag = 0;
    int main_counter = 0;
    
    // Loop in main using flag derived from condition
    while (main_flag == 0) {  // Direct comparison with 0
        main_counter++;
        
        // Create PHI for flag
        if (main_counter >= 3) {
            main_flag = 1;  // Will create PHI at loop header
        }
        
        // SSA copy chain
        int flag_copy = main_flag;
        if (flag_copy == 1) {  // Comparison with constant 1
            result += 100;
        }
    }
    
    printf("Final result: %d\n", result);
    
    // Run multiple times with different inputs
    for (int run = 0; run < 3; run++) {
        int temp = 0;
        temp += pattern1_loop_flag(5 + run);
        temp += pattern2_if_else_merge(run * 10, run * 5);
        printf("Run %d: %d\n", run, temp);
    }
    
    return 0;
}
