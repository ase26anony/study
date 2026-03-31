#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

// Function that modifies test expression via pointer
void modify_condition_variable(int *ptr) {
    // Pattern 1: Direct modification of test variable
    if (*ptr > 0) {
        *ptr = -1;  // Modifies the test expression
        *ptr += 2;  // Additional modification
    }
}

// Function with volatile test variable
void volatile_modification(volatile int *v) {
    // Pattern 4: Volatile prevents optimization
    if (*v > 10) {
        *v = 5;     // Modifies volatile test expression
        *v *= 2;    // Second instruction in then block
    }
}

// Function with array and potential aliasing
void array_modification(int arr[], int n) {
    // Pattern 5: Loop-dependent condition
    for (int i = 0; i < n; i++) {
        // Pattern 3: Array access as test expression
        if (arr[i] > 0) {
            arr[i] = 0;     // Modifies test expression location
            arr[i] -= 1;    // Additional instruction
        }
    }
}

// Function with mixed data types
void mixed_types_modification(float *fptr, int *iptr) {
    // Pattern 6: Implicit conversion in condition
    if (*fptr > 0.5f) {
        *iptr = (int)(*fptr * 2.0f);  // Modifies related variable
        *iptr += 1;                   // Second instruction
    }
}

// Function with pointer aliasing
void aliasing_modification(int *a, int *b) {
    // Pattern 3: Potential aliasing
    if (*a > *b) {
        *a = *b;        // Could modify test expression if a == b
        *a += 10;       // Additional instruction
    }
}

// Complex scenario with multiple conditions
void complex_scenario(int *data, int size) {
    volatile int threshold = 50;
    int *ptr = data;
    
    for (int i = 0; i < size; i++) {
        // Multiple conditions in sequence
        if (ptr[i] > threshold) {
            ptr[i] = threshold;     // Modifies test expression
            threshold++;            // Modifies condition variable
        }
        
        // Nested condition
        if (i % 2 == 0) {
            int *alias = &ptr[i];
            if (*alias < 0) {
                *alias = 0;         // Modifies through alias
                *alias += i;        // Additional instruction
            }
        }
    }
}

// Function with atomic operations
void atomic_modification(_Atomic int *atomic_var) {
    int expected = atomic_load(atomic_var);
    if (expected > 100) {
        atomic_store(atomic_var, 50);   // Modifies atomic test expression
        atomic_fetch_add(atomic_var, 1); // Second atomic operation
    }
}

// Main function with varied test cases
int main() {
    // Test 1: Direct variable modification
    int x = 10;
    if (x > 5) {
        x = 3;      // Modifies test variable
        x *= 2;     // Additional instruction (2nd non-label instruction)
    }
    
    // Test 2: Volatile variable
    volatile int v = 20;
    if (v > 15) {
        v = 10;     // Modifies volatile test expression
        v -= 2;     // Additional instruction
    }
    
    // Test 3: Array processing
    int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = i * 10;
    }
    array_modification(arr, 10);
    
    // Test 4: Pointer modification
    int y = 30;
    modify_condition_variable(&y);
    
    // Test 5: Mixed types
    float f = 0.75f;
    int i = 0;
    mixed_types_modification(&f, &i);
    
    // Test 6: Aliasing
    int a = 40, b = 35;
    aliasing_modification(&a, &a);  // Same pointer for aliasing
    
    // Test 7: Complex scenario
    int data[20];
    for (int j = 0; j < 20; j++) {
        data[j] = j * 5;
    }
    complex_scenario(data, 20);
    
    // Test 8: Atomic operations
    _Atomic int atomic_val = ATOMIC_VAR_INIT(150);
    atomic_modification(&atomic_val);
    
    // Test 9: Multiple modifications in then block
    int z = 25;
    if (z > 20) {
        z = z / 5;      // First modification
        z = z * 3;      // Second modification
        z += 1;         // Third modification (3 non-label instructions)
    }
    
    // Test 10: Memory aliasing with different indices
    int buffer[100];
    for (int k = 0; k < 100; k++) {
        buffer[k] = k;
    }
    
    // Create potential aliasing scenario
    int *p1 = &buffer[10];
    int *p2 = &buffer[10];  // Same location
    if (*p1 > 5) {
        *p2 = 0;            // Modifies test expression via alias
        *p2 = *p1 + 1;      // Additional instruction
    }
    
    // Use results to prevent dead code elimination
    int sum = x + v + arr[0] + y + i + a + data[0] + atomic_load(&atomic_val) + z + buffer[10];
    printf("Result: %d\n", sum);
    
    return 0;
}
