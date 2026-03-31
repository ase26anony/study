#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Loop exit condition depends on PHI-assigned flag */
int pattern1_loop_with_phi_flag(int iterations) {
    int flag = 0;  // This becomes a PHI node at loop header
    int count = 0;
    int i = 0;
    
    while (!flag) {  // Branch comparing PHI-derived value against 0
        count++;
        i++;
        
        // Create SSA copy chain
        int temp1 = flag;
        int temp2 = temp1;
        
        // This will create a conditional using the copy chain
        if (temp2 == 0) {  // Should trigger the uncovered logic
            if (i >= iterations) {
                flag = 1;  // Updates PHI node
            }
        }
    }
    return count;
}

/* Pattern 2: If-else chain setting boolean, then conditional on copy */
int pattern2_ifelse_phi(int a, int b) {
    int result;
    
    // PHI node created at control flow merge
    if (a > b) {
        result = 1;
    } else {
        result = 0;
    }
    
    // Create copy chain
    int copy1 = result;
    int copy2 = copy1;
    int copy3 = copy2;
    
    // Conditional on copy chain value
    if (copy3 == 1) {  // Should trigger uncovered logic
        return a * 2;
    } else {
        return b * 2;
    }
}

/* Helper function to create SSA copy chain */
inline int propagate_value(int v) {
    int t1 = v;
    int t2 = t1;
    return t2;
}

/* Pattern 3: Ternary operator with function-call copy chain */
int pattern3_ternary_with_copy(int x, int y) {
    // Ternary creates PHI-like behavior
    int choice = (x % 2 == 0) ? 1 : 0;
    
    // Pass through function (inlined) creates copy chain
    int propagated = propagate_value(choice);
    
    // Multiple nested conditionals for rich control flow
    if (propagated == 0) {  // Should trigger uncovered logic
        for (int i = 0; i < x; i++) {
            if (i % 3 == 0) {
                y += i;
            } else {
                y -= i;
            }
        }
        return y;
    } else {
        for (int i = 0; i < y; i++) {
            if (i % 4 == 0) {
                x += i;
            }
        }
        return x;
    }
}

/* Pattern 4: Switch-case setting 0/1 with complex control flow */
int pattern4_switch_phi(int val) {
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
            // Fall through creates additional control flow
        case 3:
            // PHI node at merge point
            indicator = (val > 10) ? 1 : 0;
            break;
    }
    
    // Multiple copy assignments
    int a = indicator;
    int b = a;
    int c = b;
    
    // Nested loops with conditional on copy chain
    int sum = 0;
    for (int i = 0; i < val; i++) {
        for (int j = 0; j < 5; j++) {
            if (c == 1) {  // Should trigger uncovered logic
                sum += i * j;
            } else {
                sum -= i * j;
            }
            
            // Additional condition to create more basic blocks
            if (j % 2 == 0) {
                int d = c;
                if (d == 0) {  // Another candidate
                    sum += 1;
                }
            }
        }
    }
    
    return sum;
}

/* Pattern 5: Complex loop with multiple PHI nodes */
int pattern5_complex_loop(int n) {
    int state = 0;  // PHI at loop header
    int total = 0;
    int i = 0;
    
    while (i < n) {
        // Multiple basic blocks inside loop
        if (i % 3 == 0) {
            state = 1;
        } else if (i % 3 == 1) {
            state = 0;
        } else {
            // Keep current state
        }
        
        // Copy through temporary
        int current_state = state;
        
        // Branch on copy
        if (current_state == 1) {  // Should trigger uncovered logic
            total += i * 2;
        } else {
            total += i;
        }
        
        // Another conditional with different constant
        int check = current_state;
        if (check != 0) {  // Another candidate (check != 0 is check == 1)
            total += 10;
        }
        
        i++;
    }
    
    return total;
}

/* Main function with rich control flow */
int main() {
    int total = 0;
    
    // Initialize array for varied inputs
    int inputs[10];
    for (int i = 0; i < 10; i++) {
        inputs[i] = (i * 7) % 13;
    }
    
    // Pattern 1: Loop with phi flag
    for (int i = 0; i < 5; i++) {
        total += pattern1_loop_with_phi_flag(inputs[i] + 3);
    }
    
    // Pattern 2: If-else phi
    for (int i = 0; i < 5; i++) {
        total += pattern2_ifelse_phi(inputs[i], inputs[9 - i]);
    }
    
    // Pattern 3: Ternary with copy
    for (int i = 0; i < 5; i++) {
        total += pattern3_ternary_with_copy(inputs[i], inputs[i + 1]);
    }
    
    // Pattern 4: Switch phi
    for (int i = 0; i < 5; i++) {
        total += pattern4_switch_phi(inputs[i] + 5);
    }
    
    // Pattern 5: Complex loop
    for (int i = 0; i < 5; i++) {
        total += pattern5_complex_loop(inputs[i] + 2);
    }
    
    // Main function also has pattern with phi-derived flag
    int done = 0;
    int attempts = 0;
    
    while (!done) {  // Branch comparing against 0
        attempts++;
        
        // Create copy chain
        int flag_copy = done;
        int flag_copy2 = flag_copy;
        
        if (flag_copy2 == 0) {  // Should trigger uncovered logic in main
            if (attempts >= 3) {
                done = 1;
            }
            
            // Additional computation
            total += attempts * 100;
        }
    }
    
    printf("Final result: %d\n", total);
    printf("Verification: %s\n", total > 0 ? "PASS" : "FAIL");
    
    return 0;
}
