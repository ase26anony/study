#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

// Function that modifies test variable directly
void direct_modification(int x, int y) {
    volatile int v = x; // volatile to prevent optimization
    
    // Pattern 1: Direct modification of test variable
    if (v > 0) {
        v = y * 2;      // Modifies the test variable
        v += 3;         // Additional modification
    }
    
    // Prevent dead code elimination
    printf("Direct: %d\n", v);
}

// Function with pointer-based modification
void pointer_modification(int *ptr1, int *ptr2) {
    // Pattern 2: Pointer aliasing scenario
    if (*ptr1 > 10) {
        *ptr1 = 0;      // Could modify test expression if ptr1 == ptr2
        *ptr2 = 5;      // Additional memory write
    }
}

// Function with array access and potential aliasing
void array_modification(int arr[], int n) {
    // Pattern 3: Loop-dependent condition with modification
    for (int i = 0; i < n; i++) {
        if (arr[i] > 100) {
            arr[i] = 50;        // Modifies test location
            arr[i % n] = 25;    // Potential aliasing
        }
    }
}

// Function with mixed data types
void mixed_types_modification(int x, float y) {
    volatile float f = y;
    
    // Pattern 4: Mixed types with implicit conversion
    if ((float)x > f) {
        x = (int)(f * 2.0f);    // Modifies variable used in test
        f = f + 1.0f;           // Additional modification
    }
    
    printf("Mixed: %d, %f\n", x, f);
}

// Function with atomic operations
void atomic_modification(_Atomic int *atomic_var) {
    // Pattern 5: Atomic variable in condition
    int val = atomic_load(atomic_var);
    if (val > 0) {
        atomic_store(atomic_var, 0);    // Modifies test variable
        // Additional non-atomic operation
        int temp = val * 2;
        (void)temp; // Use to prevent optimization
    }
}

// Complex scenario with multiple modifications
void complex_scenario(int *data, int size) {
    int *ptr = data;
    int sum = 0;
    
    for (int i = 0; i < size; i++) {
        // Pattern 6: Multiple conditions and modifications
        if (ptr[i] > 0) {
            ptr[i] = -ptr[i];           // Modifies test location
            sum += ptr[i];              // Additional operation
            if (i > 0) {
                ptr[i-1] = ptr[i] * 2;  // Another memory write
            }
        }
    }
    
    printf("Sum: %d\n", sum);
}

// Function with short then-block (good candidate for if-conversion)
void short_then_block(int *a, int *b) {
    // Pattern 7: Short block with 2-3 instructions
    if (*a > *b) {
        *a = *b;        // First modification
        *b = *a + 1;    // Second modification
    }
}

// Function with register pressure to encourage different optimizations
void register_pressure(int a, int b, int c, int d, int e) {
    volatile int v1 = a, v2 = b, v3 = c, v4 = d, v5 = e;
    
    // Multiple conditions creating register pressure
    if (v1 > v2) {
        v1 = v3 + v4;   // Modifies test variable
        v2 = v5 * 2;    // Additional modification
        v3 = v1 - v2;   // Third modification
    }
    
    printf("Reg pressure: %d %d %d\n", v1, v2, v3);
}

int main() {
    // Initialize test data
    volatile int test_var = 42;
    int array[10] = {1, 20, 300, 4, 500, 6, 700, 8, 900, 10};
    int x = 10, y = 20;
    _Atomic int atomic_val = 100;
    float f_val = 15.5f;
    
    // Test 1: Direct modification
    direct_modification(test_var, y);
    
    // Test 2: Pointer aliasing
    int *ptr1 = &x;
    int *ptr2 = &x;  // Same address - definite aliasing
    pointer_modification(ptr1, ptr2);
    
    // Test 3: Array modification in loop
    array_modification(array, 10);
    
    // Test 4: Mixed types
    mixed_types_modification(x, f_val);
    
    // Test 5: Atomic operations
    atomic_modification(&atomic_val);
    
    // Test 6: Complex scenario
    complex_scenario(array, 10);
    
    // Test 7: Short then-block
    int a = 5, b = 3;
    short_then_block(&a, &b);
    
    // Test 8: Register pressure
    register_pressure(1, 2, 3, 4, 5);
    
    // Test 9: Nested conditions
    for (int i = 0; i < 5; i++) {
        if (array[i] > 50) {
            array[i] = 0;
            if (i % 2 == 0) {
                array[i+1] = array[i] * 2;
            }
        }
    }
    
    // Test 10: Multiple modifications of same variable
    volatile int counter = 0;
    for (int i = 0; i < 10; i++) {
        if (counter < 5) {
            counter++;      // First modification
            counter *= 2;   // Second modification
        }
    }
    
    // Use results to prevent dead code elimination
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += array[i];
    }
    
    printf("Final sum: %d\n", sum);
    printf("Counter: %d\n", counter);
    printf("Atomic: %d\n", atomic_load(&atomic_val));
    
    return sum > 0 ? 0 : 1;
}
