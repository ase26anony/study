#include <stdio.h>
#include <stdlib.h>

// Global variables to prevent constant propagation
volatile int global_modifier = 0;
volatile int global_counter = 0;

// Function with side effects for use in conditions
__attribute__((noinline)) int side_effect_func(void) {
    return global_modifier++;
}

// Test function 1: Simple modification of condition variable in then block
__attribute__((noinline)) void test_simple_modification(void) {
    volatile int a = 10;
    volatile int b = 5;
    volatile int result = 0;
    
    // Loop to create basic block context
    for (int i = 0; i < 100; i++) {
        // Critical pattern: modify 'a' in then block when a > b
        if (a > b) {
            // This assignment modifies 'a' which is part of the condition
            a = b + i;  // Modifies condition variable early in then block
            result += a * b;
        } else {
            result -= a;
        }
        // Add some computation to prevent optimization
        b = (b * 3) % 100;
    }
    
    global_counter += result;
}

// Test function 2: Compound condition with multiple modifications
__attribute__((noinline)) void test_compound_condition(void) {
    volatile int x = 100;
    volatile int y = 50;
    volatile int z = 75;
    volatile int w = 25;
    
    for (int i = 0; i < 50; i++) {
        // Complex condition with multiple variables
        if (x > y && z < w && (x % 2) == 0) {
            // Modify multiple condition variables in then block
            x = y + z;      // Modifies 'x' used in first part of condition
            z = w * 2;      // Modifies 'z' used in second part
            w++;            // Modifies 'w' used in second part
            // Add more instructions to ensure they're in header
            y = x / 2;
        } else {
            x = x - 1;
            y = y + 1;
        }
        
        // Additional computation to prevent dead code elimination
        z = (z + i) % 100;
        w = (w * 2) % 100;
    }
    
    global_counter += x + y + z + w;
}

// Test function 3: Condition with function call and modification
__attribute__((noinline)) void test_func_call_condition(void) {
    volatile int a = 0;
    volatile int b = 10;
    volatile int c = 20;
    
    // Reset global modifier
    global_modifier = 0;
    
    for (int i = 0; i < 30; i++) {
        // Condition with function call that reads global_modifier
        if (a < b && side_effect_func() > 0) {
            // Modify variables that affect future function calls
            a = global_modifier;  // Modifies 'a' used in condition
            b = c - a;            // Modifies 'b' used in condition
            // Add several instructions to create header
            c = a + b;
            global_modifier += 2;
        } else {
            a = a + 1;
            b = b - 1;
        }
        
        // Loop variant
        c = (c + i) % 50;
    }
    
    global_counter += a + b + c;
}

// Test function 4: Nested conditions with modifications
__attribute__((noinline)) void test_nested_modifications(void) {
    volatile int p = 100;
    volatile int q = 200;
    volatile int r = 150;
    
    for (int outer = 0; outer < 20; outer++) {
        for (int inner = 0; inner < 10; inner++) {
            // Multiple conditions with modifications
            if (p < q) {
                if (r > 100) {
                    // Modify 'p' which is used in outer condition
                    p = q - r;  // This modifies outer condition variable
                    // Add more header instructions
                    q = p + inner;
                    r = r - outer;
                } else {
                    p = p + 5;
                }
            } else {
                q = q - 10;
            }
            
            // Additional computation
            r = (r * 3) % 300;
        }
        
        // Outer loop computation
        p = (p + outer) % 200;
    }
    
    global_counter += p + q + r;
}

// Test function 5: Pointer aliasing scenario
__attribute__((noinline)) void test_pointer_aliasing(void) {
    volatile int data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    volatile int *ptr1 = &data[0];
    volatile int *ptr2 = &data[5];
    volatile int index = 0;
    
    for (int i = 0; i < 25; i++) {
        // Condition using pointer dereference
        if (*ptr1 > *ptr2 && index < 5) {
            // Modify through pointer which might alias
            *ptr1 = *ptr2 + i;  // Modifies what ptr1 points to
            index = index + 2;   // Modifies 'index' used in condition
            // More header instructions
            ptr2 = &data[index % 10];
        } else {
            *ptr1 = *ptr1 - 1;
            index = index > 0 ? index - 1 : 0;
        }
        
        // Move pointers
        ptr1 = &data[(i + 1) % 10];
    }
    
    // Sum array to create observable effect
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += data[i];
    }
    global_counter += sum;
}

int main(int argc, char *argv[]) {
    // Use command line arguments to introduce runtime variability
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    // Initialize with some variability
    global_counter = seed;
    
    // Run all test functions
    test_simple_modification();
    test_compound_condition();
    test_func_call_condition();
    test_nested_modifications();
    test_pointer_aliasing();
    
    // Print result to ensure code isn't eliminated
    printf("Result: %d\n", global_counter);
    
    return 0;
}
