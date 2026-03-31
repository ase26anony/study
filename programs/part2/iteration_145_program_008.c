#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Function with pointer-based modification that could alias */
void process(int *p, int *q) {
    /* Pattern 1: Direct modification of test variable */
    if (*p > 0) {
        *p = -1;  /* Modifies the test expression */
        *q = *q + 1;  /* Additional instruction */
    }
}

/* Function with array access and potential aliasing */
void array_process(int arr[], int n, int idx) {
    /* Pattern 2: Array element modification */
    if (arr[idx] > 10) {
        arr[idx] = 0;  /* Modifies test expression */
        arr[idx] += 5;  /* Second modification */
        n = n + 1;  /* Third instruction */
    }
}

/* Function with volatile variable */
volatile int global_counter = 0;

void volatile_test(void) {
    volatile int v = 5;
    int local = 0;
    
    /* Pattern 3: Volatile test with modification */
    if (v > 0) {
        v = 10;  /* Modifies volatile test variable */
        local = v * 2;  /* Additional instruction */
        global_counter++;  /* Third instruction */
    }
}

/* Function with mixed data types */
void mixed_types_test(void) {
    int x = 10;
    float y = 3.14f;
    
    /* Pattern 4: Mixed types with implicit conversion */
    if ((float)x > y) {
        x = (int)(y * 2.0f);  /* Modifies variable used in test */
        y = y + 1.0f;  /* Additional float operation */
        x = x - 1;  /* Third instruction */
    }
}

/* Function with loop-dependent condition */
void loop_based_test(int data[], int size, int threshold) {
    /* Pattern 5: Loop with condition modifying test expression */
    for (int i = 0; i < size; i++) {
        if (data[i] > threshold) {
            data[i] = 0;  /* Modifies array element used in test */
            data[i] = threshold / 2;  /* Second modification */
            i = i;  /* Dummy self-assignment (third instruction) */
        }
    }
}

/* Function with pointer aliasing complexity */
void pointer_aliasing_test(int *a, int *b, int *c) {
    /* Pattern 6: Multiple pointers that might alias */
    if (*a > *b) {
        *a = *b + *c;  /* Modifies test variable */
        *b = *a - 1;   /* Modifies other test variable */
        *c = 0;        /* Third instruction */
    }
}

/* Function with struct member access */
struct Data {
    int value;
    int count;
};

void struct_test(struct Data *d) {
    /* Pattern 7: Struct member modification */
    if (d->value > 100) {
        d->value = 50;  /* Modifies test expression */
        d->count++;     /* Additional instruction */
        d->value -= 10; /* Third modification of test variable */
    }
}

/* Main function to exercise all patterns */
int main(void) {
    int arr1[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int arr2[5] = {20, 15, 30, 25, 10};
    volatile int vol_var = 7;
    int x = 5, y = 3, z = 8;
    struct Data mydata = {150, 0};
    
    /* Test 1: Direct pointer modification */
    process(&x, &y);
    
    /* Test 2: Array processing with potential aliasing */
    array_process(arr1, 10, 3);
    
    /* Test 3: Volatile variable test */
    volatile_test();
    
    /* Test 4: Mixed data types */
    mixed_types_test();
    
    /* Test 5: Loop-based modification */
    loop_based_test(arr2, 5, 20);
    
    /* Test 6: Complex pointer aliasing */
    pointer_aliasing_test(&x, &y, &z);
    
    /* Test 7: Struct member modification */
    struct_test(&mydata);
    
    /* Additional test: Nested if with multiple modifications */
    {
        int a = 10, b = 5;
        if (a > b) {
            a = b * 2;  /* First modification */
            b = a + 1;  /* Second instruction */
            a = a - 3;  /* Third modification of test variable */
        }
    }
    
    /* Test with atomic operations (prevents certain optimizations) */
    {
        _Atomic int atomic_var = 42;
        int local = 0;
        
        if (atomic_var > 0) {
            atomic_var = 0;  /* Modifies atomic test variable */
            local = atomic_var + 1;
            atomic_var = local * 2;
        }
    }
    
    /* Loop with multiple if-conversion candidates */
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        if (arr1[i] > 5) {
            arr1[i] = arr1[i] * 2;  /* Modifies test expression */
            sum += arr1[i];         /* Additional instruction */
            arr1[i] = arr1[i] - 1;  /* Third modification */
        }
    }
    
    /* Use results to prevent dead code elimination */
    printf("Results: x=%d, y=%d, z=%d, sum=%d, global=%d, struct.value=%d\n",
           x, y, z, sum, global_counter, mydata.value);
    
    return 0;
}
