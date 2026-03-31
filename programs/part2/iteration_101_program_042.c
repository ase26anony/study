#include <stdio.h>
#include <stdlib.h>

// Global variables to prevent optimization
volatile int global_counter = 0;
volatile int global_modifier = 1;

// Function prototypes with noinline attribute
__attribute__((noinline)) 
void test_simple_modification(volatile int a, volatile int b, volatile int c);

__attribute__((noinline))
void test_compound_condition(volatile int x, volatile int y, volatile int z);

__attribute__((noinline))
void test_loop_with_modification(volatile int limit);

__attribute__((noinline))
void test_function_call_in_condition(void);

__attribute__((noinline))
int side_effect_func(void);

// Test 1: Simple modification of condition variable in then block
__attribute__((noinline))
void test_simple_modification(volatile int a, volatile int b, volatile int c) {
    // The condition uses 'a', and the then block modifies 'a'
    // This should trigger modified_in_p check
    for (int i = 0; i < 10; i++) {
        if (a > b && c < 100) {
            // These instructions are in the then block header
            a = b + c;  // Modifies 'a' which is used in condition
            b = a * 2;  // Additional instruction
            c = c + 1;  // Modifies 'c' which is also in condition
        } else {
            b = a - 1;
        }
        
        // Add some computation to prevent dead code elimination
        global_counter += (a + b + c);
    }
}

// Test 2: Compound condition with multiple modifications
__attribute__((noinline))
void test_compound_condition(volatile int x, volatile int y, volatile int z) {
    int local_sum = 0;
    
    // Complex condition that creates a non-trivial test_expr
    for (int i = 0; i < 5; i++) {
        if ((x != 0 && y < z) || (x > 10 && z < 20)) {
            // Multiple modifications to condition variables
            x = y + 1;      // Modifies 'x' used in condition
            y = z * 2;      // Modifies 'y' used in condition
            z = x + y;      // Modifies 'z' used in condition
            
            // Add more non-label, non-note instructions
            local_sum += x;
            local_sum += y;
            local_sum += z;
        } else {
            z = x + y;
        }
        
        // Introduce variability
        x += global_modifier;
    }
    
    global_counter += local_sum;
}

// Test 3: Loop with counter modification in then block
__attribute__((noinline))
void test_loop_with_modification(volatile int limit) {
    static volatile int counter = 0;
    
    // The condition tests 'counter', and the then block modifies it
    for (int i = 0; i < limit; i++) {
        if (counter < limit && global_counter < 1000) {
            // This modifies 'counter' which is part of the condition
            counter++;  // Direct modification of condition variable
            
            // Additional instructions to create a proper block header
            int temp = counter * 2;
            global_counter += temp;
            
            // Another modification
            if (i % 2 == 0) {
                counter += global_modifier;
            }
        }
        
        // Ensure loop has side effects
        global_modifier = (global_modifier + 1) % 5;
    }
}

// Function with side effects for test 4
__attribute__((noinline))
int side_effect_func(void) {
    static int state = 0;
    state = (state + 1) % 10;
    return state;
}

// Test 4: Function call in condition with modification in then block
__attribute__((noinline))
void test_function_call_in_condition(void) {
    volatile int a = 5, b = 10, c = 15;
    
    // Function call in condition - creates complex RTL
    for (int i = 0; i < 8; i++) {
        if (side_effect_func() > 5 && a < b) {
            // Modify variables that might affect future calls
            a = side_effect_func() + 1;  // Modifies 'a' used in condition
            b = a * 2;                   // Modifies 'b' used in condition
            
            // More instructions for block header
            c = a + b;
            global_counter += c;
            
            // Call function again to create dependencies
            int result = side_effect_func();
            a += result;
        } else {
            b = a + 1;
        }
        
        // Loop variation
        a += i;
    }
}

int main(int argc, char *argv[]) {
    // Use command line arguments to introduce runtime variability
    volatile int base_value = (argc > 1) ? atoi(argv[1]) : 42;
    
    // Initialize volatile variables
    volatile int a = base_value;
    volatile int b = base_value + 5;
    volatile int c = base_value + 10;
    volatile int x = base_value * 2;
    volatile int y = base_value * 3;
    volatile int z = base_value * 4;
    
    printf("Starting tests with base_value = %d\n", base_value);
    
    // Run all test patterns
    test_simple_modification(a, b, c);
    printf("After test 1: global_counter = %d\n", global_counter);
    
    test_compound_condition(x, y, z);
    printf("After test 2: global_counter = %d\n", global_counter);
    
    test_loop_with_modification(base_value % 10 + 5);
    printf("After test 3: global_counter = %d\n", global_counter);
    
    test_function_call_in_condition();
    printf("After test 4: global_counter = %d\n", global_counter);
    
    // Final computation to ensure all code has observable effects
    int final_result = global_counter + global_modifier;
    printf("Final result: %d\n", final_result);
    
    return final_result % 256;
}
