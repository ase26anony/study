#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Function prototypes */
void process_direct(int *p);
void process_aliased(int *arr, int idx1, int idx2);
int process_volatile(volatile int *vp);
void process_mixed_types(float threshold);

/* Global volatile to prevent optimization */
volatile int global_counter = 0;

/* Function where then-block modifies the test variable directly */
void process_direct(int *p) {
    /* Pattern 1: Direct modification of test variable */
    if (*p > 100) {
        *p = *p / 2;      /* Modifies test expression */
        global_counter++;  /* Side effect to prevent elimination */
    }
    
    /* Pattern 2: Multiple modifications in then-block */
    int local = *p;
    if (local < 50) {
        local = local * 3;  /* First modification */
        local = local + 10; /* Second modification */
        *p = local;         /* Third modification - modifies original */
    }
}

/* Function with potential aliasing issues */
void process_aliased(int *arr, int idx1, int idx2) {
    /* Pattern 3: Array access with possible aliasing */
    if (arr[idx1] > 0) {
        arr[idx2] = 0;  /* May alias if idx1 == idx2 */
        /* Add another instruction to ensure loop iteration */
        arr[idx1] = arr[idx1] - 1;  /* Definitely modifies test expr */
    }
    
    /* Pattern 4: Pointer aliasing */
    int *ptr1 = &arr[idx1];
    int *ptr2 = &arr[idx2];
    if (*ptr1 > 10) {
        *ptr2 = *ptr1 - 5;  /* Potential aliasing modification */
        *ptr1 = *ptr1 * 2;   /* Direct modification */
    }
}

/* Function with volatile test variable */
int process_volatile(volatile int *vp) {
    int result = 0;
    
    /* Pattern 5: Volatile test with modification */
    if (*vp > 0) {
        *vp = *vp - 1;      /* Modifies volatile test expression */
        result = 1;
        *vp = *vp * 2;      /* Second modification in same block */
    }
    
    /* Pattern 6: Loop with volatile condition */
    for (int i = 0; i < 5; i++) {
        if (*vp > i) {
            *vp = *vp + i;  /* Modifies test expression in loop */
            result++;
        }
    }
    
    return result;
}

/* Function with mixed data types */
void process_mixed_types(float threshold) {
    int int_val = 42;
    float float_val = 3.14f;
    
    /* Pattern 7: Implicit conversion in test */
    if ((float)int_val > threshold) {
        int_val = (int)(float_val * 10.0f);  /* Modifies variable used in test */
        float_val = float_val + 1.0f;        /* Additional instruction */
    }
    
    /* Pattern 8: Different types with pointer */
    volatile float *fptr = &float_val;
    if (*fptr > 2.0f) {
        *fptr = *fptr / 2.0f;  /* Modifies test expression */
        int_val = int_val + (int)(*fptr);  /* Mixed type operation */
    }
    
    /* Use results to prevent dead code elimination */
    global_counter += int_val;
}

/* Main function with various if-conversion candidates */
int main() {
    int array[20];
    volatile int volatile_var = 100;
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 20; i++) {
        array[i] = i * 10;
    }
    
    /* Test 1: Direct modification patterns */
    process_direct(&array[5]);
    process_direct(&array[10]);
    
    /* Test 2: Aliasing patterns */
    process_aliased(array, 3, 3);   /* Same index - definite aliasing */
    process_aliased(array, 7, 8);   /* Different indices - potential aliasing */
    
    /* Test 3: Volatile patterns */
    sum += process_volatile(&volatile_var);
    
    /* Test 4: Mixed type patterns */
    process_mixed_types(2.5f);
    
    /* Test 5: Loop with array modification (common if-conversion candidate) */
    for (int i = 0; i < 15; i++) {
        if (array[i] > 50) {
            array[i] = array[i] - 25;  /* Modifies test expression */
            /* Add multiple instructions to ensure loop in ifcvt */
            array[i] = array[i] * 2;
            array[i] = array[i] / 3;
        }
    }
    
    /* Test 6: Nested conditions */
    for (int i = 0; i < 10; i++) {
        int *ptr = &array[i];
        if (*ptr > 30) {
            if (*ptr < 70) {
                *ptr = *ptr + 5;      /* Modifies test expression for both ifs */
                *ptr = *ptr - 2;      /* Additional modification */
            }
        }
    }
    
    /* Test 7: Atomic operations (prevent certain optimizations) */
    _Atomic int atomic_val = 50;
    if (atomic_val > 25) {
        atomic_val = atomic_val - 10;  /* Atomic modification */
        atomic_val = atomic_val * 2;   /* Second atomic operation */
    }
    
    /* Test 8: Short then-block with multiple modifications */
    int short_test = 75;
    if (short_test > 50) {
        short_test = short_test << 1;  /* First modification */
        short_test = short_test | 1;   /* Second modification */
        short_test = short_test >> 1;  /* Third modification */
    }
    
    /* Calculate final sum to use all results */
    for (int i = 0; i < 20; i++) {
        sum += array[i];
    }
    sum += volatile_var;
    sum += global_counter;
    sum += short_test;
    
    printf("Result: %d\n", sum);
    return sum > 0 ? 0 : 1;
}
