#include <stdio.h>
#include <stdlib.h>

// Global volatile variables to prevent optimization
volatile int global_a = 0;
volatile int global_b = 0;
volatile int global_c = 0;
volatile int global_d = 0;
static volatile int static_counter = 0;

// Function with side effects
int __attribute__((noinline)) side_effect_func(void) {
    static int call_count = 0;
    return ++call_count;
}

// Pattern 1: Simple modification in then block
void __attribute__((noinline)) test_pattern1(void) {
    volatile int a = global_a;
    volatile int b = global_b;
    
    // Loop context to create interesting block structure
    for (int i = 0; i < 10; i++) {
        // Condition where 'a' is tested
        if (a > b) {
            // This modifies 'a' which is part of the condition
            a = b + i;  // Modifies variable used in condition
            global_c += a;  // Additional instruction in header
            global_d = i;   // Another instruction
        } else {
            b = a + i;
        }
        
        // Additional loop body to prevent trivial optimization
        if (i % 2 == 0) {
            a += 1;
        }
    }
    
    global_a = a;
    global_b = b;
}

// Pattern 2: Compound condition with multiple modifications
void __attribute__((noinline)) test_pattern2(void) {
    volatile int x = global_a;
    volatile int y = global_b;
    volatile int z = global_c;
    
    // Complex condition with multiple variables
    if (x != 0 && y < z && global_d > 0) {
        // Modify multiple variables used in the condition
        x = y;      // Modifies 'x' from condition
        y = z + 1;  // Modifies 'y' from condition
        z = x * 2;  // Additional modification
        // Multiple instructions in the header
        global_d--;
        x += global_d;
    }
    
    global_a = x;
    global_b = y;
    global_c = z;
}

// Pattern 3: Static variable modification in then block
void __attribute__((noinline)) test_pattern3(int limit) {
    volatile int a = global_a;
    
    // Loop with condition using static variable
    for (int i = 0; i < 5; i++) {
        // Condition tests static_counter
        if (static_counter < limit && a > 0) {
            // Modify static_counter which is part of condition
            static_counter++;  // Modifies variable used in condition
            a -= static_counter;
            // Additional header instructions
            global_b += i;
            if (i == 2) {
                global_c = a;
            }
        }
    }
    
    global_a = a;
}

// Pattern 4: Function call in condition with side effects
void __attribute__((noinline)) test_pattern4(void) {
    volatile int val = global_a;
    volatile int threshold = global_b;
    
    // Multiple iterations to create loop context
    for (int i = 0; i < 8; i++) {
        // Function call in condition
        if (side_effect_func() > threshold && val < 100) {
            // Modify variables that affect future function calls
            threshold = val;  // Affects next iteration's condition
            val += side_effect_func();  // Function call in then block
            // Additional instructions
            global_c = i;
            if (val > 50) {
                global_d = threshold;
            }
        }
    }
    
    global_a = val;
    global_b = threshold;
}

// Pattern 5: Nested conditions with modifications
void __attribute__((noinline)) test_pattern5(void) {
    volatile int p = global_a;
    volatile int q = global_b;
    volatile int r = global_c;
    
    // Outer loop
    for (int outer = 0; outer < 3; outer++) {
        // Inner loop
        for (int inner = 0; inner < 4; inner++) {
            // Complex nested condition
            if ((p > q || r < p) && (q != inner)) {
                // Modify variables used in condition
                p = q + inner;  // Modifies 'p'
                q = r - outer;  // Modifies 'q'
                // Multiple instructions in header
                r += p;
                if (inner == 1) {
                    p += 2;
                }
            }
        }
    }
    
    global_a = p;
    global_b = q;
    global_c = r;
}

int main(int argc, char *argv[]) {
    // Use command line arguments to introduce runtime variability
    int seed = 1;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    // Initialize with some variability
    global_a = seed % 100;
    global_b = (seed * 3) % 100;
    global_c = (seed * 7) % 100;
    global_d = (seed * 11) % 100;
    
    printf("Initial values: a=%d, b=%d, c=%d, d=%d, counter=%d\n",
           global_a, global_b, global_c, global_d, static_counter);
    
    // Execute all test patterns
    test_pattern1();
    printf("After pattern1: a=%d, b=%d, c=%d, d=%d\n",
           global_a, global_b, global_c, global_d);
    
    test_pattern2();
    printf("After pattern2: a=%d, b=%d, c=%d, d=%d\n",
           global_a, global_b, global_c, global_d);
    
    test_pattern3(seed % 10 + 5);
    printf("After pattern3: a=%d, b=%d, c=%d, d=%d, counter=%d\n",
           global_a, global_b, global_c, global_d, static_counter);
    
    test_pattern4();
    printf("After pattern4: a=%d, b=%d, c=%d, d=%d\n",
           global_a, global_b, global_c, global_d);
    
    test_pattern5();
    printf("After pattern5: a=%d, b=%d, c=%d, d=%d\n",
           global_a, global_b, global_c, global_d);
    
    // Compute and print final result
    int result = global_a + global_b * 2 + global_c * 3 + global_d * 4;
    printf("Final result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
