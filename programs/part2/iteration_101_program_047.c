#include <stdio.h>
#include <stdlib.h>

// Global variables to prevent optimization
volatile int global_modifier = 0;
static volatile int static_counter = 0;

// Function with side effects to use in conditions
__attribute__((noinline)) int side_effect_func(void) {
    return global_modifier++;
}

// Test function 1: Simple modification in then block
__attribute__((noinline)) void test1(volatile int a, volatile int b) {
    // Loop to create basic block context
    for (int i = 0; i < 10; i++) {
        // Condition where 'a' is tested
        if (a > b) {
            // MODIFICATION IN THEN BLOCK HEADER: modifies 'a' which is in condition
            a = b + i;  // This should trigger modified_in_p check
            // Additional instructions to ensure block has body
            b = a * 2;
            global_modifier += a;
        }
    }
}

// Test function 2: Compound condition with multiple modifications
__attribute__((noinline)) void test2(volatile int x, volatile int y, volatile int z) {
    // Complex condition with multiple variables
    if (x != 0 && y < z && global_modifier > 0) {
        // Multiple modifications in then block header
        x = y;      // Modifies 'x' used in condition (x != 0)
        y = z + 1;  // Modifies 'y' used in condition (y < z)
        // More instructions
        z = x * y;
        static_counter++;
    }
}

// Test function 3: Function call in condition with modification
__attribute__((noinline)) void test3(void) {
    volatile int limit = 5;
    
    // Loop with condition that uses function with side effects
    while (static_counter < limit) {
        // Function call in condition - returns value based on global_modifier
        if (side_effect_func() > 0 && static_counter < limit) {
            // Modification that affects future calls to side_effect_func
            global_modifier = static_counter;  // Modifies what side_effect_func reads
            static_counter++;  // Modifies 'static_counter' used in condition
            // Additional non-trivial operations
            limit = limit + (global_modifier % 3);
        }
    }
}

// Test function 4: Nested conditions with modifications
__attribute__((noinline)) void test4(volatile int p, volatile int q, volatile int r) {
    for (int i = 0; i < 8; i++) {
        // Complex compound condition
        if ((p > q || r < 100) && (global_modifier % 2 == 0)) {
            // Multiple modifications of condition variables
            if (p > q) {
                p = q - 1;  // Modifies 'p' used in outer condition
            }
            q = r + p;      // Modifies 'q' used in outer condition
            r = i;          // Modifies 'r' used in outer condition
            
            // Additional operations to create more instructions in block
            for (int j = 0; j < 2; j++) {
                global_modifier += (p + q + r);
            }
        }
    }
}

// Test function 5: Pointer modification affecting condition
__attribute__((noinline)) void test5(volatile int* ptr1, volatile int* ptr2) {
    volatile int local = *ptr1;
    
    // Condition using dereferenced pointers
    if (*ptr1 > *ptr2 && local < 100) {
        // Modification through pointer - affects *ptr1 in condition
        *ptr1 = *ptr2 + 10;  // This modifies memory read by condition
        local = *ptr1 * 2;
        
        // Additional modification
        *ptr2 = local / 3;
        
        // More complex operations
        for (int k = 0; k < 3; k++) {
            global_modifier += k + *ptr1;
        }
    }
}

int main(int argc, char* argv[]) {
    // Initialize volatile variables with some randomness from argv
    volatile int a = argc > 1 ? atoi(argv[1]) : 10;
    volatile int b = argc > 2 ? atoi(argv[2]) : 5;
    volatile int c = argc > 3 ? atoi(argv[3]) : 15;
    volatile int x = argc > 4 ? atoi(argv[4]) : 7;
    volatile int y = argc > 5 ? atoi(argv[5]) : 3;
    volatile int z = argc > 6 ? atoi(argv[6]) : 12;
    
    // Test 1: Simple modification in then block
    printf("Test 1: a=%d, b=%d\n", a, b);
    test1(a, b);
    
    // Test 2: Compound condition with modifications
    printf("Test 2: x=%d, y=%d, z=%d\n", x, y, z);
    test2(x, y, z);
    
    // Test 3: Function call in condition
    printf("Test 3: global_modifier=%d, static_counter=%d\n", global_modifier, static_counter);
    test3();
    
    // Test 4: Nested conditions
    volatile int p = 20, q = 10, r = 30;
    printf("Test 4: p=%d, q=%d, r=%d\n", p, q, r);
    test4(p, q, r);
    
    // Test 5: Pointer modifications
    volatile int val1 = 25, val2 = 15;
    volatile int* ptr1 = &val1;
    volatile int* ptr2 = &val2;
    printf("Test 5: *ptr1=%d, *ptr2=%d\n", *ptr1, *ptr2);
    test5(ptr1, ptr2);
    
    // Final computation to ensure side effects are observable
    int result = global_modifier + static_counter + a + b + x + y + z;
    printf("Final result: %d\n", result);
    printf("global_modifier: %d, static_counter: %d\n", global_modifier, static_counter);
    
    return result != 0 ? 0 : 1;
}
