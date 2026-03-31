#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Function that modifies test expression via pointer */
void modify_via_pointer(int *ptr) {
    if (*ptr > 0) {  // Test expression: *ptr
        *ptr = -1;   // Modifies test expression
        *ptr += 2;   // Additional modification
    }
}

/* Function with volatile test variable */
void volatile_modification(volatile int *v) {
    if (*v > 10) {   // Test expression: *v (volatile)
        *v = 5;      // Modifies test expression
        *v *= 2;     // Second modification
    }
}

/* Function with array aliasing possibilities */
void array_aliasing(int *arr, int i, int j) {
    if (arr[i] > 0) {    // Test expression: arr[i]
        arr[j] = 0;      // May alias if i == j
        arr[i] = -5;     // Definitely modifies test expression
    }
}

/* Function with mixed data types */
void mixed_types(int x, float y) {
    if ((float)x > y) {  // Test expression involves x
        x = (int)(y * 2.0f);  // Modifies x
        x += 3;          // Additional modification
    }
}

/* Function with loop-dependent condition */
void loop_modification(int *data, int n, int threshold) {
    for (int i = 0; i < n; i++) {
        if (data[i] > threshold) {  // Test expression: data[i]
            data[i] = threshold;    // Modifies test expression
            data[i]--;              // Second modification
        }
    }
}

/* Function with struct pointer */
struct Container {
    int value;
    int *ptr;
};

void struct_modification(struct Container *c) {
    if (c->value > 100) {  // Test expression: c->value
        c->value = 50;     // Modifies test expression
        c->value += c->ptr[0];  // Additional modification
    }
}

/* Main function with various if-conversion candidates */
int main() {
    volatile int v1 = 15;
    volatile int v2 = 5;
    int arr[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int arr2[10] = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};
    int x = 8;
    float y = 3.5f;
    int *ptr1 = &arr[0];
    int *ptr2 = &arr[5];
    
    struct Container c1 = {150, arr};
    struct Container c2 = {75, arr2};
    
    /* Candidate 1: Simple pointer modification */
    modify_via_pointer(&x);
    
    /* Candidate 2: Volatile modification */
    volatile_modification(&v1);
    
    /* Candidate 3: Array with potential aliasing */
    array_aliasing(arr, 2, 2);  // i == j, definitely aliases
    array_aliasing(arr, 3, 4);  // i != j, may still alias
    
    /* Candidate 4: Mixed types */
    mixed_types(x, y);
    
    /* Candidate 5: Loop with modifications */
    loop_modification(arr2, 10, 50);
    
    /* Candidate 6: Struct modification */
    struct_modification(&c1);
    struct_modification(&c2);
    
    /* Candidate 7: Inline block with multiple modifications */
    int temp = 20;
    if (temp > 10) {      // Test expression: temp
        temp = 5;         // First modification
        temp += arr[0];   // Second modification
        temp *= 2;        // Third modification
    }
    
    /* Candidate 8: Nested conditions */
    int a = 25, b = 30;
    if (a > 20) {
        if (b > 25) {     // Test expression: b
            b = 10;       // Modifies inner test expression
            a = b + 5;    // Modifies outer test expression
        }
    }
    
    /* Candidate 9: Pointer arithmetic */
    int *p = arr;
    if (*(p + 3) > 2) {   // Test expression: *(p + 3)
        *(p + 3) = 0;     // Modifies test expression
        p[3] += 1;        // Another modification (same location)
    }
    
    /* Candidate 10: Atomic operation simulation */
    _Atomic int atomic_val = 42;
    int local_atomic = atomic_val;
    if (local_atomic > 40) {  // Test expression: local_atomic
        local_atomic = 20;    // Modifies test expression
        local_atomic += 5;    // Additional modification
    }
    atomic_val = local_atomic;
    
    /* Use results to prevent dead code elimination */
    int sum = v1 + v2 + x + arr[0] + arr[1] + arr2[0] + c1.value + c2.value + temp + a + b;
    printf("Result: %d\n", sum);
    
    return sum > 0 ? 0 : 1;
}
