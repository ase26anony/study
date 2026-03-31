#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

// Function that modifies test expression via pointer
void modify_via_pointer(int *ptr) {
    // Pattern 1: Direct modification of test expression
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
        *v *= 2;    // Second modification
    }
}

// Function with array and potential aliasing
void array_modification(int arr[], int n) {
    // Pattern 3: Loop-dependent condition with modification
    for (int i = 0; i < n; i++) {
        if (arr[i] > 0) {
            arr[i] = 0;     // Modifies test expression
            arr[i] = -arr[i]; // Second instruction
        }
    }
}

// Function with mixed data types
void mixed_types(float *fptr, int *iptr) {
    // Pattern 4: Implicit conversions
    if ((int)(*fptr) > 10) {
        *iptr = (int)(*fptr * 2.0f);  // Modifies related variable
        *iptr += 1;                    // Additional instruction
    }
}

// Function with pointer aliasing
void aliasing_modification(int *a, int *b) {
    // Pattern 5: Potential aliasing
    if (*a > *b) {
        *a = *b;        // Could modify test expression if a == b
        *b = *a + 1;    // Additional modification
    }
}

// Complex test with multiple modifications
void complex_test(int *x, int *y) {
    // Multiple instructions in then block
    if (*x > *y) {
        *x = *y - 1;    // First modification
        *y = *x * 2;    // Second modification
        *x = (*x + *y) / 2; // Third modification
    }
}

// Main function with varied test cases
int main() {
    // Test case 1: Direct modification
    int test1 = 5;
    modify_via_pointer(&test1);
    
    // Test case 2: Volatile variable
    volatile int volatile_var = 20;
    volatile_modification(&volatile_var);
    
    // Test case 3: Array modification
    int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = i - 5;
    }
    array_modification(arr, 10);
    
    // Test case 4: Mixed types
    float fval = 15.5f;
    int ival = 0;
    mixed_types(&fval, &ival);
    
    // Test case 5: Aliasing test
    int same_var = 8;
    aliasing_modification(&same_var, &same_var);
    
    // Test case 6: Complex modification
    int x = 10, y = 5;
    complex_test(&x, &y);
    
    // Test case 7: Nested conditions
    int a = 3, b = 7, c = 5;
    if (a > b) {
        a = b + c;      // Modifies a
        b = a * 2;      // Modifies b
    } else if (b > c) {
        b = c - a;      // Modifies b
        c = b * 3;      // Modifies c
    }
    
    // Test case 8: Loop with break condition modification
    int counter = 0;
    int values[5] = {10, -5, 20, -10, 15};
    for (int i = 0; i < 5; i++) {
        if (values[i] > 0) {
            values[i] = 0;          // Modifies test expression
            counter++;              // Additional instruction
        }
    }
    
    // Test case 9: Switch-like if-else chain
    int mode = 2;
    int result = 0;
    if (mode == 1) {
        result = 100;
        mode = 0;       // Modifies condition variable
    } else if (mode == 2) {
        result = 200;
        mode = 1;       // Modifies condition variable
        result += mode; // Additional instruction
    }
    
    // Test case 10: Function pointer modification
    int (*func_ptr)(int) = NULL;
    int target = 42;
    
    if (target > 0) {
        // These don't modify target directly but create complex RTL
        int temp = target * 2;
        target = temp / 3;  // Modifies test expression
    }
    
    // Use results to prevent dead code elimination
    int sum = test1 + volatile_var + arr[0] + ival + same_var + x + y + 
              a + b + c + counter + result + target;
    
    printf("Result: %d\n", sum);
    
    return sum > 0 ? 0 : 1;
}
