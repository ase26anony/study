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
    
    // This creates a basic block where 'a' is modified in the then block
    // The condition uses 'a', and it's modified inside
    for (int i = 0; i < 100; i++) {
        if (a > b) {  // test_expr: a > b
            a = b + i;  // Modifies 'a' which is part of the condition
            result += a;
        } else {
            result -= b;
        }
        // Prevent loop unrolling from simplifying too much
        b += (i % 3);
    }
    
    // Use result to prevent dead code elimination
    global_counter += result;
}

// Test 2: Compound condition with multiple modifications
__attribute__((noinline))
void test_compound_condition(void) {
    volatile int x = 100;
    volatile int y = 50;
    volatile int z = 75;
    volatile int w = 25;
    
    // Complex test_expr with multiple variables
    for (int i = 0; i < 50; i++) {
        // Compound condition: (x > y && z < w)
        if (x > y && z < (w + i)) {
            // Modify x which is used in the first part of condition
            x = y * 2;  // This should trigger modified_in_p check
            // Also modify z which is used in second part
            z = w + 10;
            global_counter += x + z;
        }
        
        // Additional modifications to create more complex CFG
        if (y < z || x > w) {
            y++;
            w = x % 20;
        }
        
        // Vary the values to prevent constant propagation
        x -= (i % 7);
        z += (i % 5);
    }
}

// Test 3: Loop with modification of condition variable
__attribute__((noinline))
void test_loop_with_modification(void) {
    static volatile int counter = 0;
    volatile int limit = 1000;
    volatile int accumulator = 0;
    
    // The condition variable 'counter' is modified in the then block
    while (counter < limit) {
        if (counter < (limit / 2)) {
            // Modify 'counter' which is part of the condition
            counter += global_modifier;  // This should trigger the check
            accumulator += counter;
            
            // Add more instructions to the header
            int temp = counter * 2;
            accumulator -= temp;
            global_modifier = (accumulator % 10) + 1;
        } else {
            accumulator += limit;
            counter += 2;
        }
        
        // Additional condition to create more basic blocks
        if (accumulator > 5000) {
            limit -= 100;
        }
    }
    
    global_counter += accumulator;
}

// Helper function that returns a value
__attribute__((noinline))
int get_value(void) {
    return global_counter % 100;
}

// Helper function that modifies global state
__attribute__((noinline))
void modify_global(void) {
    global_modifier = (global_modifier * 1103515245 + 12345) & 0x7fffffff;
}

// Test 4: Function call in condition with side effects
__attribute__((noinline))
void test_function_call_in_condition(void) {
    volatile int a = 0;
    volatile int b = 100;
    volatile int temp_result = 0;
    
    for (int i = 0; i < 200; i++) {
        // Function call in condition - creates complex RTL
        if ((a < b) && (get_value() > 50)) {
            // Modify 'a' which is used in the condition
            a = get_value();  // This modification should be checked
            
            // Also call function that modifies global state
            modify_global();
            
            // Multiple instructions in the then block header
            temp_result += a * global_modifier;
            b -= global_modifier % 5;
            
            // Additional modification of condition variable
            if (temp_result > 1000) {
                a = temp_result / 10;
            }
        } else {
            b += get_value();
            temp_result -= b;
        }
        
        // Vary the values
        a += (i % 13);
        if (i % 37 == 0) {
            modify_global();
        }
    }
    
    global_counter += temp_result;
}

int main(int argc, char *argv[]) {
    // Use command line arguments to introduce runtime variability
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    // Initialize with some variability
    global_counter = seed;
    global_modifier = (seed % 50) + 1;
    
    // Run all test patterns
    test_simple_modification();
    test_compound_condition();
    test_loop_with_modification();
    test_function_call_in_condition();
    
    // Print result to ensure code isn't eliminated
    printf("Final counter: %d\n", global_counter);
    printf("Final modifier: %d\n", global_modifier);
    
    return 0;
}
