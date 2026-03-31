#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Loop with PHI-derived exit condition */
int pattern1_loop_phi_exit(int iterations) {
    int flag = 0;  // This becomes a PHI at loop header
    int total = 0;
    int i = 0;
    
    // Loop where exit condition depends on PHI-derived flag
    while (!flag) {  // if (!flag) -> if (flag == 0)
        total += i;
        i++;
        // Set flag based on condition - creates PHI node
        if (i >= iterations) {
            flag = 1;  // Becomes part of PHI
        } else {
            flag = 0;  // Becomes part of PHI
        }
    }
    return total;
}

/* Pattern 2: If-else chain assigning 0/1, then conditional on that value */
int pattern2_phi_conditional(int x, int y) {
    int result;
    
    // This creates a PHI node at the merge point
    if (x > y) {
        result = 1;
    } else {
        result = 0;
    }
    
    // Introduce SSA copy chain
    int a = result;  // First copy
    int b = a;       // Second copy
    int c = b;       // Third copy
    
    // Conditional on the copy-chain variable
    if (c == 1) {  // Compares against constant 1
        return x * 2;
    } else {
        return y * 2;
    }
}

/* Pattern 3: Nested control with multiple PHI nodes */
int pattern3_nested_phi(int n) {
    int sum = 0;
    int toggle = 0;  // Will become PHI
    
    for (int i = 0; i < n; i++) {
        // Inner if-else creates PHI for toggle
        if (i % 3 == 0) {
            toggle = 1;
        } else {
            toggle = 0;
        }
        
        // Copy through temporary
        int temp = toggle;
        
        // Conditional on the copied PHI value
        if (temp) {  // Implicit: if (temp != 0)
            sum += i * 2;
        } else {
            sum += i;
        }
        
        // Another conditional in same basic block
        if (toggle == 0) {  // Explicit comparison to 0
            sum -= 1;
        }
    }
    return sum;
}

/* Pattern 4: Switch-case setting 0/1 followed by conditional */
int pattern4_switch_phi(int val) {
    int code;
    
    switch (val % 4) {
        case 0: code = 1; break;
        case 1: code = 0; break;
        case 2: code = 1; break;
        case 3: code = 0; break;
        default: code = 0; break;
    }
    
    // Multiple SSA copies
    int x = code;
    int y = x;
    
    // Conditional with copy chain
    if (y == 0) {  // Compare against 0
        return val + 10;
    } else {
        return val - 10;
    }
}

/* Pattern 5: Ternary operator creating PHI */
int pattern5_ternary_phi(int a, int b) {
    // Ternary creates PHI
    int is_greater = (a > b) ? 1 : 0;
    
    // Pass through function-like macro (inlined)
    #define PASS(v) (v)
    int passed = PASS(is_greater);
    
    // Conditional on passed value
    if (passed == 1) {  // Compare against 1
        return a * b;
    }
    return a + b;
}

/* Pattern 6: Complex loop with multiple exit conditions */
int pattern6_complex_loop(int limit) {
    int done = 0;  // PHI at loop header
    int count = 0;
    int value = 0;
    
    while (value < 100) {
        // Multiple assignments to 'done' create PHI
        if (count >= limit) {
            done = 1;
        } else if (value > 50) {
            done = 1;
        } else {
            done = 0;
        }
        
        // Copy chain
        int status = done;
        int check = status;
        
        // Conditional on copy chain
        if (check) {  // if (check != 0)
            break;
        }
        
        value += count;
        count++;
    }
    return value;
}

/* Pattern 7: Multiple basic blocks with annotated conditions */
int pattern7_multi_bb(int x) {
    int flag1 = 0, flag2 = 0;
    
    // First conditional structure
    if (x > 0) {
        flag1 = 1;
        if (x > 10) {
            flag2 = 1;
        } else {
            flag2 = 0;  // Creates PHI
        }
    } else {
        flag1 = 0;  // Creates PHI
        flag2 = 0;
    }
    
    // Copy through multiple variables
    int f1 = flag1;
    int f2 = flag2;
    int combined = f1 && f2;
    
    // Final conditional on combined PHI-derived value
    if (combined == 0) {  // Compare against 0
        return x * 3;
    } else {
        return x * 5;
    }
}

/* Main function with its own pattern and calling all others */
int main() {
    int total = 0;
    int data[] = {5, 10, 15, 20, 25};
    int n = sizeof(data) / sizeof(data[0]);
    
    // Main's own loop with PHI-derived condition
    int completed = 0;  // Will become PHI
    int i = 0;
    
    while (i < n) {
        // Call each pattern function
        total += pattern1_loop_phi_exit(data[i]);
        total += pattern2_phi_conditional(data[i], i);
        total += pattern3_nested_phi(data[i] % 5 + 3);
        total += pattern4_switch_phi(data[i]);
        total += pattern5_ternary_phi(data[i], i + 1);
        total += pattern6_complex_loop(data[i] % 3 + 2);
        total += pattern7_multi_bb(data[i]);
        
        i++;
        
        // Set completion flag - creates PHI
        if (i >= n) {
            completed = 1;
        } else {
            completed = 0;
        }
        
        // Conditional on PHI-derived flag (in main's loop)
        int done_flag = completed;
        if (done_flag == 1) {  // Compare against 1
            break;
        }
    }
    
    // Another conditional pattern in main
    int final_check = (total > 1000) ? 1 : 0;
    int verify = final_check;
    
    if (verify) {  // if (verify != 0)
        printf("Result: %d (above threshold)\n", total);
    } else {
        printf("Result: %d\n", total);
    }
    
    return 0;
}
