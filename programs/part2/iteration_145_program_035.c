#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Function that modifies test expression via pointer */
void modify_condition(int *ptr) {
    if (*ptr > 0) {               // Test expression: *ptr
        *ptr = -1;                // Modifies test expression
        *ptr += 2;                // Additional modification
    }
}

/* Function with volatile test variable */
void volatile_test(void) {
    volatile int v = 10;
    volatile int *vp = &v;
    
    if (v > 5) {                  // Test expression: v (volatile)
        v = 20;                   // Modifies test expression
        *vp = 30;                 // Another modification via pointer
    }
}

/* Function with array aliasing */
void array_aliasing(int *arr, int n) {
    for (int i = 0; i < n; i++) {
        if (arr[i] > 0) {         // Test expression: arr[i]
            arr[i] = 0;           // Direct modification
            if (i > 0) {
                arr[i-1] = -1;    // Potential aliasing with arr[i]
            }
        }
    }
}

/* Function with mixed data types */
void mixed_types(void) {
    int x = 5;
    float y = 3.14f;
    
    if ((float)x > y) {           // Test expression involves x
        x = (int)(y * 2);         // Modifies x
        x += 1;                   // Additional modification
    }
}

/* Function with pointer arithmetic */
void pointer_arithmetic(int *base, int offset) {
    int *ptr = base + offset;
    
    if (*ptr > 100) {             // Test expression: *ptr
        *ptr = 50;                // Modifies test expression
        *(ptr + 1) = 60;          // Adjacent memory (no alias)
    }
}

/* Complex scenario with multiple modifications */
void complex_modification(int *a, int *b) {
    // Create potential aliasing
    if (*a > *b) {                // Test expression: *a and *b
        *a = *b;                  // Modifies *a
        *b = *a + 1;              // Modifies *b
        // This creates a situation where modified_in_p needs to check
        // if either *a or *b is modified
    }
}

/* Loop with side effects in condition */
void loop_with_side_effects(int *arr, int n) {
    int i = 0;
    while (i < n) {
        // The test expression arr[i] is modified in the then block
        if (arr[i] > 0) {
            arr[i] = arr[i] * -1;  // Modifies test expression
            arr[i] += 10;          // Another modification
        }
        i++;
    }
}

/* Function with struct member access */
struct Data {
    int value;
    int *ptr;
};

void struct_test(struct Data *d) {
    if (d->value > 0) {           // Test expression: d->value
        d->value = 0;             // Modifies test expression
        *(d->ptr) = 1;            // Potential aliasing if d->ptr == &d->value
    }
}

/* Main function that exercises all patterns */
int main(void) {
    int array1[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int array2[10] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    volatile int volatile_var = 15;
    int sum = 0;
    
    // Test 1: Direct modification of test variable
    int x = 10;
    if (x > 5) {
        x = 20;      // Modifies test expression
        x = x + 5;   // Another modification
    }
    sum += x;
    
    // Test 2: Pointer-based modification
    modify_condition(&x);
    sum += x;
    
    // Test 3: Volatile variable
    volatile_test();
    
    // Test 4: Array aliasing
    array_aliasing(array1, 10);
    for (int i = 0; i < 10; i++) {
        sum += array1[i];
    }
    
    // Test 5: Mixed types
    mixed_types();
    
    // Test 6: Pointer arithmetic
    pointer_arithmetic(array2, 3);
    sum += array2[3];
    
    // Test 7: Complex modification with potential aliasing
    int a = 100, b = 50;
    complex_modification(&a, &b);
    sum += a + b;
    
    // Test 8: Loop with modifications
    int array3[5] = {5, -2, 8, -1, 3};
    loop_with_side_effects(array3, 5);
    for (int i = 0; i < 5; i++) {
        sum += array3[i];
    }
    
    // Test 9: Struct access
    struct Data data = {25, &data.value};  // Self-referential pointer
    struct_test(&data);
    sum += data.value;
    
    // Test 10: Multiple modifications in then block
    int y = 30;
    if (y > 20) {
        y = y / 2;    // Modifies test expression
        y = y * 3;    // Another modification
        y = y + 1;    // Third modification
    }
    sum += y;
    
    // Use the sum to prevent dead code elimination
    printf("Result: %d\n", sum);
    
    return sum > 0 ? 0 : 1;
}
