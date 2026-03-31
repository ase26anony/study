#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Function that modifies test expression via pointer */
void modify_via_pointer(int *ptr) {
    if (*ptr > 0) {           // Test expression: *ptr
        *ptr = -1;            // Modifies test expression
        *ptr += 2;            // Additional modification
    }
}

/* Function with volatile test variable */
void volatile_modification(volatile int *v) {
    if (*v > 10) {            // Test expression: *v (volatile)
        *v = 5;               // Modifies volatile test expression
        int temp = *v * 2;    // Additional operation
        *v = temp;            // Another modification
    }
}

/* Function with array aliasing possibilities */
void array_aliasing(int arr[], int i, int j) {
    if (arr[i] > 0) {         // Test expression: arr[i]
        arr[j] = 0;           // Potentially modifies test expression if i == j
        arr[i] = arr[i] * 2;  // Definitely modifies test expression
    }
}

/* Function with mixed data types */
void mixed_types(float threshold) {
    volatile int counter = 0;
    float f = 3.14f;
    int x = 5;
    
    if ((float)x > threshold) {  // Test involves conversion
        x = (int)(f * 2.0f);     // Modifies x used in test
        counter = x + 1;         // Modifies volatile
    }
}

/* Function with loop-dependent condition */
void loop_with_side_effects(int data[], int n, int threshold) {
    for (int i = 0; i < n; i++) {
        if (data[i] > threshold) {  // Test expression: data[i]
            data[i] = 0;            // Modifies test expression
            data[i] += i;           // Additional modification
        }
    }
}

/* Function creating complex test expression */
void complex_test(int *a, int *b) {
    int sum = *a + *b;
    if (sum > 100) {           // Test expression: sum
        *a = 50;               // Modifies component of test expression
        sum = *a + *b;         // Recalculates (but original test expr modified)
    }
}

/* Main function with varied if-conversion candidates */
int main() {
    // Setup test data
    volatile int v1 = 15;
    volatile int v2 = -5;
    int arr1[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int arr2[10] = {20, 15, 10, 5, 0, -5, -10, -15, -20, -25};
    int *ptr1 = &arr1[0];
    int *ptr2 = &arr2[5];
    
    // Test 1: Pointer-based modification
    modify_via_pointer(&arr1[3]);
    modify_via_pointer(ptr2);
    
    // Test 2: Volatile modification
    volatile_modification(&v1);
    volatile_modification(&v2);
    
    // Test 3: Array aliasing (potential i == j)
    array_aliasing(arr1, 2, 2);  // i == j case
    array_aliasing(arr1, 0, 1);  // i != j case
    
    // Test 4: Mixed data types
    mixed_types(2.5f);
    
    // Test 5: Loop with side effects
    loop_with_side_effects(arr2, 10, 0);
    
    // Test 6: Complex test expression
    int x = 60, y = 50;
    complex_test(&x, &y);
    
    // Test 7: Inline block with multiple modifications
    int counter = 0;
    for (int i = 0; i < 5; i++) {
        if (counter < 3) {          // Test expression: counter
            counter++;              // Modifies test expression
            counter *= 2;           // Additional modification
        }
    }
    
    // Test 8: Nested condition with modification
    int a = 10, b = 20;
    if (a > 5) {
        if (b > 10) {               // Test expression: b
            b = a;                  // Modifies test expression
            a = b + 1;              // Additional operation
        }
    }
    
    // Test 9: Struct member modification
    struct Data {
        int value;
        volatile int status;
    } data = {25, 0};
    
    if (data.value > 20) {          // Test expression: data.value
        data.value = 15;            // Modifies test expression
        data.status = 1;            // Modifies volatile member
    }
    
    // Test 10: Function pointer-like behavior
    int (*modifier)(int) = NULL;
    int base = 100;
    
    if (base > 50) {                // Test expression: base
        base = 25;                  // Modifies test expression
        int temp = base * 3;        // Additional computation
        base = temp / 2;            // Another modification
    }
    
    // Use results to prevent dead code elimination
    int sum = v1 + v2 + arr1[0] + arr1[2] + arr1[3] + arr2[0] + x + y + counter + a + b + data.value + base;
    
    printf("Result: %d\n", sum);
    return sum > 0 ? 0 : 1;
}
