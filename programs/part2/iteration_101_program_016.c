#include <stdio.h>
#include <stdlib.h>

// Global variables to prevent optimization
volatile int global_modifier = 0;
static volatile int static_counter = 0;

// Function with side effects to use in conditions
__attribute__((noinline)) int side_effect_func(void) {
    return global_modifier++;
}

// Pattern 1: Simple modification of condition variable in then block
__attribute__((noinline)) void test_pattern1(volatile int a, volatile int b, volatile int c) {
    // Loop context to create interesting block structures
    for (int i = 0; i < 10; i++) {
        // Compound condition with multiple variables
        if (a > b && c < 100) {
            // This modifies 'a' which is used in the condition
            a = b + c;  // Modifies condition variable early in then block
            c = a * 2;  // Additional instruction to create header length
            b++;        // Another modification
        }
    }
}

// Pattern 2: Complex condition with multiple modifications
__attribute__((noinline)) void test_pattern2(volatile int x, volatile int y, volatile int z) {
    volatile int local = 0;
    
    while (local < 20) {
        // Complex condition with function call
        if ((x != y || side_effect_func() > 0) && z < 50) {
            // Multiple modifications of condition variables
            x = y;      // Modifies 'x' used in condition
            y = z + 1;  // Modifies 'y' used in condition
            z = x * y;  // Modifies 'z' used in condition
            // Additional non-label, non-note instructions
            local += x + y + z;
        }
        local++;
    }
}

// Pattern 3: Static variable modification in then block
__attribute__((noinline)) void test_pattern3(volatile int limit) {
    for (int iteration = 0; iteration < 5; iteration++) {
        // Condition uses static variable
        if (static_counter < limit && global_modifier > 0) {
            // Modify static variable used in condition
            static_counter++;  // This modifies the condition variable
            global_modifier--; // Also modifies global used in condition
            // Add more instructions to ensure header analysis
            limit = static_counter * 2;
            volatile int temp = limit + global_modifier;
            (void)temp; // Use temp to prevent optimization
        }
    }
}

// Pattern 4: Nested conditions with modifications
__attribute__((noinline)) void test_pattern4(volatile int p, volatile int q) {
    volatile int r = 0;
    
    do {
        // Compound condition with all variables
        if (p > q && q < r && r < 100) {
            // Modify multiple condition variables
            p = q;          // Modifies 'p' from condition
            q = r + p;      // Modifies 'q' from condition  
            r = p * q;      // Modifies 'r' from condition
            // Ensure these are not the first instructions after label
            volatile int sum = p + q + r;
            (void)sum;
        }
        r++;
    } while (r < 15);
}

// Pattern 5: Pointer aliasing could affect condition
__attribute__((noinline)) void test_pattern5(volatile int* ptr1, volatile int* ptr2) {
    for (int i = 0; i < 8; i++) {
        // Condition using dereferenced pointers
        if (*ptr1 > *ptr2 && i < 5) {
            // Modify through pointers - could alias
            *ptr1 = *ptr2;      // Modifies condition operand
            *ptr2 = i;          // Modifies other condition operand
            // Additional instructions
            volatile int diff = *ptr1 - *ptr2;
            (void)diff;
        }
    }
}

int main(int argc, char* argv[]) {
    // Initialize with runtime values to prevent constant folding
    volatile int a = argc > 1 ? atoi(argv[1]) : 10;
    volatile int b = argc > 2 ? atoi(argv[2]) : 20;
    volatile int c = argc > 3 ? atoi(argv[3]) : 30;
    volatile int x = argc > 4 ? atoi(argv[4]) : 40;
    volatile int y = argc > 5 ? atoi(argv[5]) : 50;
    volatile int z = argc > 6 ? atoi(argv[6]) : 60;
    
    // Call all test patterns
    test_pattern1(a, b, c);
    test_pattern2(x, y, z);
    test_pattern3(a + b);
    
    volatile int p = x + y;
    volatile int q = y + z;
    test_pattern4(p, q);
    
    volatile int val1 = a + x;
    volatile int val2 = b + y;
    test_pattern5(&val1, &val2);
    
    // Compute and print result to ensure side effects
    int result = a + b + c + x + y + z + p + q + val1 + val2 + static_counter + global_modifier;
    printf("Result: %d\n", result);
    
    return 0;
}
