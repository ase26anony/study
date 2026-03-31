#include <stdio.h>
#include <stdlib.h>

// Global variables to prevent constant propagation
volatile int global_modifier = 0;
volatile int global_counter = 0;

// Function prototypes with noinline attribute
__attribute__((noinline)) void test_simple_modification();
__attribute__((noinline)) void test_compound_condition();
__attribute__((noinline)) void test_loop_with_modification();
__attribute__((noinline)) void test_function_call_side_effect();
__attribute__((noinline)) void test_nested_modifications();
__attribute__((noinline)) int side_effect_func(int* ptr);

// Helper function that reads input to prevent optimization
__attribute__((noinline)) int read_input() {
    static int counter = 0;
    return counter++ % 5;
}

// Test 1: Simple modification of condition variable in then block
__attribute__((noinline)) 
void test_simple_modification() {
    volatile int a = 10;
    volatile int b = 20;
    volatile int c = 30;
    
    // Loop to create basic block context
    for (int i = 0; i < 100; i++) {
        // The condition uses 'a' and 'b'
        if (a > b) {
            // This modifies 'a' which is part of the condition
            a = b + i;  // Line in then block header that modifies condition variable
            c = a * 2;
        } else {
            b = a + i;
        }
        
        // Prevent loop unrolling from simplifying too much
        if (i % 7 == read_input()) {
            a += 2;
        }
    }
    
    global_modifier += a + b + c;
}

// Test 2: Compound condition with multiple modifications
__attribute__((noinline))
void test_compound_condition() {
    volatile int x = 5;
    volatile int y = 15;
    volatile int z = 25;
    volatile int w = 35;
    
    int iterations = 50 + read_input();
    
    for (int i = 0; i < iterations; i++) {
        // Complex condition with multiple variables
        if (x < y && z > w && (x + z) < (y + w)) {
            // Multiple modifications of condition variables
            x = y - i;    // Modifies 'x' used in condition
            z = w + i;    // Modifies 'z' used in condition  
            // More instructions in header
            int temp = x * z;
            w = temp % 17;
        } else {
            y = x + z;
            w = y * 2;
        }
        
        // Additional computation to prevent dead code elimination
        if (i % 13 == 0) {
            x += read_input();
        }
    }
    
    global_counter += x + y + z + w;
}

// Test 3: Function call with side effects in condition
__attribute__((noinline))
int side_effect_func(int* ptr) {
    int result = *ptr + global_counter;
    // Modify global state
    global_modifier++;
    return result;
}

__attribute__((noinline))
void test_function_call_side_effect() {
    volatile int p = 100;
    volatile int q = 200;
    volatile int* ptr = &p;
    
    for (int j = 0; j < 75; j++) {
        // Condition with function call that has side effects
        if (side_effect_func(&p) > q && p < 150) {
            // Modify variable that function reads
            p = q - j;      // Modifies 'p' used in condition
            *ptr += 5;      // Also modifies through pointer
            q = p * 3;
            
            // Additional non-trivial instructions
            for (int k = 0; k < 3; k++) {
                p += k;
            }
        } else {
            q = p + j;
        }
        
        // Vary the condition
        if (j % 11 == read_input()) {
            p += 10;
        }
    }
}

// Test 4: Nested conditions with modifications
__attribute__((noinline))
void test_nested_modifications() {
    volatile int m = 0;
    volatile int n = 10;
    volatile int o = 20;
    
    int limit = 60 + read_input();
    
    while (m < limit) {
        // Outer condition
        if (m < n) {
            // Inner condition with modification
            if (n > o && m != 0) {
                // Modify multiple condition variables
                m = n - o;    // Modifies 'm' used in outer condition
                n = o + m;    // Modifies 'n' used in inner condition
                o = m * n;    // Modifies 'o' used in inner condition
                
                // Several instructions in the header
                int sum = m + n + o;
                m = sum % 50;
                n = (sum / 3) + 1;
            } else {
                o = m + n;
            }
        }
        
        m++;
        n += read_input() % 3;
    }
    
    global_counter += m + n + o;
}

// Test 5: Loop with modification in header
__attribute__((noinline))
void test_loop_with_modification() {
    volatile int r = 1;
    volatile int s = 2;
    volatile int t = 3;
    
    // Complex loop condition
    for (int i = 0; i < 80 && r < 1000; i += 1 + (read_input() % 2)) {
        // Condition where then block modifies the test expression
        if (r * s > t + i && (r % 7) != 0) {
            // Direct modification of condition variables
            r = s * i;      // Modifies 'r' used in condition
            s = t + r;      // Modifies 's' used in condition
            t = r - s;      // Modifies 't' used in condition
            
            // More arithmetic to create several instructions
            int prod = r * s * t;
            r = prod % 97;
            s = (prod / 3) % 53;
        }
        
        // Alternate path
        if (i % 17 == 0) {
            t = r + s + read_input();
        }
    }
    
    global_modifier += r - s + t;
}

int main(int argc, char* argv[]) {
    // Use command line arguments to vary behavior
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    // Initialize with some variability
    global_counter = seed;
    global_modifier = seed * 2;
    
    // Run all test patterns
    test_simple_modification();
    test_compound_condition();
    test_function_call_side_effect();
    test_nested_modifications();
    test_loop_with_modification();
    
    // Final computation to ensure all code has observable effects
    int result = global_counter + global_modifier;
    
    // Print result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    // Additional print to ensure all paths are used
    if (result % 2 == 0) {
        printf("Even result pattern triggered\n");
    } else {
        printf("Odd result pattern triggered\n");
    }
    
    return result % 256;
}
