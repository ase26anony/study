/* Test program specifically designed to trigger uncovered lines in ifcvt.cc
 * Lines 577-583: modified_in_p check for test expression modification in then block
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Global volatile to prevent optimization */
volatile int global_volatile = 0;

/* Function with pointer-based modification - Pattern 3 */
void modify_via_pointer(int *ptr, int threshold) {
    /* Pattern 1 & 3: Test expression modified in then block via pointer */
    if (*ptr > threshold) {
        *ptr = threshold;  /* Modifies the test expression */
        global_volatile++; /* Side effect to prevent dead code elimination */
    }
}

/* Function with array aliasing - Pattern 3 & 5 */
void process_array(int *arr, int size, int threshold) {
    /* Pattern 5: Loop-dependent condition with side effects */
    for (int i = 0; i < size; i++) {
        /* Pattern 1 & 3: Test expression may be modified via aliasing */
        if (arr[i] > threshold) {
            arr[i] = 0;  /* Direct modification of test expression */
            /* Pattern 2: Multiple non-label instructions */
            int temp = arr[i] * 2;
            arr[i] = temp / 2;  /* Another modification */
            global_volatile += temp;
        }
    }
}

/* Function with mixed data types - Pattern 6 */
void mixed_types_test(int int_val, float float_val) {
    /* Pattern 6: Mixed data types with implicit conversions */
    if ((float)int_val > float_val) {
        /* Pattern 1: Modifying the variable used in test */
        int_val = (int)(float_val * 2.0f);  /* Modifies test expression */
        /* Pattern 2: Additional arithmetic operations */
        float_val = float_val + 1.0f;
        global_volatile += int_val;
    }
}

/* Function with volatile test variable - Pattern 4 */
void volatile_test(void) {
    volatile int local_volatile = global_volatile;
    
    /* Pattern 4: Volatile prevents optimization */
    if (local_volatile > 10) {
        /* Pattern 1: Modifying the volatile test variable */
        local_volatile = 5;  /* Direct modification */
        /* Pattern 2: Multiple operations */
        int temp = local_volatile * 2;
        local_volatile = temp - 3;
        global_volatile = local_volatile;
    }
}

/* Complex aliasing scenario - Pattern 3 */
void complex_aliasing(int *a, int *b, int *c) {
    /* Create potential aliasing */
    *b = *a + 1;
    *c = *b - 1;
    
    /* Pattern 3: Pointer-based test with potential aliasing */
    if (*a > 0) {
        /* These could alias with *a */
        *b = 0;
        *c = 0;
        /* Pattern 2: Multiple instructions */
        *a = (*b + *c) * 2;
        global_volatile += *a;
    }
}

/* Main function with varied control flow */
int main(void) {
    int array[10];
    int *ptr_array[5];
    int result = 0;
    
    /* Initialize arrays */
    for (int i = 0; i < 10; i++) {
        array[i] = i * 3;
    }
    
    /* Pattern 1 & 5: Simple if with modification in loop */
    for (int i = 0; i < 10; i++) {
        if (array[i] > 10) {
            array[i] = array[i] / 2;  /* Modifies test expression */
            /* Pattern 2: Additional operation */
            result += array[i];
        }
    }
    
    /* Test 1: Pointer-based modification */
    int x = 15;
    modify_via_pointer(&x, 10);
    result += x;
    
    /* Test 2: Array processing */
    process_array(array, 10, 5);
    for (int i = 0; i < 10; i++) {
        result += array[i];
    }
    
    /* Test 3: Mixed types */
    mixed_types_test(20, 15.5f);
    result += global_volatile;
    
    /* Test 4: Volatile test */
    volatile_test();
    result += global_volatile;
    
    /* Test 5: Complex aliasing */
    int a = 8, b = 4, c = 2;
    complex_aliasing(&a, &b, &c);
    result += a + b + c;
    
    /* Additional test: Nested conditions */
    int y = 25;
    int z = 30;
    if (y > 20) {
        if (z > 25) {
            y = z - 10;  /* Modifies outer condition variable */
            z = y * 2;   /* Multiple modifications */
            result += y + z;
        }
    }
    
    /* Test with atomic operations (similar to volatile) */
    _Atomic int atomic_var = 0;
    atomic_store(&atomic_var, 100);
    
    if (atomic_load(&atomic_var) > 50) {
        atomic_store(&atomic_var, 25);  /* Modifies test expression */
        result += atomic_load(&atomic_var);
    }
    
    /* Final output to prevent dead code elimination */
    printf("Result: %d\n", result);
    return result > 0 ? 0 : 1;
}
