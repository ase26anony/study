#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Function that modifies test expression via pointer */
void process_direct(int *p) {
    // Pattern 1: Direct modification of test expression
    if (*p > 0) {
        *p = -1;  // Modifies the memory tested in condition
        *p += 2;  // Additional modification
    }
}

/* Function with potential aliasing */
void process_aliased(int *arr, int i, int j) {
    // Pattern 2: Array access with possible aliasing
    if (arr[i] > 10) {
        arr[j] = 0;  // Could modify same location if i == j
        arr[i] = arr[i] * 2;  // Direct modification
    }
}

/* Function with volatile variable */
void process_volatile(volatile int *vp) {
    // Pattern 3: Volatile prevents optimization
    if (*vp > 100) {
        *vp = 50;
        *vp = *vp + 25;  // Multiple modifications
    }
}

/* Function with mixed data types */
void process_mixed(int x, float y) {
    // Pattern 4: Implicit conversions
    if ((float)x > y) {
        x = (int)(y * 3.0f);  // Modifies variable used in test
        x = x ^ 0xFF;  // Additional operation
    }
}

/* Function with loop-dependent condition */
void process_array(int *arr, int n, int threshold) {
    // Pattern 5: Loop with side-effect modification
    for (int i = 0; i < n; i++) {
        if (arr[i] > threshold) {
            arr[i] = threshold;  // Modifies test expression
            arr[i]--;  // Second modification
        }
    }
}

/* Function with atomic operations */
void process_atomic(_Atomic int *atom) {
    // Pattern 6: Atomic operations
    int val = atomic_load(atom);
    if (val > 0) {
        atomic_store(atom, val / 2);  // Modifies atomic variable
        atomic_fetch_add(atom, 1);  // Another modification
    }
}

/* Complex scenario with multiple pointers */
void process_complex(int *a, int *b, int *c) {
    // Pattern 7: Multiple potential aliases
    if (*a > *b) {
        *a = *b + *c;  // Modifies a which was tested
        *b = *a - *c;  // Modifies b which was also in test
        *c = 0;  // Third modification
    }
}

int main() {
    // Initialize test data
    volatile int v1 = 150;
    volatile int v2 = 75;
    int arr1[10] = {5, 15, 25, 35, 45, 55, 65, 75, 85, 95};
    int arr2[10] = {0};
    _Atomic int atomic_val = 42;
    
    int x = 100;
    float y = 50.5f;
    
    // Test 1: Direct pointer modification
    process_direct(&x);
    
    // Test 2: Aliasing scenario (i == j)
    process_aliased(arr1, 3, 3);  // i == j ensures aliasing
    
    // Test 3: Volatile modification
    process_volatile(&v1);
    
    // Test 4: Mixed data types
    process_mixed(x, y);
    
    // Test 5: Loop with array modification
    process_array(arr1, 10, 50);
    
    // Test 6: Atomic operations
    process_atomic(&atomic_val);
    
    // Test 7: Complex pointer scenario
    int a = 10, b = 5, c = 3;
    process_complex(&a, &b, &c);
    
    // Additional inline test cases to ensure coverage
    
    // Inline test 1: Short then block with multiple modifications
    int test1 = 42;
    if (test1 > 0) {
        test1 = test1 * 2;  // First modification
        test1 = test1 + 1;  // Second modification
    }
    
    // Inline test 2: Volatile with multiple operations
    volatile int test2 = 100;
    if (test2 > 50) {
        test2 = 25;
        test2 = test2 * 2;
        test2 = test2 - 10;
    }
    
    // Inline test 3: Pointer arithmetic with modification
    int array[5] = {1, 2, 3, 4, 5};
    int *ptr = array;
    if (*ptr > 0) {
        *ptr = 10;
        *(ptr + 1) = 20;  // Could alias if ptr moves
    }
    
    // Inline test 4: Nested condition with modification
    int test3 = 30;
    int test4 = 20;
    if (test3 > test4) {
        test3 = test4;  // Modifies test3
        if (test3 > 15) {  // New condition
            test4 = test3 * 2;  // Modifies test4
        }
    }
    
    // Inline test 5: Loop with break condition modification
    int counter = 0;
    for (int i = 0; i < 10; i++) {
        if (counter > 5) {
            counter = 0;  // Modifies loop condition variable
            break;
        }
        counter++;
    }
    
    // Use results to prevent dead code elimination
    int sum = x + v1 + v2 + (int)atomic_val + test1 + test2 + test3 + test4 + counter;
    for (int i = 0; i < 10; i++) {
        sum += arr1[i] + arr2[i];
    }
    
    printf("Result: %d\n", sum);
    
    return 0;
}
