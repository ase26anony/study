#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

// Function with pointer-based modification
void process_with_modification(int *p, int *q) {
    // Pattern 1: Direct modification of test variable
    if (*p > 0) {
        *p = -1;  // Modifies the test expression directly
        *q = *q + 1;  // Additional instruction
    }
}

// Function with array aliasing
void process_array(int arr[], int n, int threshold) {
    // Pattern 2: Loop-dependent condition with modification
    for (int i = 0; i < n; i++) {
        if (arr[i] > threshold) {
            arr[i] = 0;  // Modifies the test expression
            arr[i] = arr[i] + 1;  // Second modification
        }
    }
}

// Function with mixed data types
void mixed_types(volatile float *fptr, int *iptr) {
    // Pattern 3: Mixed types with implicit conversion
    if (*fptr > 0.5f) {
        *iptr = (int)(*fptr * 2.0f);  // Modifies through pointer
        *fptr = *fptr * 0.5f;  // Second modification
    }
}

// Function with potential aliasing
void aliasing_test(int *a, int *b, int *c) {
    // Pattern 4: Multiple pointers that might alias
    if (*a > *b) {
        *a = *c;  // Modifies test variable
        *b = *b + 1;  // Additional instruction
        *c = *a * 2;  // Third instruction
    }
}

// Function with volatile and atomic operations
void volatile_test(volatile int *vptr, atomic_int *aptr) {
    // Pattern 5: Volatile prevents optimization
    if (*vptr > 100) {
        *vptr = 50;  // Modifies volatile test variable
        atomic_store(aptr, *vptr);  // Atomic operation
    }
}

// Complex condition with side effects
int complex_condition(int *x, int *y, int *z) {
    // Pattern 6: Multiple test expressions
    int result = 0;
    if (*x > 0 && *y < 10) {
        *x = *x - 1;  // Modifies first test variable
        *y = *y + 1;  // Modifies second test variable
        *z = *x + *y;  // Third instruction
        result = 1;
    }
    return result;
}

// Nested if-statements
void nested_ifs(int *arr, int size) {
    for (int i = 0; i < size; i++) {
        if (arr[i] > 0) {
            if (arr[i] < 100) {
                arr[i] = arr[i] * 2;  // Modifies test variable
                arr[i] = arr[i] - 1;  // Second modification
            }
        }
    }
}

// Main function with varied test cases
int main() {
    // Initialize test data
    volatile int v1 = 42;
    volatile float vf = 1.5f;
    int arr1[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int arr2[5] = {100, 200, 300, 400, 500};
    atomic_int atomic_val = ATOMIC_VAR_INIT(0);
    int x = 5, y = 3, z = 0;
    
    // Test 1: Direct modification with pointers
    process_with_modification(&arr1[0], &arr1[1]);
    
    // Test 2: Array processing with loop
    process_array(arr1, 10, 5);
    
    // Test 3: Mixed data types with volatile
    mixed_types(&vf, &arr1[2]);
    
    // Test 4: Aliasing test
    aliasing_test(&x, &y, &z);
    
    // Test 5: Volatile and atomic
    volatile_test(&v1, &atomic_val);
    
    // Test 6: Complex condition
    int res = complex_condition(&arr1[3], &arr1[4], &arr1[5]);
    
    // Test 7: Nested ifs
    nested_ifs(arr2, 5);
    
    // Test 8: Inline block with multiple modifications
    int a = 10, b = 20, c = 30;
    if (a > b) {
        a = b;      // First modification
        b = c;      // Second
        c = a + b;  // Third - enough for loop iteration
    }
    
    // Test 9: Memory aliasing through different indices
    int buffer[10] = {0};
    for (int i = 0; i < 10; i++) {
        buffer[i] = i * 10;
    }
    
    // This creates potential aliasing
    int *p1 = &buffer[2];
    int *p2 = &buffer[2];  // Same location!
    if (*p1 > 15) {
        *p2 = 0;  // Modifies the test expression through alias
        buffer[3] = *p1 + 1;
    }
    
    // Test 10: Short but with multiple arithmetic ops
    int counter = 0;
    volatile int trigger = 1;
    if (trigger > 0) {
        counter = counter + 1;  // Modification
        counter = counter * 2;  // Second
        trigger = counter;      // Third - modifies test variable
    }
    
    // Use results to prevent dead code elimination
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += arr1[i] + arr2[i % 5];
    }
    sum += v1 + (int)vf + x + y + z + res + a + b + c + counter;
    sum += atomic_load(&atomic_val);
    
    printf("Result: %d\n", sum);
    return sum > 0 ? 0 : 1;
}
