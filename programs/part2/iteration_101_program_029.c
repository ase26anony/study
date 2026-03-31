#include <stdio.h>
#include <stdlib.h>

// Global volatile variables to prevent optimization
volatile int g1 = 0, g2 = 0, g3 = 0, g4 = 0;
volatile int g_counter = 0;
volatile int g_limit = 100;

// Function prototypes with noinline attribute
__attribute__((noinline)) void test1(volatile int a, volatile int b, volatile int c);
__attribute__((noinline)) void test2(volatile int x, volatile int y, volatile int z);
__attribute__((noinline)) void test3(volatile int p, volatile int q);
__attribute__((noinline)) void test4(volatile int m, volatile int n);
__attribute__((noinline)) int side_effect_func(void);

// Function with side effects
__attribute__((noinline)) int side_effect_func(void) {
    g_counter++;
    return g_counter;
}

// Test 1: Simple modification of condition variable in then block
__attribute__((noinline)) void test1(volatile int a, volatile int b, volatile int c) {
    // Loop context to create interesting block structure
    for (int i = 0; i < 10; i++) {
        // Condition where 'a' is tested
        if (a > b) {
            // This modifies 'a' which is part of the condition
            a = b + c;  // Modifies condition variable early in then block
            g1 = a * 2;
            // Additional instructions to create non-trivial block header
            c = a + b;
            b = c - a;
        } else {
            g2 = b - a;
        }
        
        // Loop-dependent modification
        a += i;
        b -= i;
    }
    
    // Store results to prevent elimination
    g3 = a + b + c;
}

// Test 2: Compound condition with multiple modifications
__attribute__((noinline)) void test2(volatile int x, volatile int y, volatile int z) {
    int local_sum = 0;
    
    // Complex compound condition
    for (int i = 0; i < 5; i++) {
        // Compound condition using && with multiple variables
        if (x != 0 && y < z && z > 10) {
            // Modify multiple condition variables
            x = y;      // Modifies 'x' used in first part of condition
            y = z + 1;  // Modifies 'y' used in second part
            z = x * 2;  // Modifies 'z' used in third part
            
            // Additional non-trivial operations
            local_sum += x + y + z;
            g1 = local_sum;
        } else {
            x++;
            y--;
        }
        
        // Vary the condition variables
        z += i;
    }
    
    g2 = x + y + z;
}

// Test 3: Static variable modified in then block
__attribute__((noinline)) void test3(volatile int p, volatile int q) {
    static volatile int static_var = 0;
    
    // Loop with condition involving static variable
    for (int i = 0; i < 8; i++) {
        // Condition tests static_var
        if (static_var < g_limit && p > q) {
            // Modify static_var which is part of the condition
            static_var += p;  // Modifies condition variable
            
            // Also modify other condition variable
            p = q - 1;
            
            // Complex operations to create non-trivial block
            q = static_var * 2;
            g_counter += static_var;
        }
        
        // Update variables
        p += i;
        q -= i;
        static_var++;
    }
    
    g3 = static_var + p + q;
}

// Test 4: Function call in condition with side effects
__attribute__((noinline)) void test4(volatile int m, volatile int n) {
    int result = 0;
    
    // Multiple iterations
    for (int i = 0; i < 6; i++) {
        // Condition with function call that has side effects
        if (side_effect_func() > 2 && m < n) {
            // Modify global variable that side_effect_func() reads
            g_counter += 5;  // This affects future calls to side_effect_func()
            
            // Also modify condition variables
            m = n + g_counter;
            n = m - g_counter;
            
            // Complex operations
            result = m * n / (g_counter + 1);
            g4 = result;
        } else {
            m++;
            n--;
        }
        
        // Additional modifications
        m += i * 2;
        n -= i;
    }
    
    g1 = m + n + result;
}

int main(int argc, char *argv[]) {
    // Initialize with some values, potentially from command line
    volatile int a = (argc > 1) ? atoi(argv[1]) : 10;
    volatile int b = (argc > 2) ? atoi(argv[2]) : 5;
    volatile int c = (argc > 3) ? atoi(argv[3]) : 15;
    volatile int x = (argc > 4) ? atoi(argv[4]) : 7;
    volatile int y = (argc > 5) ? atoi(argv[5]) : 12;
    volatile int z = (argc > 6) ? atoi(argv[6]) : 20;
    volatile int p = (argc > 7) ? atoi(argv[7]) : 8;
    volatile int q = (argc > 8) ? atoi(argv[8]) : 3;
    volatile int m = (argc > 9) ? atoi(argv[9]) : 25;
    volatile int n = (argc > 10) ? atoi(argv[10]) : 30;
    
    // Reset global counter
    g_counter = 0;
    
    // Call test functions with different patterns
    test1(a, b, c);
    test2(x, y, z);
    test3(p, q);
    test4(m, n);
    
    // Compute and print result to ensure side effects are observable
    int final_result = g1 + g2 + g3 + g4 + g_counter;
    printf("Result: %d (g1=%d, g2=%d, g3=%d, g4=%d, counter=%d)\n", 
           final_result, g1, g2, g3, g4, g_counter);
    
    return final_result != 0 ? 0 : 1;
}
