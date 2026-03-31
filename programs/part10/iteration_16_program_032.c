#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Loop exit condition depends on PHI-assigned flag */
int pattern1_loop_exit(int iterations) {
    int flag = 0;  // Initial value
    int count = 0;
    
    // Loop header creates PHI node for 'flag'
    while (!flag) {  // if (flag == 0) - compares against 0
        count++;
        // This assignment creates a PHI node at loop header
        flag = (count >= iterations) ? 1 : 0;
    }
    return count;
}

/* Pattern 2: If-else chain sets 0/1, later conditional uses it */
int pattern2_if_else_merge(int a, int b) {
    int result;
    
    // PHI node at merge point
    if (a > b) {
        result = 1;
    } else {
        result = 0;
    }
    
    // SSA copy chain
    int temp1 = result;
    int temp2 = temp1;
    
    // Conditional comparing against 1
    if (temp2 == 1) {  // Compares against constant 1
        return a * 2;
    } else {
        return b * 2;
    }
}

/* Pattern 3: Nested control with multiple PHI nodes */
int pattern3_nested_control(int n) {
    int sum = 0;
    int continue_flag = 1;  // PHI at loop header
    
    while (continue_flag) {  // if (continue_flag != 0)
        int inner_flag = 0;  // PHI at inner loop header
        
        for (int i = 0; i < 5; i++) {
            if (n % 2 == 0) {
                inner_flag = 1;
            }
            sum += i;
            
            // SSA copy through assignment
            int check = inner_flag;
            if (check) {  // if (check != 0)
                sum += 100;
            }
        }
        
        // Multiple SSA copies
        int flag_copy1 = continue_flag;
        int flag_copy2 = flag_copy1;
        int flag_copy3 = flag_copy2;
        
        if (flag_copy3 == 0) {  // Compares against constant 0
            break;
        }
        
        n--;
        if (n <= 0) {
            continue_flag = 0;
        }
    }
    return sum;
}

/* Pattern 4: Switch-case sets 0/1 value */
int pattern4_switch_case(int val) {
    int indicator = 0;
    
    switch (val % 3) {
        case 0:
            indicator = 1;
            break;
        case 1:
            indicator = 0;
            break;
        case 2:
            indicator = 1;
            break;
    }
    
    // Chain of assignments
    int a = indicator;
    int b = a;
    int c = b;
    
    // Final conditional
    if (c == 1) {  // Compares against constant 1
        return val * 10;
    }
    return val;
}

/* Pattern 5: Ternary operator producing 0/1 */
int pattern5_ternary(int x, int y) {
    // Ternary creates PHI-like structure
    int is_greater = (x > y) ? 1 : 0;
    
    // Multiple SSA copies
    int copy1 = is_greater;
    int copy2 = copy1;
    
    if (copy2 == 0) {  // Compares against constant 0
        return y - x;
    }
    return x - y;
}

/* Pattern 6: Complex PHI with multiple predecessors */
int pattern6_complex_phi(int limit) {
    int state = 0;
    int total = 0;
    
    for (int i = 0; i < limit; i++) {
        // Multiple assignments to 'state' create PHI
        if (i % 3 == 0) {
            state = 1;
        } else if (i % 3 == 1) {
            state = 0;
        } else {
            state = (i % 2 == 0) ? 1 : 0;
        }
        
        // Use state in condition
        int check_state = state;
        if (check_state) {  // if (check_state != 0)
            total += i * 2;
        } else {
            total += i;
        }
    }
    return total;
}

/* Helper function to create SSA copy chain */
static inline int pass_through(int v) {
    int temp = v;
    return temp;
}

/* Pattern 7: Function call in SSA chain */
int pattern7_function_chain(int a, int b) {
    int cmp_result = (a == b) ? 1 : 0;
    
    // Pass through function (inlined)
    int passed = pass_through(cmp_result);
    int passed2 = pass_through(passed);
    
    if (passed2 == 1) {  // Compares against constant 1
        return 1;
    }
    return 0;
}

/* Main function with its own pattern */
int main() {
    int total = 0;
    int data[10];
    
    // Initialize array
    for (int i = 0; i < 10; i++) {
        data[i] = i * 2;
    }
    
    // Main loop with flag-based exit (pattern similar to target)
    int done = 0;
    int index = 0;
    
    while (!done) {  // if (done == 0)
        // Call pattern functions with varying inputs
        total += pattern1_loop_exit(data[index] % 5 + 1);
        total += pattern2_if_else_merge(data[index], data[(index + 1) % 10]);
        total += pattern3_nested_control(data[index] % 4 + 1);
        total += pattern4_switch_case(data[index]);
        total += pattern5_ternary(data[index], data[(index + 2) % 10]);
        total += pattern6_complex_phi(data[index] % 6 + 3);
        total += pattern7_function_chain(data[index], index);
        
        index++;
        
        // PHI-based exit condition
        int should_continue = (index < 10) ? 1 : 0;
        int check_continue = should_continue;
        
        if (check_continue == 0) {  // Compares against constant 0
            done = 1;
        }
    }
    
    // Additional control flow to ensure rich basic blocks
    int final_check = (total > 1000) ? 1 : 0;
    int verify = final_check;
    
    if (verify) {  // if (verify != 0)
        printf("Result: %d (above threshold)\n", total);
    } else {
        printf("Result: %d\n", total);
    }
    
    // More patterns in main
    for (int i = 0; i < 5; i++) {
        int flag = (i % 2 == 0) ? 1 : 0;
        int f1 = flag;
        int f2 = f1;
        
        if (f2 == 1) {  // Compares against constant 1
            total += 100;
        } else {
            total += 50;
        }
    }
    
    printf("Final total: %d\n", total);
    return 0;
}
