#include <stdio.h>
#include <stdlib.h>

// Global variables to prevent optimization
volatile int global_counter = 0;
volatile int global_modifier = 1;

// Function prototypes with noinline attribute
__attribute__((noinline)) void test_simple_modification(void);
__attribute__((noinline)) void test_compound_condition(void);
__attribute__((noinline)) void test_loop_with_modification(void);
__attribute__((noinline)) void test_function_call_in_condition(void);
__attribute__((noinline)) int get_value(void);
__attribute__((noinline)) void modify_global(void);

// Test 1: Simple modification of condition variable in then block
__attribute__((noinline)) 
void test_simple_modification(void) {
    volatile int a = 10;
    volatile int b = 5;
    volatile int result = 0;
    
    // Loop to create basic block context
    for (int i = 0; i < 100; i++) {
        // Critical: condition uses 'a', then block modifies 'a'
        if (a > b) {  // test_expr: (a > b)
            // This instruction should be in then_bb header
            a = b + i;  // MODIFIES 'a' which is in test_expr
            result += a;
        } else {
            result -= b;
        }
        // Prevent loop unrolling from simplifying too much
        b += (i % 3);
    }
    
    // Use result to prevent elimination
    global_counter += result;
}

// Test 2: Compound condition with multiple modifications
__attribute__((noinline))
void test_compound_condition(void) {
    volatile int x = 100;
    volatile int y = 50;
    volatile int z = 75;
    volatile int temp = 0;
    
    // Multiple iterations for loop context
    for (int i = 0; i < 50; i++) {
        // Complex test_expr with && operator
        if (x != 0 && y < z) {  // test_expr: (x != 0 && y < z)
            // Multiple instructions in then block header
            x = y * 2;    // MODIFIES 'x' used in first part of test_expr
            y = z + i;    // MODIFIES 'y' used in second part of test_expr
            temp += x + y;
            
            // Additional non-modifying instructions to fill header
            volatile int dummy = x * y;
            dummy = dummy / 2;
            temp += dummy;
        }
        z += (i % 5);
    }
    
    global_counter += temp;
}

// Test 3: Static variable modified in then block
__attribute__((noinline))
void test_loop_with_modification(void) {
    static volatile int counter = 0;
    volatile int limit = 1000;
    volatile int accumulator = 0;
    
    // While loop instead of for for variety
    int iterations = 0;
    while (iterations < 100) {
        // Condition uses static variable
        if (counter < limit) {  // test_expr: (counter < limit)
            // Modify the condition variable
            counter += global_modifier;  // MODIFIES 'counter' in test_expr
            
            // Additional header instructions
            accumulator += counter;
            volatile int tmp = accumulator;
            tmp = tmp >> 1;
            accumulator = tmp;
        }
        
        // Complex loop update to prevent simplification
        limit -= (iterations % 7 == 0) ? 1 : 0;
        iterations++;
        
        // External modification to force re-evaluation
        if (iterations % 13 == 0) {
            global_modifier = (global_modifier == 1) ? 2 : 1;
        }
    }
    
    global_counter += accumulator;
}

// Helper function with side effects
__attribute__((noinline))
int get_value(void) {
    static int internal = 0;
    internal = (internal + 1) % 100;
    return internal + global_counter;
}

// Function that modifies global state
__attribute__((noinline))
void modify_global(void) {
    global_counter = (global_counter + 1) % 1000;
}

// Test 4: Function call in condition with modification in then block
__attribute__((noinline))
void test_function_call_in_condition(void) {
    volatile int threshold = 50;
    volatile int data = 0;
    
    for (int i = 0; i < 75; i++) {
        // Function call in condition - creates complex test_expr
        if (get_value() > threshold) {  // test_expr involves function call
            // Modify global state that affects future get_value() calls
            modify_global();  // This affects the test_expr for future iterations
            
            // Also modify local variables
            data += i * 2;
            threshold += (i % 3);  // MODIFIES 'threshold' used in test_expr
            
            // Multiple instructions in header
            volatile int calc = data * threshold;
            calc = calc / (i + 1);
            data = calc;
        }
        
        // Vary the condition
        if (i % 11 == 0) {
            threshold -= 5;
        }
    }
    
    global_counter += data;
}

// Main function with runtime variability
int main(int argc, char *argv[]) {
    // Use command line arguments to introduce runtime variability
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    // Initialize with some variability
    global_modifier = (seed % 3) + 1;
    
    // Run all test patterns
    test_simple_modification();
    test_compound_condition();
    test_loop_with_modification();
    test_function_call_in_condition();
    
    // Print result to ensure side effects are observable
    printf("Final counter value: %d\n", global_counter);
    printf("Final modifier value: %d\n", global_modifier);
    
    return 0;
}
