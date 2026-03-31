#include <stdio.h>
#include <stdlib.h>

// Global variables to prevent constant propagation
volatile int global_modifier = 0;
volatile int global_counter = 0;

// Function prototypes with noinline attribute
__attribute__((noinline)) void test_simple_modification();
__attribute__((noinline)) void test_compound_condition();
__attribute__((noinline)) void test_loop_with_modification();
__attribute__((noinline)) void test_function_call_in_condition();
__attribute__((noinline)) int side_effect_func();

// Helper function that reads input to prevent optimization
__attribute__((noinline)) int get_input() {
    int x;
    // Use inline assembly to prevent constant folding
    asm volatile("" : "=r"(x) : "0"(global_counter));
    return x;
}

// Test 1: Simple modification of condition variable in then block
__attribute__((noinline)) 
void test_simple_modification() {
    volatile int a = get_input();
    volatile int b = get_input() + 1;
    
    // Loop to create basic block structure
    for (int i = 0; i < 10; i++) {
        // Critical: 'a' is used in condition and modified in then block
        if (a > b) {
            // This modification should trigger modified_in_p check
            a = b + i;  // Modifies 'a' which is part of condition
            global_counter++;
        }
        // Add some other operations to create more complex CFG
        b += (i % 2);
    }
}

// Test 2: Compound condition with multiple modifications
__attribute__((noinline))
void test_compound_condition() {
    volatile int x = get_input();
    volatile int y = get_input() + 2;
    volatile int z = get_input() + 3;
    
    // Complex condition with multiple variables
    if (x > 0 && y < z && x != y) {
        // Multiple modifications of condition variables
        x = y * 2;      // Modifies 'x' used in condition
        y = z - 1;      // Modifies 'y' used in condition
        // Add non-label, non-note instructions
        global_modifier = x + y;
        z = global_modifier;
    }
    
    // Additional loop to prevent simplification
    for (int i = 0; i < 5; i++) {
        if (z > x) {
            x += i;
        }
    }
}

// Test 3: Modification with static variable
__attribute__((noinline))
void test_loop_with_modification() {
    static volatile int counter = 0;
    volatile int limit = get_input() % 20 + 5;
    
    // Loop where condition variable is modified in then block
    while (counter < limit) {
        // Complex condition
        if (counter > 0 && (counter % 3) == 0) {
            // This modifies 'counter' which is in the condition
            counter += 2;  // Direct modification
            global_counter += counter;
            
            // Add more instructions to ensure they're in header
            int temp = global_modifier;
            global_modifier = temp + 1;
        } else {
            counter++;
        }
        
        // Prevent infinite loops
        if (counter > 100) break;
    }
}

// Function with side effects
__attribute__((noinline))
int side_effect_func() {
    return global_counter++;
}

// Test 4: Function call in condition with modification in then block
__attribute__((noinline))
void test_function_call_in_condition() {
    volatile int a = get_input();
    volatile int b = get_input();
    
    // Function call in condition
    if (side_effect_func() > 0 && a < b) {
        // Modify variable that could affect future calls
        global_counter += 10;  // Affects side_effect_func's return
        a = b * 2;             // Modifies 'a' used in condition
        
        // Multiple instructions in then block header
        int tmp1 = a + b;
        int tmp2 = tmp1 * 3;
        global_modifier = tmp2;
    }
    
    // Nested condition
    for (int i = 0; i < 3; i++) {
        if (a > global_modifier) {
            a -= i;
            // Another modification
            global_counter = a % 10;
        }
    }
}

int main(int argc, char *argv[]) {
    // Use command line arguments to introduce runtime variability
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    // Initialize with some variability
    global_counter = seed;
    global_modifier = seed * 2;
    
    // Call all test functions
    test_simple_modification();
    test_compound_condition();
    test_loop_with_modification();
    test_function_call_in_condition();
    
    // Compute and print result to ensure side effects are observable
    int result = global_counter + global_modifier;
    printf("Result: %d (counter=%d, modifier=%d)\n", 
           result, global_counter, global_modifier);
    
    return result % 100;
}
