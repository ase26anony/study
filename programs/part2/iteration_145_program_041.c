#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Function with pointer-based modification */
void process_int(int *p) {
    if (*p > 0) {           // Test expression: *p
        *p = -1;            // Modifies test expression
        *p += 2;            // Additional modification
    }
}

/* Function with array aliasing */
void process_array(int arr[], int i, int j) {
    if (arr[i] > 10) {      // Test expression: arr[i]
        arr[j] = 0;         // Potentially modifies arr[i] if i == j
        arr[i] = arr[i] * 2; // Definitely modifies test expression
    }
}

/* Function with volatile variable */
void process_volatile(volatile int *vp) {
    if (*vp > 5) {          // Test expression: *vp
        *vp = 10;           // Modifies volatile test expression
        int temp = *vp + 1; // Additional operation
        (void)temp;         // Use temp to prevent optimization
    }
}

/* Function with mixed data types */
void process_mixed_types(int x, float y) {
    if ((float)x > y) {     // Test expression involves x
        x = (int)(y * 2.0f); // Modifies x (test expression component)
        x = x + 1;          // Additional modification
    }
}

/* Function with atomic operations */
void process_atomic(_Atomic int *ap) {
    int val = atomic_load(ap);
    if (val > 100) {        // Test expression: val (loaded from *ap)
        atomic_store(ap, 50); // Modifies the memory location
        atomic_fetch_add(ap, 1); // Additional atomic modification
    }
}

int main() {
    volatile int v1 = 15;
    volatile int v2 = 3;
    int arr[10] = {20, 5, 15, 8, 25, 3, 18, 9, 12, 7};
    int x = 10;
    float y = 5.5f;
    _Atomic int atomic_val = 200;
    
    /* Pattern 1: Simple variable modification in then block */
    if (x > 5) {            // Test expression: x
        x = x * 2;          // Modifies test expression
        x = x - 3;          // Additional modification
    }
    
    /* Pattern 2: Volatile variable with multiple operations */
    if (v1 > 10) {          // Test expression: v1 (volatile)
        v1 = v1 / 2;        // Modifies volatile test expression
        v2 = v1 + 1;        // Uses modified value
    }
    
    /* Pattern 3: Loop with array modification */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        if (arr[i] > 10) {  // Test expression: arr[i]
            arr[i] = 0;     // Modifies test expression
            sum += i;       // Side effect
        }
    }
    
    /* Pattern 4: Pointer aliasing scenario */
    int *ptr1 = &arr[2];
    int *ptr2 = &arr[2];    // Same location
    if (*ptr1 > 10) {       // Test expression: *ptr1
        *ptr2 = 5;          // Modifies same memory as test expression
        *ptr1 = *ptr1 + 1;  // Additional modification
    }
    
    /* Pattern 5: Nested if with modification */
    int a = 8, b = 12;
    if (a > 5) {            // Test expression: a
        if (b > 10) {       // Nested condition
            a = a + b;      // Modifies outer test expression
            b = a - 3;      // Modifies inner test expression
        }
        a = a * 2;          // Additional modification
    }
    
    /* Pattern 6: Mixed types with implicit conversions */
    char c = 'A';
    int int_val = 65;
    if (c == int_val) {     // Test involves c and int_val
        c = 'B';            // Modifies c
        int_val = (int)c + 1; // Modifies int_val
    }
    
    /* Call functions with different patterns */
    process_int(&x);
    process_array(arr, 3, 3);  // i == j case
    process_volatile(&v1);
    process_mixed_types(x, y);
    process_atomic(&atomic_val);
    
    /* Pattern 7: Complex expression with modification */
    int m = 7, n = 4;
    if ((m * n) > 20) {     // Test expression: m * n
        m = m - 2;          // Modifies m (component of test expression)
        n = n + 1;          // Modifies n (component of test expression)
        int product = m * n; // Uses modified values
        (void)product;
    }
    
    /* Pattern 8: Short then block ideal for if-conversion */
    int simple = 42;
    if (simple > 40) {      // Simple test
        simple = 0;         // Single modification
    }
    
    /* Pattern 9: Longer then block that should still be analyzed */
    int counter = 100;
    if (counter > 50) {     // Test expression: counter
        counter = counter / 2;  // Modification 1
        counter = counter * 3;  // Modification 2
        counter = counter + 10; // Modification 3
        int dummy = counter;    // Additional operation
        (void)dummy;
    }
    
    /* Use results to prevent dead code elimination */
    int total = x + v1 + v2 + sum + a + b + int_val + m + n + simple + counter;
    
    /* Also use array elements */
    for (int i = 0; i < 10; i++) {
        total += arr[i];
    }
    
    printf("Result: %d\n", total);
    printf("Volatile values: %d, %d\n", v1, v2);
    printf("Atomic value: %d\n", atomic_load(&atomic_val));
    
    return total > 0 ? 0 : 1;
}
