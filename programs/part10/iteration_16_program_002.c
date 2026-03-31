#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Loop with PHI-derived exit condition */
int pattern1_loop_phi_exit(int iterations) {
    int flag = 0;  // Initial value
    int count = 0;
    
    while (!flag) {  // Conditional on PHI-derived value (flag)
        count++;
        if (count >= iterations) {
            flag = 1;  // Sets flag in loop body
        }
        // Create SSA copy chain
        int temp1 = flag;
        int temp2 = temp1;
        if (temp2 == 1) {  // Conditional with constant RHS 1
            // Additional nested control flow
            for (int i = 0; i < 3; i++) {
                if (i % 2 == 0) {
                    count += 2;
                }
            }
        }
    }
    return count;
}

/* Pattern 2: If-else chain with PHI merge */
int pattern2_ifelse_phi(int a, int b) {
    int result;
    
    // Creates PHI node at merge point
    if (a > b) {
        result = 1;
    } else {
        result = 0;
    }
    
    // SSA copy chain
    int x = result;
    int y = x;
    int z = y;
    
    // Conditional on PHI-derived value with constant 0
    if (z == 0) {  // Constant RHS 0
        return a + b;
    } else {
        // Nested control structure
        for (int i = 0; i < a; i++) {
            if (i % 3 == 0) {
                b += i;
            }
        }
        return a * b;
    }
}

/* Helper function to create SSA copy chain */
static inline int propagate_value(int v) {
    int t1 = v;
    int t2 = t1;
    return t2;  // Creates copy chain
}

/* Pattern 3: Ternary operator creating PHI */
int pattern3_ternary_phi(int x, int y, int z) {
    // Ternary creates PHI-like behavior
    int condition = (x > y) ? 1 : 0;
    
    // Multiple copy chains through function calls
    int c1 = propagate_value(condition);
    int c2 = propagate_value(c1);
    
    // Conditional with implicit boolean check (if (c2) == if (c2 != 0))
    if (c2) {  // Implicit comparison with 0
        int sum = 0;
        for (int i = 0; i < z; i++) {
            if (i % 2 == condition) {  // Complex condition
                sum += i;
            }
        }
        return sum;
    }
    
    return x + y + z;
}

/* Pattern 4: Switch-case setting 0/1 value */
int pattern4_switch_phi(int option) {
    int flag;
    
    switch (option % 4) {
        case 0:
            flag = 1;
            break;
        case 1:
            flag = 0;
            break;
        case 2:
            flag = 1;
            break;
        default:
            flag = 0;
            break;
    }
    
    // Create longer copy chain
    int a = flag;
    int b = a;
    int c = b;
    int d = c;
    
    // Conditional with != 1 comparison
    if (d != 1) {  // Constant RHS 1
        // Loop with internal branches
        int total = 0;
        for (int i = 0; i < 10; i++) {
            if (i < 5) {
                total += i * 2;
            } else {
                total -= i;
            }
        }
        return total;
    }
    
    return option * 2;
}

/* Pattern 5: Nested loops with PHI in outer loop */
int pattern5_nested_phi(int n) {
    int outer_flag = 0;
    int sum = 0;
    
    for (int i = 0; i < n; i++) {
        // Inner loop with its own PHI
        int inner_flag = 0;
        int j = 0;
        
        while (!inner_flag) {  // Conditional on PHI
            sum += i + j;
            j++;
            
            // SSA copy
            int check = inner_flag;
            if (check == 0) {  // Constant RHS 0
                if (j >= 3) {
                    inner_flag = 1;
                }
            }
        }
        
        // Copy chain for outer flag
        int of1 = outer_flag;
        int of2 = of1;
        
        if (of2 == 0 && i >= n/2) {
            outer_flag = 1;
        }
    }
    
    // Final conditional on outer_flag
    int final_check = outer_flag;
    if (final_check) {  // Implicit != 0
        sum *= 2;
    }
    
    return sum;
}

/* Pattern 6: Complex control flow with multiple PHIs */
int pattern6_complex_phi(int a, int b, int c) {
    int val1, val2;
    
    // First PHI
    if (a > 0) {
        val1 = 1;
    } else {
        val1 = 0;
    }
    
    // Second PHI with dependency
    if (b > 0) {
        val2 = val1;  // Uses first PHI
    } else {
        val2 = (c > 0) ? 1 : 0;
    }
    
    // Multiple copy operations
    int t1 = val2;
    int t2 = t1;
    int t3 = propagate_value(t2);
    
    // Conditional with == 1
    if (t3 == 1) {
        // Deeply nested control
        for (int i = 0; i < a; i++) {
            for (int j = 0; j < b; j++) {
                if ((i + j) % 2 == 0) {
                    c += i * j;
                } else {
                    c -= 1;
                }
            }
        }
        return c;
    }
    
    return a + b;
}

/* Main function with its own pattern */
int main() {
    int total = 0;
    int data[6] = {5, 3, 7, 2, 4, 6};
    
    // Main loop with PHI-derived condition
    int done = 0;
    int idx = 0;
    
    while (!done) {  // Conditional on PHI-derived 'done'
        // Process each pattern function
        switch (idx % 6) {
            case 0:
                total += pattern1_loop_phi_exit(data[idx]);
                break;
            case 1:
                total += pattern2_ifelse_phi(data[idx], data[(idx + 1) % 6]);
                break;
            case 2:
                total += pattern3_ternary_phi(data[idx], data[(idx + 2) % 6], 
                                            data[(idx + 3) % 6]);
                break;
            case 3:
                total += pattern4_switch_phi(data[idx]);
                break;
            case 4:
                total += pattern5_nested_phi(data[idx]);
                break;
            case 5:
                total += pattern6_complex_phi(data[idx], data[(idx + 4) % 6],
                                            data[(idx + 5) % 6]);
                break;
        }
        
        idx++;
        
        // Create SSA copy chain for done flag
        int check1 = done;
        int check2 = check1;
        
        // Conditional with constant 0
        if (check2 == 0 && idx >= 6) {
            done = 1;
        }
    }
    
    // Final conditional with implicit check
    int final_flag = (total > 100) ? 1 : 0;
    int f1 = final_flag;
    if (f1) {  // if (f1 != 0)
        total += 50;
    }
    
    printf("Result: %d\n", total);
    
    // Additional test cases to exercise different paths
    for (int i = 0; i < 3; i++) {
        int test_val = pattern2_ifelse_phi(i, i * 2);
        int test_copy = test_val;
        if (test_copy == 0) {
            printf("Path %d taken\n", i);
        }
    }
    
    return 0;
}
