#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

// Function that modifies test expression via pointer
void modify_via_pointer(int *ptr) {
    // Pattern 1: Direct modification of test expression
    if (*ptr > 0) {
        *ptr = -1;  // Modifies the test expression directly
        *ptr += 2;  // Additional modification
    }
}

// Function with volatile test variable
void volatile_modification(volatile int *v) {
    // Pattern 2: Volatile variable modification
    if (*v > 10) {
        *v = 5;     // Modifies volatile test expression
        int temp = *v * 2;
        *v = temp;  // Second modification
    }
}

// Function with array aliasing
void array_aliasing(int *arr, int i, int j) {
    // Pattern 3: Potential aliasing through array indices
    if (arr[i] > 0) {
        arr[j] = 0;  // Could modify test expression if i == j
        arr[i] = arr[i] * 2;  // Direct modification
    }
}

// Function with mixed data types
void mixed_types(float *fptr, int *iptr) {
    // Pattern 4: Mixed types with implicit conversions
    if ((float)*iptr > 0.5f) {
        *iptr = (int)(*fptr * 2.0f);  // Modifies integer used in float conversion
        *iptr += 1;  // Additional modification
    }
}

// Function with loop-dependent condition
void loop_modification(int *arr, int n, int threshold) {
    // Pattern 5: Loop with condition-dependent modification
    for (int i = 0; i < n; i++) {
        if (arr[i] > threshold) {
            arr[i] = 0;  // Modifies test expression
            arr[i] = arr[i] + 1;  // Second instruction in then block
        }
    }
}

// Function with atomic operations (prevents certain optimizations)
void atomic_modification(_Atomic int *a) {
    // Pattern 6: Atomic variable in condition
    if (atomic_load(a) > 0) {
        atomic_store(a, 10);  // Modifies atomic test expression
        atomic_fetch_add(a, 1);  // Additional atomic modification
    }
}

// Complex scenario with multiple potential modifications
void complex_scenario(int *data, volatile int *flag, int idx) {
    int *alias = data + idx;
    
    // Multiple conditions with modifications
    if (data[idx] > 100) {
        *alias = 50;  // Potential alias modification
        data[idx] = data[idx] / 2;  // Direct modification
        *flag = 1;  // Volatile modification
    }
    
    // Nested if with different test
    if (*flag > 0) {
        data[idx] = -data[idx];  // Modifies previously used variable
        *flag = 0;  // Modifies test expression
    }
}

int main() {
    // Initialize test data
    volatile int v1 = 15;
    volatile int v2 = -5;
    int arr1[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int arr2[10] = {20, 15, 10, 5, 0, -5, -10, -15, -20, -25};
    float farr[5] = {0.1f, 0.5f, 1.0f, 1.5f, 2.0f};
    _Atomic int atomic_val = 5;
    
    int sum = 0;
    
    // Test 1: Direct pointer modification
    int x = 10;
    modify_via_pointer(&x);
    sum += x;
    
    // Test 2: Volatile modification
    volatile_modification(&v1);
    sum += v1;
    
    // Test 3: Array aliasing (with potential i == j)
    array_aliasing(arr1, 3, 3);  // i == j case
    array_aliasing(arr1, 2, 4);  // i != j case
    sum += arr1[3] + arr1[2];
    
    // Test 4: Mixed types
    int y = 1;
    mixed_types(&farr[2], &y);
    sum += y;
    
    // Test 5: Loop modification
    loop_modification(arr2, 10, 0);
    for (int i = 0; i < 10; i++) {
        sum += arr2[i];
    }
    
    // Test 6: Atomic modification
    atomic_modification(&atomic_val);
    sum += atomic_val;
    
    // Test 7: Complex scenario
    complex_scenario(arr1, &v2, 5);
    sum += arr1[5] + v2;
    
    // Additional small blocks for if-conversion candidates
    // Short then block with 2-3 instructions
    int z = 20;
    if (z > 15) {
        z = z - 5;  // First modification
        z = z * 2;  // Second modification
        // Third implicit: use in sum
    }
    sum += z;
    
    // Another candidate with memory aliasing
    int *p1 = &arr1[0];
    int *p2 = &arr1[0];  // Same location
    if (*p1 > 0) {
        *p2 = 0;  // Modifies test expression via alias
        *p1 = *p1 + 1;  // Additional modification
    }
    sum += arr1[0];
    
    // Prevent dead code elimination
    printf("Result: %d\n", sum);
    
    // Use results to prevent optimization
    if (sum > 1000) {
        return 1;
    }
    
    return 0;
}
