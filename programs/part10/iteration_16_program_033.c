#include <stdio.h>
#include <stdlib.h>

/* Pattern 1: Loop with PHI-derived exit condition */
int pattern1_loop_phi_exit(int iterations) {
    int flag = 0;  // Initial definition
    int count = 0;
    
    while (!flag) {  // Conditional branch comparing flag against 0
        count++;
        if (count >= iterations) {
            flag = 1;  // One assignment to flag
        } else {
            flag = 0;  // Another assignment to flag - creates PHI at loop header
        }
        
        // Create SSA copy chain
        int temp1 = flag;
        int temp2 = temp1;
        if (temp2 == 0) {  // Conditional on copy of PHI result
            // Do some work
            count += 1;
        }
    }
    return count;
}

/* Pattern 2: If-else chain with PHI merge */
int pattern2_ifelse_phi(int a, int b) {
    int result;
    
    if (a > b) {
        result = 1;
    } else {
        result = 0;
    }
    
    // Create copy chain
    int x = result;
    int y = x;
    int z = y;
    
    if (z == 1) {  // Conditional on copy of PHI result
        return a * 2;
    } else {
        return b * 2;
    }
}

/* Pattern 3: Nested control with ternary operator */
int pattern3_ternary_phi(int value) {
    int is_even = (value % 2 == 0) ? 1 : 0;  // Ternary creates PHI
    
    // Multiple copy levels
    int copy1 = is_even;
    int copy2 = copy1;
    
    if (copy2 != 0) {  // Conditional on copy chain
        return value / 2;
    }
    return value * 3 + 1;
}

/* Pattern 4: Switch-case with PHI */
int pattern4_switch_phi(char op, int x, int y) {
    int flag;
    
    switch (op) {
        case '+':
            flag = 1;
            break;
        case '-':
            flag = 1;
            break;
        case '*':
            flag = 0;
            break;
        case '/':
            flag = 0;
            break;
        default:
            flag = 1;
    }
    
    // Pass through inline function simulation
    int tmp = flag;
    tmp = tmp;  // Another assignment
    int final = tmp;
    
    if (final == 0) {  // Conditional on PHI-derived value
        return x * y;
    } else {
        return x + y;
    }
}

/* Pattern 5: Complex loop with multiple PHIs */
int pattern5_complex_phi(int n) {
    int sum = 0;
    int continue_flag = 1;
    int i = 0;
    
    while (continue_flag) {
        int inner_flag;
        
        if (i % 3 == 0) {
            inner_flag = 1;
        } else {
            inner_flag = 0;
        }
        
        // Copy chain
        int f1 = inner_flag;
        int f2 = f1;
        
        if (f2) {  // Implicit comparison against 0
            sum += i * 2;
        } else {
            sum += i;
        }
        
        i++;
        if (i >= n) {
            continue_flag = 0;  // Creates PHI at loop header
        } else {
            continue_flag = 1;  // Creates PHI at loop header
        }
        
        // Another conditional on the outer flag
        int check = continue_flag;
        if (check == 1) {
            // Continue processing
            sum += 100;
        }
    }
    
    return sum;
}

/* Helper to create more SSA copies */
static inline int pass_value(int v) {
    int local = v;
    return local;  // Creates assignment
}

/* Pattern 6: Function call in copy chain */
int pattern6_function_phi(int a, int b) {
    int comparison;
    
    if (a == b) {
        comparison = 1;
    } else if (a > b) {
        comparison = 1;
    } else {
        comparison = 0;
    }
    
    // Pass through function
    int passed = pass_value(comparison);
    int final_check = passed;
    
    if (final_check != 1) {  // Conditional on function-processed PHI value
        return b - a;
    }
    return a - b;
}

/* Pattern 7: Loop with break condition PHI */
int pattern7_break_phi(int limit) {
    int found = 0;
    int value = 0;
    
    for (int i = 0; i < 100; i++) {
        value += i;
        
        if (value > limit) {
            found = 1;
        } else {
            found = 0;
        }
        
        // Multiple copy levels
        int f1 = found;
        int f2 = f1;
        int f3 = f2;
        
        if (f3 == 1) {  // Conditional on copy chain
            break;
        }
    }
    
    return value;
}

/* Main function with rich control flow */
int main(int argc, char **argv) {
    int total = 0;
    int iterations = 10;
    
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations <= 0) iterations = 5;
    }
    
    // Main loop with flag-based exit (mimics target pattern)
    int done = 0;
    int counter = 0;
    
    while (!done) {  // Outer loop with PHI-derived condition
        // Call each pattern function
        total += pattern1_loop_phi_exit(iterations);
        total += pattern2_ifelse_phi(counter, iterations);
        total += pattern3_ternary_phi(counter);
        total += pattern4_switch_phi('+', counter, iterations);
        total += pattern5_complex_phi(iterations);
        total += pattern6_function_phi(counter, iterations);
        total += pattern7_break_phi(iterations * 10);
        
        counter++;
        
        // Create PHI for done flag
        if (counter >= iterations) {
            done = 1;
        } else {
            done = 0;
        }
        
        // Copy chain for the done flag
        int d1 = done;
        int d2 = d1;
        if (d2 == 0) {  // Conditional on copy of PHI result
            total += 1;
        }
    }
    
    // Final conditional on accumulated total
    int check_total = total;
    if (check_total > 1000) {
        printf("Large result: %d\n", total);
    } else {
        printf("Result: %d\n", total);
    }
    
    // Additional nested control flow
    for (int i = 0; i < 3; i++) {
        int inner_flag = (i % 2 == 0) ? 1 : 0;
        int copy = inner_flag;
        
        if (copy) {
            total += pattern2_ifelse_phi(i, total % 10);
        }
    }
    
    return total > 0 ? 0 : 1;
}
