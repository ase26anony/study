#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Loop with PHI-derived exit condition */
int pattern1_loop_with_phi_flag(int n) {
    int flag = 0;  // This becomes a PHI node at loop header
    int sum = 0;
    int i = 0;
    
    while (!flag) {  // Conditional on PHI-derived value
        sum += i;
        i++;
        
        // This creates a PHI for 'flag' at loop header
        if (i >= n) {
            flag = 1;  // Sets one arm of PHI
        } else if (sum > 1000) {
            flag = 1;  // Sets another arm of PHI
        }
        // Implicit else: flag remains 0 (third arm of PHI)
    }
    
    // Add SSA copy chain
    int temp1 = flag;
    int temp2 = temp1;
    if (temp2 == 1) {  // Conditional with copy chain
        sum += 100;
    }
    
    return sum;
}

/* Pattern 2: If-else chain with PHI assignment */
int pattern2_if_else_phi(int a, int b) {
    int result;
    
    // This creates a PHI node at the merge point
    if (a > b) {
        result = 1;
    } else if (a < b) {
        result = 0;
    } else {
        result = 1;  // Equal case
    }
    
    // Multiple SSA copies
    int x = result;
    int y = x;
    int z = y;
    
    // Conditional on the copy chain
    if (z == 0) {  // Compares against constant 0
        return a + b;
    } else {  // Implicit comparison against non-zero
        return a * b;
    }
}

/* Helper function to create SSA copy chain */
static inline int pass_through(int v) {
    int t1 = v;
    int t2 = t1;
    return t2;  // Creates copy chain
}

/* Pattern 3: Nested control with ternary operator */
int pattern3_ternary_phi(int *arr, int size) {
    int all_positive = 1;  // Initial value
    
    for (int i = 0; i < size; i++) {
        // Ternary creates PHI for all_positive
        all_positive = (arr[i] > 0) ? all_positive : 0;
        
        // Nested if to create complex control flow
        if (arr[i] % 2 == 0) {
            arr[i] *= 2;
        } else {
            arr[i] += 1;
        }
    }
    
    // Use pass_through to create copy chain
    int check = pass_through(all_positive);
    
    // Conditional with constant 1 comparison
    if (check == 1) {
        return 1;
    }
    return 0;
}

/* Pattern 4: Switch-case with PHI to conditional */
int pattern4_switch_phi(int code) {
    int status;
    
    switch (code) {
        case 0:
            status = 1;
            break;
        case 1:
            status = 0;
            break;
        case 2:
            status = 1;
            break;
        default:
            status = 0;
            break;
    }
    
    // Create longer copy chain
    int a = status;
    int b = a;
    int c = b;
    int d = c;
    
    // Multiple conditionals to increase annotation likelihood
    if (d != 0) {  // Comparison with constant 0
        if (code > 10) {
            return code * 2;
        }
        return code + 1;
    }
    
    return -1;
}

/* Pattern 5: Complex loop with multiple PHIs */
int pattern5_complex_loop(int iterations) {
    int continue_loop = 1;  // PHI at loop header
    int modify_flag = 0;     // Another PHI
    int total = 0;
    int i = 0;
    
    while (continue_loop) {  // Conditional on PHI
        total += i;
        
        // Multiple assignments create complex PHI web
        if (i % 3 == 0) {
            modify_flag = 1;
        } else if (i % 5 == 0) {
            modify_flag = 0;
        }
        
        // Use modify_flag through copy chain
        int f1 = modify_flag;
        int f2 = f1;
        if (f2 == 1) {  // Comparison with constant 1
            total += 10;
        }
        
        i++;
        if (i >= iterations) {
            continue_loop = 0;  // Sets one arm of PHI
        } else if (total > 10000) {
            continue_loop = 0;  // Sets another arm
        }
        // Implicit else: continue_loop remains 1
    }
    
    return total;
}

/* Main function with diverse control flow */
int main() {
    int total_result = 0;
    
    // Initialize test data
    int test_array[] = {1, -2, 3, -4, 5, -6, 7, -8, 9, -10};
    int array_size = sizeof(test_array) / sizeof(test_array[0]);
    
    // Pattern 1
    printf("Testing pattern1...\n");
    total_result += pattern1_loop_with_phi_flag(20);
    
    // Pattern 2 with different inputs
    printf("Testing pattern2...\n");
    total_result += pattern2_if_else_phi(10, 5);
    total_result += pattern2_if_else_phi(5, 10);
    total_result += pattern2_if_else_phi(7, 7);
    
    // Pattern 3
    printf("Testing pattern3...\n");
    int arr_copy[10];
    for (int i = 0; i < 10; i++) arr_copy[i] = test_array[i];
    total_result += pattern3_ternary_phi(arr_copy, 10);
    
    // Pattern 4
    printf("Testing pattern4...\n");
    for (int code = 0; code < 5; code++) {
        total_result += pattern4_switch_phi(code);
    }
    
    // Pattern 5
    printf("Testing pattern5...\n");
    total_result += pattern5_complex_loop(50);
    
    // Additional loop in main with PHI-derived condition
    printf("Testing main loop pattern...\n");
    int done = 0;  // Will become PHI
    int main_counter = 0;
    
    while (!done) {  // Conditional on PHI
        main_counter++;
        
        // Create SSA copy chain
        int d1 = done;
        int d2 = d1;
        
        if (d2 == 0) {  // Comparison with constant 0
            total_result += main_counter;
        }
        
        // Multiple conditions to set done
        if (main_counter >= 100) {
            done = 1;
        } else if (total_result > 100000) {
            done = 1;
        }
    }
    
    printf("Final result: %d\n", total_result);
    
    // Verify with simple computation
    int verification = pattern1_loop_with_phi_flag(10) +
                      pattern2_if_else_phi(3, 4) +
                      pattern5_complex_loop(20);
    printf("Verification result: %d\n", verification);
    
    return 0;
}
