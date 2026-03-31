#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

// Function with pointer-based modification
void process_pointer(int *p) {
    // Pattern 1: Direct modification of test variable
    if (*p > 0) {
        *p = -1;  // Modifies the test expression
        *p += 2;  // Additional modification
    }
}

// Function with array aliasing
void process_array(int arr[], int n) {
    for (int i = 0; i < n; i++) {
        // Pattern 2: Loop-dependent condition with modification
        if (arr[i] > 100) {
            arr[i] = 0;  // Modifies the test expression
            arr[i] += 5; // Second instruction in then block
        }
    }
}

// Function with volatile and mixed types
void process_volatile(volatile int *v) {
    // Pattern 3: Volatile test with modification
    if (*v > 10) {
        *v = 5;     // First modification
        *v *= 2;    // Second modification
    }
}

// Function with potential aliasing through pointers
void process_aliasing(int *a, int *b, int idx) {
    // Pattern 4: Aliasing concern - b might point to same location as a[idx]
    if (a[idx] > 50) {
        *b = 0;     // Could modify a[idx] if aliased
        a[idx] = 25; // Definitely modifies test expression
    }
}

// Function with different data types
void process_mixed_types(float *f, int *i) {
    // Pattern 5: Mixed types and implicit conversions
    if ((float)*i > 0.5f) {
        *i = (int)(*f * 2.0f);  // Modifies variable used in test
        *i += 1;                // Additional instruction
    }
}

// Function with atomic operations
void process_atomic(_Atomic int *atom) {
    // Pattern 6: Atomic variable in condition
    int val = atomic_load(atom);
    if (val > 0) {
        atomic_store(atom, 0);  // Modifies test variable
        atomic_fetch_add(atom, 1); // Second atomic operation
    }
}

// Complex scenario with nested conditions
void process_complex(int data[], int n, int *threshold) {
    for (int i = 0; i < n; i++) {
        // Pattern 7: Multiple conditions and modifications
        if (data[i] > *threshold) {
            data[i] = *threshold;  // Modifies test expression
            *threshold += 1;       // Modifies threshold used in condition
            data[i] -= 2;          // Third instruction
        }
    }
}

int main() {
    // Setup test data
    volatile int volatile_var = 15;
    int array[10] = {5, 150, 25, 200, 75, 300, 10, 400, 50, 250};
    int scalar = 42;
    int alias_test[3] = {100, 200, 300};
    float float_val = 3.14f;
    _Atomic int atomic_val = ATOMIC_VAR_INIT(10);
    int threshold = 100;
    
    // Test 1: Direct pointer modification
    process_pointer(&scalar);
    
    // Test 2: Array processing with loop
    process_array(array, 10);
    
    // Test 3: Volatile access
    process_volatile(&volatile_var);
    
    // Test 4: Potential aliasing
    process_aliasing(alias_test, &alias_test[1], 0);
    
    // Test 5: Mixed data types
    process_mixed_types(&float_val, &scalar);
    
    // Test 6: Atomic operations
    process_atomic(&atomic_val);
    
    // Test 7: Complex scenario
    process_complex(array, 10, &threshold);
    
    // Additional inline test cases to ensure coverage
    
    // Inline pattern 1: Short then block with modification
    int x = 10;
    if (x > 5) {
        x = 20;  // First modification
        x += 5;  // Second modification
    }
    
    // Inline pattern 2: Memory aliasing with pointers
    int arr1[5] = {1, 2, 3, 4, 5};
    int *ptr1 = &arr1[2];
    int *ptr2 = &arr1[2];
    if (*ptr1 > 2) {
        *ptr2 = 10;  // Aliases with ptr1
        *ptr1 = 15;  // Direct modification
    }
    
    // Inline pattern 3: Volatile in condition
    volatile int v = 25;
    if (v > 20) {
        v = 30;
        v -= 5;
    }
    
    // Inline pattern 4: Loop with condition modification
    int loop_arr[8] = {10, 20, 30, 40, 50, 60, 70, 80};
    for (int i = 0; i < 8; i++) {
        if (loop_arr[i] > 35) {
            loop_arr[i] = 0;    // Modifies test expression
            loop_arr[i] += i;   // Second instruction
        }
    }
    
    // Inline pattern 5: Different scopes to create multiple basic blocks
    {
        int local_var = 100;
        if (local_var > 50) {
            local_var = 200;
            local_var /= 2;
        }
    }
    
    // Use results to prevent dead code elimination
    int sum = scalar + array[0] + array[5] + alias_test[0] + x + arr1[2] + loop_arr[3];
    
    printf("Result: %d\n", sum);
    printf("Volatile: %d\n", volatile_var);
    printf("Atomic: %d\n", atomic_load(&atomic_val));
    printf("Threshold: %d\n", threshold);
    
    return sum != 0 ? 0 : 1;
}
