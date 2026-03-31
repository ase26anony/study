#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Function that modifies test expression via pointer */
void process_direct(int *p) {
    /* Pattern 1: Direct modification of test variable */
    if (*p > 0) {
        *p = -1;  /* Modifies the test expression */
        *p += 2;  /* Additional modification */
    }
}

/* Function with aliasing possibilities */
void process_aliased(int *arr, int i, int j) {
    /* Pattern 2: Potential aliasing with array access */
    if (arr[i] > 10) {
        arr[j] = 0;  /* May alias if i == j */
        arr[i] = arr[i] * 2;  /* Direct modification */
    }
}

/* Function with mixed types and conversions */
void process_mixed(volatile float *fptr, int *iptr) {
    /* Pattern 3: Mixed types with implicit conversions */
    if (*fptr > 0.5f) {
        *iptr = (int)(*fptr * 2.0f);  /* Modifies integer that might be related */
        *fptr = *fptr * 0.5f;  /* Direct modification of test expression */
    }
}

/* Function with loop-dependent condition */
void process_loop(int *arr, int n, int threshold) {
    /* Pattern 4: Loop with side-effect modification */
    for (int i = 0; i < n; i++) {
        if (arr[i] > threshold) {
            arr[i] = threshold;  /* Modifies test expression */
            arr[i] -= 1;         /* Additional modification */
        }
    }
}

/* Function with volatile test variable */
void process_volatile(volatile int *vptr) {
    /* Pattern 5: Volatile prevents optimization */
    if (*vptr > 100) {
        *vptr = 50;  /* Modifies volatile test expression */
        *vptr = *vptr * 2;  /* Second modification */
    }
}

/* Function with pointer arithmetic */
void process_ptr_arithmetic(int *base, int offset) {
    int *ptr = base + offset;
    /* Pattern 6: Test expression via computed pointer */
    if (*ptr > 0) {
        *ptr = 0;  /* Modifies test expression */
        *(ptr + 1) = 1;  /* May alias if offset calculations overlap */
    }
}

/* Main function creating various if-conversion scenarios */
int main() {
    /* Setup test data */
    volatile int v1 = 150;
    volatile float v2 = 1.0f;
    int arr1[10] = {5, 15, 25, 35, 45, 55, 65, 75, 85, 95};
    int arr2[10] = {0};
    int x = 10, y = 20, z = 30;
    
    /* Pattern 1: Direct modification in then block */
    if (x > 5) {
        x = 0;      /* Modifies test variable */
        x = y + z;  /* Additional modification */
    }
    
    /* Pattern 2: Array access with potential aliasing */
    int *ptr1 = &arr1[2];
    int *ptr2 = &arr1[2];  /* Same location */
    if (*ptr1 > 20) {
        *ptr2 = 0;  /* Aliases with test expression */
        arr1[3] = *ptr1 + 1;  /* Nearby memory */
    }
    
    /* Pattern 3: Mixed types with volatile */
    volatile int counter = 100;
    if (counter > 50) {
        counter = 25;  /* Modifies volatile test expression */
        counter = counter * 2;
    }
    
    /* Call functions to create more complex CFGs */
    process_direct(&x);
    process_aliased(arr1, 3, 3);  /* i == j causes aliasing */
    process_mixed(&v2, &y);
    process_loop(arr1, 10, 50);
    process_volatile(&v1);
    process_ptr_arithmetic(arr1, 4);
    
    /* Pattern 4: Nested conditions */
    for (int i = 0; i < 5; i++) {
        if (arr1[i] > 30) {
            arr1[i] = 30;  /* Modifies test expression */
            arr2[i] = arr1[i] * 2;  /* Additional operation */
        }
    }
    
    /* Pattern 5: Complex test expression */
    int a = 10, b = 20, c = 30;
    if ((a + b) > c) {
        a = b;      /* Modifies part of test expression */
        b = c + 5;  /* Additional modification */
    }
    
    /* Pattern 6: Memory modification via different pointer */
    int array[5] = {1, 2, 3, 4, 5};
    int *p1 = &array[0];
    int *p2 = &array[0];
    if (array[0] > 0) {
        *p1 = 10;  /* Aliases with test expression */
        *p2 = 20;  /* Also aliases */
    }
    
    /* Use results to prevent dead code elimination */
    int sum = x + y + z + counter + v1 + (int)v2;
    for (int i = 0; i < 10; i++) {
        sum += arr1[i] + arr2[i];
    }
    
    printf("Result: %d\n", sum);
    return sum > 1000 ? 0 : 1;
}
