#include <stdio.h>
#include <stdlib.h>

// Global volatile variables to prevent optimization
volatile int g1 = 0, g2 = 0, g3 = 0;
volatile int counter = 0;
volatile int limit = 100;

// Function prototypes with noinline attribute
__attribute__((noinline)) void test1(volatile int a, volatile int b);
__attribute__((noinline)) void test2(volatile int a, volatile int b, volatile int c);
__attribute__((noinline)) void test3(void);
__attribute__((noinline)) void test4(volatile int x, volatile int y);
__attribute__((noinline)) int side_effect_func(void);

// Function with side effects used in conditions
__attribute__((noinline)) int side_effect_func(void) {
    static int call_count = 0;
    call_count++;
    return call_count;
}

// Test 1: Simple modification of condition variable in then block
__attribute__((noinline)) void test1(volatile int a, volatile int b) {
    // Loop context to create interesting block structure
    for (int i = 0; i < 10; i++) {
        // Condition where 'a' is tested
        if (a > b) {
            // This modifies 'a' which is part of the condition
            a = b + i;  // Modifies condition variable early in then block
            g1 += a * i;
        } else {
            g1 -= b;
        }
        
        // Additional code to create more instructions in the block
        b += i;
    }
}

// Test 2: Compound condition with multiple modifications
__attribute__((noinline)) void test2(volatile int a, volatile int b, volatile int c) {
    int i;
    
    // Complex loop structure
    for (i = 0; i < 5; i++) {
        // Compound condition using multiple variables
        if (a != 0 && b < c) {
            // Multiple modifications of condition variables
            b = a;      // Modifies 'b' used in condition (b < c)
            a = 0;      // Modifies 'a' used in condition (a != 0)
            c += i;     // Modifies 'c' used in condition (b < c)
            
            // Additional non-trivial code
            g2 += b * c;
        }
        
        // More operations to prevent block merging
        a += i;
        c -= i;
    }
}

// Test 3: Static variable modified in then block
__attribute__((noinline)) void test3(void) {
    volatile int local_counter = counter;
    volatile int local_limit = limit;
    
    // Loop with condition using static-like variable
    while (local_counter < local_limit) {
        // Condition test
        if (local_counter < (local_limit / 2)) {
            // Modifies the variable used in condition
            local_counter++;  // Direct modification
            counter = local_counter;  // Also modifies global
            
            // Additional instructions
            g3 += local_counter;
        } else {
            local_counter += 2;
        }
        
        // Prevent infinite loops
        if (local_counter > local_limit * 2) break;
    }
}

// Test 4: Function call in condition with side effects
__attribute__((noinline)) void test4(volatile int x, volatile int y) {
    int result = 0;
    
    for (int i = 0; i < 3; i++) {
        // Function call in condition
        if (side_effect_func() > 2 && x < y) {
            // Modify variables that might affect future function calls
            x = y + i;  // Modifies 'x' used in condition
            y++;        // Modifies 'y' used in condition
            
            // Additional computation
            result += x * y;
        }
        
        // More operations
        x += i;
        y -= i;
    }
    
    g1 += result;
}

// Test 5: Nested conditions with modifications
__attribute__((noinline)) void test5(volatile int a, volatile int b, volatile int c) {
    // Multiple nested loops to create complex CFG
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 3; j++) {
            // Complex condition
            if ((a > b || c < 10) && (i != j)) {
                // Modify multiple condition variables
                a = b + j;  // Modifies 'a' from (a > b)
                b += i;     // Modifies 'b' from (a > b)
                c = i * j;  // Modifies 'c' from (c < 10)
                
                // Additional code to create more instructions
                g2 += a + b + c;
            }
            
            // Alternate path
            if (b > c) {
                c = a;
            }
        }
    }
}

int main(int argc, char *argv[]) {
    // Initialize with some values, potentially from command line
    volatile int a = 10, b = 5, c = 15;
    
    // Use command line arguments to introduce runtime variability
    if (argc > 1) {
        a = atoi(argv[1]);
        if (argc > 2) b = atoi(argv[2]);
        if (argc > 3) c = atoi(argv[3]);
        if (argc > 4) limit = atoi(argv[4]);
    }
    
    // Run all test functions
    test1(a, b);
    test2(a, b, c);
    test3();
    test4(a, b);
    test5(a, b, c);
    
    // Compute and print result to ensure side effects are observable
    int result = g1 + g2 + g3 + counter;
    printf("Result: %d (g1=%d, g2=%d, g3=%d, counter=%d)\n", 
           result, g1, g2, g3, counter);
    
    return 0;
}
