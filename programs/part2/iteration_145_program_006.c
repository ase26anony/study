#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

// Function prototypes
void process_direct(int *p);
void process_aliased(int *p, int *q);
void process_volatile(volatile int *p);
int process_array(int *arr, int n, int threshold);
float process_mixed_types(int x, float y);

// Global volatile variable to prevent optimization
volatile int global_counter = 0;

// Function with pointer modification in then-block
void process_direct(int *p) {
    // Pattern 1: Direct modification of test variable
    if (*p > 0) {
        *p = *p * 2;      // First modification
        *p = *p - 1;      // Second modification
        // Total: 2 non-label instructions that modify test expression
    }
}

// Function with potential aliasing
void process_aliased(int *p, int *q) {
    // Pattern 2: Aliasing scenario
    // The compiler must check if p and q could alias
    if (*p > 10) {
        *q = 20;          // Could modify *p if p == q
        *p = *p + *q;     // Definitely modifies *p
        // Two instructions that could modify test expression
    }
}

// Function with volatile access
void process_volatile(volatile int *p) {
    // Pattern 3: Volatile prevents optimization
    if (*p > 5) {
        *p = 10;          // First volatile write
        *p = *p + 5;      // Second volatile write
        // Volatile ensures instructions aren't optimized away
    }
}

// Function with loop-dependent condition
int process_array(int *arr, int n, int threshold) {
    int sum = 0;
    
    // Pattern 4: Loop with side-effect in condition
    for (int i = 0; i < n; i++) {
        // Test expression is arr[i] which gets modified in then-block
        if (arr[i] > threshold) {
            arr[i] = threshold;      // First modification
            arr[i] = arr[i] * 2;     // Second modification
            sum += arr[i];
        }
    }
    return sum;
}

// Function with mixed data types
float process_mixed_types(int x, float y) {
    float result = y;
    
    // Pattern 5: Mixed types with implicit conversions
    if ((float)x > 0.5f) {
        x = (int)(y * 2.0f);        // First modification (implicit conversion)
        result = (float)x + y;      // Second operation
        x = x / 2;                  // Third modification
    }
    
    return result;
}

// Function with atomic operations (prevents certain optimizations)
void process_atomic(_Atomic int *a) {
    // Pattern 6: Atomic operations create memory barriers
    int val = atomic_load(a);
    if (val > 100) {
        atomic_store(a, 50);        // First atomic modification
        atomic_fetch_add(a, 10);    // Second atomic modification
    }
}

// Main function with varied test cases
int main() {
    int a = 15, b = 20, c = 5;
    volatile int v = 8;
    int arr[10] = {1, 5, 10, 15, 20, 25, 30, 35, 40, 45};
    _Atomic int atomic_val = 150;
    
    // Test 1: Direct modification with multiple instructions
    process_direct(&a);
    
    // Test 2: Aliasing scenario (p and q point to same location)
    process_aliased(&b, &b);  // Self-aliasing
    
    // Test 3: Volatile access
    process_volatile(&v);
    
    // Test 4: Loop with array modification
    int sum = process_array(arr, 10, 20);
    
    // Test 5: Mixed data types
    float mixed_result = process_mixed_types(c, 3.14f);
    
    // Test 6: Atomic operations
    process_atomic(&atomic_val);
    
    // Test 7: Inline block with short then-block (ideal for if-conversion)
    int x = 10, y = 20;
    if (x > y) {
        x = y + 5;      // First instruction
        y = x * 2;      // Second instruction
    }
    
    // Test 8: Nested condition with modification
    int *ptr1 = &a, *ptr2 = &b;
    if (ptr1 != NULL && *ptr1 > 0) {
        *ptr1 = 0;      // Modifies test expression
        *ptr2 = *ptr1;  // Second modification
    }
    
    // Test 9: Complex test expression with modification
    int array[5] = {1, 2, 3, 4, 5};
    int index = 2;
    if (array[index] + array[index+1] > 5) {
        array[index] = 0;           // Could affect test expression
        array[index+1] = array[index] * 2;  // Second modification
    }
    
    // Test 10: Multiple conditions with shared variable
    int counter = 0;
    for (int i = 0; i < 5; i++) {
        if (counter < 3) {
            counter++;              // Modifies condition variable
            arr[i] = counter * 10;  // Second instruction
        }
    }
    
    // Use results to prevent dead code elimination
    printf("Results: a=%d, b=%d, v=%d, sum=%d, mixed=%.2f, atomic=%d, x=%d, y=%d\n",
           a, b, v, sum, mixed_result, atomic_val, x, y);
    
    // Additional print to use array values
    printf("Array: ");
    for (int i = 0; i < 10; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
    
    return 0;
}
