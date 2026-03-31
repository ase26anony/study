#include <stdio.h>
#include <stdlib.h>

// Global variables to prevent optimization
volatile int global_counter = 0;
volatile int global_modifier = 1;

// Function prototypes with noinline attribute
__attribute__((noinline)) void test_simple_modification();
__attribute__((noinline)) void test_compound_condition();
__attribute__((noinline)) void test_loop_with_modification();
__attribute__((noinline)) void test_function_call_in_condition();
__attribute__((noinline)) void test_nested_modifications();
__attribute__((noinline)) int side_effect_func(int* ptr);

// Helper function that reads input to prevent constant folding
__attribute__((noinline)) int get_input_value() {
    static int counter = 0;
    return counter++ % 5;
}

// Test 1: Simple modification of condition variable in then block
__attribute__((noinline)) 
void test_simple_modification() {
    volatile int a = 10 + get_input_value();
    volatile int b = 5;
    volatile int result = 0;
    
    // Loop to create basic blocks for if-conversion
    for (int i = 0; i < 100; i++) {
        // Critical: 'a' is used in condition and modified in then block
        if (a > b) {  // test_expr: (a > b)
            a = b;    // This modifies 'a' which is part of test_expr
            result++;
        } else {
            b++;
        }
        
        // Additional code to prevent tail optimization
        if (i % 10 == 0) {
            global_counter++;
        }
    }
    
    // Use result to prevent dead code elimination
    global_modifier += result;
}

// Test 2: Compound condition with multiple modifications
__attribute__((noinline))
void test_compound_condition() {
    volatile int x = 20 + get_input_value();
    volatile int y = 15;
    volatile int z = 25;
    volatile int temp = 0;
    
    for (int i = 0; i < 50; i++) {
        // Complex test_expr with multiple variables
        if (x != 0 && y < z && z > 10) {  // test_expr uses x, y, z
            // Modify x which is used in the first part of condition
            x = y + 1;    // First modifying instruction in then block header
            y = z - 1;    // Second instruction - still in header
            temp = x * y; // Third instruction
            global_counter += temp;
        } else {
            z = x + y;
        }
        
        // Vary the condition for next iteration
        if (i % 3 == 0) {
            x += get_input_value();
        }
    }
}

// Test 3: Static variable modified in then block
__attribute__((noinline))
void test_loop_with_modification() {
    static volatile int counter = 0;
    volatile int limit = 30 + get_input_value();
    volatile int accumulator = 0;
    
    // Multiple iterations to create interesting block structures
    while (counter < limit) {
        // Condition uses counter, modified in then block
        if (counter < limit / 2) {  // test_expr: (counter < limit/2)
            counter++;  // Modifies variable used in condition
            accumulator += counter;
            
            // Additional non-label, non-note instructions in header
            int temp = accumulator * 2;
            global_modifier ^= temp;
        } else {
            accumulator -= counter;
        }
        
        // Force loop continuation condition
        if (accumulator > 1000) break;
    }
}

// Function with side effects for use in conditions
__attribute__((noinline))
int side_effect_func(int* ptr) {
    (*ptr)++;
    return *ptr % 3;
}

// Test 4: Function call in condition with modification in then block
__attribute__((noinline))
void test_function_call_in_condition() {
    volatile int state = 10 + get_input_value();
    volatile int threshold = 15;
    volatile int output = 0;
    
    for (int iteration = 0; iteration < 40; iteration++) {
        // Function call in condition - creates complex RTL
        if (side_effect_func((int*)&state) > 0 && state < threshold) {
            // Modify state which is used by side_effect_func
            state = threshold - 1;  // Modifies variable read by condition function
            output += state;
            
            // Multiple instructions in then block header
            int intermediate = output * 3;
            global_counter += intermediate;
            intermediate /= 2;
            global_modifier |= intermediate;
        } else {
            threshold++;
        }
        
        // Prevent infinite loops
        if (iteration > 35) break;
    }
}

// Test 5: Nested modifications with pointer aliasing
__attribute__((noinline))
void test_nested_modifications() {
    volatile int base = 100 + get_input_value();
    volatile int compare = 90;
    volatile int* ptr1 = &base;
    volatile int* ptr2 = &compare;
    volatile int sum = 0;
    
    // Complex loop structure
    for (int outer = 0; outer < 10; outer++) {
        for (int inner = 0; inner < 5; inner++) {
            // Condition with pointer dereference
            if (*ptr1 > *ptr2 && base < 150) {  // test_expr uses *ptr1, *ptr2, base
                // Multiple modifications in then block header
                *ptr1 = *ptr2 + inner;  // Modifies *ptr1 which is in condition
                base = *ptr1;           // Modifies base which is also in condition
                sum += base;
                
                // More header instructions
                int calc = sum * outer;
                global_counter ^= calc;
                calc = calc / (inner + 1);
                global_modifier += calc;
            } else {
                *ptr2 = *ptr1 - inner;
            }
        }
        
        // Change pointers to create aliasing effects
        if (outer % 2 == 0) {
            ptr1 = &compare;
            ptr2 = &base;
        } else {
            ptr1 = &base;
            ptr2 = &compare;
        }
    }
}

int main(int argc, char** argv) {
    // Use command line arguments to introduce runtime variability
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
        global_counter = seed;
    }
    
    printf("Starting if-conversion tests with seed: %d\n", seed);
    
    // Run all test patterns
    test_simple_modification();
    printf("After test_simple_modification: global_counter = %d\n", global_counter);
    
    test_compound_condition();
    printf("After test_compound_condition: global_counter = %d\n", global_counter);
    
    test_loop_with_modification();
    printf("After test_loop_with_modification: global_counter = %d\n", global_counter);
    
    test_function_call_in_condition();
    printf("After test_function_call_in_condition: global_counter = %d\n", global_counter);
    
    test_nested_modifications();
    printf("After test_nested_modifications: global_counter = %d\n", global_counter);
    
    // Final computation to ensure all code has observable effects
    int final_result = global_counter + global_modifier;
    printf("Final result: %d\n", final_result);
    
    return final_result % 256;
}
