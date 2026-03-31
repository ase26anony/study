#include <stdio.h>
#include <stdlib.h>

// Global variables to create dependencies
volatile int global_x = 0;
volatile int global_y = 0;
volatile int global_z = 0;
static volatile int static_counter = 0;

// Function with side effects for condition testing
int __attribute__((noinline)) side_effect_func(void) {
    return global_x + global_y;
}

// Pattern 1: Simple modification in then block
void __attribute__((noinline)) test_pattern1(void) {
    volatile int a = 10;
    volatile int b = 5;
    volatile int c = 0;
    
    // Loop to create basic block context
    for (int i = 0; i < 100; i++) {
        // Critical: 'a' is modified in then block after being used in condition
        if (a > b && c < 20) {
            // This modification should trigger modified_in_p check
            a = b + i;  // Modifies 'a' which is part of condition
            c++;
            global_x = a;  // Additional side effect
        } else {
            b++;
        }
        
        // Prevent loop unrolling from simplifying too much
        if (i % 10 == 0) {
            global_y = i;
        }
    }
}

// Pattern 2: Compound condition with multiple modifications
void __attribute__((noinline)) test_pattern2(void) {
    volatile int p = 100;
    volatile int q = 50;
    volatile int r = 75;
    volatile int s = 0;
    
    // Complex condition with multiple variables
    for (int i = 0; i < 50; i++) {
        // Compound condition using && and ||
        if ((p > q || r < s) && (p != 0) && (q < r + i)) {
            // Multiple modifications of condition variables
            p = q + 1;      // Modifies 'p' used in condition
            q = r - i;      // Modifies 'q' used in condition
            r++;            // Modifies 'r' used in condition
            s = p + q;      // Chain of modifications
            
            // Additional instructions to create header block
            global_z = p * q;
            static_counter++;
        } else {
            s = i;
            p--;
        }
        
        // Prevent optimization
        asm volatile("" : : "r"(p), "r"(q), "r"(r), "r"(s));
    }
}

// Pattern 3: Function call in condition with modification
void __attribute__((noinline)) test_pattern3(void) {
    volatile int m = 0;
    volatile int n = 100;
    
    for (int iter = 0; iter < 30; iter++) {
        // Function call in condition - creates complex test_expr
        if (side_effect_func() > 0 && m < n) {
            // Modify globals that side_effect_func() reads
            global_x = iter;      // Affects future calls to side_effect_func()
            global_y = m + n;     // Affects future calls to side_effect_func()
            
            // Also modify local condition variables
            m = n / 2;            // Modifies 'm' used in condition
            n--;                  // Modifies 'n' used in condition
            
            // Multiple instructions in header
            int temp = m * n;
            global_z = temp;
            static_counter += 2;
        } else {
            m += 2;
            n += 3;
        }
        
        // Memory barrier to prevent reordering
        asm volatile("" : : : "memory");
    }
}

// Pattern 4: Nested conditions with modification
void __attribute__((noinline)) test_pattern4(void) {
    volatile int x = 0;
    volatile int y = 100;
    volatile int z = 50;
    
    int outer;
    for (outer = 0; outer < 20; outer++) {
        // Outer condition
        if (x < y) {
            // Inner condition - both blocks could be if-converted
            if (y > z && x != 0) {
                // Modify variables from both conditions
                x = y - z;      // Modifies 'x' used in outer condition
                y = z + outer;  // Modifies 'y' used in both conditions
                z = x * 2;      // Modifies 'z' used in inner condition
                
                // Several non-label instructions
                global_x += x;
                global_y += y;
                global_z += z;
            } else {
                z = x + y;
            }
        } else {
            y = x - z;
        }
        
        // Volatile access to prevent dead code elimination
        (void)*(volatile int*)&x;
    }
}

// Pattern 5: Switch-like pattern with modification
void __attribute__((noinline)) test_pattern5(void) {
    volatile int val = 0;
    volatile int threshold = 50;
    volatile int result = 0;
    
    for (int cycle = 0; cycle < 40; cycle++) {
        // Multiple conditions in sequence
        if (val < threshold) {
            // Early modification in then block
            val = threshold - cycle;  // Modifies 'val' used in condition
            result += val;
            
            // Additional instructions
            global_x ^= result;
            static_counter++;
        }
        
        if (result > 25) {
            threshold = result / 2;  // Modifies 'threshold' used in first condition
            val += cycle;
        }
        
        if (global_x > global_y) {
            global_y = global_x;
            val = global_y;  // Cross-modification
        }
        
        // Compiler barrier
        asm volatile("" : : : "memory");
    }
}

int main(int argc, char *argv[]) {
    // Use command line arguments to introduce runtime variability
    int seed = 0;
    if (argc > 1) {
        seed = atoi(argv[1]);
    }
    
    // Initialize with some variability
    global_x = seed % 100;
    global_y = (seed * 3) % 100;
    global_z = (seed * 7) % 100;
    static_counter = 0;
    
    // Execute all test patterns
    test_pattern1();
    test_pattern2();
    test_pattern3();
    test_pattern4();
    test_pattern5();
    
    // Compute and print result to ensure side effects are observable
    int final_result = global_x + global_y + global_z + static_counter;
    printf("Result: %d (x=%d, y=%d, z=%d, counter=%d)\n", 
           final_result, global_x, global_y, global_z, static_counter);
    
    return final_result % 100;
}
