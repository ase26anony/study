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
        arr[j] = 0;         // Potential aliasing if i == j
        arr[i] = arr[i] * 2; // Direct modification
    }
}

/* Function with mixed types */
void process_mixed(int x, float y) {
    if ((float)x > y) {     // Test involves conversion
        x = (int)(y * 2.0f); // Modifies original variable
        x = x + 1;          // Additional modification
    }
}

/* Function with volatile variable */
void process_volatile(volatile int *vp) {
    if (*vp > 100) {        // Test volatile memory
        *vp = 50;           // Modify volatile
        *vp = *vp * 2;      // Additional volatile modification
    }
}

/* Main function with various if-conversion candidates */
int main() {
    volatile int v1 = 150;
    volatile int v2 = 75;
    int arr[20];
    int *ptr = &arr[5];
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 20; i++) {
        arr[i] = i * 10;
    }
    
    /* Pattern 1: Simple variable modification in then-block */
    int x = 42;
    if (x > 0) {            // Test expression: x
        x = x * 2;          // Modifies test variable
        x = x - 1;          // Additional modification
    }
    sum += x;
    
    /* Pattern 2: Volatile modification */
    if (v1 > 100) {         // Test volatile
        v1 = v1 / 2;        // Modifies volatile test expression
        v1 = v1 + 10;       // Additional volatile modification
    }
    sum += v1;
    
    /* Pattern 3: Pointer-based with potential aliasing */
    int a = 25, b = 30;
    int *p1 = &a, *p2 = &a; // Same address - definite aliasing
    if (*p1 > 20) {
        *p2 = 0;            // Modifies through alias
        *p1 = *p1 + 5;      // Additional modification
    }
    sum += a;
    
    /* Pattern 4: Loop-dependent condition with modification */
    for (int i = 0; i < 10; i++) {
        if (arr[i] > 50) {  // Test array element
            arr[i] = 0;     // Modifies test expression
            arr[i] = arr[i] + i; // Additional modification
        }
        sum += arr[i];
    }
    
    /* Pattern 5: Mixed types with implicit conversions */
    float f = 3.14f;
    int y = 10;
    if ((float)y > f) {     // Test with conversion
        y = (int)(f * 10.0f); // Modifies test variable
        y = y % 7;          // Additional modification
    }
    sum += y;
    
    /* Pattern 6: Nested conditions */
    int z = 100;
    if (z > 50) {
        if (z < 200) {      // Inner if with same variable
            z = z / 2;      // Modifies variable used in outer test
            z = z * 3;      // Additional modification
        }
    }
    sum += z;
    
    /* Pattern 7: Function calls with pointer arguments */
    process_int(&arr[15]);
    sum += arr[15];
    
    /* Pattern 8: Array with potential aliasing */
    process_array(arr, 3, 3); // i == j, definite aliasing
    sum += arr[3];
    
    /* Pattern 9: Mixed types function */
    process_mixed(8, 2.5f);
    
    /* Pattern 10: Volatile through function */
    process_volatile(&v2);
    sum += v2;
    
    /* Pattern 11: Complex test expression */
    int m = 5, n = 10;
    if (m + n > 10) {       // Test expression: m + n
        m = 0;              // Modifies part of test expression
        n = n * 2;          // Modifies other part
    }
    sum += m + n;
    
    /* Pattern 12: Memory access through different pointers */
    int buffer[5] = {100, 200, 300, 400, 500};
    int *buf_ptr1 = &buffer[2];
    int *buf_ptr2 = &buffer[2]; // Same location
    if (*buf_ptr1 > 250) {
        *buf_ptr2 = 0;      // Modifies through alias
        buffer[2] = buffer[2] + 100; // Direct modification
    }
    sum += buffer[2];
    
    /* Prevent dead code elimination */
    printf("Result sum: %d\n", sum);
    
    /* Additional loop with side effects to create more basic blocks */
    for (int i = 0; i < 5; i++) {
        volatile int temp = i * 20;
        if (temp > 30) {
            temp = temp - 10;
            temp = temp * 2;
        }
        sum += temp;
    }
    
    return sum > 0 ? 0 : 1;
}
