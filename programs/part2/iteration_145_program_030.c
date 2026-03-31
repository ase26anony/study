#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

// Function prototypes
void process_pointer(int *p);
void process_array(int arr[], int n);
int conditional_modification(int x, int y);
float mixed_type_condition(int x, float y);

// Global volatile variables
volatile int global_volatile = 10;
volatile int *volatile volatile_ptr = NULL;

int main() {
    int result = 0;
    
    // 1. Basic test with volatile variable modification
    volatile int v = 5;
    if (v > 0) {
        v = 10;  // Modifies test expression
        v += 2;  // Additional modification
    }
    result += v;
    
    // 2. Pointer-based modification with potential aliasing
    int data[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int *ptr1 = &data[0];
    int *ptr2 = &data[0];  // Same location - definite aliasing
    
    if (*ptr1 > 0) {
        *ptr2 = 0;  // Modifies through aliased pointer
        *ptr1 = -1; // Another modification
    }
    result += data[0];
    
    // 3. Array access with index that might be the same
    for (int i = 0; i < 10; i++) {
        if (data[i] > 5) {
            data[i] = 0;  // Modifies the tested element
            data[i] += i; // Additional modification
        }
    }
    
    // 4. Mixed data types with implicit conversions
    int x = 7;
    float y = 3.5f;
    if ((float)x > y) {
        x = (int)(y * 2.0f);  // Modifies x used in test expression
        x += 1;               // Another modification
    }
    result += x;
    
    // 5. Function call with pointer modification
    int local_var = 15;
    process_pointer(&local_var);
    result += local_var;
    
    // 6. Process array with loop-dependent conditions
    process_array(data, 10);
    
    // 7. Complex condition with multiple modifications
    int a = 20, b = 30;
    if (a > 10 && b > 20) {
        a = b - 10;  // Modifies a used in condition
        b = a * 2;   // Modifies b used in condition
        a += 5;      // Another modification
    }
    result += a + b;
    
    // 8. Volatile pointer dereference
    int normal_var = 25;
    volatile_ptr = &normal_var;
    if (*volatile_ptr > 20) {
        *volatile_ptr = 5;  // Modifies through volatile pointer
        normal_var *= 2;    // Direct modification
    }
    result += normal_var;
    
    // 9. Nested conditions with modifications
    int n = 8;
    if (n > 5) {
        if (n < 10) {
            n = 12;  // Modifies outer condition variable
            n -= 3;  // Another modification
        }
    }
    result += n;
    
    // 10. Atomic operations (prevent certain optimizations)
    _Atomic int atomic_var = 42;
    if (atomic_var > 40) {
        atomic_var = 30;  // Atomic modification
        atomic_var += 2;  // Another atomic operation
    }
    result += atomic_var;
    
    // Prevent dead code elimination
    printf("Result: %d\n", result);
    
    // Additional test cases in different scopes
    {
        // Short then-block with multiple modifications
        int temp = 100;
        if (temp > 50) {
            temp = 75;
            temp += 10;
            temp -= 5;
        }
        result += temp;
    }
    
    {
        // Memory aliasing through different indices
        int arr[5] = {10, 20, 30, 40, 50};
        int idx1 = 2;
        int idx2 = 2;  // Same index
        
        if (arr[idx1] > 25) {
            arr[idx2] = 0;    // Modifies tested element
            arr[idx1] = 100;  // Another modification
        }
        result += arr[2];
    }
    
    // Final output to use all results
    printf("Final result: %d\n", result);
    
    return result > 0 ? 0 : 1;
}

// Function that modifies through pointer
void process_pointer(int *p) {
    if (*p > 0) {
        *p = -1;  // Modifies test expression
        *p *= 2;  // Additional modification
    }
}

// Function with loop and condition
void process_array(int arr[], int n) {
    for (int i = 0; i < n; i++) {
        if (arr[i] > 20) {
            arr[i] = 0;    // Modifies test expression
            arr[i] += i;   // Additional modification
        }
    }
}

// Function with mixed types
float mixed_type_condition(int x, float y) {
    if ((float)x > y) {
        x = (int)(y * 2.0f);  // Modifies x used in test
        x += 3;               // Another modification
    }
    return (float)x;
}
