#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Function with pointer-based modification that could alias */
void process_with_alias(int *p, int *q) {
    /* Pattern 1: Direct modification of test variable */
    if (*p > 0) {
        *p = -1;  /* This modifies the test expression */
        *q = *p + 1;  /* Additional instruction in then block */
    }
}

/* Function with volatile variable */
void process_volatile(volatile int *vp) {
    /* Pattern 2: Volatile prevents optimization */
    if (*vp > 10) {
        *vp = 5;  /* Modifies volatile test expression */
        int temp = *vp * 2;  /* Second non-label instruction */
        (void)temp;  /* Use to prevent elimination */
    }
}

/* Function with array access and potential aliasing */
void process_array(int arr[], int n, int idx1, int idx2) {
    /* Pattern 3: Array access with possible aliasing */
    if (arr[idx1] > 0) {
        arr[idx2] = 0;  /* Could alias if idx1 == idx2 */
        arr[idx1] = arr[idx1] * 2;  /* Explicit modification */
    }
}

/* Function with mixed data types */
void process_mixed_types(int x, float y) {
    /* Pattern 4: Implicit conversions */
    if ((float)x > y) {
        x = (int)(y * 2.0f);  /* Modifies variable used in test */
        float temp = (float)x + 1.5f;  /* Additional instruction */
        (void)temp;
    }
}

/* Function with loop-dependent condition */
void process_loop(int data[], int n, int threshold) {
    /* Pattern 5: Loop with side-effect modification */
    for (int i = 0; i < n; i++) {
        if (data[i] > threshold) {
            data[i] = 0;  /* Modifies test expression */
            /* Multiple instructions in then block */
            int temp = i * 2;
            data[i] += temp % 3;
        }
    }
}

/* Function with atomic operations */
void process_atomic(_Atomic int *atom) {
    /* Pattern 6: Atomic access */
    int val = atomic_load(atom);
    if (val > 100) {
        atomic_store(atom, 50);  /* Modifies atomic variable */
        int temp = val / 2;  /* Additional instruction */
        (void)temp;
    }
}

/* Main function creating various if-conversion candidates */
int main() {
    /* Setup test data */
    volatile int v1 = 15;
    volatile int v2 = 25;
    int arr1[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int arr2[5] = {100, 200, 300, 400, 500};
    _Atomic int atomic_val = 150;
    
    /* Pattern A: Direct variable modification in then block */
    int x = 10;
    if (x > 5) {
        x = 20;  /* Modifies test variable */
        int y = x + 5;  /* Second non-label instruction */
        (void)y;
    }
    
    /* Pattern B: Pointer modification with possible aliasing */
    int a = 30, b = 40;
    int *ptr1 = &a;
    int *ptr2 = &b;
    if (*ptr1 > 20) {
        *ptr1 = 0;  /* Modifies through pointer */
        *ptr2 = *ptr1 + 10;  /* Additional instruction */
    }
    
    /* Pattern C: Multiple modifications in then block */
    int counter = 8;
    if (counter > 0) {
        counter--;  /* First modification */
        counter *= 2;  /* Second modification */
        counter += 3;  /* Third modification - all modify test expr */
    }
    
    /* Call functions with different patterns */
    process_with_alias(&arr1[0], &arr1[1]);
    process_with_alias(&arr1[2], &arr1[2]);  /* Same address - definite aliasing */
    
    process_volatile(&v1);
    
    process_array(arr1, 10, 3, 3);  /* idx1 == idx2 */
    process_array(arr1, 10, 4, 5);  /* idx1 != idx2 */
    
    process_mixed_types(7, 3.5f);
    
    process_loop(arr2, 5, 250);
    
    process_atomic(&atomic_val);
    
    /* Pattern D: Nested conditions */
    int m = 12, n = 8;
    if (m > n) {
        if (m > 10) {
            m = n * 2;  /* Modifies outer condition variable */
            n = m / 3;  /* Additional instruction */
        }
    }
    
    /* Pattern E: Complex expression modification */
    int values[3] = {5, 15, 25};
    int *current = &values[1];
    if (*current > 10) {
        current = &values[0];  /* Changes pointer */
        *current = 99;  /* Modifies different memory */
        values[1] = *current / 3;  /* Modifies original test location */
    }
    
    /* Use results to prevent dead code elimination */
    int sum = x + a + b + counter + m + n + values[0] + values[1] + values[2];
    sum += arr1[0] + arr1[3] + arr1[4];
    sum += arr2[0] + arr2[2] + arr2[4];
    sum += v1 + v2;
    
    printf("Result: %d\n", sum);
    printf("Atomic value: %d\n", atomic_load(&atomic_val));
    
    return sum > 1000 ? 0 : 1;
}
