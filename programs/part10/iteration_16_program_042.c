#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Loop exit condition depends on PHI-assigned flag */
int pattern1_loop_flag(int iterations) {
    int flag = 0;
    int count = 0;
    
    while (!flag) {  // This creates a PHI node for 'flag' at loop header
        count++;
        if (count >= iterations) {
            flag = 1;  // Sets flag inside loop
        }
    }
    
    // Create SSA copy chain
    int temp1 = flag;
    int temp2 = temp1;
    
    // Conditional branch comparing PHI-derived value against 0
    if (temp2 == 0) {  // Should be false at this point
        return -1;
    }
    return count;
}

/* Pattern 2: If-else chain setting boolean, then conditional on it */
int pattern2_if_else_merge(int a, int b) {
    int result;
    
    // This creates a PHI node at the merge point
    if (a > b) {
        result = 1;
    } else {
        result = 0;
    }
    
    // Create copy chain through assignments
    int x = result;
    int y = x;
    int z = y;
    
    // Conditional branch comparing against 1
    if (z == 1) {
        return a;
    } else {
        return b;
    }
}

/* Pattern 3: Nested control with ternary operator */
int pattern3_ternary_phi(int x, int y, int z) {
    // Ternary creates PHI node
    int condition = (x > 0) ? 1 : 0;
    
    // Multiple copy levels
    int level1 = condition;
    int level2 = level1;
    
    if (level2 != 0) {  // Compare against 0
        return y * z;
    }
    
    // Nested loop with flag
    int flag = 0;
    int sum = 0;
    for (int i = 0; i < x && !flag; i++) {
        sum += i;
        if (sum > 100) {
            flag = 1;  // Sets flag inside loop
        }
        
        // Another copy chain inside loop
        int local_copy = flag;
        if (local_copy == 1) {  // Compare against 1
            break;
        }
    }
    
    return sum;
}

/* Pattern 4: Switch-case setting 0/1 value */
int pattern4_switch_phi(int option) {
    int value;
    
    switch (option % 3) {
        case 0:
            value = 1;
            break;
        case 1:
            value = 0;
            break;
        default:
            value = 1;
            break;
    }
    
    // Copy through multiple variables
    int a = value;
    int b = a;
    
    // Conditional with implicit boolean check (compares against 0)
    if (b) {
        return option * 2;
    }
    return option / 2;
}

/* Pattern 5: Complex nested control flow */
int pattern5_complex_nested(int n) {
    int outer_flag = 0;
    int total = 0;
    
    for (int i = 0; i < n; i++) {
        int inner_flag = 0;
        
        // Inner loop with PHI-derived condition
        for (int j = 0; j < i && !inner_flag; j++) {
            total += j;
            
            if (total > 50) {
                inner_flag = 1;
            }
            
            // Copy chain in inner loop
            int copy1 = inner_flag;
            int copy2 = copy1;
            
            if (copy2 == 1) {  // Compare against 1
                outer_flag = 1;
                break;
            }
        }
        
        // Another conditional using outer_flag
        int flag_copy = outer_flag;
        if (flag_copy != 0) {  // Compare against 0
            break;
        }
    }
    
    return total;
}

/* Helper function to create copy chain through function call */
static inline int pass_value(int v) {
    // Simple pass-through creates SSA copies
    return v;
}

/* Pattern 6: Using function calls in copy chain */
int pattern6_function_copy(int a, int b) {
    int cmp = (a == b) ? 1 : 0;
    
    // Pass through function calls
    int x = pass_value(cmp);
    int y = pass_value(x);
    
    if (y == 0) {
        return a - b;
    } else {
        return a + b;
    }
}

/* Pattern 7: Multiple PHIs in loop header */
int pattern7_multi_phi(int limit) {
    int i = 0;
    int done = 0;
    int result = 0;
    
    // Loop header has PHI nodes for i, done, result
    while (!done) {
        result += i;
        i++;
        
        if (i >= limit) {
            done = 1;
        }
        
        // Create copy from PHI variable
        int done_copy = done;
        if (done_copy == 1) {  // Compare against 1
            break;
        }
    }
    
    // Post-loop conditional
    int final_copy = done;
    if (final_copy != 0) {  // Compare against 0
        return result;
    }
    return -1;
}

int main() {
    int total = 0;
    
    // Call each pattern function with different inputs
    total += pattern1_loop_flag(10);
    total += pattern2_if_else_merge(5, 3);
    total += pattern3_ternary_phi(8, 2, 3);
    total += pattern4_switch_phi(7);
    total += pattern5_complex_nested(15);
    total += pattern6_function_copy(12, 8);
    total += pattern7_multi_phi(20);
    
    // Main loop with flag pattern (mimics target at top level)
    int main_flag = 0;
    int main_counter = 0;
    
    while (main_flag == 0) {  // Explicit comparison against 0
        main_counter++;
        total += main_counter;
        
        if (main_counter >= 5) {
            main_flag = 1;  // Set flag inside loop
        }
        
        // Create copy chain in main
        int flag_copy = main_flag;
        int another_copy = flag_copy;
        
        if (another_copy == 1) {  // Compare against 1
            break;
        }
    }
    
    // Final conditional with copy chain
    int final_check = main_flag;
    int final_copy1 = final_check;
    int final_copy2 = final_copy1;
    
    if (final_copy2 != 0) {  // Compare against 0
        printf("Result: %d\n", total);
    } else {
        printf("Unexpected: %d\n", total);
    }
    
    // Additional test cases with array to vary control flow
    int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = i * 2;
    }
    
    int search_flag = 0;
    int found_value = 0;
    
    for (int i = 0; i < 10 && search_flag == 0; i++) {
        if (arr[i] > 10) {
            search_flag = 1;
            found_value = arr[i];
        }
        
        int flag_copy = search_flag;
        if (flag_copy == 1) {  // Compare against 1
            break;
        }
    }
    
    printf("Found value: %d\n", found_value);
    
    return 0;
}
