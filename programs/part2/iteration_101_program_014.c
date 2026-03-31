#include <stdio.h>
#include <stdlib.h>

// Global volatile variables to prevent optimization
volatile int global_a = 10;
volatile int global_b = 20;
volatile int global_c = 30;
volatile int global_d = 40;
static volatile int counter = 0;
volatile int limit = 5;

// Function with side effects
volatile int __attribute__((noinline)) side_effect_func(void) {
    static volatile int state = 0;
    return state++;
}

// Test 1: Simple modification of condition variable in then block
void __attribute__((noinline)) test1_modify_in_then(void) {
    volatile int a = global_a;
    volatile int b = global_b;
    
    // Loop to create basic block context
    for (int i = 0; i < 3; i++) {
        // Condition with variable used in test
        if (a > b) {
            // This modifies 'a' which is part of the condition
            // Should trigger modified_in_p check in ifcvt
            a = b + i;  // Modification in then block header
            global_c = a * 2;
        } else {
            b = a + i;
        }
    }
    global_a = a;
    global_b = b;
}

// Test 2: Compound condition with multiple modifications
void __attribute__((noinline)) test2_compound_condition(void) {
    volatile int x = global_a;
    volatile int y = global_b;
    volatile int z = global_c;
    
    // Complex condition
    if (x != 0 && y < z) {
        // Multiple modifications of condition variables
        y = x;      // Modifies 'y' used in (y < z)
        x = 0;      // Modifies 'x' used in (x != 0)
        z = y + x;  // Modifies 'z' used in (y < z)
        
        // Additional non-trivial code to create block header
        global_d = x + y + z;
    }
    
    global_a = x;
    global_b = y;
    global_c = z;
}

// Test 3: Static variable modified in then block
void __attribute__((noinline)) test3_static_modification(void) {
    volatile int local_counter = counter;
    
    while (local_counter < limit) {
        // Condition test uses local_counter
        if (local_counter < (limit - 1)) {
            // Modification of the condition variable
            local_counter++;  // This changes local_counter used in condition
            counter = local_counter;
            
            // Additional operations to ensure block has header instructions
            global_a += local_counter;
            global_b -= local_counter;
        } else {
            break;
        }
    }
}

// Test 4: Function call in condition with side effects
void __attribute__((noinline)) test4_func_in_condition(void) {
    volatile int val1 = global_a;
    volatile int val2 = global_b;
    
    // Loop with function call in condition
    for (int i = 0; i < 2; i++) {
        // Function call that reads global state
        if (side_effect_func() > 0 && val1 < val2) {
            // Modify global state that side_effect_func() might read
            global_a = val2;  // Could affect future calls to side_effect_func
            val1 = global_a;  // Modifies val1 used in condition
            
            // More operations to fill block header
            val2 = val1 + i;
            global_c = val1 * val2;
        }
    }
}

// Test 5: Nested conditions with modifications
void __attribute__((noinline)) test5_nested_modifications(void) {
    volatile int p = global_a;
    volatile int q = global_b;
    volatile int r = global_c;
    
    if (p > q) {
        // First level then block
        p = q - 1;  // Modifies p
        
        if (r != 0) {
            // Nested then block with modification
            r = p;  // Modifies r used in condition (r != 0)
            q = r * 2;  // Modifies q
            
            // This creates a then block header with multiple instructions
            global_d = p + q + r;
        }
    }
    
    global_a = p;
    global_b = q;
    global_c = r;
}

int main(int argc, char *argv[]) {
    // Use command line arguments to introduce runtime variability
    if (argc > 1) {
        global_a = atoi(argv[1]);
        if (argc > 2) global_b = atoi(argv[2]);
        if (argc > 3) global_c = atoi(argv[3]);
        if (argc > 4) limit = atoi(argv[4]);
    }
    
    printf("Initial values: a=%d, b=%d, c=%d, d=%d, counter=%d, limit=%d\n",
           global_a, global_b, global_c, global_d, counter, limit);
    
    // Run all test functions
    test1_modify_in_then();
    printf("After test1: a=%d, b=%d, c=%d\n", global_a, global_b, global_c);
    
    test2_compound_condition();
    printf("After test2: a=%d, b=%d, c=%d, d=%d\n", global_a, global_b, global_c, global_d);
    
    test3_static_modification();
    printf("After test3: counter=%d, a=%d, b=%d\n", counter, global_a, global_b);
    
    test4_func_in_condition();
    printf("After test4: a=%d, c=%d\n", global_a, global_c);
    
    test5_nested_modifications();
    printf("After test5: a=%d, b=%d, c=%d, d=%d\n", global_a, global_b, global_c, global_d);
    
    // Final computation to ensure side effects are observable
    int result = global_a + global_b * 2 + global_c * 3 + global_d * 4 + counter;
    printf("Final result: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
