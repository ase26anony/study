#include <stdio.h>
#include <stdlib.h>

// Global variables to prevent constant propagation
volatile int global_modifier = 0;
volatile int global_counter = 0;

// Function prototypes with noinline attribute
__attribute__((noinline)) void test_simple_modification();
__attribute__((noinline)) void test_compound_condition();
__attribute__((noinline)) void test_loop_with_modification();
__attribute__((noinline)) void test_function_call_side_effect();
__attribute__((noinline)) int side_effect_func();

// Prevent dead code elimination
volatile int result = 0;

// Pattern 1: Simple modification of condition variable in then block
__attribute__((noinline)) 
void test_simple_modification() {
    volatile int a = 10;
    volatile int b = 5;
    volatile int c = 0;
    
    // This creates a test_expr (a > b) that gets modified in the then block
    if (a > b) {
        // This assignment modifies 'a' which is part of the condition
        a = b + 1;  // Should trigger modified_in_p check
        c = a * 2;
    }
    
    // Add some extra instructions to ensure basic block has content
    for (int i = 0; i < 3; i++) {
        c += i;
    }
    
    result += a + b + c;
}

// Pattern 2: Compound condition with multiple modifications
__attribute__((noinline))
void test_compound_condition() {
    volatile int x = 100;
    volatile int y = 50;
    volatile int z = 75;
    volatile int w = 0;
    
    // Complex test_expr with && operator
    // Both x and y are used in the condition
    if (x > y && z < x) {
        // Modify x which is used in both parts of the condition
        x = y - 10;  // Should trigger modified_in_p for test_expr
        // Also modify z which is part of the condition
        z = x + 5;   // This also modifies part of test_expr
        w = x * z;
    }
    
    // Add loop to create interesting control flow
    int temp = 0;
    for (int i = 0; i < x; i++) {
        temp += i % 2;
    }
    
    result += x + y + z + w + temp;
}

// Pattern 3: Loop with modification in conditional block
__attribute__((noinline))
void test_loop_with_modification() {
    volatile int counter = 0;
    volatile int limit = 10;
    volatile int accumulator = 0;
    
    // Loop creates multiple basic blocks
    for (int i = 0; i < limit; i++) {
        // Condition uses counter which gets modified in then block
        if (counter < limit && i > 2) {
            // This modifies counter which is part of the condition
            counter++;  // Should trigger modified_in_p
            accumulator += counter * i;
            
            // Add more instructions to ensure header has content
            int temp = accumulator;
            for (int j = 0; j < 2; j++) {
                temp += j;
            }
            accumulator = temp;
        } else {
            accumulator += i;
        }
        
        // Additional computation to prevent simplification
        global_modifier = i % 3;
    }
    
    result += counter + accumulator;
}

// Pattern 4: Function call with side effects in condition
volatile int func_state = 0;

__attribute__((noinline))
int side_effect_func() {
    // Function with side effect
    int val = func_state + global_counter;
    global_counter++;
    return val;
}

__attribute__((noinline))
void test_function_call_side_effect() {
    volatile int threshold = 5;
    volatile int output = 0;
    
    // Function call in condition creates complex test_expr
    if (side_effect_func() > threshold && func_state < 10) {
        // Modify func_state which affects future calls to side_effect_func
        func_state += 2;  // Modifies variable used in condition
        
        // Also modify threshold which is part of the condition
        threshold = side_effect_func();  // Another function call
        
        output = func_state * threshold;
        
        // Multiple instructions in header
        int temp = output;
        temp += global_modifier;
        output = temp;
    }
    
    // Additional loop to create more basic blocks
    for (int i = 0; i < 3; i++) {
        output += side_effect_func();
    }
    
    result += output + threshold;
}

// Pattern 5: Nested conditions with modifications
__attribute__((noinline))
void test_nested_modifications() {
    volatile int a = 20, b = 15, c = 25, d = 10;
    volatile int sum = 0;
    
    // Outer condition
    if (a > b) {
        // First modification in outer then block
        a = b + 5;  // Modifies 'a' used in outer condition
        
        // Inner condition
        if (c > d && a < c) {
            // Modify 'c' which is used in inner condition
            c = d * 2;  // Should trigger modified_in_p for inner test_expr
            
            // Also modify 'a' which is used in both conditions
            a++;  // Modifies variable used in outer condition
            
            sum = a + c;
        }
        
        // More instructions in outer then block
        for (int i = 0; i < a; i++) {
            sum += i % 3;
        }
    }
    
    result += sum + a + b + c + d;
}

int main(int argc, char *argv[]) {
    // Use command line arguments to introduce runtime variability
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
        global_modifier = seed % 100;
    }
    
    // Call all test functions
    test_simple_modification();
    test_compound_condition();
    test_loop_with_modification();
    test_function_call_side_effect();
    test_nested_modifications();
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    printf("Global counter: %d\n", global_counter);
    printf("Function state: %d\n", func_state);
    
    // Read from stdin to prevent optimization
    if (argc > 2) {
        int dummy;
        printf("Enter a number: ");
        scanf("%d", &dummy);
        result += dummy;
    }
    
    return result > 0 ? 0 : 1;
}
