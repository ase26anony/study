#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

// Function with pointer-based modification
void process_pointer(int *p) {
    // Pattern 1: Direct modification of test expression
    if (*p > 0) {
        *p = -1;  // Modifies the memory tested in condition
    }
}

// Function with array aliasing
void process_array(int arr[], int n) {
    for (int i = 0; i < n; i++) {
        // Pattern 2: Loop-dependent condition with modification
        if (arr[i] > 10) {
            arr[i] = 0;  // Modifies the test expression
        }
    }
}

// Function with volatile variable
void process_volatile(volatile int *v) {
    // Pattern 3: Volatile prevents optimization
    if (*v > 5) {
        *v = 2;  // Modifies volatile test expression
    }
}

// Function with mixed data types
void process_mixed_types(float *f, int *i) {
    // Pattern 4: Implicit conversions in condition
    if ((float)*i > 0.5f) {
        *i = (int)(*f * 2.0f);  // Modifies variable used in test
    }
}

// Function with multiple modifications in then block
void process_multiple_mods(int *x, int *y) {
    // Pattern 5: Multiple instructions in then block
    if (*x > 0) {
        *x = *x + 1;    // First modification
        *y = *x * 2;    // Second instruction
        *x = *x - 1;    // Third modification (net effect)
    }
}

// Function with potential aliasing
void process_aliasing(int *a, int *b, int idx1, int idx2) {
    // Pattern 6: Potential aliasing through different indices
    if (a[idx1] > 0) {
        a[idx2] = 0;  // Could modify same memory if idx1 == idx2
    }
}

// Main function with various if-conversion candidates
int main() {
    // Initialize test data
    volatile int v1 = 8;
    volatile int v2 = 3;
    int arr[10] = {15, 2, 8, 20, 5, 12, 7, 18, 9, 1};
    int x = 10, y = 20;
    float f = 3.14f;
    
    // Test 1: Direct pointer modification
    int test1 = 7;
    process_pointer(&test1);
    
    // Test 2: Array processing with loop
    process_array(arr, 10);
    
    // Test 3: Volatile variable
    process_volatile(&v1);
    
    // Test 4: Mixed data types
    int int_val = 1;
    process_mixed_types(&f, &int_val);
    
    // Test 5: Multiple modifications
    int mx = 5, my = 10;
    process_multiple_mods(&mx, &my);
    
    // Test 6: Potential aliasing
    int a[5] = {1, 2, 3, 4, 5};
    process_aliasing(a, a, 2, 2);  // Same index - will alias
    
    // Test 7: Inline if with modification (short then block)
    int z = 15;
    if (z > 10) {
        z = z - 5;  // Single modification
    }
    
    // Test 8: More complex then block with 2-3 operations
    int counter = 0;
    for (int i = 0; i < 5; i++) {
        if (arr[i] > 0) {
            arr[i] = arr[i] * 2;  // First operation
            counter++;            // Second operation
            arr[i] = arr[i] / 2;  // Third operation
        }
    }
    
    // Test 9: Nested conditions
    int n1 = 8, n2 = 12;
    if (n1 > 5) {
        if (n2 > 10) {
            n1 = n2;  // Modifies variable from outer condition
        }
    }
    
    // Test 10: Function pointer to prevent inlining
    void (*func_ptr)(int*) = process_pointer;
    int fp_test = 6;
    func_ptr(&fp_test);
    
    // Use results to prevent dead code elimination
    int sum = test1 + v1 + v2 + x + y + int_val + mx + my + z + counter + n1 + n2 + fp_test;
    for (int i = 0; i < 10; i++) {
        sum += arr[i];
    }
    for (int i = 0; i < 5; i++) {
        sum += a[i];
    }
    
    printf("Result: %d\n", sum);
    return sum > 100 ? 0 : 1;
}
