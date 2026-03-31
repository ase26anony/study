#include <stdio.h>
#include <stdlib.h>

// Global variables to prevent optimization
volatile int global_counter = 0;
volatile int global_modifier = 1;

// Function prototypes with noinline attribute
__attribute__((noinline)) void test_simple_modification(void);
__attribute__((noinline)) void test_compound_condition(void);
__attribute__((noinline)) void test_loop_with_modification(void);
__attribute__((noinline)) void test_function_call_side_effect(void);
__attribute__((noinline)) int side_effect_func(void);

// Test 1: Simple modification of condition variable in then block
__attribute__((noinline)) 
void test_simple_modification(void) {
    volatile int a = 10;
    volatile int b = 5;
    volatile int c = 0;
    
    // This creates a then block where 'a' is modified
    // The condition uses 'a', and it's modified in the then block
    if (a > b) {
        // This assignment modifies 'a' which is part of the condition
        a = b + 1;  // Modifies condition variable
        c = a * 2;  // Additional instruction to create block header
        global_counter += c;  // Prevent dead code elimination
    }
    
    // Loop to create more complex control flow
    for (int i = 0; i < 3; i++) {
        if (a < b + i) {
            a = i;  // Another modification in nested if
        }
    }
}

// Test 2: Compound condition with multiple modifications
__attribute__((noinline))
void test_compound_condition(void) {
    volatile int x = 0;
    volatile int y = 10;
    volatile int z = 5;
    
    // Compound condition with && - creates complex test_expr
    if (x != 0 && y > z) {
        // Multiple instructions in header before any branch
        x = y;      // Modifies 'x' used in first part of condition
        y = z - 1;  // Modifies 'y' used in second part of condition
        z = x + y;  // Additional instruction
        global_counter += x;
    }
    
    // Another case with || operator
    volatile int p = 8, q = 3, r = 7;
    if (p > q || r < 10) {
        p = r;      // Modifies 'p' used in condition
        q = p + 2;  // Chain of modifications
        r = q - 1;
    }
}

// Test 3: Loop with modification of condition variable
__attribute__((noinline))
void test_loop_with_modification(void) {
    volatile int counter = 0;
    volatile int limit = 5;
    volatile int accumulator = 0;
    
    // Loop where condition variable is modified in then block
    for (int i = 0; i < 10; i++) {
        // Condition uses 'counter' and 'limit'
        if (counter < limit && accumulator < 20) {
            // These instructions will be in the then block header
            counter++;          // Modifies 'counter' used in condition
            accumulator += i;   // Additional instruction
            limit = counter + 2; // Modifies 'limit' used in condition
        } else {
            accumulator -= 1;
        }
    }
    
    global_counter += accumulator;
}

// Helper function with side effects
__attribute__((noinline))
int side_effect_func(void) {
    static int state = 0;
    state = (state + global_modifier) % 10;
    return state;
}

// Test 4: Function call in condition with side effects
__attribute__((noinline))
void test_function_call_side_effect(void) {
    volatile int threshold = 5;
    
    // Function call in condition - creates complex RTL
    if (side_effect_func() > threshold) {
        // Modify global variable that affects future calls to side_effect_func
        global_modifier = 2;  // Changes behavior of side_effect_func
        threshold = side_effect_func();  // Another call with modified behavior
        global_counter += threshold;
    }
    
    // Nested case
    volatile int a = 3, b = 7;
    if (side_effect_func() < a || b > 5) {
        a = side_effect_func();  // Function call in then block
        b = a * 2;               // Regular modification
        global_modifier = a % 3; // Modify the global modifier
    }
}

// Main function with runtime variability
int main(int argc, char *argv[]) {
    // Use command line arguments to introduce runtime variability
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    // Initialize volatile variables with runtime-dependent values
    volatile int init_val = seed % 100;
    global_counter = init_val;
    global_modifier = (seed % 5) + 1;
    
    // Call all test functions multiple times with different patterns
    for (int iteration = 0; iteration < 3; iteration++) {
        test_simple_modification();
        test_compound_condition();
        test_loop_with_modification();
        test_function_call_side_effect();
        
        // Modify globals between iterations
        global_counter += iteration;
        global_modifier = (global_modifier + 1) % 4;
    }
    
    // Compute and print result to ensure side effects are observable
    printf("Final counter value: %d\n", global_counter);
    printf("Final modifier value: %d\n", global_modifier);
    
    // Read from stdin to prevent optimization
    if (argc > 2) {
        int dummy;
        printf("Enter a number: ");
        scanf("%d", &dummy);
        global_counter += dummy;
    }
    
    return global_counter > 0 ? 0 : 1;
}
