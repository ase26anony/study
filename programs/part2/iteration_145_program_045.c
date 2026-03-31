#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

// Function prototypes
void process_direct(int *p);
void process_aliased(int *p, int *q);
int process_volatile(volatile int *vp);
void process_array(int arr[], int n, int threshold);
float process_mixed_types(int x, float y);

// Global volatile variable
volatile int global_volatile = 0;

int main() {
    int result = 0;
    
    // 1. Direct modification of test variable
    int x = 10;
    if (x > 5) {
        x = x * 2;      // Modifies test variable
        x = x - 3;      // Another modification
        // Third instruction to ensure loop iteration
        x = x + 1;
    }
    result += x;
    
    // 2. Volatile variable test
    volatile int v = 20;
    if (v > 15) {
        v = 25;         // Modifies volatile test variable
        v = v * 2;      // Second modification
    }
    result += v;
    
    // 3. Pointer-based modification with potential aliasing
    int a = 30, b = 40;
    int *ptr1 = &a;
    int *ptr2 = &a;     // Same address - definite aliasing
    
    if (*ptr1 > 20) {
        *ptr2 = 35;     // Modifies through aliased pointer
        *ptr1 = *ptr1 + 5;  // Second modification
    }
    result += a;
    
    // 4. Array access with index calculations
    int arr[10];
    for (int i = 0; i < 10; i++) {
        arr[i] = i * 10;
    }
    
    for (int i = 0; i < 10; i++) {
        if (arr[i] > 25) {
            arr[i] = 0;     // Modifies test expression memory
            arr[i] = -1;    // Second modification
        }
    }
    
    // Sum array to prevent elimination
    for (int i = 0; i < 10; i++) {
        result += arr[i];
    }
    
    // 5. Mixed data types with implicit conversions
    int int_val = 50;
    float float_val = 3.14f;
    if ((float)int_val > float_val) {
        int_val = (int)(float_val * 20.0f);  // Modifies test variable
        int_val = int_val / 2;               // Second modification
    }
    result += int_val;
    
    // 6. Function calls with pointer arguments
    int local_var = 60;
    process_direct(&local_var);
    result += local_var;
    
    // 7. Complex aliasing scenario
    int var1 = 70, var2 = 80;
    process_aliased(&var1, &var1);  // Same variable
    result += var1;
    
    // 8. Volatile pointer in function
    volatile int vlocal = 90;
    result += process_volatile(&vlocal);
    
    // 9. Array processing function
    int data[5] = {100, 200, 300, 400, 500};
    process_array(data, 5, 250);
    for (int i = 0; i < 5; i++) {
        result += data[i];
    }
    
    // 10. Mixed type function
    result += (int)process_mixed_types(150, 2.5f);
    
    // 11. Loop with side effects in condition
    int counter = 0;
    for (int i = 0; i < 5; i++) {
        // Test expression depends on loop variable
        if (data[i] > 200 + i) {
            data[i] = data[i] - 100;  // Modifies test memory
            counter++;
        }
    }
    result += counter;
    
    // 12. Nested if with multiple modifications
    int n = 1000;
    if (n > 500) {
        n = n >> 1;     // First modification
        if (n > 250) {
            n = n * 3;  // Second modification
            n = n + 7;  // Third modification
        }
    }
    result += n;
    
    // 13. Atomic operations (prevent certain optimizations)
    _Atomic int atomic_val = 2000;
    int expected = 2000;
    if (atomic_val > 1500) {
        // This creates a more complex modification pattern
        atomic_val = 1800;
    }
    result += atomic_val;
    
    // Use result to prevent dead code elimination
    printf("Result: %d\n", result);
    
    return result > 0 ? 0 : 1;
}

// Function that modifies through pointer
void process_direct(int *p) {
    if (*p > 55) {
        *p = *p / 2;    // Modifies test expression
        *p = *p * 3;    // Second modification
    }
}

// Function with potential aliasing
void process_aliased(int *p, int *q) {
    // p and q may alias
    if (*p > 65) {
        *q = 75;        // May modify test expression if aliased
        *p = *p + 10;   // Second modification
    }
}

// Function with volatile pointer
int process_volatile(volatile int *vp) {
    int temp = 0;
    if (*vp > 85) {
        *vp = 95;       // Modifies volatile test expression
        *vp = *vp - 5;  // Second modification
        temp = 1;
    }
    return temp;
}

// Array processing with threshold
void process_array(int arr[], int n, int threshold) {
    for (int i = 0; i < n; i++) {
        if (arr[i] > threshold) {
            arr[i] = threshold;     // Modifies test memory
            arr[i] = arr[i] - 50;   // Second modification
        }
    }
}

// Mixed data types
float process_mixed_types(int x, float y) {
    float result = 0.0f;
    if ((float)x > y) {
        x = (int)(y * 10.0f);   // Modifies test variable
        x = x % 7;              // Second modification
        result = (float)x;
    }
    return result;
}
