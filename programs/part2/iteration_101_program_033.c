#include <stdio.h>
#include <stdlib.h>

// Global variables to prevent constant propagation
volatile int global_modifier = 0;
volatile int global_counter = 0;

// Function to introduce side effects
int __attribute__((noinline)) side_effect_func(int x) {
    global_modifier ^= x;
    return global_modifier;
}

// Function to read volatile variable
int __attribute__((noinline)) read_volatile(void) {
    return global_counter;
}

// Test 1: Simple modification of condition variable in then block
void __attribute__((noinline)) test1_modify_in_then(void) {
    volatile int a = 10, b = 5, c = 0;
    
    // Loop to create basic block context
    for (int i = 0; i < 100; i++) {
        // Complex condition with multiple variables
        if (a > b && c < 20 && global_counter < 50) {
            // CRITICAL: Modify 'a' which is used in the condition
            a = b + 1;  // This should trigger modified_in_p check
            c++;
            global_counter++;
        } else {
            b++;
        }
        
        // Add some other operations to prevent optimization
        if (i % 2 == 0) {
            a ^= 1;
        }
    }
    
    // Use results to prevent dead code elimination
    printf("Test1: a=%d, b=%d, c=%d\n", a, b, c);
}

// Test 2: Compound condition with multiple modifications
void __attribute__((noinline)) test2_complex_modification(void) {
    volatile int x = 100, y = 50, z = 75;
    volatile int flag = 0;
    
    for (int i = 0; i < 50; i++) {
        // Complex condition using function call
        if (x != y && side_effect_func(i) > 0 && z < 100) {
            // Modify multiple variables used in condition
            x = y + i;      // Modifies x from condition
            y = z - 1;      // Modifies y from condition
            // Add non-label, non-note instructions in header
            flag = 1;
            global_modifier++;
        }
        
        // Additional operations
        z += (i % 3);
    }
    
    printf("Test2: x=%d, y=%d, z=%d, flag=%d\n", x, y, z, flag);
}

// Test 3: Self-modifying condition with static variable
void __attribute__((noinline)) test3_self_modifying_condition(void) {
    static volatile int counter = 0;
    volatile int limit = 10;
    volatile int data[20];
    
    // Initialize array
    for (int i = 0; i < 20; i++) {
        data[i] = i * 2;
    }
    
    for (int i = 0; i < 30; i++) {
        // Condition where counter is both tested and modified
        if (counter < limit && data[counter] > 0 && global_counter < 100) {
            // Direct modification of condition variable
            counter++;  // This should trigger modified_in_p
            data[counter] = -1;
            global_counter += 2;
        }
        
        // Mix in some other conditions
        if (i % 5 == 0) {
            limit += read_volatile();
        }
    }
    
    printf("Test3: counter=%d, limit=%d\n", counter, limit);
}

// Test 4: Nested conditions with modification
void __attribute__((noinline)) test4_nested_modification(void) {
    volatile int a = 0, b = 10, c = 20, d = 30;
    
    for (int i = 0; i < 40; i++) {
        // Outer condition
        if (a < b) {
            // Inner condition with modification
            if (c > d && global_modifier < 100) {
                // Modify 'c' which is in the inner condition
                c = d - a;  // Should be in then_bb header
                a++;        // Also modifies outer condition variable
                b--;
            }
            
            // More instructions in the then block
            d += c;
            global_modifier ^= i;
        }
        
        // Loop variation
        if (i % 7 == 0) {
            b += side_effect_func(i);
        }
    }
    
    printf("Test4: a=%d, b=%d, c=%d, d=%d\n", a, b, c, d);
}

// Test 5: Pointer modification affecting condition
void __attribute__((noinline)) test5_pointer_modification(void) {
    volatile int arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    volatile int *ptr = &arr[0];
    volatile int idx = 0;
    
    for (int i = 0; i < 25; i++) {
        // Condition using pointer dereference
        if (idx < 10 && *ptr > 0 && global_counter < 150) {
            // Modify through pointer - could affect condition
            *ptr = -(*ptr);  // Modifies what *ptr points to
            ptr++;          // Changes pointer itself
            idx++;          // Changes index
            global_counter++;
        }
        
        // Reset pointer occasionally
        if (i % 8 == 0) {
            ptr = &arr[idx % 10];
        }
    }
    
    printf("Test5: idx=%d, arr[0]=%d\n", idx, arr[0]);
}

int main(int argc, char *argv[]) {
    // Use command line arguments to introduce runtime variability
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    // Initialize with some randomness
    global_counter = seed;
    global_modifier = seed * 2;
    
    printf("Starting tests with seed=%d\n", seed);
    
    // Run all test functions
    test1_modify_in_then();
    test2_complex_modification();
    test3_self_modifying_condition();
    test4_nested_modification();
    test5_pointer_modification();
    
    // Final result based on all modifications
    int result = global_counter + global_modifier;
    printf("Final result: %d\n", result);
    
    return result % 256;
}
