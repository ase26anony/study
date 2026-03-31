#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

// Function with pointer-based modification
void process_int(int *p) {
    // Pattern 1: Direct modification of test variable
    if (*p > 0) {
        *p = -1;  // Modifies the test expression
        *p += 2;  // Additional modification
    }
}

// Function with array aliasing
void process_array(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        // Pattern 2: Loop-dependent condition with modification
        if (arr[i] > 10) {
            arr[i] = 0;  // Modifies test expression
            arr[i] += 5; // Second instruction in then block
        }
    }
}

// Function with volatile and mixed types
void process_mixed(volatile int *v, float *f) {
    // Pattern 3: Volatile test with modification
    if (*v > 100) {
        *v = 50;  // Modifies volatile test expression
        *f = (float)(*v) * 1.5f;  // Mixed type operation
    }
}

// Function with potential aliasing through pointers
void process_aliasing(int *a, int *b, int idx) {
    // Pattern 4: Aliasing test
    if (a[idx] > b[0]) {
        a[idx] = b[0];  // Could modify test expression if aliased
        b[0] = a[idx] + 1;  // Additional modification
    }
}

// Function with atomic operations
void process_atomic(_Atomic int *atom) {
    // Pattern 5: Atomic test with modification
    int val = atomic_load(atom);
    if (val > 0) {
        atomic_store(atom, 0);  // Modifies test expression
        atomic_store(atom, val / 2);  // Second atomic operation
    }
}

// Function with complex test expression
void process_complex(int x, int y, float threshold) {
    // Pattern 6: Mixed data types and implicit conversions
    if ((float)x / (y + 1) > threshold) {
        x = y * 2;  // Modifies variable used in test
        y = x + 1;  // Additional modification
        // Use volatile to prevent optimization
        volatile int dummy = x + y;
        (void)dummy;
    }
}

// Main function creating various if-conversion candidates
int main() {
    // Initialize test data
    volatile int v1 = 150;
    volatile int v2 = -5;
    int arr[10] = {5, 15, 3, 20, 8, 25, 10, 30, 1, 18};
    float farr[5] = {1.5f, 2.5f, 3.5f, 4.5f, 5.5f};
    _Atomic int atom = 42;
    
    int x = 10, y = 3;
    float threshold = 2.0f;
    
    // Test 1: Direct pointer modification
    int test1 = 25;
    process_int(&test1);
    
    // Test 2: Array processing with loop
    process_array(arr, 10);
    
    // Test 3: Volatile and mixed types
    process_mixed(&v1, &farr[0]);
    
    // Test 4: Potential aliasing
    int a[5] = {1, 2, 3, 4, 5};
    int b[5] = {5, 4, 3, 2, 1};
    process_aliasing(a, b, 2);
    
    // Test 5: Atomic operations
    process_atomic(&atom);
    
    // Test 6: Complex test expression
    process_complex(x, y, threshold);
    
    // Additional inline patterns to increase coverage
    
    // Pattern 7: Short then block with multiple modifications
    int counter = 0;
    for (int i = 0; i < 10; i++) {
        if (counter < 5) {
            counter++;  // First modification
            counter *= 2;  // Second modification
            counter--;  // Third modification
        }
    }
    
    // Pattern 8: Nested if with modification
    int val = 100;
    if (val > 50) {
        if (val < 200) {
            val = 75;  // Modifies outer test variable
            val += 10; // Additional instruction
        }
    }
    
    // Pattern 9: Modification through different pointer
    int data = 42;
    int *ptr1 = &data;
    int *ptr2 = &data;
    if (*ptr1 > 0) {
        *ptr2 = 0;  // Aliases with test expression
        *ptr1 = *ptr2 + 1;  // Additional modification
    }
    
    // Pattern 10: Mixed operations in then block
    float fval = 3.14f;
    int ival = 10;
    if (ival > 5) {
        ival = (int)fval;  // Type conversion
        fval = (float)ival * 2.0f;  // Float operation
        ival = (int)fval;  // Back to int
    }
    
    // Use results to prevent dead code elimination
    int sum = test1 + arr[0] + arr[5] + v1 + a[2] + (int)atom + counter + val + data + ival;
    printf("Result: %d\n", sum);
    
    return sum > 0 ? 0 : 1;
}
