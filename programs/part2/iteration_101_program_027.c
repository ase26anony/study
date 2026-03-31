#include <stdio.h>
#include <stdlib.h>

// Global variables to prevent constant propagation
volatile int global_modifier = 0;
volatile int global_counter = 0;

// Function to introduce side effects
int __attribute__((noinline)) side_effect_func(int x) {
    global_modifier++;
    return x + global_modifier;
}

// Function to read input for variability
int __attribute__((noinline)) get_input() {
    return global_counter++ % 5;
}

// Test function 1: Simple modification in then block
void __attribute__((noinline)) test_simple_modification() {
    volatile int a = 10, b = 5, c = 0;
    
    // Loop to create basic block context
    for (int i = 0; i < 100; i++) {
        // Critical: 'a' is used in condition and modified in then block
        if (a > b) {
            // This assignment modifies 'a' which is part of the condition
            a = b + i;  // Line in header that should trigger modified_in_p
            c += a * 2;
        } else {
            b = a + 1;
        }
        
        // Prevent loop unrolling from eliminating the if
        if (i % 7 == get_input()) {
            a += 2;
        }
    }
    
    // Use results to prevent dead code elimination
    global_modifier += a + b + c;
}

// Test function 2: Compound condition with multiple modifications
void __attribute__((noinline)) test_compound_condition() {
    volatile int x = 100, y = 50, z = 75;
    volatile int flag = 0;
    
    for (int i = 0; i < 50; i++) {
        // Complex condition with multiple variables
        if (x != 0 && y < z && flag == 0) {
            // Multiple modifications to variables used in condition
            x = y + i;      // Modifies 'x' from condition
            y = z - 1;      // Modifies 'y' from condition
            z = x * 2;      // Modifies 'z' from condition
            // Additional non-debug, non-label instructions
            flag = 1;
            global_counter += x;
        } else {
            z = x + y;
            flag = 0;
        }
        
        // Add variability
        if (get_input() > 2) {
            x += 3;
        }
    }
    
    global_modifier += x - y + z;
}

// Test function 3: Static variable modification
void __attribute__((noinline)) test_static_modification() {
    static volatile int counter = 0;
    volatile int limit = 30;
    volatile int accumulator = 0;
    
    for (int i = 0; i < 40; i++) {
        // Condition uses static variable
        if (counter < limit && i > 10) {
            // Modify the static variable used in condition
            counter++;      // This should trigger modified_in_p
            accumulator += counter * i;
            
            // Additional instructions in header
            limit = limit - (counter % 3);
            global_modifier += 1;
        } else {
            accumulator -= i;
            if (counter > 0) counter--;
        }
        
        // External call for variability
        side_effect_func(i);
    }
    
    global_counter += accumulator;
}

// Test function 4: Function call in condition with side effects
void __attribute__((noinline)) test_function_in_condition() {
    volatile int base = 20;
    volatile int threshold = 15;
    
    for (int i = 0; i < 25; i++) {
        // Function call in condition
        if (side_effect_func(base) > threshold) {
            // Modify variable that affects future function calls
            base = threshold - i;  // Modifies 'base' used in side_effect_func
            threshold += global_modifier;
            
            // Multiple instructions in header
            global_counter += 2;
            base = base % 20 + 1;
        } else {
            threshold = base + i;
            base += get_input();
        }
    }
    
    global_modifier += base * threshold;
}

// Test function 5: Nested conditions with modifications
void __attribute__((noinline)) test_nested_modifications() {
    volatile int p = 5, q = 10, r = 15;
    volatile int result = 0;
    
    for (int i = 0; i < 30; i++) {
        if (p < q) {
            // First level modification
            p = q - i;
            
            if (r > p && q < 20) {
                // Second level - modify multiple condition variables
                r = p + 2;      // Modifies 'r' from inner condition
                q = r - 1;      // Modifies 'q' from inner condition
                p = p * 2;      // Modifies 'p' from outer condition
                
                // Additional header instructions
                result += p + q + r;
                global_modifier++;
            }
        } else {
            q = p + r;
            r = get_input();
        }
        
        // Loop variability
        if (i % 4 == 0) {
            p += side_effect_func(i);
        }
    }
    
    global_counter += result;
}

int main(int argc, char *argv[]) {
    // Use command line arguments for runtime variability
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    global_counter = seed;
    
    // Run all test functions
    test_simple_modification();
    test_compound_condition();
    test_static_modification();
    test_function_in_condition();
    test_nested_modifications();
    
    // Compute and print result to ensure side effects
    int final_result = global_modifier + global_counter;
    printf("Result: %d\n", final_result);
    
    return final_result % 100;
}
