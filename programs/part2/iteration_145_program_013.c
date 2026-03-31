#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

// Function with pointer-based modification that aliases with test expression
void process_with_alias(int *p, int *q) {
    // This creates potential aliasing - p and q might point to same location
    if (*p > 0) {
        *q = *p * 2;  // Could modify *p if q aliases p
    }
}

// Function with direct modification of test variable
void modify_test_in_then(int *counter) {
    volatile int v = *counter;
    
    // Test expression uses v, then block modifies it
    if (v > 100) {
        v = v / 2;      // First modification
        v = v + 1;      // Second modification
        *counter = v;   // Third operation
    }
}

// Function with array access and potential self-modification
void array_self_modify(int arr[], int n) {
    for (int i = 0; i < n; i++) {
        // Test expression uses arr[i], then block modifies it
        if (arr[i] > 50) {
            arr[i] = 0;         // Direct modification of test expression
            arr[i] += 10;       // Another modification
        }
    }
}

// Function with mixed data types and implicit conversions
void mixed_types_modify(float threshold) {
    volatile int x = 100;
    float y = 3.14f;
    
    // Complex test expression with implicit conversion
    if ((float)x > threshold) {
        x = (int)(y * 20.0f);   // Modifies x used in test expression
        x = x % 7;              // Another modification
    }
}

// Function with pointer arithmetic and aliasing
void pointer_aliasing_modify(int *base, int offset1, int offset2) {
    // Test uses one memory location, modification might affect another
    if (base[offset1] > 0) {
        base[offset2] = -1;     // Might alias if offset1 == offset2
        base[offset1] = base[offset1] * 2;  // Direct modification
    }
}

// Main function creating various if-conversion scenarios
int main() {
    volatile int test_var = 42;
    int array[100];
    int *ptr1, *ptr2;
    int sum = 0;
    
    // Initialize array with random values
    for (int i = 0; i < 100; i++) {
        array[i] = rand() % 100;
    }
    
    // Scenario 1: Direct modification in then block
    if (test_var > 0) {
        test_var = test_var * 2;    // First modification
        test_var = test_var - 10;   // Second modification
        test_var = test_var / 3;    // Third modification
    }
    
    // Scenario 2: Array self-modification in loop
    for (int i = 0; i < 50; i++) {
        if (array[i] > 50) {
            array[i] = 0;           // Modifies test expression
            array[i] = array[i] + 5; // Another modification
        }
    }
    
    // Scenario 3: Pointer aliasing
    ptr1 = &array[10];
    ptr2 = &array[10];  // Same location - definite aliasing
    if (*ptr1 > 25) {
        *ptr2 = 99;     // Modifies *ptr1 through alias
        *ptr1 = *ptr1 + 1; // Direct modification
    }
    
    // Scenario 4: Mixed types with volatile
    volatile float f = 2.5f;
    int x = 75;
    if ((float)x > f) {
        x = (int)(f * 30.0f);   // Modifies x used in test
        x = x & 0xFF;           // Another modification
    }
    
    // Scenario 5: Function call with modification
    modify_test_in_then(&array[20]);
    
    // Scenario 6: Potential aliasing through function
    process_with_alias(&array[30], &array[30]);  // Same location
    
    // Scenario 7: Complex loop with multiple modifications
    for (int i = 0; i < 100; i++) {
        if (array[i] > 75) {
            array[i] = array[i] * 2;  // Modification 1
            array[i] = array[i] % 50; // Modification 2
            if (i % 2 == 0) {
                array[i] = -array[i]; // Modification 3 (nested)
            }
        }
    }
    
    // Scenario 8: Pointer arithmetic with potential equality
    pointer_aliasing_modify(array, 40, 40);  // Same offset
    
    // Scenario 9: Mixed types function
    mixed_types_modify(10.5f);
    
    // Use results to prevent dead code elimination
    for (int i = 0; i < 100; i++) {
        sum += array[i];
    }
    sum += test_var + x;
    
    printf("Result: %d\n", sum);
    
    return sum > 0 ? 0 : 1;
}
