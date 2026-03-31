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

// Function with side effects used in conditions
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
        // Condition where 'a' is tested
        if (a > b) {
            // MODIFICATION IN THEN BLOCK: 'a' is modified here
            // This should trigger modified_in_p check
            a = b - 1;  // Modifies 'a' which is part of condition
            result += a;
        } else {
            b = a + 1;
            result -= b;
        }
        
        // Additional instructions to create non-trivial basic block
        volatile int temp = a * b;
        result += temp % 7;
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
    
    // Multiple iterations to create loop context
    for (int i = 0; i < 50; i++) {
        // Compound condition using && with multiple variables
        if (x < y && y > z) {
            // MODIFICATIONS: Both x and y are modified
            // x is used in first part of condition
            // y is used in both parts of condition
            x = y + z;  // Modifies x which is in condition
            y = z - 1;  // Modifies y which is in condition
            sum += x * y;
        } else {
            z = x + y;
            sum -= z;
        }
        
        // Complex expression to prevent simplification
        volatile int tmp = (x ^ y) | z;
        sum += tmp & 0xFF;
    }
    
    global_modifier *= (sum % 100);
}

// Test 3: Modification of variable used in loop condition
__attribute__((noinline)) void test_loop_with_modification(void) {
    volatile int counter = 0;
    volatile int limit = 100;
    volatile int accumulator = 0;
    
    // Loop where condition variable is modified in then block
    while (counter < limit) {
        // Nested if inside loop
        if (counter > limit / 2) {
            // MODIFICATION: counter is modified in then block
            // counter is used in the while condition
            counter += 2;  // This modifies the loop condition variable
            accumulator += counter * 3;
        } else {
            counter++;
            accumulator -= counter;
        }
        
        // Additional computation
        volatile int check = accumulator % 13;
        if (check == 0) {
            limit--;  // Also modify limit which is in while condition
        }
    }
    
    global_modifier += accumulator;
}

// Test 4: Function call in condition with side effects
__attribute__((noinline)) void test_function_call_condition(void) {
    volatile int threshold = 50;
    volatile int total = 0;
    
    // Reset global counter
    global_counter = 0;
    
    for (int i = 0; i < 100; i++) {
        // Function call in condition - creates complex RTL
        if (side_effect_func() > threshold) {
            // MODIFICATION: Modify global variable that function reads
            global_counter += 5;  // Modifies variable used by side_effect_func
            total += global_counter;
            
            // Also modify threshold which is in condition
            threshold -= 1;  // This modifies the condition variable
        } else {
            total -= global_counter;
            threshold += 2;
        }
        
        // Additional instructions to create block header
        volatile int mix = total ^ threshold;
        mix = mix * 1103515245 + 12345;
        total += (mix >> 16) & 0x7FFF;
    }
    
    global_modifier ^= total;
}

int main(int argc, char *argv[]) {
    // Use command line arguments to introduce runtime variability
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    // Initialize volatile variables with some randomness
    srand(seed);
    global_modifier = rand() % 100;
    
    printf("Starting IF-CONVERSION test patterns...\n");
    printf("Initial global_modifier: %d\n", global_modifier);
    
    // Execute all test patterns
    test_simple_modification();
    printf("After test_simple_modification: %d\n", global_modifier);
    
    test_compound_condition();
    printf("After test_compound_condition: %d\n", global_modifier);
    
    test_loop_with_modification();
    printf("After test_loop_with_modification: %d\n", global_modifier);
    
    test_function_call_condition();
    printf("After test_function_call_condition: %d\n", global_modifier);
    
    // Final computation to ensure all code has observable effects
    int final_result = global_modifier * 2 + global_counter;
    printf("Final result: %d (global_counter: %d)\n", final_result, global_counter);
    
    return final_result % 256;
}
