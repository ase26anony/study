#include <stdio.h>
#include <stdlib.h>

// Global variables to prevent constant propagation
volatile int global_modifier = 0;
volatile int global_counter = 0;

// Function with side effects for use in conditions
__attribute__((noinline)) 
int side_effect_func(void) {
    return global_modifier++;
}

// Test function 1: Simple modification in then block
__attribute__((noinline))
void test_simple_modification(void) {
    volatile int a = 10, b = 5, c = 0;
    
    // Loop to create basic block context
    for (int i = 0; i < 100; i++) {
        // CRITICAL: Condition uses 'a', then block modifies 'a'
        if (a > b) {  // test_expr: (a > b)
            // These instructions are in the then block header
            a = b + i;  // MODIFIES 'a' which is in test_expr
            c = a * 2;
            // Additional instructions to ensure non-empty header
            b = c - 1;
        } else {
            b = a + 1;
        }
        
        // Prevent loop unrolling from simplifying too much
        if (i % 10 == 0) {
            global_counter++;
        }
    }
}

// Test function 2: Compound condition with multiple modifications
__attribute__((noinline))
void test_compound_condition(void) {
    volatile int x = 100, y = 50, z = 75;
    volatile int flag = 0;
    
    while (x > 0) {
        // Complex test_expr with multiple variables
        if (x > y && z < (x + y) && flag == 0) {
            // Multiple modifications to variables in test_expr
            x = y * 2;      // MODIFIES 'x' used in test_expr
            y = z + 1;      // MODIFIES 'y' used in test_expr
            z = x - y;      // MODIFIES 'z' used in test_expr
            // More instructions in header
            flag = 1;
            global_modifier = x;
        } else {
            x--;
            flag = 0;
        }
        
        // Add function call to prevent over-optimization
        if (side_effect_func() > 50) {
            break;
        }
    }
}

// Test function 3: Modification through pointer/array
__attribute__((noinline))
void test_pointer_modification(void) {
    volatile int arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    volatile int *ptr = arr;
    volatile int idx = 0;
    
    for (int i = 0; i < 9; i++) {
        // Condition uses array element, then modifies it
        if (arr[idx] > arr[idx + 1] && ptr[idx] < 10) {
            // Direct modification of array element used in condition
            arr[idx] = arr[idx + 1];  // MODIFIES arr[idx] in test_expr
            ptr[idx + 1] = ptr[idx] * 2;
            idx = (idx + 1) % 9;
            
            // Additional non-trivial operations
            global_counter += arr[idx];
        } else {
            idx = (idx + 2) % 9;
        }
    }
}

// Test function 4: Nested conditions with modifications
__attribute__((noinline))
void test_nested_modification(void) {
    volatile int a = 100, b = 200, c = 300;
    static volatile int static_var = 0;
    
    for (int iter = 0; iter < 50; iter++) {
        // Outer condition
        if (a < b || c > static_var) {
            // Inner condition that modifies outer condition variables
            if (b > a + static_var) {
                // These should be in the then block header
                a = b - c;      // MODIFIES 'a' from outer condition
                static_var++;   // MODIFIES static_var from outer condition
                b = a * 2;
                
                // Call function that might affect subsequent conditions
                side_effect_func();
            }
            c = a + b;
        }
        
        // Mix in some unpredictable control flow
        switch (iter % 3) {
            case 0: a += 1; break;
            case 1: b -= 1; break;
            case 2: c *= 1; break;
        }
    }
}

// Test function 5: Modification via function call in then block
__attribute__((noinline))
int modify_condition_var(int *var) {
    *var = *var / 2;
    return *var;
}

__attribute__((noinline))
void test_function_call_modification(void) {
    volatile int p = 1000, q = 500, r = 750;
    
    while (p > 100) {
        // Condition with multiple variables
        if (p > q && (p - q) > r) {
            // Function call modifies 'p' which is in test_expr
            modify_condition_var(&p);  // MODIFIES 'p' used in condition
            q = r + p;
            r = p - q;
            
            // Additional arithmetic
            global_modifier += p;
        } else {
            p -= 50;
            q += 25;
        }
        
        // Prevent infinite loop optimization
        if (side_effect_func() > 1000) {
            break;
        }
    }
}

int main(int argc, char *argv[]) {
    // Use command line arguments to introduce runtime variability
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    // Initialize with some randomness
    global_modifier = seed;
    global_counter = seed % 100;
    
    // Execute all test functions
    test_simple_modification();
    test_compound_condition();
    test_pointer_modification();
    test_nested_modification();
    test_function_call_modification();
    
    // Compute and print result to ensure side effects are observable
    int result = global_modifier + global_counter;
    printf("Result: %d (global_modifier=%d, global_counter=%d)\n", 
           result, global_modifier, global_counter);
    
    return result % 2;
}
