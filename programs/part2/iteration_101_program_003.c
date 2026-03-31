#include <stdio.h>
#include <stdlib.h>

// Global variables that can be modified in conditions
volatile int global_a = 0;
volatile int global_b = 0;
volatile int global_c = 0;
static volatile int static_counter = 0;

// Function with side effects for use in conditions
volatile int __attribute__((noinline)) side_effect_func(void) {
    return global_a + global_b;
}

// Test function 1: Simple modification in then block
void __attribute__((noinline)) test_simple_modification(void) {
    volatile int a = global_a;
    volatile int b = global_b;
    
    // Loop to create basic blocks for if-conversion
    for (int i = 0; i < 10; i++) {
        // Condition where 'a' is tested and modified in then block
        if (a > b) {
            // This modifies 'a' which is part of the condition
            a = b + i;  // Modifies condition variable early in then block
            global_c += a;
        } else {
            b = a + i;
        }
        
        // Additional instructions to prevent block merging
        asm volatile("" : : : "memory");
    }
    
    global_a = a;
    global_b = b;
}

// Test function 2: Compound condition with multiple modifications
void __attribute__((noinline)) test_compound_condition(void) {
    volatile int x = global_a;
    volatile int y = global_b;
    volatile int z = global_c;
    
    // Create a loop with complex control flow
    for (int i = 0; i < 8; i++) {
        // Compound condition using multiple variables
        if (x != 0 && y < z && (x + y) > i) {
            // Modify multiple condition variables in then block
            x = y + 1;      // Modifies 'x' used in (x != 0)
            y = z - i;      // Modifies 'y' used in (y < z)
            z++;            // Modifies 'z' used in (y < z)
            
            // Additional non-trivial instructions
            global_a ^= x;
            global_b |= y;
        } else {
            x = i;
            y = x * 2;
        }
        
        // Memory barrier to prevent reordering
        asm volatile("" : : : "memory");
    }
    
    global_c = z;
}

// Test function 3: Static variable modification in condition
void __attribute__((noinline)) test_static_modification(void) {
    volatile int limit = 5;
    
    for (int i = 0; i < 10; i++) {
        // Condition using static variable
        if (static_counter < limit && global_a > i) {
            // Modify static variable used in condition
            static_counter++;  // Modifies condition variable
            
            // Also modify global_a which is part of condition
            global_a--;
            
            // Complex expression to create interesting RTL
            global_b = (global_a * static_counter) / (limit + 1);
        } else {
            static_counter--;
            if (static_counter < 0) static_counter = 0;
        }
        
        // Prevent optimization
        asm volatile("" : : : "memory");
    }
}

// Test function 4: Function call in condition with side effects
void __attribute__((noinline)) test_func_call_condition(void) {
    volatile int prev_value = 0;
    
    for (int i = 0; i < 6; i++) {
        // Function call in condition
        int current = side_effect_func();
        
        if (current > prev_value && global_c < 100) {
            // Modify globals that affect future function calls
            global_a += i;      // Affects side_effect_func()
            global_b -= 1;      // Affects side_effect_func()
            
            // Also modify variable used in condition
            global_c = current; // Modifies 'global_c' used in (global_c < 100)
            
            prev_value = current * 2;
        } else {
            global_c += i;
            prev_value = current / 2;
        }
        
        // Complex control to create basic block boundaries
        switch (i % 3) {
            case 0: global_a ^= 1; break;
            case 1: global_b ^= 2; break;
            case 2: global_c ^= 3; break;
        }
    }
}

// Test function 5: Nested conditions with modifications
void __attribute__((noinline)) test_nested_modifications(void) {
    volatile int a = global_a;
    volatile int b = global_b;
    volatile int c = global_c;
    
    // Outer loop
    for (int i = 0; i < 5; i++) {
        // Inner loop to create more complex CFG
        for (int j = 0; j < 3; j++) {
            // Multiple conditions in sequence
            if (a > b) {
                b = a + j;  // Modifies 'b' used in next condition
                
                // Additional instruction before next condition check
                c += i;
                
                if (b < c) {
                    // Modify 'a' which was used in outer condition
                    a = c - b;  // Modifies condition variable
                    global_a = a;
                }
            }
            
            // Alternate path
            if (c == a + b) {
                a = j;
                b = i;
            }
        }
        
        // Prevent tail merging
        asm volatile("" : : : "memory");
    }
}

int main(int argc, char *argv[]) {
    // Initialize with command-line arguments for runtime variability
    if (argc > 1) {
        global_a = atoi(argv[1]) % 100;
    }
    if (argc > 2) {
        global_b = atoi(argv[2]) % 100;
    }
    if (argc > 3) {
        global_c = atoi(argv[3]) % 100;
    }
    
    // Reset static counter
    static_counter = 0;
    
    // Execute all test functions
    test_simple_modification();
    test_compound_condition();
    test_static_modification();
    test_func_call_condition();
    test_nested_modifications();
    
    // Compute and print result to ensure side effects are observable
    int result = global_a + global_b + global_c + static_counter;
    printf("Result: %d (a=%d, b=%d, c=%d, counter=%d)\n", 
           result, global_a, global_b, global_c, static_counter);
    
    return result != 0 ? 0 : 1;
}
