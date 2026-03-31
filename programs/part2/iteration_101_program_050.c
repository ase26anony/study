#include <stdio.h>
#include <stdlib.h>

// Global variables that can be modified
volatile int global_mod = 0;
volatile int global_counter = 0;

// Function with side effects
int __attribute__((noinline)) side_effect_func(void) {
    return global_mod++;
}

// Test function 1: Simple modification in then block
void __attribute__((noinline)) test_simple_modification(volatile int a, volatile int b) {
    // Loop to create basic block context
    for (int i = 0; i < 10; i++) {
        // Condition with variable used in test
        if (a > b) {
            // This modifies 'a' which is part of the condition
            // Should trigger modified_in_p check
            a = b + i;  // Line in then block header that modifies condition variable
            global_counter += a;
        }
        // Prevent dead code elimination
        if (i % 2 == 0) {
            b++;
        }
    }
}

// Test function 2: Compound condition with multiple modifications
void __attribute__((noinline)) test_compound_condition(volatile int x, volatile int y, volatile int z) {
    int local_sum = 0;
    
    // Loop context
    while (local_sum < 100) {
        // Complex condition with multiple variables
        if (x != 0 && y < z && global_mod > 0) {
            // Multiple modifications of condition variables
            x = y * 2;      // Modifies 'x' used in condition
            y++;            // Modifies 'y' used in condition  
            z = x + y;      // Modifies 'z' used in condition
            // Additional non-trivial code
            local_sum += x + y + z;
        } else {
            x = (x + 1) % 10;
        }
        
        // Add some control flow complexity
        switch (local_sum % 3) {
            case 0: global_mod++; break;
            case 1: y--; break;
            case 2: z += 2; break;
        }
    }
}

// Test function 3: Function call in condition with modification
void __attribute__((noinline)) test_func_call_condition(volatile int p, volatile int q) {
    static volatile int static_var = 0;
    
    for (int iter = 0; iter < 5; iter++) {
        // Condition with function call that reads global_mod
        if (side_effect_func() > 0 && p < q) {
            // Modify global variable that side_effect_func() reads
            global_mod += 2;    // Modifies what side_effect_func() would return
            p = q - iter;       // Modifies 'p' used in condition
            static_var++;       // Modifies static variable
            
            // Additional instructions to create header block
            int temp = p * q;
            global_counter += temp;
            if (temp > 100) {
                q = temp % 50;
            }
        }
        
        // Alternate path
        if (iter % 2 == 1) {
            q = p + iter;
        }
    }
}

// Test function 4: Nested conditions with modifications
void __attribute__((noinline)) test_nested_modifications(volatile int m, volatile int n) {
    volatile int result = 0;
    
    // Outer loop
    for (int outer = 0; outer < 3; outer++) {
        // Inner loop
        for (int inner = 0; inner < 4; inner++) {
            // Condition with multiple uses
            if (m > n && (m + n) % 2 == 0) {
                // Modify both condition variables
                m = n + inner;      // Modifies 'm' used in condition
                n = m - outer;      // Modifies 'n' used in condition
                
                // More instructions in header
                result += m * n;
                global_counter++;
                
                // Another condition inside
                if (result > 50) {
                    n = result % 30;
                }
            } else {
                m = (m + 1) % 20;
            }
        }
        
        // Modify after inner loop
        n += outer;
    }
}

// Test function 5: Pointer modification affecting condition
void __attribute__((noinline)) test_pointer_modification(volatile int *ptr1, volatile int *ptr2) {
    volatile int local = 0;
    
    while (local < 20) {
        // Condition using dereferenced pointers
        if (*ptr1 > *ptr2 && local < 15) {
            // Modify through pointers - affects condition values
            *ptr1 = *ptr2 + local;    // Modifies what *ptr1 points to
            *ptr2 += 1;               // Modifies what *ptr2 points to
            
            // Additional header instructions
            local += *ptr1;
            global_mod += local;
            
            // Array-like access
            int temp[3] = {*ptr1, *ptr2, local};
            for (int i = 0; i < 3; i++) {
                global_counter += temp[i];
            }
        }
        
        local++;
        if (local % 4 == 0) {
            (*ptr1)--;
        }
    }
}

int main(int argc, char *argv[]) {
    // Initialize volatile variables with some randomness from argv
    volatile int a = argc > 1 ? atoi(argv[1]) : 10;
    volatile int b = argc > 2 ? atoi(argv[2]) : 5;
    volatile int c = argc > 3 ? atoi(argv[3]) : 15;
    volatile int d = argc > 4 ? atoi(argv[4]) : 8;
    
    volatile int x = 20, y = 10, z = 30;
    volatile int p = 7, q = 12;
    volatile int m = 25, n = 15;
    
    // Allocate memory for pointer test
    volatile int ptr1_val = 40;
    volatile int ptr2_val = 30;
    volatile int *ptr1 = &ptr1_val;
    volatile int *ptr2 = &ptr2_val;
    
    printf("Starting tests with: a=%d, b=%d, c=%d, d=%d\n", a, b, c, d);
    
    // Run all test functions
    test_simple_modification(a, b);
    printf("After test 1: global_counter=%d, global_mod=%d\n", global_counter, global_mod);
    
    test_compound_condition(x, y, z);
    printf("After test 2: global_counter=%d, global_mod=%d\n", global_counter, global_mod);
    
    test_func_call_condition(p, q);
    printf("After test 3: global_counter=%d, global_mod=%d\n", global_counter, global_mod);
    
    test_nested_modifications(m, n);
    printf("After test 4: global_counter=%d, global_mod=%d\n", global_counter, global_mod);
    
    test_pointer_modification(ptr1, ptr2);
    printf("After test 5: global_counter=%d, global_mod=%d\n", global_counter, global_mod);
    
    // Final computation to ensure all code has observable effects
    int final_result = global_counter + global_mod + a + b + *ptr1 + *ptr2;
    printf("Final result: %d\n", final_result);
    
    return final_result > 100 ? 0 : 1;
}
