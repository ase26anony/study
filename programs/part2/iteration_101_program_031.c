#include <stdio.h>
#include <stdlib.h>

// Global variables to prevent optimization
volatile int global_counter = 0;
volatile int global_modifier = 1;

// Function prototypes with noinline attribute
__attribute__((noinline)) void test_simple_modification(void);
__attribute__((noinline)) void test_compound_condition(void);
__attribute__((noinline)) void test_loop_with_modification(void);
__attribute__((noinline)) void test_function_call_condition(void);
__attribute__((noinline)) int side_effect_func(void);

// Function with side effects for condition testing
__attribute__((noinline)) int side_effect_func(void) {
    return global_counter++;
}

// Test 1: Simple modification of condition variable in then block
__attribute__((noinline)) void test_simple_modification(void) {
    volatile int a = 10;
    volatile int b = 20;
    volatile int result = 0;
    
    // Loop to create basic block context
    for (int i = 0; i < 100; i++) {
        // Critical: condition uses 'a', then block modifies 'a'
        if (a > b) {  // test_expr: (a > b)
            // This instruction should be in then_bb header
            a = b;    // MODIFIES 'a' which is in test_expr
            result += 1;
        } else {
            b = a + 1;
        }
        
        // Add more instructions to ensure proper block structure
        result += a * b;
    }
    
    // Use result to prevent dead code elimination
    global_modifier += result;
}

// Test 2: Compound condition with multiple modifications
__attribute__((noinline)) void test_compound_condition(void) {
    volatile int x = 5;
    volatile int y = 15;
    volatile int z = 10;
    volatile int sum = 0;
    
    for (int i = 0; i < 50; i++) {
        // Complex test_expr with multiple variables
        if (x != 0 && y < z && z > x) {  // test_expr uses x, y, z
            // Multiple modifications of condition variables
            x = y;      // MODIFIES 'x' used in test_expr
            y++;        // MODIFIES 'y' used in test_expr
            sum += x + y + z;
            
            // Additional non-modifying instructions in header
            int temp = x * y;
            sum += temp;
        } else {
            z = x + y;
        }
        
        // Vary the condition variables
        x += i % 3;
        y -= i % 2;
    }
    
    global_modifier += sum;
}

// Test 3: Loop with modification of condition variable
__attribute__((noinline)) void test_loop_with_modification(void) {
    volatile int counter = 0;
    volatile int limit = 100;
    volatile int accumulator = 0;
    
    // Outer loop to create interesting control flow
    for (int outer = 0; outer < 10; outer++) {
        // Inner loop with condition modification
        for (int i = 0; i < 20; i++) {
            // Condition uses counter, then block modifies it
            if (counter < limit) {  // test_expr: (counter < limit)
                // Early modification in then block header
                counter++;          // MODIFIES 'counter' used in test_expr
                
                // Additional instructions before any branch
                int temp = counter * 2;
                accumulator += temp;
                
                if (accumulator > 1000) {
                    accumulator = 0;
                }
            } else {
                limit = counter / 2;
            }
        }
        
        // Mix in some unconditional modifications
        counter += outer;
    }
    
    global_counter += accumulator;
}

// Test 4: Function call in condition with side effects
__attribute__((noinline)) void test_function_call_condition(void) {
    volatile int threshold = 50;
    volatile int data[10] = {0};
    
    for (int i = 0; i < 10; i++) {
        // Function call in condition - creates complex test_expr
        if (side_effect_func() > threshold) {  // test_expr involves function call
            // Modify global variable that affects future function calls
            global_counter += 10;  // MODIFIES variable used by side_effect_func()
            
            // Additional computations in then block header
            data[i] = global_counter;
            threshold = data[i] / 2;
        } else {
            threshold += 5;
        }
        
        // Ensure loop has multiple iterations
        data[i] += i;
    }
}

int main(int argc, char *argv[]) {
    // Use command line arguments to introduce runtime variability
    int iterations = 1;
    if (argc > 1) {
        iterations = atoi(argv[1]);
        if (iterations < 1) iterations = 1;
        if (iterations > 10) iterations = 10;
    }
    
    printf("Starting IF-CONVERSION test with %d iterations\n", iterations);
    
    // Run tests multiple times to ensure coverage
    for (int i = 0; i < iterations; i++) {
        test_simple_modification();
        test_compound_condition();
        test_loop_with_modification();
        test_function_call_condition();
        
        // Modify globals between tests to prevent constant folding
        global_modifier = (global_modifier * 13 + 7) % 100;
    }
    
    // Compute and print result to ensure code isn't eliminated
    int final_result = global_counter + global_modifier;
    printf("Final result: %d (counter=%d, modifier=%d)\n", 
           final_result, global_counter, global_modifier);
    
    return final_result != 0 ? 0 : 1;
}
