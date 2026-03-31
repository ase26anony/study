#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Function 1: Direct modification of test variable in then block */
int test_direct_modification(volatile int x) {
    int sum = 0;
    // This should trigger modified_in_p check
    if (x > 0) {
        x = x * 2;      // Modifies test variable
        sum += x;       // Additional instruction
        x = x - 1;      // Another modification
    }
    return sum + x;
}

/* Function 2: Pointer-based modification with potential aliasing */
void process_with_aliasing(int *ptr1, int *ptr2, volatile int *vptr) {
    // Test expression uses memory location
    if (*ptr1 > 10) {
        *ptr2 = 20;     // Could alias with ptr1
        *vptr = 30;     // Volatile ensures no optimization
        *ptr1 = 15;     // Direct modification of test expression
    }
}

/* Function 3: Array access with loop-dependent condition */
int process_array(int *arr, int n, volatile int threshold) {
    int count = 0;
    for (int i = 0; i < n; i++) {
        // Loop creates multiple basic blocks
        if (arr[i] > threshold) {
            arr[i] = 0;         // Modifies test expression memory
            count++;
            arr[i] = -1;        // Second modification
        }
    }
    return count;
}

/* Function 4: Mixed data types and implicit conversions */
float test_mixed_types(int x, volatile float y) {
    float result = 0.0f;
    // Complex test expression
    if ((float)x > y) {
        x = (int)(y * 2.5f);    // Modifies variable used in test
        result = (float)x;
        x = x / 2;              // Another modification
    }
    return result;
}

/* Function 5: Nested conditions with side effects */
int nested_conditions(volatile int a, int b, int *ptr) {
    int temp = 0;
    
    if (a > 0) {
        if (b < 10) {
            a = b * 2;          // Modifies outer condition variable
            temp = a + b;
            *ptr = temp;        // Pointer modification
        }
        a = a - 5;              // Another modification
    }
    
    return temp;
}

/* Function 6: Structure with pointer members */
struct Data {
    int value;
    volatile int flag;
    int *ptr;
};

void process_struct(struct Data *d) {
    // Test expression uses structure member
    if (d->value > 100) {
        d->value = 50;          // Direct modification
        d->flag = 1;            // Volatile modification
        *(d->ptr) = d->value;   // Pointer modification
    }
}

/* Function 7: Complex expression with multiple variables */
int complex_test(int x, int y, volatile int z, int *arr) {
    int result = 0;
    
    // Test expression uses multiple variables
    if (x + y > z) {
        x = y * 2;              // Modifies x used in test
        arr[0] = x;             // Memory modification
        z = x + y;              // Modifies volatile used in test
        result = x + z;
    }
    
    return result;
}

int main() {
    volatile int v1 = 5, v2 = 10, v3 = 15;
    int arr1[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int arr2[5] = {20, 30, 40, 50, 60};
    int x = 25, y = 35, z = 45;
    int *ptr1 = &x;
    int *ptr2 = &y;
    volatile int *vptr = &v1;
    
    int total = 0;
    
    // Test 1: Direct modification
    total += test_direct_modification(v1);
    
    // Test 2: Aliasing with pointers
    process_with_aliasing(ptr1, ptr2, vptr);
    total += *ptr1 + *ptr2;
    
    // Test 3: Array processing with loop
    total += process_array(arr1, 10, v2);
    
    // Test 4: Mixed types
    float fresult = test_mixed_types(x, 3.14f);
    total += (int)fresult;
    
    // Test 5: Nested conditions
    total += nested_conditions(v3, 8, &z);
    
    // Test 6: Structure processing
    struct Data d = {150, 0, &z};
    process_struct(&d);
    total += d.value + d.flag;
    
    // Test 7: Complex test
    total += complex_test(10, 20, v1, arr2);
    
    // Additional loop with multiple conditions
    for (int i = 0; i < 5; i++) {
        volatile int local_var = i * 10;
        if (local_var > 20) {
            local_var = local_var / 2;      // Modification
            arr2[i] = local_var;            // Memory write
            local_var = local_var + 1;      // Another modification
        }
        total += arr2[i];
    }
    
    // Prevent dead code elimination
    printf("Total result: %d\n", total);
    
    // Return value based on all computations
    return (total > 1000) ? 0 : 1;
}
