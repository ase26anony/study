#include <stdio.h>
#include <stdlib.h>

// Volatile variables to prevent optimization
volatile int a = 5, b = 10, c = 15, d = 20;
volatile int x = 0, y = 1, z = 2;
volatile int counter = 0;
static volatile int static_var = 100;
volatile int limit = 5;

// Function with side effects
volatile int side_effect_func(void) {
    static volatile int state = 0;
    return state++;
}

// Test function 1: Simple modification in then block
__attribute__((noinline))
void test_simple_modification(void) {
    // Loop context to create interesting block structure
    for (int i = 0; i < 10; i++) {
        // Condition variable 'a' modified in then block
        if (a > b) {
            // This modifies 'a' which is used in the condition
            a = b;  // Should trigger modified_in_p check
            x = i * 2;
        }
        // Prevent loop unrolling from simplifying too much
        if (i % 3 == 0) {
            b++;
        }
    }
}

// Test function 2: Compound condition with multiple modifications
__attribute__((noinline))
void test_compound_condition(void) {
    volatile int local_a = a;
    volatile int local_b = b;
    volatile int local_c = c;
    
    // Complex condition with multiple variables
    if (local_a != 0 && local_b < local_c) {
        // Modify 'local_b' which is used in the condition
        local_b = local_a;  // First modification
        // Also modify 'local_a' which is also in condition
        local_a = 0;        // Second modification
        // Add more instructions to ensure they're in header
        local_c = local_b * 2;
        x = local_a + local_b + local_c;
    }
    
    // Use results to prevent elimination
    a = local_a;
    b = local_b;
    c = local_c;
}

// Test function 3: Static variable modification
__attribute__((noinline))
void test_static_modification(void) {
    // Loop with static variable in condition
    for (int i = 0; i < 8; i++) {
        // Static variable used in condition, modified in then block
        if (static_var < limit) {
            // This modifies static_var which is in the condition
            static_var++;  // Should trigger modified_in_p check
            counter++;
            // Additional instructions in header
            y = static_var * i;
            z = counter + y;
        }
        
        // Vary limit to create different paths
        if (i % 2 == 0) {
            limit--;
        }
    }
}

// Test function 4: Function call in condition with side effects
__attribute__((noinline))
void test_func_call_condition(void) {
    volatile int result1, result2;
    
    // Multiple function calls in condition
    result1 = side_effect_func();
    result2 = side_effect_func();
    
    // Complex condition with function results
    if (result1 > 0 && result2 < 10) {
        // Modify global state that function reads
        counter = result1 + result2;  // Modifies counter
        // Function uses counter indirectly through global state
        a = side_effect_func() + counter;
        b = a * 2;
    }
    
    // Additional control flow to create basic blocks
    switch (counter % 3) {
        case 0: x = 1; break;
        case 1: x = 2; break;
        case 2: x = 3; break;
    }
}

// Test function 5: Nested conditions with modifications
__attribute__((noinline))
void test_nested_modifications(void) {
    volatile int p = a, q = b, r = c;
    
    // Outer condition
    if (p > q) {
        // Inner condition with modification
        if (q < r) {
            // Modify q which is used in outer condition
            q = p + r;  // Modifies variable from outer condition
            p = q / 2;  // Modifies variable from outer condition
        } else {
            r = p - q;
        }
        // More modifications in outer then block
        a = p;
        b = q;
        c = r;
    }
    
    // Loop to create more complex CFG
    for (int i = 0; i < 5; i++) {
        if (i % 2 == 0 && a < b) {
            a++;
            b--;
        }
    }
}

// Test function 6: Pointer aliasing potential
__attribute__((noinline))
void test_pointer_aliasing(void) {
    volatile int val1 = 10, val2 = 20;
    volatile int *ptr1 = &val1;
    volatile int *ptr2 = &val2;
    
    // Condition with pointer dereference
    if (*ptr1 > *ptr2 && val1 < 30) {
        // Modify through pointer - could alias with val1
        *ptr1 = *ptr2 + 5;  // Modifies val1 which is in condition
        val2 = *ptr1 * 2;   // Modifies val2 which is in condition
        // Additional instructions
        x = val1 + val2;
        y = x * 3;
    }
    
    // Prevent dead code elimination
    a = val1;
    b = val2;
}

int main(int argc, char *argv[]) {
    // Use command line arguments to introduce runtime variability
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
        a = seed % 20;
        b = (seed + 1) % 20;
        c = (seed + 2) % 20;
    }
    
    printf("Starting values: a=%d, b=%d, c=%d, counter=%d\n", a, b, c, counter);
    
    // Call all test functions
    test_simple_modification();
    printf("After test 1: a=%d, b=%d, x=%d\n", a, b, x);
    
    test_compound_condition();
    printf("After test 2: a=%d, b=%d, c=%d\n", a, b, c);
    
    test_static_modification();
    printf("After test 3: static_var=%d, counter=%d, limit=%d\n", static_var, counter, limit);
    
    test_func_call_condition();
    printf("After test 4: counter=%d, a=%d, b=%d\n", counter, a, b);
    
    test_nested_modifications();
    printf("After test 5: a=%d, b=%d, c=%d\n", a, b, c);
    
    test_pointer_aliasing();
    printf("After test 6: a=%d, b=%d, x=%d, y=%d\n", a, b, x, y);
    
    // Final computation to ensure all code has effect
    int result = a + b + c + x + y + z + counter + static_var;
    printf("Final result: %d\n", result);
    
    return result % 256;
}
