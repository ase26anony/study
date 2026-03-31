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
        arr[j] = 0;         // May alias with arr[i] if i == j
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
    if ((float)x > y) {     // Test expression involves conversion
        x = (int)(y * 2.0f); // Modifies original variable
        x = x + 1;          // Additional modification
    }
}

/* Function with atomic operations */
void process_atomic(_Atomic int *ap) {
    int val = atomic_load(ap);
    if (val > 100) {        // Test expression: loaded atomic value
        atomic_store(ap, 50); // Modifies atomic location
        atomic_fetch_add(ap, 1); // Additional atomic modification
    }
}

int main() {
    volatile int v1 = 15;
    volatile int v2 = 3;
    int arr[10] = {5, 15, 25, 35, 45, 55, 65, 75, 85, 95};
    _Atomic int atomic_val = 150;
    int sum = 0;
    
    /* Pattern 1: Simple variable modification in then block */
    int x = 20;
    if (x > 10) {           // Test expression: x
        x = 5;              // Modifies test expression
        x = x * 2;          // Additional modification
    }
    sum += x;
    
    /* Pattern 2: Volatile variable with multiple modifications */
    if (v1 > 10) {          // Test expression: v1 (volatile)
        v1 = 20;            // First modification
        v1 = v1 - 5;        // Second modification
        int dummy = v1;     // Use to prevent dead code elimination
        (void)dummy;
    }
    
    /* Pattern 3: Pointer-based test with modification */
    int *ptr = &arr[2];
    if (*ptr > 20) {        // Test expression: *ptr
        *ptr = 0;           // Modifies through pointer
        *ptr += 10;         // Additional modification
    }
    
    /* Pattern 4: Loop with array modification */
    for (int i = 0; i < 10; i++) {
        if (arr[i] > 50) {  // Test expression: arr[i]
            arr[i] = 40;    // Modifies test expression
            arr[i]++;       // Additional modification
        }
    }
    
    /* Pattern 5: Aliasing scenario */
    int idx = 3;
    if (arr[idx] > 30) {    // Test expression: arr[3]
        arr[idx] = 25;      // Direct modification
        arr[3] = 30;        // Aliasing modification (same location)
    }
    
    /* Pattern 6: Mixed types with implicit conversion */
    float f = 7.5f;
    int y = 8;
    if ((float)y > f) {     // Test expression involves conversion
        y = (int)f;         // Modifies test variable
        y = y * 2;          // Additional modification
    }
    sum += y;
    
    /* Pattern 7: Nested if with multiple modifications */
    int z = 100;
    if (z > 50) {
        if (z < 150) {      // Nested condition
            z = 75;         // Modifies outer test variable
            z += z % 10;    // Additional modification
        }
    }
    sum += z;
    
    /* Call functions with different patterns */
    process_int(&arr[0]);
    process_array(arr, 1, 1);  // i == j case
    process_volatile(&v2);
    process_mixed_types(12, 10.0f);
    process_atomic(&atomic_val);
    
    /* Loop with side effects in condition */
    int counter = 0;
    for (int i = 0; i < 5; i++) {
        // Condition depends on loop variable and array access
        if (arr[i] > counter) {
            counter = arr[i];  // Modifies variable used in future iterations
            arr[i] = counter / 2;  // Additional modification
        }
    }
    
    /* Complex test expression with multiple variables */
    int a = 10, b = 20, c = 30;
    if (a + b > c) {        // Test expression: a + b
        a = 15;             // Modifies part of test expression
        b = 25;             // Modifies another part
        c = a + b;          // Uses modified values
    }
    sum += a + b + c;
    
    /* Final computation to prevent dead code elimination */
    for (int i = 0; i < 10; i++) {
        sum += arr[i];
    }
    sum += v1 + v2 + atomic_load(&atomic_val);
    
    printf("Result: %d\n", sum);
    return sum > 1000 ? 0 : 1;  // Return different values based on computation
}
