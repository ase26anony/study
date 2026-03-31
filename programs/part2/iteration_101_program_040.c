#include <stdio.h>
#include <stdlib.h>

// Global volatile variables to prevent optimization
volatile int g1 = 0;
volatile int g2 = 0;
volatile int g3 = 0;
volatile int g4 = 0;
volatile int g5 = 0;

// Function with side effects for condition testing
static volatile int side_effect_counter = 0;
int __attribute__((noinline)) func_with_side_effect() {
    return side_effect_counter++;
}

// Test function 1: Simple modification of condition variable in then block
__attribute__((noinline)) 
void test1_modify_in_then(volatile int a, volatile int b) {
    // Loop context to create interesting block structure
    for (int i = 0; i < 10; i++) {
        // Condition where 'a' is tested
        if (a > b) {
            // This modifies 'a' which is part of the condition
            // Should trigger modified_in_p check in ifcvt
            a = b + i;  // Modifies condition variable 'a'
            g1 += a * b;
        } else {
            g1 -= 1;
        }
        // Prevent loop unrolling from simplifying too much
        b += func_with_side_effect();
    }
}

// Test function 2: Compound condition with multiple modifications
__attribute__((noinline))
void test2_compound_condition(volatile int x, volatile int y, volatile int z) {
    int iterations = (x % 5) + 5;  // Dynamic iteration count
    
    for (int i = 0; i < iterations; i++) {
        // Complex compound condition
        if (x != 0 && y < z && (x + y) > (z / 2)) {
            // Multiple modifications of condition variables
            x = y + 1;      // Modifies 'x' used in condition
            y = z - x;      // Modifies 'y' used in condition
            z = x * y;      // Modifies 'z' used in condition
            g2 += x + y + z;
        } else {
            // Else block with its own logic
            x = (x + 1) % 10;
            y = (y + 2) % 10;
            g2 -= 1;
        }
        
        // Additional non-label, non-note instructions in header
        volatile int temp = x * y * z;
        if (temp > 100) {
            g3 += temp;
        }
    }
}

// Test function 3: Static variable modification in then block
__attribute__((noinline))
void test3_static_modification(volatile int limit) {
    static volatile int counter = 0;
    
    // Loop with condition using static variable
    while (counter < limit) {
        // Condition test uses 'counter'
        if (counter < (limit / 2) && g4 < 1000) {
            // Modifies 'counter' which is used in condition
            counter += 2;  // Direct modification
            g4 += counter * 3;
            
            // Additional instruction to ensure it's in header
            volatile int check = g4 - counter;
            if (check > 50) {
                g5 += check;
            }
        } else {
            counter += 1;
            g4 -= 1;
        }
        
        // Prevent infinite loops
        if (counter > 1000) break;
    }
}

// Test function 4: Function call in condition with side effects
__attribute__((noinline))
void test4_function_in_condition(volatile int base) {
    volatile int a = base;
    volatile int b = base + 10;
    
    for (int i = 0; i < 8; i++) {
        // Function call in condition - creates complex RTL
        if (func_with_side_effect() > (i / 2) && a < b) {
            // Modify variables that might affect future function calls
            side_effect_counter += a;  // Modifies global used by func_with_side_effect
            a = b - i;                  // Modifies 'a' used in condition
            b = a + func_with_side_effect(); // Modifies 'b' used in condition
            
            g1 += a;
            g2 += b;
        } else {
            a += i;
            b -= i;
            g3 += i;
        }
    }
}

// Test function 5: Nested conditions with modifications
__attribute__((noinline))
void test5_nested_modifications(volatile int p, volatile int q) {
    volatile int r = p * q;
    volatile int s = p + q;
    
    // Outer loop
    for (int outer = 0; outer < 5; outer++) {
        // Inner loop with condition
        for (int inner = 0; inner < 3; inner++) {
            // Nested condition structure
            if (p > q) {
                if (r < s) {
                    // Modify multiple condition variables
                    p = q + inner;  // Modifies 'p' from outer condition
                    r = s * 2;      // Modifies 'r' from inner condition
                    q = p - 1;      // Modifies 'q' from outer condition
                    
                    g4 += p + q + r + s;
                } else {
                    s = r / 2;
                    g5 += s;
                }
            } else {
                p = q + outer;
                g1 += p;
            }
            
            // Additional computation
            volatile int temp = (p * q) % 7;
            if (temp > 3) {
                r += temp;
                s -= temp;
            }
        }
        
        // Loop-carried dependency
        q = (q + 1) % 10;
    }
}

int main(int argc, char *argv[]) {
    // Use command line arguments for runtime variability
    int seed = 1;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    // Initialize with some variability
    volatile int a = seed;
    volatile int b = seed * 2 + 1;
    volatile int c = seed * 3 + 2;
    volatile int d = seed * 4 + 3;
    
    printf("Starting tests with seed=%d\n", seed);
    
    // Run all test functions
    test1_modify_in_then(a, b);
    printf("After test1: g1=%d\n", g1);
    
    test2_compound_condition(b, c, d);
    printf("After test2: g2=%d, g3=%d\n", g2, g3);
    
    test3_static_modification(c);
    printf("After test3: g4=%d, g5=%d\n", g4, g5);
    
    test4_function_in_condition(d);
    printf("After test4: g1=%d, g2=%d, g3=%d\n", g1, g2, g3);
    
    test5_nested_modifications(a + 1, b + 2);
    printf("After test5: g1=%d, g4=%d, g5=%d\n", g1, g4, g5);
    
    // Final computation to ensure all code has effect
    int result = g1 + g2 * 2 + g3 * 3 + g4 * 4 + g5 * 5;
    printf("Final result: %d\n", result);
    
    return result % 100;
}
