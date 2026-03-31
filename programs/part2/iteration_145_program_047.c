#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

// Function that modifies test expression via pointer
void modify_if_positive(int *ptr) {
    // Pattern 1: Direct modification of test variable
    if (*ptr > 0) {
        *ptr = -1;  // Modifies the test expression
        *ptr += 2;  // Additional modification
    }
}

// Function with volatile test variable
void volatile_modification(volatile int *v) {
    // Pattern 2: Volatile prevents optimization
    if (*v > 10) {
        *v = 5;     // Modifies volatile test expression
        int temp = *v * 2;
        *v = temp;  // Second modification
    }
}

// Function with array aliasing
void array_aliasing(int *arr, int i, int j) {
    // Pattern 3: Potential aliasing
    if (arr[i] > 0) {
        arr[j] = 0;  // May alias if i == j
        arr[i] = arr[i] * 2;  // Direct modification
    }
}

// Function with mixed data types
void mixed_types(float *farr, int *iarr, int idx) {
    // Pattern 4: Implicit conversions
    if ((float)iarr[idx] > 0.5f) {
        iarr[idx] = (int)(farr[idx] * 2.0f);  // Modifies test expression
        iarr[idx] += 1;  // Additional modification
    }
}

// Function with loop-dependent condition
void loop_modification(int *data, int n, int threshold) {
    // Pattern 5: Loop with side effects
    for (int i = 0; i < n; i++) {
        if (data[i] > threshold) {
            data[i] = 0;  // Modifies test expression
            data[i] = threshold - 1;  // Second modification
        }
    }
}

// Function with multiple short then-blocks
void multiple_conditions(int a, int b, int c, int *result) {
    // Several if-conversion candidates
    if (a > 0) {
        a = b + c;  // Modifies test variable
        a *= 2;     // Additional instruction
    }
    
    if (b > a) {    // b tested against modified a
        b = a - 1;  // Modifies test variable
    }
    
    if (c > 100) {
        c = 50;     // Modifies test variable
        c += b;     // Additional instruction
    }
    
    *result = a + b + c;
}

// Function with pointer arithmetic
void pointer_aliasing(int *base, int offset1, int offset2) {
    int *p1 = base + offset1;
    int *p2 = base + offset2;
    
    // Pattern: Test via p1, modify via p2 (potential aliasing)
    if (*p1 > 0) {
        *p2 = 0;        // May alias with p1
        *p1 = *p1 / 2;  // Direct modification
    }
}

int main() {
    // Initialize test data
    volatile int volatile_var = 15;
    int array1[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int array2[10] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    float farray[10] = {0.1f, 0.2f, 0.3f, 0.4f, 0.5f, 0.6f, 0.7f, 0.8f, 0.9f, 1.0f};
    
    int local_var = 5;
    int result = 0;
    
    // Test 1: Direct pointer modification
    modify_if_positive(&local_var);
    
    // Test 2: Volatile modification
    volatile_modification(&volatile_var);
    
    // Test 3: Array aliasing (with potential i == j)
    array_aliasing(array1, 3, 3);  // i == j case
    array_aliasing(array1, 2, 4);  // i != j case
    
    // Test 4: Mixed data types
    mixed_types(farray, array2, 5);
    
    // Test 5: Loop-based modification
    loop_modification(array1, 10, 5);
    
    // Test 6: Multiple conditions
    multiple_conditions(10, 20, 30, &result);
    
    // Test 7: Pointer arithmetic with potential aliasing
    pointer_aliasing(array1, 1, 1);  // Same location
    pointer_aliasing(array1, 0, 2);  // Different locations
    
    // Use results to prevent dead code elimination
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum += array1[i] + array2[i];
    }
    sum += local_var + volatile_var + result;
    
    printf("Result: %d\n", sum);
    
    return sum > 0 ? 0 : 1;
}
