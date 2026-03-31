#include <stdio.h>
#include <stdlib.h>

// Global volatile variables to prevent optimization
volatile int g1 = 0;
volatile int g2 = 0;
volatile int g3 = 0;
volatile int g4 = 0;
static volatile int static_counter = 0;

// Function with side effects for condition testing
int __attribute__((noinline)) side_effect_func(void) {
    static int call_count = 0;
    return ++call_count;
}

// Function that reads and modifies global state
int __attribute__((noinline)) read_and_modify(void) {
    int temp = g1;
    g1 = (g1 + 1) % 100;
    return temp;
}

// Test 1: Simple modification of condition variable in then block
void __attribute__((noinline)) test1_modify_in_then(void) {
    volatile int a = g1;
    volatile int b = g2;
    
    // Loop to create basic block context
    for (int i = 0; i < 10; i++) {
        // Condition uses a and b
        if (a > b) {
            // CRITICAL: Modify 'a' which is used in the condition
            a = b + i;  // This should trigger modified_in_p check
            g3 += a;
        }
        // Ensure loop has side effects
        b += i;
    }
    
    g1 = a;
    g2 = b;
}

// Test 2: Compound condition with multiple modifications
void __attribute__((noinline)) test2_compound_condition(void) {
    volatile int x = g1;
    volatile int y = g2;
    volatile int z = g3;
    
    // Complex condition
    if (x != 0 && y < z && (x + y) > 10) {
        // Multiple modifications of condition variables
        x = y * 2;      // Modifies x used in (x != 0) and (x + y) > 10
        y = z - 1;      // Modifies y used in (y < z) and (x + y) > 10
        z = x + y;      // Modifies z used in (y < z)
        
        // Additional non-trivial operations to create more instructions
        // in the then block header
        g4 = x + y + z;
        static_counter++;
    }
    
    g1 = x;
    g2 = y;
    g3 = z;
}

// Test 3: Function call in condition with modification in then block
void __attribute__((noinline)) test3_func_in_condition(void) {
    volatile int base = g1;
    
    // Loop with function call in condition
    for (int i = 0; i < 5; i++) {
        // Function call in condition - creates complex RTL
        if (read_and_modify() > base && side_effect_func() < 10) {
            // Modify variable that read_and_modify() reads (g1)
            g1 = base + i;  // This modifies global state used in condition
            
            // Additional instructions to ensure we have a block header
            int temp = g2;
            g2 = temp * 2;
            g3 += i;
        }
        base += 2;
    }
}

// Test 4: Static variable modification in then block
void __attribute__((noinline)) test4_static_modification(void) {
    volatile int limit = g4;
    
    // Use static variable in condition
    if (static_counter < limit && g1 > 0) {
        // Modify static variable used in condition
        static_counter += g1;  // Modifies static_counter used in condition
        
        // Also modify other condition variables
        g1 = g1 / 2;
        g4 = static_counter;
        
        // Multiple instructions in then block header
        volatile int a = g2;
        volatile int b = g3;
        g2 = a + b;
        g3 = a - b;
    }
}

// Test 5: Nested conditions with modifications
void __attribute__((noinline)) test5_nested_modifications(void) {
    volatile int p = g1;
    volatile int q = g2;
    volatile int r = g3;
    
    for (int iter = 0; iter < 3; iter++) {
        // Outer condition
        if (p > q) {
            // Inner condition with modification
            if (q < r) {
                // Modify variables from outer condition
                p = q + r;  // Modifies p used in outer condition (p > q)
                q = p - 1;  // Modifies q used in both conditions
                
                // More instructions
                r += iter;
                g4 = p + q + r;
            }
            // Additional modification in outer then block
            p += 1;
        }
        // Loop increment
        r += 2;
    }
    
    g1 = p;
    g2 = q;
    g3 = r;
}

// Test 6: Pointer modification affecting condition
void __attribute__((noinline)) test6_pointer_modification(void) {
    volatile int arr[4] = {g1, g2, g3, g4};
    volatile int *ptr = &arr[0];
    volatile int idx = 0;
    
    // Loop with pointer arithmetic in condition
    while (idx < 3) {
        // Condition using pointer dereference
        if (*ptr > arr[idx + 1] && idx < 2) {
            // Modify what ptr points to (affects *ptr in condition)
            *ptr = arr[idx + 1] + 1;  // Modifies *ptr used in condition
            
            // Also modify index
            idx = (idx + 1) % 3;  // Modifies idx used in condition
            
            // Additional operations
            arr[3] += *ptr;
        }
        ptr = &arr[idx];
        idx++;
    }
    
    g1 = arr[0];
    g2 = arr[1];
    g3 = arr[2];
    g4 = arr[3];
}

int main(int argc, char *argv[]) {
    // Initialize with command line or random values to prevent constant folding
    if (argc > 1) {
        g1 = atoi(argv[1]) % 100;
        g2 = atoi(argv[1]) % 50 + 10;
        g3 = atoi(argv[1]) % 30 + 20;
        g4 = atoi(argv[1]) % 20 + 5;
    } else {
        g1 = 42;
        g2 = 17;
        g3 = 89;
        g4 = 12;
    }
    
    static_counter = 0;
    
    // Run all test functions
    test1_modify_in_then();
    test2_compound_condition();
    test3_func_in_condition();
    test4_static_modification();
    test5_nested_modifications();
    test6_pointer_modification();
    
    // Compute and print result to ensure side effects are observable
    int result = g1 + g2 * 2 + g3 * 3 + g4 * 4 + static_counter;
    printf("Result: %d (g1=%d, g2=%d, g3=%d, g4=%d, static_counter=%d)\n",
           result, g1, g2, g3, g4, static_counter);
    
    return 0;
}
