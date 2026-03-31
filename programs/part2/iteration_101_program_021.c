#include <stdio.h>
#include <stdlib.h>

// Global variables to prevent constant propagation
volatile int global_modifier = 0;
static volatile int static_counter = 0;

// Function with side effects for condition testing
__attribute__((noinline)) 
int side_effect_func(void) {
    return global_modifier++;
}

// Test function 1: Simple modification of condition variable in then block
__attribute__((noinline))
void test1_modify_in_then(volatile int a, volatile int b) {
    // Loop context to create interesting block structure
    for (int i = 0; i < 10; i++) {
        // Condition where 'a' is tested
        if (a > b) {
            // MODIFICATION: 'a' is modified in the then block
            // This should trigger modified_in_p check
            a = b + i;  // Line in uncovered range
            b = a * 2;  // Another instruction in header
            // More instructions to ensure header has content
            global_modifier += a;
        }
        // Prevent loop unrolling from simplifying too much
        if (i % 3 == 0) {
            side_effect_func();
        }
    }
}

// Test function 2: Compound condition with multiple modifications
__attribute__((noinline))
void test2_compound_condition(volatile int x, volatile int y, volatile int z) {
    int iterations = 5;
    
    while (iterations-- > 0) {
        // Complex compound condition
        if (x != 0 && y < z && global_modifier > 0) {
            // Multiple modifications of condition variables
            x = y;      // Modifies 'x' used in condition
            y = z + 1;  // Modifies 'y' used in condition
            z = x * 2;  // Another modification
            // Additional non-label, non-note instructions
            static_counter++;
            global_modifier = x + y;
        }
        
        // Alternate path to create control flow
        if (iterations % 2 == 0) {
            z = side_effect_func();
        }
    }
}

// Test function 3: Function call in condition with modification
__attribute__((noinline))
void test3_func_in_condition(volatile int m, volatile int n) {
    for (int j = 0; j < 8; j++) {
        // Condition with function call that reads global_modifier
        if (m > n && side_effect_func() < 10) {
            // Modify variable that affects future function calls
            global_modifier = m;  // Affects side_effect_func()
            m = n - j;            // Modifies 'm' used in condition
            n = m + global_modifier;
            
            // More header instructions
            int temp = m * n;
            static_counter += temp;
        }
        
        // Prevent dead code elimination
        if (j == 3) {
            m = side_effect_func();
        }
    }
}

// Test function 4: Nested conditions with modifications
__attribute__((noinline))
void test4_nested_modifications(volatile int p, volatile int q, volatile int r) {
    // Outer loop
    for (int outer = 0; outer < 3; outer++) {
        // Inner loop with condition
        for (int inner = 0; inner < 4; inner++) {
            // Condition using all three parameters
            if ((p < q || r > 0) && (q != r || p == inner)) {
                // Modify p which is used in the condition
                p = q + inner;  // This is in the then block header
                
                // Additional modifications
                if (r > 0) {
                    q = p * 2;  // Modifies q used in condition
                    r--;        // Modifies r used in condition
                }
                
                // More instructions in header
                global_modifier += p;
                static_counter = q;
            }
            
            // Vary r to change condition
            r = (r + 1) % 5;
        }
        
        // Change q between outer iterations
        q = side_effect_func();
    }
}

// Test function 5: Switch-like pattern with modification
__attribute__((noinline))
void test5_switch_modification(volatile int val) {
    int count = 0;
    
    while (count < 6) {
        // Multi-part condition
        if (val > 0 && val < 100 && global_modifier != static_counter) {
            // Modify val which is used in the condition
            val = global_modifier;  // Modification in then block
            
            // Additional header instructions
            int old_val = val;
            val = old_val * 2 + 1;
            static_counter = val % 7;
            
            // Function call that might be analyzed
            side_effect_func();
        }
        
        // Alternate modification
        if (count % 2 == 0) {
            val = side_effect_func();
        }
        
        count++;
    }
}

int main(int argc, char *argv[]) {
    // Initialize with some values, potentially from command line
    volatile int a = (argc > 1) ? atoi(argv[1]) : 10;
    volatile int b = (argc > 2) ? atoi(argv[2]) : 5;
    volatile int c = (argc > 3) ? atoi(argv[3]) : 15;
    volatile int x = 7, y = 3, z = 20;
    volatile int m = 100, n = 50;
    volatile int p = 1, q = 2, r = 3;
    volatile int val = 25;
    
    // Reset globals
    global_modifier = 1;
    static_counter = 0;
    
    // Run all test functions
    test1_modify_in_then(a, b);
    test2_compound_condition(x, y, z);
    test3_func_in_condition(m, n);
    test4_nested_modifications(p, q, r);
    test5_switch_modification(val);
    
    // Use results to prevent dead code elimination
    int result = a + b + x + y + z + m + n + p + q + r + val 
                 + global_modifier + static_counter;
    
    printf("Result: %d\n", result);
    printf("Global modifier: %d, Static counter: %d\n", 
           global_modifier, static_counter);
    
    return 0;
}
