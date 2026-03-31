#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

// Function 1: Direct modification of test variable in then block
void test_direct_modification(void) {
    volatile int x = 10;
    int y = 5;
    
    // Pattern 1: Direct modification of test variable
    // This should trigger modified_in_p check
    if (x > 0) {
        x = 20;          // Modifies the test variable
        y = x + 5;       // Additional instruction
        x = y - 3;       // Another modification
    }
    
    // Pattern 2: Multiple non-label instructions
    volatile int a = 15;
    int b = 3;
    if (a < 100) {
        a = a * 2;       // Modifies test variable
        b = a + 10;      // Additional instruction
        a = b / 2;       // Another modification
    }
}

// Function 2: Pointer-based modification with potential aliasing
void test_pointer_aliasing(int *arr, int n) {
    volatile int *ptr = arr;
    
    // Pattern 3: Pointer modification that could alias
    if (*ptr > 0) {
        *ptr = 0;        // Direct modification through pointer
        ptr[1] = *ptr;   // Additional memory access
        *ptr = -1;       // Another modification
    }
    
    // Pattern 4: Array access with potential aliasing
    int i = 0;
    int j = 0;  // Same index -> definite aliasing
    if (arr[i] > 10) {
        arr[j] = 0;      // Could modify same location
        arr[i] = 5;      // Definitely modifies test location
        arr[j] = arr[i] * 2;
    }
}

// Function 3: Mixed data types and implicit conversions
void test_mixed_types(void) {
    volatile float f = 3.14f;
    int x = 10;
    double d = 2.718;
    
    // Pattern 5: Mixed types with conversions
    if ((float)x > 2.5f) {
        x = (int)(f * 2.0f);  // Modifies variable used in test
        f = (float)x / 3.0f;  // Additional modification
        x = (int)d + 5;       // Another instruction
    }
    
    // Pattern 6: Char and int mixing
    volatile char c = 'A';
    if (c < 'Z') {
        c = c + 1;            // Modifies test variable
        int temp = c * 2;     // Additional instruction
        c = temp % 26 + 'A';  // Another modification
    }
}

// Function 4: Loop-dependent conditions
void test_loop_dependent(int *array, int size, int threshold) {
    // Pattern 7: Loop with condition modifying test location
    for (int i = 0; i < size; i++) {
        if (array[i] > threshold) {
            array[i] = 0;          // Modifies test location
            array[i] = threshold;  // Additional modification
            array[i] = -1;         // Another modification
        }
    }
    
    // Pattern 8: Nested conditions in loop
    volatile int counter = 0;
    for (int i = 0; i < size; i++) {
        if (array[i] != 0) {
            if (counter < 5) {     // Inner if with volatile
                counter++;          // Modifies test variable
                array[i] = counter; // Additional instruction
                counter = counter * 2;
            }
        }
    }
}

// Function 5: Complex expression with side effects
int process_with_side_effects(int *p, int *q) {
    // Pattern 9: Test expression with memory dereference
    if (*p > *q) {
        *p = *q;            // Modifies one operand of test
        *q = *p + 1;        // Additional modification
        *p = *q * 2;        // Another modification
        return 1;
    }
    return 0;
}

// Function 6: Atomic operations (prevent optimization)
void test_atomic_operations(void) {
    _Atomic int atomic_var = 0;
    volatile int regular_var = 5;
    
    // Pattern 10: Atomic variable in condition
    if (atomic_var > 0) {
        atomic_var = 10;           // Atomic modification
        regular_var = atomic_var;  // Additional instruction
        atomic_var = regular_var / 2;
    }
}

// Main function with varied control flow
int main(void) {
    const int ARRAY_SIZE = 100;
    int *array = (int*)malloc(ARRAY_SIZE * sizeof(int));
    int sum = 0;
    
    // Initialize array with pattern
    for (int i = 0; i < ARRAY_SIZE; i++) {
        array[i] = (i * 3) % 50;
    }
    
    // Test 1: Direct modification
    test_direct_modification();
    
    // Test 2: Pointer aliasing
    test_pointer_aliasing(array, ARRAY_SIZE);
    
    // Test 3: Mixed types
    test_mixed_types();
    
    // Test 4: Loop-dependent conditions
    test_loop_dependent(array, ARRAY_SIZE, 25);
    
    // Test 5: Complex expressions
    int x = 10, y = 20;
    for (int i = 0; i < 10; i++) {
        sum += process_with_side_effects(&x, &y);
        x = (x + 1) % 30;
        y = (y - 1) % 30;
    }
    
    // Test 6: Atomic operations
    test_atomic_operations();
    
    // Use results to prevent dead code elimination
    for (int i = 0; i < ARRAY_SIZE; i++) {
        sum += array[i];
    }
    
    printf("Final sum: %d\n", sum);
    
    free(array);
    return sum != 0 ? 0 : 1;
}
