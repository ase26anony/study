#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Loop exit condition depends on PHI-assigned flag */
int pattern1_loop_exit_flag(int iterations) {
    int flag = 0;  // Initial value
    int count = 0;
    
    // Loop where exit condition depends on flag
    while (!flag) {  // Compares flag against 0
        count++;
        if (count >= iterations) {
            flag = 1;  // One branch sets flag to 1
        } else {
            flag = 0;  // Other branch sets flag to 0
        }
        // PHI node for flag at loop header
    }
    return count;
}

/* Pattern 2: If-else chain sets boolean, later conditional uses it */
int pattern2_if_else_merge(int a, int b) {
    int result;
    
    // First if-else sets result to 0 or 1
    if (a > b) {
        result = 1;
    } else {
        result = 0;
    }
    
    // Copy chain to exercise SSA propagation
    int temp1 = result;
    int temp2 = temp1;
    
    // Conditional on the PHI-derived value (compares against 1)
    if (temp2 == 1) {
        return a + b;
    } else {
        return a - b;
    }
}

/* Pattern 3: Nested control with multiple PHI nodes */
int pattern3_nested_control(int n) {
    int sum = 0;
    int continue_flag = 1;  // Initial flag
    
    for (int i = 0; i < n && continue_flag; i++) {
        int inner_flag;
        
        if (i % 2 == 0) {
            inner_flag = 1;
        } else {
            inner_flag = 0;
        }
        
        // Copy through temporary
        int check = inner_flag;
        
        // Conditional on PHI-derived value (compares against 0)
        if (check != 0) {
            sum += i * 2;
        } else {
            sum += i;
        }
        
        // Update outer loop flag based on condition
        if (sum > 100) {
            continue_flag = 0;  // Set to 0 to potentially exit
        } else {
            continue_flag = 1;  // Set to 1 to continue
        }
    }
    return sum;
}

/* Pattern 4: Switch-case sets 0/1 value */
int pattern4_switch_case(int code) {
    int status;
    
    switch (code) {
        case 1:
        case 2:
            status = 1;
            break;
        case 3:
        case 4:
            status = 0;
            break;
        default:
            status = 1;
    }
    
    // Multiple copy chain
    int s1 = status;
    int s2 = s1;
    int s3 = s2;
    
    // Conditional on the value (implicit comparison with 0)
    if (s3) {  // Equivalent to s3 != 0
        return code * 10;
    } else {
        return code * 5;
    }
}

/* Pattern 5: Ternary operator sets 0/1 */
int pattern5_ternary(int x, int y) {
    // Ternary produces 0 or 1
    int is_greater = (x > y) ? 1 : 0;
    
    // Simple copy
    int check = is_greater;
    
    // Conditional with explicit comparison to 1
    if (check == 1) {
        return x * y;
    } else {
        return x + y;
    }
}

/* Pattern 6: Complex chain with function call */
static inline int pass_value(int v) {
    // Simple pass-through function
    return v;
}

int pattern6_function_pass(int a, int b, int c) {
    int decision;
    
    // Set decision based on multiple conditions
    if (a > b && b > c) {
        decision = 1;
    } else if (a < b && b < c) {
        decision = 0;
    } else {
        decision = 1;
    }
    
    // Pass through function (creates SSA copies)
    int d1 = pass_value(decision);
    int d2 = pass_value(d1);
    
    // Final conditional
    if (d2 == 0) {
        return a + b + c;
    } else {
        return a * b * c;
    }
}

/* Pattern 7: Loop with multiple exit conditions */
int pattern7_multi_exit(int limit) {
    int done = 0;
    int value = 0;
    int i = 0;
    
    while (!done) {  // Compare against 0
        value += i;
        
        // Multiple conditions that could set done
        if (i >= limit) {
            done = 1;  // Set to 1
        } else if (value > 1000) {
            done = 1;  // Set to 1
        } else {
            done = 0;  // Set to 0
        }
        
        // Copy chain
        int check_done = done;
        
        // Additional conditional inside loop
        if (check_done == 1) {
            // Do something
            value += 100;
        }
        
        i++;
    }
    return value;
}

/* Main function with its own pattern */
int main() {
    int total = 0;
    int data[10];
    
    // Initialize array
    for (int i = 0; i < 10; i++) {
        data[i] = i * 2;
    }
    
    // Use a flag in main's loop to create pattern
    int process_flag = 1;
    int main_index = 0;
    
    while (process_flag) {  // Compare against 0 (implicit)
        // Call pattern functions with different inputs
        total += pattern1_loop_exit_flag(data[main_index % 10] % 5 + 1);
        total += pattern2_if_else_merge(data[main_index % 10], data[(main_index + 1) % 10]);
        total += pattern3_nested_control(data[main_index % 10] % 4 + 2);
        total += pattern4_switch_case(data[main_index % 10] % 5);
        total += pattern5_ternary(data[main_index % 10], data[(main_index + 2) % 10]);
        total += pattern6_function_pass(data[main_index % 10], 
                                       data[(main_index + 1) % 10], 
                                       data[(main_index + 2) % 10]);
        total += pattern7_multi_exit(data[main_index % 10] % 3 + 3);
        
        main_index++;
        
        // Update flag based on condition (creates PHI)
        if (main_index >= 5) {
            process_flag = 0;  // Set to 0
        } else {
            process_flag = 1;  // Set to 1
        }
        
        // Copy through temporary
        int flag_copy = process_flag;
        
        // Additional conditional in main
        if (flag_copy == 0) {
            total += 1000;  // Bonus when done
        }
    }
    
    printf("Result: %d\n", total);
    
    // Additional calls to ensure different paths are taken
    printf("Pattern1: %d\n", pattern1_loop_exit_flag(3));
    printf("Pattern2: %d\n", pattern2_if_else_merge(10, 5));
    printf("Pattern3: %d\n", pattern3_nested_control(8));
    printf("Pattern4: %d\n", pattern4_switch_case(2));
    printf("Pattern5: %d\n", pattern5_ternary(7, 3));
    printf("Pattern6: %d\n", pattern6_function_pass(2, 3, 4));
    printf("Pattern7: %d\n", pattern7_multi_exit(5));
    
    return 0;
}
