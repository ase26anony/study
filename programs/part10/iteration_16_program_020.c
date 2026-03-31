#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Loop exit condition depends on PHI-assigned flag */
int pattern1_loop_flag(int iterations) {
    int flag = 0;
    int count = 0;
    
    while (!flag) {  // PHI node for flag at loop header
        count++;
        if (count >= iterations) {
            flag = 1;  // Sets flag to 1, creating PHI
        }
    }
    
    // SSA copy chain
    int a = flag;
    int b = a;
    
    if (b == 1) {  // Conditional on constant 1
        return count * 2;
    }
    return count;
}

/* Pattern 2: If-else chain assigning 0/1 to variable */
int pattern2_if_else_phi(int x, int y) {
    int result;
    
    if (x > y) {
        result = 1;  // First PHI operand
    } else {
        result = 0;  // Second PHI operand
    }
    
    // Multiple SSA copies
    int temp1 = result;
    int temp2 = temp1;
    int temp3 = temp2;
    
    if (temp3 != 0) {  // Conditional on constant 0
        return x - y;
    }
    return y - x;
}

/* Pattern 3: Nested control with ternary operator */
int pattern3_ternary_phi(int *arr, int size) {
    int found = 0;
    int sum = 0;
    
    for (int i = 0; i < size; i++) {
        // Complex condition creating PHI
        int is_special = (arr[i] % 2 == 0) ? 1 : 0;
        
        // SSA copy through assignment
        int check = is_special;
        
        if (check == 1) {  // Conditional on constant 1
            sum += arr[i];
            found = 1;  // Affects loop exit condition
        }
        
        if (found && i > size/2) {
            break;
        }
    }
    
    int verify = found;
    if (verify) {  // Implicit comparison with 0
        return sum * 2;
    }
    return sum;
}

/* Pattern 4: Switch-case setting 0/1 value */
int pattern4_switch_phi(char op, int a, int b) {
    int valid = 0;  // Initial value
    
    switch (op) {
        case '+':
        case '-':
        case '*':
        case '/':
            valid = 1;  // Sets to 1
            break;
        default:
            valid = 0;  // Sets to 0
    }
    
    // Chain of assignments
    int v1 = valid;
    int v2 = v1;
    
    if (v2 == 0) {  // Conditional on constant 0
        return -1;
    }
    
    // Use in another conditional
    int result;
    if (op == '+') result = a + b;
    else if (op == '-') result = a - b;
    else if (op == '*') result = a * b;
    else if (op == '/') result = (b != 0) ? a / b : 0;
    else result = 0;
    
    return result;
}

/* Pattern 5: Multiple PHI nodes in loop */
int pattern5_complex_phi(int n) {
    int even_count = 0;
    int odd_count = 0;
    int toggle = 0;
    
    for (int i = 0; i < n; i++) {
        // PHI for toggle at loop header
        int current = toggle;
        
        if (current == 0) {  // Conditional on constant 0
            even_count++;
            toggle = 1;  // Changes PHI operand
        } else {
            odd_count++;
            toggle = 0;  // Changes PHI operand
        }
        
        // Another conditional with copy chain
        int check = (even_count > odd_count) ? 1 : 0;
        int copy1 = check;
        int copy2 = copy1;
        
        if (copy2 == 1) {  // Conditional on constant 1
            // Do something
        }
    }
    
    // Final conditional
    int diff = even_count - odd_count;
    int pos = (diff > 0) ? 1 : 0;
    
    if (pos) {  // Implicit comparison with 0
        return even_count;
    }
    return odd_count;
}

/* Helper function to create SSA copy chain */
static inline int pass_through(int x) {
    int y = x;
    return y;
}

/* Pattern 6: Function calls in SSA chain */
int pattern6_function_phi(int x) {
    int state = 0;
    
    // Complex control flow
    if (x > 100) {
        state = 1;
    } else if (x > 50) {
        state = 0;
    } else {
        state = (x % 2 == 0) ? 1 : 0;
    }
    
    // Pass through function (inlined)
    int s1 = pass_through(state);
    int s2 = pass_through(s1);
    
    if (s2 == 1) {  // Conditional on constant 1
        return x * 2;
    }
    
    // Another chain
    int t1 = state;
    int t2 = t1;
    int t3 = t2;
    
    if (!t3) {  // Conditional on constant 0 (implicit)
        return x / 2;
    }
    
    return x;
}

int main() {
    int total = 0;
    
    // Test pattern1
    total += pattern1_loop_flag(10);
    
    // Test pattern2
    total += pattern2_if_else_phi(20, 15);
    total += pattern2_if_else_phi(10, 20);
    
    // Test pattern3
    int arr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    total += pattern3_ternary_phi(arr, 10);
    
    // Test pattern4
    total += pattern4_switch_phi('+', 10, 5);
    total += pattern4_switch_phi('?', 10, 5);
    
    // Test pattern5
    total += pattern5_complex_phi(20);
    
    // Test pattern6
    total += pattern6_function_phi(75);
    total += pattern6_function_phi(120);
    total += pattern6_function_phi(25);
    
    // Main loop with flag pattern (mimics target at top level)
    int done = 0;
    int counter = 0;
    
    while (!done) {
        counter++;
        
        // Create PHI for done
        if (counter >= 5) {
            done = 1;
        }
        
        // SSA copies
        int d1 = done;
        int d2 = d1;
        
        if (d2 == 0) {  // Conditional on constant 0
            total += counter;
        }
    }
    
    printf("Result: %d\n", total);
    
    // Additional test cases with different branch probabilities
    for (int i = 0; i < 100; i++) {
        int val = i % 3;
        int flag = (val == 0) ? 1 : 0;
        int f1 = flag;
        
        if (f1 == 1) {
            total++;
        } else {
            total--;
        }
    }
    
    printf("Final result: %d\n", total);
    
    return 0;
}
