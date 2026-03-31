#include <stdio.h>
#include <stdlib.h>

// Global variables to create dependencies
volatile int global_counter = 0;
volatile int global_modifier = 1;

// Function prototypes with noinline to prevent inlining
__attribute__((noinline)) int func_with_side_effect(int x);
__attribute__((noinline)) void test_simple_modification(void);
__attribute__((noinline)) void test_compound_condition(void);
__attribute__((noinline)) void test_loop_with_modification(void);
__attribute__((noinline)) void test_function_call_in_condition(void);
__attribute__((noinline)) void test_nested_modifications(void);

// Function with side effects
__attribute__((noinline)) 
int func_with_side_effect(int x) {
    global_counter++;
    return x + global_modifier;
}

// Test 1: Simple modification of condition variable in then block
__attribute__((noinline))
void test_simple_modification(void) {
    volatile int a = 10;
    volatile int b = 20;
    volatile int result = 0;
    
    // This creates a basic block where 'a' is modified in the then block
    // The condition uses 'a', and it's modified inside
    for (int i = 0; i < 100; i++) {
        if (a > b) {           // Condition test
            a = b;             // Modifies 'a' which is in the condition
            result += a;       // Additional instruction in header
            result *= 2;       // Another instruction
        } else {
            b = a + 1;
        }
        // Prevent loop unrolling from simplifying too much
        if (i % 7 == 0) {
            a += i;
        }
    }
    
    // Use result to prevent elimination
    printf("Test1: %d\n", result);
}

// Test 2: Compound condition with modifications
__attribute__((noinline))
void test_compound_condition(void) {
    volatile int x = 5;
    volatile int y = 15;
    volatile int z = 10;
    volatile int sum = 0;
    
    // Complex condition with multiple variables
    // The then block modifies 'x' which is used in the condition
    for (int i = 0; i < 50; i++) {
        if (x < y && z > x && y != 0) {  // Compound condition
            x = y + z;      // Modifies 'x' used in condition
            sum += x;       // Additional non-label instruction
            y = x / 2;      // Modifies 'y' also used in condition
            sum *= 3;       // Another instruction
        } else {
            z = x + y;
        }
        
        // Create some variability
        if (i % 3 == 0) {
            x += i;
            y -= i;
        }
    }
    
    printf("Test2: %d\n", sum);
}

// Test 3: Loop with counter modification in then block
__attribute__((noinline))
void test_loop_with_modification(void) {
    volatile int counter = 0;
    volatile int limit = 100;
    volatile int accumulator = 0;
    
    // The condition uses 'counter', and it's incremented in the then block
    while (counter < limit) {
        if (counter < (limit / 2)) {  // Condition test
            counter++;                // Modifies 'counter' used in condition
            accumulator += counter;   // Additional instruction
            accumulator &= 0xFF;      // Another instruction
            counter *= 1;             // Another modification (no-op but still an instruction)
        } else {
            accumulator -= counter;
        }
        
        // Add some complexity
        if (accumulator > 1000) {
            limit--;
        }
    }
    
    printf("Test3: %d\n", accumulator);
}

// Test 4: Function call in condition with side effects
__attribute__((noinline))
void test_function_call_in_condition(void) {
    volatile int a = 0;
    volatile int b = 10;
    volatile int total = 0;
    
    for (int i = 0; i < 30; i++) {
        // Function call in condition - creates complex RTL
        if (func_with_side_effect(a) > b && a < 100) {
            // Modify global variable that func_with_side_effect reads
            global_modifier = a + 1;  // This affects future calls to func_with_side_effect
            a = b * 2;                // Modifies 'a' used in condition
            total += a;               // Additional instruction
            b = a / 3;                // Modifies 'b' used in func_with_side_effect
            total &= 0xFFFF;          // Another instruction
        } else {
            a = func_with_side_effect(b);
        }
        
        // Vary the condition
        if (i % 5 == 0) {
            b += i;
        }
    }
    
    printf("Test4: %d (global_counter=%d)\n", total, global_counter);
}

// Test 5: Nested modifications with multiple variables
__attribute__((noinline))
void test_nested_modifications(void) {
    volatile int p = 1;
    volatile int q = 2;
    volatile int r = 3;
    volatile int s = 4;
    volatile int output = 0;
    
    // Very complex condition with many variables
    for (int i = 0; i < 20; i++) {
        if ((p < q || r > s) && (p + q) < (r * s) && p != 0) {
            // Modify multiple variables used in the condition
            p = q + r;      // Modifies 'p' used in condition
            output += p;    // Instruction 1 in header
            q = s - p;      // Modifies 'q' used in condition  
            output *= 2;    // Instruction 2
            r = p * q;      // Modifies 'r' used in condition
            output |= 1;    // Instruction 3
            s = r / 2;      // Modifies 's' used in condition
            output ^= 0xFF; // Instruction 4
            // Multiple instructions ensure they're in the header
        } else {
            p = q + s;
        }
        
        // Add unpredictability
        if (output > 1000) {
            q += i;
            r -= i;
        }
    }
    
    printf("Test5: %d\n", output);
}

int main(int argc, char *argv[]) {
    // Use command line arguments to add runtime variability
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    // Initialize with some variability
    srand(seed);
    
    // Run all test patterns
    test_simple_modification();
    test_compound_condition();
    test_loop_with_modification();
    test_function_call_in_condition();
    test_nested_modifications();
    
    // Final computation to ensure all code has observable effects
    volatile int final_result = 
        global_counter + global_modifier;
    
    printf("Final: %d\n", final_result);
    
    return final_result > 0 ? 0 : 1;
}
