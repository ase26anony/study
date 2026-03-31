#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* Function with pointer-based modification */
void process(int *p, int *q) {
    /* Pattern 1: Direct modification of test variable */
    if (*p > 0) {
        *p = -1;           /* Modifies test expression directly */
        *q = *q + 1;       /* Additional instruction */
    }
}

/* Function with array aliasing */
void array_process(int arr[], int n, int idx) {
    volatile int threshold = 10;  /* Volatile to prevent optimization */
    
    /* Pattern 2: Array element modification with possible aliasing */
    if (arr[idx] > threshold) {
        arr[idx] = 0;             /* Modifies test expression */
        arr[(idx + 1) % n] = 5;   /* Could alias if n=1 */
        threshold = 20;           /* Modifies volatile test variable */
    }
}

/* Function with mixed data types */
void mixed_types(int x, float y) {
    /* Pattern 3: Implicit conversions and type mixing */
    if ((float)x > y) {
        x = (int)(y * 2.0f);      /* Modifies variable used in test */
        y = y + 1.0f;             /* Additional float operation */
        x = x - 1;                /* Another modification */
    }
}

/* Function with loop-dependent condition */
void loop_processing(int data[], int size) {
    /* Pattern 4: Loop with side-effecting condition */
    for (int i = 0; i < size; i++) {
        volatile int current = data[i];  /* Volatile copy */
        
        if (current > 50) {
            data[i] = 25;          /* Modifies array element tested */
            current = 75;          /* Modifies volatile test variable */
            data[i] = data[i] * 2; /* Second modification */
        }
    }
}

/* Function with pointer aliasing complexity */
void pointer_aliasing(int *a, int *b, int *c) {
    /* Pattern 5: Complex pointer-based test with modification */
    if (*a > *b) {
        *a = *b + *c;             /* Modifies test variable */
        *b = *a / 2;              /* Could alias with 'a' */
        *c = *c + 1;              /* Additional modification */
    }
}

/* Main function creating various if-conversion scenarios */
int main() {
    /* Initialize test data */
    volatile int v1 = 15, v2 = 20, v3 = 5;
    int arr1[10] = {5, 15, 25, 35, 45, 55, 65, 75, 85, 95};
    int arr2[5] = {100, 200, 300, 400, 500};
    int x = 10;
    float y = 3.14f;
    
    /* Pattern 1: Direct variable modification in then block */
    if (v1 > 10) {
        v1 = v2 + v3;      /* Modifies volatile test variable */
        v2 = v1 - 5;       /* Additional instruction */
        v3 = v3 * 2;       /* Third instruction */
    }
    
    /* Pattern 2: Multiple modifications in short then block */
    int *ptr1 = &arr1[2];
    int *ptr2 = &arr1[3];
    if (*ptr1 > 20) {
        *ptr1 = 0;         /* Modifies memory tested */
        *ptr2 = *ptr2 + 1; /* Could alias if ptr1 == ptr2 */
        ptr1 = &arr1[4];   /* Pointer reassignment */
    }
    
    /* Call functions with different patterns */
    process(&arr1[0], &arr1[1]);
    array_process(arr1, 10, 3);
    mixed_types(x, y);
    loop_processing(arr1, 10);
    pointer_aliasing(&arr1[5], &arr1[6], &arr1[7]);
    
    /* Pattern 3: Nested conditions */
    for (int i = 0; i < 5; i++) {
        volatile int temp = arr2[i];
        
        if (temp > 150) {
            arr2[i] = arr2[i] / 2;    /* Modifies test expression */
            temp = temp + i;          /* Modifies volatile copy */
            if (arr2[i] > 100) {      /* Nested if */
                arr2[i] = 99;
            }
        }
    }
    
    /* Pattern 4: Different data types with implicit conversions */
    char c = 'A';
    short s = 100;
    if ((int)c > s) {
        c = 'B';            /* Modifies variable used in test */
        s = s + c;          /* Mixed-type operation */
        v1 = (int)c;        /* Uses volatile */
    }
    
    /* Pattern 5: Complex expression with multiple variables */
    int a = 5, b = 10, c_val = 15;
    if (a + b > c_val) {
        a = b * 2;          /* Modifies variable in test expression */
        b = a + c_val;      /* Uses modified variable */
        c_val = a - b;      /* Third modification */
    }
    
    /* Use results to prevent dead code elimination */
    int sum = v1 + v2 + v3 + x + (int)y + arr1[0] + arr2[0] + a + b + c_val;
    printf("Result: %d\n", sum);
    
    return 0;
}
