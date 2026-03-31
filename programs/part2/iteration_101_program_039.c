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
__attribute__((noinline)) int side_effect_func(void);

// Test 1: Simple modification of condition variable in then block
__attribute__((noinline)) 
void test_simple_modification(void) {
    volatile int a = 10;
    volatile int b = 5;
    volatile int result = 0;
    
    // Loop to create multiple basic blocks
    for (int i = 0; i < 100; i++) {
        // Critical: 'a' is used in condition and modified in then block
        if (a > b) {  // test_expr: (a > b)
            // This assignment modifies 'a' which is part of test_expr
            a = b + i;  // Should trigger modified_in_p check
            result += a;
            
            // Add more instructions to ensure they're in the block header
            volatile int temp = a * 2;
            result += temp;
            
            // Another modification of condition variable
            if (i % 2 == 0) {
                b++;  // Also modifies 'b' from condition
            }
        } else {
            result -= 1;
        }
        
        // Prevent loop unrolling from simplifying too much
        if (i % 10 == 0) {
            global_counter++;
        }
    }
    
    // Use result to prevent dead code elimination
    global_modifier = result % 100;
}

// Test 2: Compound condition with multiple modifications
__attribute__((noinline))
void test_compound_condition(void) {
    volatile int x = 100;
    volatile int y = 50;
    volatile int z = 75;
    volatile int sum = 0;
    
    int iterations = 50;
    while (iterations-- > 0) {
        // Complex test_expr with multiple variables
        if (x > y && z < x && y != 0) {  // test_expr uses x, y, z
            // Modify x which is used in multiple parts of test_expr
            x = y + z;  // Should trigger modified_in_p
            
            // Add non-trivial instructions to the block header
            volatile int diff = x - y;
            sum += diff;
            
            // Also modify y which is in the condition
            y += global_counter;
            
            // More instructions to ensure they're in header
            z = (z * 2) % 100;
        } else {
            x = x / 2;
            sum -= x;
        }
        
        // Create data dependency to prevent reordering
        global_counter = (global_counter + sum) % 1000;
    }
}

// Test 3: Loop with modification of induction variable in condition
__attribute__((noinline))
void test_loop_with_modification(void) {
    volatile int limit = 100;
    volatile int counter = 0;
    volatile int accumulator = 0;
    
    // Outer loop
    for (int outer = 0; outer < 10; outer++) {
        counter = 0;
        
        // Inner loop where condition variable is modified
        while (counter < limit) {  // test_expr: (counter < limit)
            if (counter < (limit / 2)) {  // Nested if
                // Modify counter which is in the outer while condition
                counter += outer + 1;  // Should trigger modified_in_p
                
                // Additional instructions
                accumulator += counter;
                
                // Modify limit which is also in condition
                if (accumulator % 7 == 0) {
                    limit--;
                }
            } else {
                counter += 2;
                accumulator -= 1;
            }
            
            // Complex computation to prevent simplification
            accumulator = (accumulator * 13 + 17) % 1000;
        }
        
        // Vary limit to prevent constant propagation
        limit = 80 + (outer % 20);
    }
}

// Helper function with side effects
__attribute__((noinline))
int side_effect_func(void) {
    static int state = 0;
    state = (state * 1103515245 + 12345) & 0x7fffffff;
    return (state >> 16) & 0xFF;
}

// Test 4: Function call in condition with modification
__attribute__((noinline))
void test_function_call_in_condition(void) {
    volatile int a = 0;
    volatile int b = 100;
    volatile int c = 50;
    int result = 0;
    
    for (int i = 0; i < 30; i++) {
        // Function call in condition - creates complex test_expr
        if ((a < b) && (side_effect_func() > c)) {
            // Modify 'a' which is used in the condition
            a = side_effect_func();  // Should trigger modified_in_p
            
            // Multiple instructions in block header
            volatile int temp = a + b + c;
            result += temp;
            
            // Modify 'b' which is also in condition
            b -= global_counter;
            
            // More complex operations
            c = (a * b) % 100;
        } else {
            a++;
            result -= b;
        }
        
        // Cross-modification to prevent optimization
        global_counter = (global_counter + i) % 256;
    }
}

int main(int argc, char *argv[]) {
    // Use command line arguments to introduce runtime variability
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
        global_counter = seed % 100;
    }
    
    printf("Starting IF-CONVERSION test with seed: %d\n", seed);
    
    // Run all test patterns
    test_simple_modification();
    printf("Test 1 completed. Global counter: %d\n", global_counter);
    
    test_compound_condition();
    printf("Test 2 completed. Global counter: %d\n", global_counter);
    
    test_loop_with_modification();
    printf("Test 3 completed. Global counter: %d\n", global_counter);
    
    test_function_call_in_condition();
    printf("Test 4 completed. Global counter: %d\n", global_counter);
    
    // Final computation to ensure all code has observable effects
    int final_result = global_counter + global_modifier;
    printf("Final result: %d\n", final_result);
    
    return final_result % 2;
}
