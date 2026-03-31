#include <stdio.h>
#include <stdlib.h>

// Global variables to prevent constant propagation
volatile int global_modifier = 0;
static volatile int static_counter = 0;

// Function with side effects to use in conditions
__attribute__((noinline)) int side_effect_func(void) {
    return global_modifier++;
}

// Test function 1: Simple modification in then block
__attribute__((noinline)) void test_simple_modification(volatile int a, volatile int b) {
    // Loop to create basic block context
    for (int i = 0; i < 10; i++) {
        // Condition variable 'a' modified in then block
        if (a > b) {
            // This assignment modifies 'a' which is used in the condition
            a = b + i;  // Line should be in then_bb header
            // Additional instructions to ensure non-empty block
            b = a * 2;
            global_modifier += a;
        }
    }
}

// Test function 2: Compound condition with multiple modifications
__attribute__((noinline)) void test_compound_condition(volatile int x, volatile int y, volatile int z) {
    int iterations = 5;
    
    while (iterations-- > 0) {
        // Compound condition using multiple variables
        if (x != 0 && y < z && global_modifier > 0) {
            // Modify 'x' which is used in the first part of condition
            x = y * 2;  // This should trigger modified_in_p check
            
            // Also modify 'y' used in second part
            y++;
            
            // Add more instructions to ensure block has header content
            z = x + y;
            static_counter++;
        }
        
        // Add else block to create proper diamond pattern
        else {
            x = z;
            y = x - 1;
        }
    }
}

// Test function 3: Function call in condition with modification
__attribute__((noinline)) void test_func_call_condition(volatile int m, volatile int n) {
    for (int j = 0; j < 8; j++) {
        // Function call in condition, then modifies global variable
        // that the function reads
        if (side_effect_func() > m && n < 100) {
            // Modify global variable that side_effect_func() reads
            global_modifier += 5;  // This affects future calls to side_effect_func()
            
            // Also modify local variables used in condition
            m = n * 3;
            n = j + m;
            
            // Multiple instructions in header
            int temp = m + n;
            global_modifier ^= temp;
        }
    }
}

// Test function 4: Complex nested conditions
__attribute__((noinline)) void test_nested_modification(volatile int p, volatile int q, volatile int r) {
    int limit = 7;
    
    for (int k = 0; k < limit; k++) {
        // Complex condition with OR
        if ((p > q || r < static_counter) && k % 2 == 0) {
            // Modify 'p' which is used in condition
            p = q + r;  // Should trigger modified_in_p
            
            // Chain of modifications
            q = p * k;
            r = q - p;
            
            // More header instructions
            static_counter += p;
            global_modifier = q % 10;
        }
        
        // Alternate path
        else if (p == q) {
            r = k * 2;
        }
    }
}

// Test function 5: Pointer modification affecting condition
__attribute__((noinline)) void test_pointer_modification(volatile int* ptr1, volatile int* ptr2) {
    volatile int local = 0;
    
    for (int i = 0; i < 6; i++) {
        // Condition using dereferenced pointers
        if (*ptr1 > *ptr2 && local < 50) {
            // Modify through pointer - affects condition value
            *ptr1 = *ptr2 + i;  // This modifies memory used in condition
            
            // Additional modifications
            local += *ptr1;
            *ptr2 = local;
            
            // More instructions
            int calc = *ptr1 * *ptr2;
            global_modifier += calc;
        }
        local++;
    }
}

int main(int argc, char* argv[]) {
    // Initialize volatile variables with some randomness from argv
    volatile int a = argc > 1 ? atoi(argv[1]) : 10;
    volatile int b = argc > 2 ? atoi(argv[2]) : 5;
    volatile int c = argc > 3 ? atoi(argv[3]) : 15;
    volatile int d = argc > 4 ? atoi(argv[4]) : 20;
    
    volatile int x = 8, y = 12, z = 6;
    volatile int m = 3, n = 7;
    volatile int p = 25, q = 30, r = 10;
    
    volatile int val1 = 100, val2 = 50;
    volatile int* ptr1 = &val1;
    volatile int* ptr2 = &val2;
    
    // Call all test functions to exercise different patterns
    test_simple_modification(a, b);
    test_compound_condition(x, y, z);
    test_func_call_condition(m, n);
    test_nested_modification(p, q, r);
    test_pointer_modification(ptr1, ptr2);
    
    // Use results to prevent dead code elimination
    int result = a + b + x + y + z + m + n + p + q + r + *ptr1 + *ptr2 
                 + global_modifier + static_counter;
    
    printf("Result: %d\n", result);
    printf("Global modifier: %d, Static counter: %d\n", global_modifier, static_counter);
    
    return 0;
}
