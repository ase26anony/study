#include <stdio.h>
#include <stdlib.h>

/* Function with pointer-based modification - aliasing possible */
void process(int *p, int *q) {
    /* Pattern 1: Direct modification of test variable */
    if (*p > 0) {
        *p = -1;  /* Modifies the test expression */
        *q = *q + 1;  /* Additional instruction */
    }
}

/* Function with array access - potential aliasing */
void process_array(int arr[], int n) {
    /* Pattern 2: Loop-dependent condition with modification */
    for (int i = 0; i < n; i++) {
        if (arr[i] > 10) {
            arr[i] = 0;  /* Modifies the test expression */
            arr[i] += 5;  /* Second modification - multiple instructions */
        }
    }
}

/* Function with mixed data types and implicit conversions */
void mixed_types(int x, float y) {
    /* Pattern 3: Mixed types with conversion */
    if ((float)x > y) {
        x = (int)(y * 2.0f);  /* Modifies variable used in test */
        x = x + 1;  /* Additional instruction */
    }
}

/* Function with volatile variable */
void volatile_test(void) {
    /* Pattern 4: Volatile prevents optimization */
    volatile int v = 5;
    volatile int w = 10;
    
    if (v > 0) {
        v = 10;  /* Modifies volatile test variable */
        w = w + v;  /* Additional volatile operation */
    }
}

/* Function with multiple modifications in then block */
void multi_modify(int *a, int *b) {
    /* Pattern 5: Multiple arithmetic operations */
    if (*a > *b) {
        *a = *b;  /* First modification */
        *b = *a + 1;  /* Second modification */
        *a = *a - 1;  /* Third modification - 3+ instructions */
    }
}

/* Function with potential self-aliasing */
void self_modify(int *ptr) {
    /* Pattern 6: Pointer could alias with itself */
    int local = *ptr;
    if (local > 100) {
        *ptr = 50;  /* Modifies what might be the test expression */
        local = local + *ptr;  /* Uses both values */
    }
}

int main(void) {
    /* Initialize test data */
    volatile int vol_var = 42;
    int array[20];
    int x = 10, y = 20, z = 30;
    int *ptr1 = &x;
    int *ptr2 = &y;
    
    /* Initialize array with varying values */
    for (int i = 0; i < 20; i++) {
        array[i] = i * 3;
    }
    
    /* Test 1: Direct modification with pointers */
    process(&x, &y);
    
    /* Test 2: Array processing with loop */
    process_array(array, 20);
    
    /* Test 3: Mixed types */
    mixed_types(z, 15.5f);
    
    /* Test 4: Volatile operations */
    volatile_test();
    
    /* Test 5: Multiple modifications */
    multi_modify(&x, &y);
    
    /* Test 6: Self-modifying pattern */
    self_modify(&z);
    
    /* Test 7: Complex condition with multiple variables */
    int a = 5, b = 10, c = 15;
    if (a + b > c) {
        a = b + c;  /* Modifies variable used in test (a) */
        b = a - c;  /* Additional modification */
        c = a + b;  /* Third modification */
    }
    
    /* Test 8: Nested conditions */
    for (int i = 0; i < 10; i++) {
        if (array[i] > 20) {
            array[i] = array[i] / 2;  /* Modifies test expression */
            if (array[i] < 10) {
                array[i] = array[i] * 3;  /* Nested modification */
            }
        }
    }
    
    /* Test 9: Pointer arithmetic with potential aliasing */
    int *arr_ptr = array;
    for (int i = 0; i < 19; i++) {
        if (arr_ptr[i] > arr_ptr[i + 1]) {
            arr_ptr[i] = arr_ptr[i + 1];  /* Modifies test expression */
            arr_ptr[i + 1] = arr_ptr[i] + 1;  /* Could alias */
        }
    }
    
    /* Test 10: Short but modifying then block */
    int counter = 0;
    for (int i = 0; i < 20; i++) {
        if (array[i] % 2 == 0) {
            array[i] = array[i] + 1;  /* Single modification */
            counter++;  /* Additional instruction */
        }
    }
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 20; i++) {
        sum += array[i];
    }
    sum += x + y + z + a + b + c + counter + vol_var;
    
    printf("Result: %d\n", sum);
    
    return 0;
}
