#include <stdio.h>
#include <stdlib.h>

// Global variables that can be modified in conditions
static volatile int global_counter = 0;
static volatile int global_flag = 0;

// Function with side effects
static volatile int side_effect_var = 0;
__attribute__((noinline)) int func_with_side_effects(void) {
    return side_effect_var++;
}

// Test function 1: Simple modification of condition variable in then block
__attribute__((noinline)) 
void test_simple_modification(volatile int a, volatile int b) {
    // Loop context to create interesting block structure
    for (int i = 0; i < 10; i++) {
        // The condition uses 'a' and 'b'
        if (a > b) {
            // This modifies 'a' which is used in the condition
            // This should trigger modified_in_p check
            a = b + i;  // Modification of condition variable
            global_counter += a;
        }
        // Prevent loop unrolling from simplifying too much
        if (i % 2 == 0) {
            b += 1;
        }
    }
}

// Test function 2: Compound condition with multiple modifications
__attribute__((noinline))
void test_compound_condition(volatile int x, volatile int y, volatile int z) {
    int iterations = 5;
    
    while (iterations-- > 0) {
        // Compound condition with && - creates complex test_expr
        if (x != 0 && y < z) {
            // Multiple modifications of condition variables
            x = y * 2;      // Modifies 'x' used in first part of condition
            y = z + 1;      // Modifies 'y' used in second part of condition
            z = x + y;      // Modifies 'z' used in condition
            global_flag ^= 1;
        }
        
        // Add some other operations to create larger basic blocks
        if (global_flag) {
            x += iterations;
        }
    }
}

// Test function 3: Function call in condition with modification
__attribute__((noinline))
void test_func_in_condition(volatile int m, volatile int n) {
    volatile static int local_static = 0;
    
    for (int j = 0; j < 8; j++) {
        // Function call in condition - creates complex RTL
        if (func_with_side_effects() > m && n < 100) {
            // Modify variables that affect future function calls
            side_effect_var = m;  // Affects next call to func_with_side_effects()
            m = n * 2;            // Modifies 'm' used in condition
            n += local_static;    // Modifies 'n' used in condition
            local_static++;       // Modifies static variable
        }
        
        // Alternate path to create control flow complexity
        if (j % 3 == 0) {
            m -= 1;
        }
    }
}

// Test function 4: Nested conditions with modifications
__attribute__((noinline))
void test_nested_modifications(volatile int p, volatile int q, volatile int r) {
    volatile int temp = p;
    
    // Outer loop
    for (int outer = 0; outer < 4; outer++) {
        // Inner loop with condition
        for (int inner = 0; inner < 3; inner++) {
            // Complex condition with || operator
            if (p < q || r > temp) {
                // Multiple instructions in header before any branch
                p = q + inner;      // Modifies 'p' used in condition
                temp = r * 2;       // Modifies 'temp' used in condition
                q = p - 1;          // Modifies 'q' used in condition
                
                // Additional non-modifying instructions to extend block
                global_counter += p;
                global_counter -= q;
            }
            
            // Additional condition to prevent over-optimization
            if (inner == 1) {
                r += outer;
            }
        }
        
        // Modify variables between iterations
        if (outer % 2 == 0) {
            q = p + r;
        }
    }
}

// Test function 5: Pointer modification affecting condition
__attribute__((noinline))
void test_pointer_modification(volatile int *ptr1, volatile int *ptr2) {
    volatile int local = 0;
    
    for (int i = 0; i < 6; i++) {
        // Condition dereferencing pointers
        if (*ptr1 > *ptr2 && local < 10) {
            // Modify through pointers - affects condition values
            *ptr1 = *ptr2 + i;    // Modifies *ptr1 used in condition
            local = *ptr1;        // Modifies 'local' used in condition
            *ptr2 += 1;           // Modifies *ptr2 used in condition
            
            // Additional computation
            global_flag = *ptr1 ^ *ptr2;
        }
        
        // Vary the pointers occasionally
        if (i % 2 == 0) {
            ptr1 = &global_counter;
        }
    }
}

int main(int argc, char *argv[]) {
    // Initialize volatile variables with some values
    volatile int a = 10, b = 5, c = 15;
    volatile int x = 1, y = 2, z = 3;
    volatile int m = 20, n = 30;
    volatile int p = 100, q = 200, r = 300;
    volatile int ptr1_val = 50, ptr2_val = 60;
    volatile int *ptr1 = &ptr1_val;
    volatile int *ptr2 = &ptr2_val;
    
    // Use command line arguments to introduce runtime variability
    // This prevents constant folding optimizations
    if (argc > 1) {
        a = atoi(argv[1]);
        if (argc > 2) b = atoi(argv[2]);
        if (argc > 3) c = atoi(argv[3]);
    }
    
    printf("Starting values: a=%d, b=%d, c=%d\n", a, b, c);
    
    // Call all test functions to exercise different patterns
    test_simple_modification(a, b);
    printf("After test 1: global_counter=%d\n", global_counter);
    
    test_compound_condition(x, y, z);
    printf("After test 2: x=%d, y=%d, z=%d, global_flag=%d\n", x, y, z, global_flag);
    
    test_func_in_condition(m, n);
    printf("After test 3: m=%d, n=%d, side_effect_var=%d\n", m, n, side_effect_var);
    
    test_nested_modifications(p, q, r);
    printf("After test 4: p=%d, q=%d, r=%d, global_counter=%d\n", p, q, r, global_counter);
    
    test_pointer_modification(ptr1, ptr2);
    printf("After test 5: *ptr1=%d, *ptr2=%d, global_flag=%d\n", *ptr1, *ptr2, global_flag);
    
    // Compute and print final result
    int result = global_counter + global_flag + side_effect_var;
    printf("Final result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
