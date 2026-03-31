#include <stdio.h>
#include <stdlib.h>

// Global variables to prevent optimization
volatile int global_modifier = 0;
static volatile int static_counter = 0;

// Function with side effects
int __attribute__((noinline)) side_effect_func(void) {
    return global_modifier++;
}

// Test function 1: Simple modification in then block
void __attribute__((noinline)) test_modify_in_then(void) {
    volatile int a = 10, b = 5, c = 0;
    
    // Loop to create basic block context
    for (int i = 0; i < 100; i++) {
        // CRITICAL: Condition uses 'a', then block modifies 'a'
        if (a > b && c < 20) {  // Compound condition
            a = b;  // This modifies 'a' which is used in condition
            c++;    // Additional instruction in header
            b += 2; // Another instruction
        } else {
            b = a;
        }
        
        // Prevent loop unrolling from simplifying too much
        if (i % 10 == 0) {
            global_modifier ^= i;
        }
    }
}

// Test function 2: Complex condition with multiple modifications
void __attribute__((noinline)) test_complex_modification(void) {
    volatile int x = 100, y = 50, z = 75;
    volatile int* ptr = &x;
    
    for (int i = 0; i < 50; i++) {
        // Complex condition with function call
        if ((x != y || z > 25) && side_effect_func() < 100) {
            // Multiple modifications of condition variables
            x = y + z;  // Modifies 'x' used in condition
            y++;        // Modifies 'y' used in condition
            *ptr = i;   // Pointer write that could affect condition
            z = x - y;  // Modifies 'z' used in condition
        }
        
        // Mix in some arithmetic to prevent dead code elimination
        z = (z * 3 + 7) % 100;
    }
}

// Test function 3: Static variable modification
void __attribute__((noinline)) test_static_modification(void) {
    volatile int limit = 1000;
    
    for (int i = 0; i < 200; i++) {
        // Condition uses static variable
        if (static_counter < limit && global_modifier > 0) {
            static_counter++;  // Modifies static_counter used in condition
            limit--;           // Modifies limit used in condition
            global_modifier = static_counter % 100;
        }
        
        // Alternate path to ensure both branches exist
        if (i % 3 == 0) {
            static_counter -= 2;
        }
    }
}

// Test function 4: Nested conditions with modifications
void __attribute__((noinline)) test_nested_modifications(void) {
    volatile int a = 0, b = 10, c = 20, d = 30;
    
    for (int outer = 0; outer < 10; outer++) {
        for (int inner = 0; inner < 20; inner++) {
            // Multiple conditions with modifications in then block
            if (a < b && (c > d || b != 0)) {
                a = b + 1;      // Modifies 'a' from condition
                b = c - d;      // Modifies 'b' from condition
                // Additional non-label, non-note instructions
                c = a * 2;
                d = b / 2;
            } else {
                d = a + b + c;
            }
            
            // Create data flow to prevent elimination
            a = (a + 1) % 100;
        }
        
        // Outer loop modifications
        b = (b * 2) % 50;
    }
}

// Test function 5: Pointer aliasing modifications
void __attribute__((noinline)) test_pointer_aliasing(void) {
    volatile int data[4] = {1, 2, 3, 4};
    volatile int* p1 = &data[0];
    volatile int* p2 = &data[1];
    volatile int idx = 0;
    
    for (int i = 0; i < 100; i++) {
        // Condition using array elements
        if (data[0] > data[1] && *p1 < *p2) {
            // Modify array elements used in condition
            data[0] = data[1] + 1;  // Modifies data[0]
            *p2 = data[0] * 2;      // Modifies *p2 which aliases data[1]
            idx = data[0] - data[1];
        }
        
        // Rotate array to create changing conditions
        int temp = data[3];
        data[3] = data[2];
        data[2] = data[1];
        data[1] = data[0];
        data[0] = temp;
    }
}

int main(int argc, char* argv[]) {
    // Use command line arguments to introduce runtime variability
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    // Initialize with some variability
    global_modifier = seed % 100;
    static_counter = seed % 50;
    
    // Run all test functions
    test_modify_in_then();
    test_complex_modification();
    test_static_modification();
    test_nested_modifications();
    test_pointer_aliasing();
    
    // Compute and print result to ensure side effects are observable
    int result = global_modifier + static_counter;
    printf("Result: %d (global_modifier=%d, static_counter=%d)\n", 
           result, global_modifier, static_counter);
    
    return result != 0 ? 0 : 1;
}
