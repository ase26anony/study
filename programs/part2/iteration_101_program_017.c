#include <stdio.h>
#include <stdlib.h>

// Global volatile variables to prevent optimization
volatile int global_a = 10;
volatile int global_b = 20;
volatile int global_c = 30;
volatile int global_d = 40;
volatile int global_counter = 0;
volatile int global_limit = 5;

// Function with side effects for condition testing
static volatile int side_effect_var = 0;
int __attribute__((noinline)) func_with_side_effect() {
    return side_effect_var++;
}

// Test function 1: Simple modification in then block
void __attribute__((noinline)) test_simple_modification() {
    volatile int a = global_a;
    volatile int b = global_b;
    
    // Loop to create basic block context
    for (int i = 0; i < 3; i++) {
        // Critical: 'a' is used in condition and modified in then block
        if (a > b) {
            // This modification should trigger modified_in_p check
            a = b;  // Modifies variable used in condition
            global_c = a + b;
        }
        b++;  // Change condition for next iteration
    }
    
    global_a = a;
}

// Test function 2: Compound condition with multiple modifications
void __attribute__((noinline)) test_compound_condition() {
    volatile int x = global_a;
    volatile int y = global_b;
    volatile int z = global_c;
    
    // Multiple iterations to create loop context
    for (int i = 0; i < 4; i++) {
        // Complex condition with multiple variables
        if (x != 0 && y < z && global_d > 10) {
            // Multiple modifications including variables from condition
            x = y;      // Modifies 'x' used in (x != 0)
            y = z + 1;  // Modifies 'y' used in (y < z)
            z = x * 2;  // Modifies 'z' used in (y < z)
        }
        global_d--;
    }
    
    global_a = x;
    global_b = y;
    global_c = z;
}

// Test function 3: Static variable modification in then block
void __attribute__((noinline)) test_static_modification() {
    static volatile int counter = 0;
    volatile int limit = global_limit;
    
    // Loop with condition using static variable
    for (int i = 0; i < 10; i++) {
        if (counter < limit) {
            // Modifies variable used in condition
            counter++;  // This should trigger modified_in_p
            global_a += counter;
            
            // Additional non-trivial operations
            volatile int temp = global_b;
            global_b = global_c;
            global_c = temp;
        }
        limit = (limit + 1) % 7;  // Change limit dynamically
    }
    
    global_counter = counter;
}

// Test function 4: Function call in condition with side effects
void __attribute__((noinline)) test_function_call_condition() {
    volatile int a = global_a;
    volatile int b = global_b;
    
    for (int i = 0; i < 3; i++) {
        // Function call in condition
        if (func_with_side_effect() > 0 && a < b) {
            // Modify variable that affects future function calls
            side_effect_var = a;  // Modifies global used by func_with_side_effect
            a = b * 2;            // Modifies 'a' used in condition
            b = a / 2;
        }
        a++;
        b--;
    }
    
    global_a = a;
    global_b = b;
}

// Test function 5: Nested conditions with modifications
void __attribute__((noinline)) test_nested_modifications() {
    volatile int p = global_a;
    volatile int q = global_b;
    volatile int r = global_c;
    
    for (int outer = 0; outer < 2; outer++) {
        for (int inner = 0; inner < 3; inner++) {
            if (p > q) {
                // First modification in then block header
                p = q + r;  // Modifies 'p' used in condition
                
                if (r < p) {
                    // Additional nested modification
                    r = p * 2;  // Modifies 'r' used in inner condition
                    q = r - p;  // Modifies 'q' used in outer condition
                }
            }
            // Rotate values
            volatile int temp = p;
            p = q;
            q = r;
            r = temp;
        }
    }
    
    global_a = p;
    global_b = q;
    global_c = r;
}

// Test function 6: Pointer modification affecting condition
void __attribute__((noinline)) test_pointer_modification() {
    volatile int data[4] = {global_a, global_b, global_c, global_d};
    volatile int *ptr1 = &data[0];
    volatile int *ptr2 = &data[1];
    
    for (int i = 0; i < 4; i++) {
        // Condition using pointer dereference
        if (*ptr1 > *ptr2 && ptr1 < &data[3]) {
            // Modify through pointer - affects condition
            *ptr1 = *ptr2 + i;  // Modifies *ptr1 used in condition
            ptr1++;              // Modifies ptr1 used in condition
            *ptr2 = *ptr1 - 1;
        }
        ptr2 = &data[(i + 2) % 4];
    }
    
    global_a = data[0];
    global_b = data[1];
    global_c = data[2];
    global_d = data[3];
}

int main(int argc, char *argv[]) {
    // Use command line arguments to introduce runtime variability
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    // Initialize with some variability
    global_a = 10 + (seed % 5);
    global_b = 20 + (seed % 7);
    global_c = 30 + (seed % 3);
    global_d = 40 + (seed % 9);
    global_limit = 5 + (seed % 3);
    
    // Call all test functions
    test_simple_modification();
    test_compound_condition();
    test_static_modification();
    test_function_call_condition();
    test_nested_modifications();
    test_pointer_modification();
    
    // Compute and print result to ensure side effects
    int result = global_a + global_b + global_c + global_d + global_counter;
    printf("Result: %d\n", result);
    
    // Additional loop to create more optimization context
    volatile int final_check = 0;
    for (int i = 0; i < 100; i++) {
        if (global_a < global_b && global_c > global_d) {
            global_a++;  // Modification in then block
            global_b--;
        } else if (global_b < global_c) {
            global_c = global_a;  // Another modification
        }
        final_check += i;
    }
    
    printf("Final values: a=%d, b=%d, c=%d, d=%d, counter=%d\n",
           global_a, global_b, global_c, global_d, global_counter);
    
    return result % 256;
}
