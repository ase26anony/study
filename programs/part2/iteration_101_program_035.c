#include <stdio.h>
#include <stdlib.h>

// Global variables to prevent constant propagation
volatile int global_modifier = 0;
static volatile int static_counter = 0;

// Function with side effects to use in conditions
int __attribute__((noinline)) side_effect_func(void) {
    return global_modifier++;
}

// Pattern 1: Simple modification of condition variable in then block
void __attribute__((noinline)) pattern1(volatile int a, volatile int b) {
    for (int i = 0; i < 10; i++) {
        // The condition uses 'a', and the then block modifies 'a'
        if (a > b) {
            // This instruction should be in the then block header
            a = b + i;  // Modifies 'a' which is used in the condition
            b = a * 2;  // Additional instruction to create block header
            global_modifier += a;
        }
        // Prevent loop unrolling from simplifying too much
        if (i % 3 == 0) {
            b++;
        }
    }
}

// Pattern 2: Compound condition with multiple modifications
void __attribute__((noinline)) pattern2(volatile int x, volatile int y, volatile int z) {
    int iterations = 5;
    while (iterations-- > 0) {
        // Complex condition with multiple variables
        if (x != 0 && y < z && global_modifier > 0) {
            // Modify 'x' which is used in the first part of condition
            x = y * 2;      // First modifying instruction in header
            y = x + z;      // Second instruction
            z = side_effect_func();  // Function call that reads global_modifier
            // Add more instructions to ensure a substantial block header
            global_modifier += x - y;
            static_counter++;
        } else {
            x = iterations;
        }
        
        // Create data dependency to prevent dead code elimination
        volatile int temp = x + y + z;
        (void)temp;
    }
}

// Pattern 3: Self-modifying condition with static variable
void __attribute__((noinline)) pattern3(volatile int limit) {
    volatile int local_var = 10;
    
    for (int i = 0; i < 8; i++) {
        // Condition uses static_counter which gets modified in then block
        if (static_counter < limit && local_var > 0) {
            // This modifies static_counter used in the condition
            static_counter++;      // First modification
            limit = static_counter / 2;  // Second modification
            local_var--;           // Third modification (local_var in condition)
            global_modifier += 2;  // Fourth instruction
            
            // Add arithmetic to create more RTL instructions
            volatile int calc = static_counter * limit;
            (void)calc;
        }
        
        // Alternate path to create control flow complexity
        if (i % 2 == 0) {
            local_var += i;
        }
    }
}

// Pattern 4: Nested conditions with pointer modification
void __attribute__((noinline)) pattern4(volatile int *ptr1, volatile int *ptr2) {
    volatile int temp = 100;
    
    for (int j = 0; j < 6; j++) {
        // Condition involving pointer dereferences
        if (*ptr1 > *ptr2 && temp < 200) {
            // Modify *ptr1 which is used in the condition
            *ptr1 = *ptr2 + j;     // First modification
            temp = *ptr1 * 3;      // Second instruction
            *ptr2 = temp / 2;      // Third instruction
            
            // Additional instructions for block header
            global_modifier = *ptr1 - *ptr2;
            static_counter += j;
            
            // Memory barrier to prevent reordering
            asm volatile("" ::: "memory");
        }
        
        // Vary the pointers' targets
        if (j % 3 == 1) {
            temp += 50;
        }
    }
}

// Pattern 5: Function call in condition with modification in then block
void __attribute__((noinline)) pattern5(void) {
    volatile int base = 20;
    
    for (int k = 0; k < 7; k++) {
        // Function call in condition that reads global_modifier
        if (side_effect_func() > base && static_counter < 30) {
            // Modify global_modifier which side_effect_func() reads
            global_modifier += 5;      // First modification
            base = global_modifier;    // Second instruction
            static_counter += 2;       // Third instruction
            
            // Complex expression to generate more RTL
            volatile int result = (global_modifier * static_counter) / (base + 1);
            (void)result;
            
            // Another modification
            if (k % 2 == 0) {
                global_modifier -= 3;
            }
        }
        
        // Loop variant to prevent optimization
        base += k;
    }
}

int main(int argc, char *argv[]) {
    // Initialize with command-line arguments for runtime variability
    volatile int a = argc > 1 ? atoi(argv[1]) : 15;
    volatile int b = argc > 2 ? atoi(argv[2]) : 10;
    volatile int c = argc > 3 ? atoi(argv[3]) : 25;
    volatile int x = argc > 4 ? atoi(argv[4]) : 5;
    volatile int y = argc > 5 ? atoi(argv[5]) : 20;
    volatile int z = argc > 6 ? atoi(argv[6]) : 15;
    volatile int limit = argc > 7 ? atoi(argv[7]) : 5;
    
    volatile int ptr1_val = 30;
    volatile int ptr2_val = 40;
    volatile int *ptr1 = &ptr1_val;
    volatile int *ptr2 = &ptr2_val;
    
    // Reset globals
    global_modifier = 1;
    static_counter = 0;
    
    // Execute all patterns
    pattern1(a, b);
    pattern2(x, y, z);
    pattern3(limit);
    pattern4(ptr1, ptr2);
    pattern5();
    
    // Compute and print result to ensure side effects are observable
    int result = global_modifier + static_counter + *ptr1 + *ptr2;
    printf("Result: %d (global_modifier=%d, static_counter=%d, *ptr1=%d, *ptr2=%d)\n",
           result, global_modifier, static_counter, *ptr1, *ptr2);
    
    return 0;
}
