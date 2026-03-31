#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

// Function that modifies test expression via pointer
void modify_via_pointer(int *ptr) {
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
        *v *= 2;    // Second modification
    }
}

// Function with array and potential aliasing
void array_modification(int *arr, int i, int j) {
    // Pattern 3: Potential aliasing
    if (arr[i] > 0) {
        arr[j] = 0;     // May alias if i == j
        arr[i] = -1;    // Direct modification of test expression
    }
}

// Function with mixed data types
void mixed_types(float threshold) {
    volatile int x = 10;
    float y = 3.14f;
    
    // Pattern 6: Implicit conversions
    if ((float)x > threshold) {
        x = (int)(y * 2.0f);  // Modifies x used in test
        x += 1;               // Additional modification
    }
}

// Function with loop-dependent condition
void loop_dependent(int *data, int n, int threshold) {
    // Pattern 5: Loop with side effects
    for (int i = 0; i < n; i++) {
        // Pattern 2: Multiple non-label instructions
        if (data[i] > threshold) {
            data[i] = 0;        // Modifies test expression
            data[i] += i;       // Second modification
            threshold = data[i]; // Third modification (modifies condition variable)
        }
    }
}

// Complex scenario with multiple patterns
void complex_scenario(int *ptr1, int *ptr2, volatile int *vptr) {
    int local = *ptr1;
    
    // Multiple conditions in sequence
    if (local > 0) {
        local = *ptr2;      // Modifies test variable
        local *= 2;         // Additional modification
    }
    
    if (*vptr < 100) {
        *vptr = 150;        // Modifies volatile test expression
        local = *vptr;      // Uses modified value
    }
    
    // Nested condition
    if (local > 50) {
        *ptr1 = local;      // Modifies original pointer
        if (*ptr1 > 75) {   // Test expression may be modified
            *ptr1 = 100;
            local = *ptr1;
        }
    }
}

int main() {
    // Initialize test data
    volatile int volatile_var = 20;
    int array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int *ptr = &array[0];
    int result = 0;
    
    // Test 1: Direct pointer modification
    modify_via_pointer(ptr);
    result += array[0];
    
    // Test 2: Volatile modification
    volatile_modification(&volatile_var);
    result += volatile_var;
    
    // Test 3: Array with potential aliasing
    // Create aliasing scenario
    int idx = 3;
    array_modification(array, idx, idx);  // i == j causes aliasing
    result += array[idx];
    
    // Test 4: Mixed data types
    mixed_types(5.0f);
    
    // Test 5: Loop-dependent with side effects
    int threshold = 5;
    loop_dependent(array, 10, threshold);
    for (int i = 0; i < 10; i++) {
        result += array[i];
    }
    
    // Test 6: Complex scenario
    int var1 = 30, var2 = 40;
    complex_scenario(&var1, &var2, &volatile_var);
    result += var1 + var2 + volatile_var;
    
    // Additional test: Multiple short blocks
    for (int i = 0; i < 5; i++) {
        int temp = array[i];
        // Pattern: Very short then block that modifies test expression
        if (temp > 0) {
            temp = 0;       // Single modification
        }
        // Pattern: Slightly longer then block
        if (temp == 0) {
            temp = i;       // First modification
            temp *= 2;      // Second modification
        }
        array[i] = temp;
        result += temp;
    }
    
    // Test with atomic operations (prevents certain optimizations)
    _Atomic int atomic_var = 42;
    int expected = 42;
    if (atomic_var > 40) {
        atomic_var = 30;    // Modifies atomic test expression
        atomic_var += 5;    // Additional modification
    }
    result += atomic_var;
    
    // Use result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    return result > 0 ? 0 : 1;
}
