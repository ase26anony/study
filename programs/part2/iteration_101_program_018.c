#include <stdio.h>
#include <stdlib.h>

// Global volatile variables to prevent optimization
volatile int g1 = 0;
volatile int g2 = 0;
volatile int g3 = 0;
volatile int g4 = 0;
volatile int g5 = 0;

// Static counter for function 3
static volatile int counter = 0;

// Non-pure function with side effects
__attribute__((noinline)) int side_effect_func(void) {
    static int state = 0;
    return state++ % 3;
}

// Function 1: Simple modification in then block
__attribute__((noinline)) void test_func1(volatile int a, volatile int b) {
    // Loop context to create interesting block structure
    for (int i = 0; i < 100; i++) {
        // Simple condition with modification in then block
        if (a > b) {
            // This modifies 'a' which is used in the condition
            // Should be in the then block header analyzed by the for loop
            a = b + i;  // Line in uncovered region: modifies test_expr variable
            g1 = a * b; // Additional instruction to ensure block has content
        }
        // Prevent loop unrolling from simplifying too much
        if (i % 7 == 0) {
            b++;
        }
    }
    g2 = a + b;
}

// Function 2: Compound condition with multiple modifications
__attribute__((noinline)) void test_func2(volatile int x, volatile int y, volatile int z) {
    int iterations = 50;
    
    while (iterations-- > 0) {
        // Compound condition using multiple variables
        if (x != 0 && y < z && x > y) {
            // Multiple modifications to condition variables
            x = y * 2;      // Modifies 'x' used in (x != 0) and (x > y)
            y = z - 1;      // Modifies 'y' used in (y < z) and (x > y)
            g3 = x + y + z; // Additional computation
        }
        
        // Alternate path to create control flow complexity
        if (z > 100) {
            x = z / 2;
        }
        
        // Modify z to change condition over time
        z += iterations % 3;
    }
    g4 = x + y + z;
}

// Function 3: Condition with static variable modification
__attribute__((noinline)) void test_func3(volatile int limit) {
    // Reset counter
    counter = 0;
    
    for (int i = 0; i < 200; i++) {
        // Condition using static variable
        if (counter < limit && i % 2 == 0) {
            // Modify the static variable used in condition
            counter++;          // Modifies 'counter' used in (counter < limit)
            g5 = counter * i;   // Additional instruction
            
            // Nested condition to create more basic blocks
            if (g5 > 1000) {
                limit--;        // Also modifies limit used in outer condition
            }
        }
        
        // Additional modification in loop
        if (i % 13 == 0) {
            limit += 2;
        }
    }
}

// Function 4: Function call in condition with side effects
__attribute__((noinline)) void test_func4(void) {
    volatile int a = 10, b = 20, c = 30;
    
    for (int i = 0; i < 75; i++) {
        // Function call in condition
        if (side_effect_func() > 0 && a < b) {
            // Modify variables that affect future function calls
            a = b - side_effect_func();  // side_effect_func() reads static 'state'
            b = c + i;
            g1 = a + b + c;
        }
        
        // Additional complexity
        c += i % 5;
    }
}

// Function 5: Complex nested conditions with early modifications
__attribute__((noinline)) void test_func5(volatile int p, volatile int q, volatile int r) {
    int temp = 0;
    
    for (int i = 0; i < 60; i++) {
        // Very complex condition
        if ((p > q || r < p) && (q != r) && (p + q > r)) {
            // Immediate modification of condition variables
            p = q + r;      // Modifies 'p' used in multiple parts of condition
            temp = p * 2;   // Non-modifying instruction
            
            // Additional modification
            if (temp > 50) {
                q = r - 1;  // Modifies 'q' used in condition
            }
            
            r = temp / 3;   // Modifies 'r' used in condition
            g2 = p + q + r;
        }
        
        // Rotate values
        temp = p;
        p = q;
        q = r;
        r = temp;
    }
}

int main(int argc, char *argv[]) {
    // Use command line arguments to introduce runtime variability
    int base = argc > 1 ? atoi(argv[1]) : 42;
    
    // Initialize volatile variables with runtime-dependent values
    volatile int a = base;
    volatile int b = base + 10;
    volatile int c = base * 2;
    volatile int x = base - 5;
    volatile int y = base + 7;
    volatile int z = base * 3;
    
    printf("Starting tests with base = %d\n", base);
    
    // Call all test functions
    test_func1(a, b);
    printf("After func1: g1=%d, g2=%d\n", g1, g2);
    
    test_func2(x, y, z);
    printf("After func2: g3=%d, g4=%d\n", g3, g4);
    
    test_func3(base / 2);
    printf("After func3: counter=%d, g5=%d\n", counter, g5);
    
    test_func4();
    printf("After func4: g1=%d\n", g1);
    
    test_func5(a, b, c);
    printf("After func5: g2=%d\n", g2);
    
    // Compute and print final result
    int result = g1 + g2 + g3 + g4 + g5 + counter;
    printf("Final result: %d\n", result);
    
    return result % 256;
}
