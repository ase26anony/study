#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Function that modifies test expression via pointer */
void process_with_modification(int *p, int *q) {
    /* Pattern 1: Direct modification of test variable */
    if (*p > 0) {
        *p = *p * 2;  // Modifies the test expression
        *q += 1;      // Additional instruction
    }
}

/* Function with volatile test variable */
void volatile_modification(volatile int *vp) {
    /* Pattern 2: Volatile variable modification */
    if (*vp > 10) {
        *vp = 5;      // Modifies volatile test expression
        *vp += 2;     // Second modification
    }
}

/* Function with array aliasing possibilities */
void array_aliasing(int arr[], int n, int idx1, int idx2) {
    /* Pattern 3: Potential aliasing through array indices */
    if (arr[idx1] > 0) {
        arr[idx2] = 0;  // Could modify test expression if idx1 == idx2
        arr[idx1] = arr[idx1] * 3;  // Definitely modifies test expression
    }
}

/* Function with mixed data types */
void mixed_types(float *fp, int *ip) {
    /* Pattern 4: Implicit conversions and type mixing */
    if ((float)*ip > 0.5f) {
        *ip = (int)(*fp * 2.0f);  // Modifies integer used in test
        *fp = *fp + 1.0f;         // Additional float operation
    }
}

/* Function with loop-dependent condition */
void loop_dependent_modification(int data[], int n, int threshold) {
    /* Pattern 5: Loop with modifying condition */
    for (int i = 0; i < n; i++) {
        if (data[i] > threshold) {
            data[i] = 0;          // Modifies test expression
            data[i] += i;         // Second modification
            threshold = data[i];  // Modifies threshold (could affect future iterations)
        }
    }
}

/* Function with multiple short then-blocks */
void multiple_short_blocks(int *a, int *b, int *c) {
    /* Several if-conversion candidates in sequence */
    if (*a > 0) {
        *a = *b + *c;  // Modifies test variable
    }
    
    if (*b < *a) {
        *b = *a * 2;   // Modifies test variable
        *c = *b / 2;   // Additional instruction
    }
    
    if (*c != 0) {
        *c = 0;        // Modifies test variable
    }
}

/* Function with pointer aliasing complexity */
void pointer_aliasing(int *ptr1, int *ptr2, int *ptr3) {
    /* Pattern 6: Complex pointer relationships */
    if (*ptr1 > *ptr2) {
        *ptr3 = *ptr1 + *ptr2;  // Doesn't modify test expressions directly
        *ptr1 = *ptr3 - *ptr2;  // Modifies ptr1 which is in test
        *ptr2 += 1;             // Modifies ptr2 which is in test
    }
}

int main() {
    /* Initialize test data with volatile and non-volatile variables */
    volatile int volatile_var = 15;
    int array1[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int array2[10] = {10, 9, 8, 7, 6, 5, 4, 3, 2, 1};
    int x = 5, y = 10, z = 15;
    float f = 2.5f;
    
    /* Test 1: Direct modification with pointers */
    process_with_modification(&x, &y);
    
    /* Test 2: Volatile modification */
    volatile_modification(&volatile_var);
    
    /* Test 3: Array aliasing (force potential aliasing) */
    array_aliasing(array1, 10, 3, 3);  // Same index ensures modification
    
    /* Test 4: Mixed data types */
    mixed_types(&f, &z);
    
    /* Test 5: Loop-dependent modifications */
    loop_dependent_modification(array2, 10, 5);
    
    /* Test 6: Multiple short blocks */
    multiple_short_blocks(&x, &y, &z);
    
    /* Test 7: Pointer aliasing */
    int a = 20, b = 10, c = 0;
    pointer_aliasing(&a, &b, &c);
    
    /* Additional complex scenario with nested conditions */
    for (int i = 0; i < 5; i++) {
        volatile int *ptr = &array1[i];
        if (*ptr > i) {
            *ptr = i;           // Modifies test expression
            array2[i] = *ptr;   // Additional modification
            if (array2[i] < 3) {
                array2[i] *= 2; // Nested modification
            }
        }
    }
    
    /* Use results to prevent dead code elimination */
    int sum = x + y + z + volatile_var + (int)f;
    for (int i = 0; i < 10; i++) {
        sum += array1[i] + array2[i];
    }
    
    printf("Result: %d\n", sum);
    return sum > 100 ? 0 : 1;
}
