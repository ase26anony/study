#include <stdio.h>
#include <stdlib.h>

// Global variables to prevent optimization
volatile int global_modifier = 0;
static volatile int static_counter = 0;

// Function with side effects for condition testing
__attribute__((noinline)) 
int side_effect_func(void) {
    return global_modifier++;
}

// Test function 1: Simple modification in then block
__attribute__((noinline))
void test_simple_modification(volatile int a, volatile int b) {
    // Loop context to create interesting block structure
    for (int i = 0; i < 10; i++) {
        // Condition variable 'a' modified in then block
        if (a > b) {
            // This assignment modifies 'a' which is used in the condition
            a = b + i;  // Line in then block header that modifies condition variable
            global_modifier += a;
        }
        // Prevent loop unrolling from simplifying too much
        if (i % 3 == 0) {
            b++;
        }
    }
}

// Test function 2: Compound condition with multiple modifications
__attribute__((noinline))
void test_compound_condition(volatile int x, volatile int y, volatile int z) {
    int iterations = 5;
    
    while (iterations-- > 0) {
        // Compound condition with multiple variables
        // Both 'x' and 'y' are used in the condition
        if (x != 0 && y < z && global_modifier > 0) {
            // Multiple modifications of condition variables in then block
            x = y * 2;      // Modifies 'x' from condition
            y++;            // Modifies 'y' from condition  
            z = x + y;      // Chain of modifications
            
            // Add more non-label, non-note instructions
            static_counter += x;
            global_modifier -= y;
            
            // Early continue to create interesting control flow
            if (z > 100) continue;
        }
        
        // Additional code to prevent block merging
        volatile int temp = x + y + z;
        if (temp % 2 == 0) {
            side_effect_func();
        }
    }
}

// Test function 3: Function call in condition with modification
__attribute__((noinline))
void test_func_call_condition(volatile int m, volatile int n) {
    // Complex loop with multiple conditions
    for (volatile int i = 0; i < 8; i++) {
        // Condition includes function call with side effects
        if (side_effect_func() > 0 && m < n) {
            // Modify 'm' which is used in the condition
            m = n + i;  // This should trigger modified_in_p check
            
            // Additional instructions in then block header
            n = m * 2;
            global_modifier = m + n;
            
            // Prevent optimization
            asm volatile("" : : "r"(m), "r"(n) : "memory");
        }
        
        // Alternate path to create diamond control flow
        else {
            m = i;
            n = i * 3;
        }
        
        // Use variables to prevent dead code elimination
        volatile int result = m * n + i;
        if (result > 1000) {
            static_counter++;
        }
    }
}

// Test function 4: Nested conditions with modifications
__attribute__((noinline))
void test_nested_modification(volatile int p, volatile int q, volatile int r) {
    for (int outer = 0; outer < 4; outer++) {
        // Outer condition
        if (p > q) {
            // Inner condition with modification of outer condition variable
            if (q < r && p != 0) {
                // This modifies 'p' which is used in outer condition
                p = q + r;  // Should trigger analysis
                
                // More instructions in then block
                r = p * 2;
                q = r / 3;
                
                // Function call that might affect condition
                int val = side_effect_func();
                if (val > 50) {
                    p += val;
                }
            }
            
            // Additional modification in outer then block
            q = p + outer;
        }
        
        // Loop variable modification
        p += outer;
        r -= 1;
    }
}

// Main function with runtime variability
int main(int argc, char *argv[]) {
    // Initialize with command line or default values
    volatile int a = (argc > 1) ? atoi(argv[1]) : 10;
    volatile int b = (argc > 2) ? atoi(argv[2]) : 5;
    volatile int c = (argc > 3) ? atoi(argv[3]) : 15;
    volatile int x = (argc > 4) ? atoi(argv[4]) : 7;
    volatile int y = (argc > 5) ? atoi(argv[5]) : 12;
    
    printf("Starting values: a=%d, b=%d, c=%d, x=%d, y=%d\n", a, b, c, x, y);
    printf("Initial global_modifier=%d, static_counter=%d\n", global_modifier, static_counter);
    
    // Call test functions with different patterns
    test_simple_modification(a, b);
    printf("After test_simple_modification: global_modifier=%d\n", global_modifier);
    
    test_compound_condition(x, y, c);
    printf("After test_compound_condition: static_counter=%d\n", static_counter);
    
    test_func_call_condition(a, c);
    printf("After test_func_call_condition: global_modifier=%d\n", global_modifier);
    
    test_nested_modification(b, x, y);
    printf("After test_nested_modification: static_counter=%d\n", static_counter);
    
    // Compute and print final result
    int final_result = global_modifier + static_counter + a + b + c + x + y;
    printf("Final result: %d\n", final_result);
    
    // Read from stdin to prevent constant folding
    if (argc == 1) {
        printf("Enter a number: ");
        int input;
        if (scanf("%d", &input) == 1) {
            printf("Input affected result: %d\n", final_result + input);
        }
    }
    
    return final_result > 1000 ? 0 : 1;
}
