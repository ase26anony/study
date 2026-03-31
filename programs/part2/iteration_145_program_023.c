#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Function with pointer-based modification */
void process_int(int *p) {
    /* Pattern 1: Direct modification of test variable */
    if (*p > 0) {
        *p = -1;  /* Modifies the test expression */
        *p += 2;  /* Additional modification */
    }
}

/* Function with array aliasing */
void process_array(int arr[], int n) {
    for (int i = 0; i < n; i++) {
        /* Pattern 2: Loop-dependent condition with modification */
        if (arr[i] > 10) {
            arr[i] = 0;  /* Modifies test expression */
            arr[i] += 5; /* Second instruction in then block */
        }
    }
}

/* Function with potential aliasing */
void aliasing_test(int *a, int *b) {
    /* Pattern 3: Aliasing through pointers */
    if (*a > *b) {
        *a = *b;  /* Could modify test expression if a == b */
        *b = *a + 1;
    }
}

/* Function with mixed data types */
void mixed_types(int x, float y) {
    /* Pattern 4: Implicit conversions */
    if ((float)x > y) {
        x = (int)(y * 2.0f);  /* Modifies x used in test */
        x += 3;               /* Additional modification */
    }
}

/* Function with volatile variables */
void volatile_test(volatile int *v) {
    /* Pattern 5: Volatile prevents optimization */
    if (*v > 100) {
        *v = 50;   /* Modifies volatile test expression */
        *v *= 2;   /* Second volatile modification */
    }
}

/* Complex scenario with multiple conditions */
void complex_conditions(int data[], int n, int threshold) {
    int *ptr = &data[0];
    
    for (int i = 0; i < n; i++) {
        /* Pattern 6: Multiple test modifications */
        if (data[i] > threshold) {
            data[i] = threshold;  /* Modifies test expression */
            ptr = &data[i];       /* Pointer assignment */
            *ptr += i;            /* Additional modification */
        }
        
        /* Nested if for more complexity */
        if (i % 2 == 0 && data[i] < 0) {
            data[i] = -data[i];   /* Modifies in nested block */
        }
    }
}

/* Function with atomic operations */
void atomic_test(_Atomic int *atom) {
    int expected = 0;
    /* Pattern 7: Atomic operations */
    if (atomic_load(atom) > 0) {
        atomic_store(atom, 0);  /* Modifies atomic test expression */
        atomic_fetch_add(atom, 1);  /* Additional atomic op */
    }
}

/* Main function creating various if-conversion candidates */
int main() {
    /* Setup test data */
    volatile int volatile_var = 150;
    int array[20];
    int scalar = 42;
    _Atomic int atomic_var = 5;
    
    /* Initialize array with varied values */
    for (int i = 0; i < 20; i++) {
        array[i] = i * 3 - 10;
    }
    
    /* Pattern 1: Direct modification */
    process_int(&scalar);
    
    /* Pattern 2: Array processing with loop */
    process_array(array, 20);
    
    /* Pattern 3: Potential aliasing */
    aliasing_test(&array[0], &array[1]);
    aliasing_test(&array[5], &array[5]);  /* Same address - definite aliasing */
    
    /* Pattern 4: Mixed types */
    mixed_types(scalar, 15.5f);
    
    /* Pattern 5: Volatile access */
    volatile_test(&volatile_var);
    
    /* Pattern 6: Complex conditions */
    complex_conditions(array, 20, 25);
    
    /* Pattern 7: Atomic operations */
    atomic_test(&atomic_var);
    
    /* Additional inline patterns */
    
    /* Pattern A: Short then block with multiple modifications */
    int x = 100;
    if (x > 50) {
        x = 75;   /* First modification */
        x += 10;  /* Second modification */
        x *= 2;   /* Third modification - total 3 non-label instructions */
    }
    
    /* Pattern B: Pointer arithmetic with modification */
    int *p = array;
    if (*(p + 3) > 0) {
        *(p + 3) = -1;  /* Modifies test expression */
        p[3] += 100;    /* Same location, different syntax */
    }
    
    /* Pattern C: Nested conditions */
    int y = 200;
    int z = 300;
    if (y > 100) {
        if (z > 200) {
            y = z;      /* Modifies outer condition variable */
            z = y / 2;  /* Additional modification */
        }
    }
    
    /* Pattern D: Switch-like with modification */
    int mode = 2;
    int result = 0;
    if (mode == 1) {
        result = 100;
        mode = 0;  /* Modifies test variable */
    } else if (mode == 2) {
        result = 200;
        mode = 1;  /* Modifies test variable */
    }
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 20; i++) {
        sum += array[i];
    }
    sum += scalar + x + y + z + result + volatile_var + atomic_var;
    
    printf("Result: %d\n", sum);
    return sum > 0 ? 0 : 1;
}
