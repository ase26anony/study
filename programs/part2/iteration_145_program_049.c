#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Function that modifies test expression via pointer */
void modify_via_pointer(int *ptr) {
    /* Pattern 1: Direct modification of test expression */
    if (*ptr > 0) {
        *ptr = -1;  /* This modifies the test expression */
        *ptr += 2;  /* Additional modification */
    }
}

/* Function with volatile test variable */
void volatile_modification(volatile int *v) {
    /* Pattern 2: Volatile prevents optimization */
    if (*v > 100) {
        *v = 50;    /* Modifies volatile test expression */
        int temp = *v * 2;
        *v = temp;  /* Second modification */
    }
}

/* Function with array aliasing possibilities */
void array_aliasing(int arr[], int i, int j) {
    /* Pattern 3: Potential aliasing - arr[i] and arr[j] might be same */
    if (arr[i] > 0) {
        arr[j] = 0;      /* Could modify test expression if i == j */
        arr[i] = -5;     /* Definitely modifies test expression */
    }
}

/* Function with mixed data types */
void mixed_types(int x, float y) {
    /* Pattern 4: Implicit conversions in test */
    if ((float)x > y) {
        x = (int)(y * 2.0f);  /* Modifies variable used in test */
        x = x + 1;            /* Additional modification */
    }
}

/* Function with loop-dependent condition */
void loop_modification(int data[], int n, int threshold) {
    /* Pattern 5: Loop with modifying condition */
    for (int i = 0; i < n; i++) {
        if (data[i] > threshold) {
            data[i] = 0;      /* Modifies test expression */
            data[i] += i;     /* Second modification in same block */
        }
    }
}

/* Function with atomic operations */
void atomic_modification(_Atomic int *atom) {
    /* Pattern 6: Atomic prevents certain optimizations */
    int val = atomic_load(atom);
    if (val > 10) {
        atomic_store(atom, 5);  /* Modifies atomic test expression */
        atomic_fetch_add(atom, 1);  /* Another modification */
    }
}

/* Complex scenario with multiple modifications */
void complex_scenario(int *a, int *b, volatile int *c) {
    /* Multiple test expressions and modifications */
    if (*a > *b) {
        *a = *b;          /* Modifies a which was in test */
        *b = *a + 1;      /* Modifies b which was in test */
        *c = *a + *b;     /* Volatile write */
    }
}

int main() {
    /* Initialize test data with various patterns */
    volatile int v1 = 150;
    volatile int v2 = 75;
    int arr1[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int arr2[5] = {20, 15, 30, 25, 40};
    _Atomic int atomic_val = 20;
    int x = 10;
    float y = 5.5f;
    
    /* Execute various patterns to create if-conversion candidates */
    
    /* Pattern 1: Direct pointer modification */
    int test1 = 5;
    modify_via_pointer(&test1);
    
    /* Pattern 2: Volatile modification */
    volatile_modification(&v1);
    
    /* Pattern 3: Array aliasing (force potential aliasing) */
    array_aliasing(arr1, 2, 2);  /* i == j guarantees aliasing */
    array_aliasing(arr1, 3, 4);  /* i != j, but compiler doesn't know */
    
    /* Pattern 4: Mixed types */
    mixed_types(x, y);
    
    /* Pattern 5: Loop with modifications */
    loop_modification(arr2, 5, 20);
    
    /* Pattern 6: Atomic operations */
    atomic_modification(&atomic_val);
    
    /* Pattern 7: Complex scenario */
    int a = 100, b = 50;
    complex_scenario(&a, &b, &v2);
    
    /* Additional loop to increase if-conversion opportunities */
    for (int i = 0; i < 10; i++) {
        volatile int local_vol = i * 10;
        if (local_vol > 25) {
            local_vol = 0;      /* Modifies test expression */
            local_vol += i;     /* Another modification */
        }
        
        /* Nested if with modification */
        if (arr1[i] > 5) {
            arr1[i] = arr1[i] * 2;  /* Modifies test expression */
            arr1[i] -= 1;           /* Second modification */
        }
    }
    
    /* Use results to prevent dead code elimination */
    int sum = test1 + v1 + v2 + atomic_val + a + b + arr1[0] + arr2[0];
    printf("Result: %d\n", sum);
    
    return 0;
}
