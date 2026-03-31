#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Function prototypes */
void process_pointer(int *p);
void process_aliased(int *arr, int i, int j);
int process_volatile(volatile int *v);

int main() {
    /* 1. Basic test variable modification in then block */
    int x = 10;
    volatile int v = 5;
    
    /* Pattern 1: Direct modification of test variable */
    if (x > 0) {
        x = x * 2;      /* First modification */
        x = x - 3;      /* Second modification - multiple non-label instructions */
        /* This should trigger modified_in_p check */
    }
    
    /* Pattern 2: Volatile variable modification */
    if (v > 0) {
        v = 10;         /* Modifies volatile test variable */
        v = v + 1;      /* Additional modification */
    }
    
    /* 2. Array processing with potential aliasing */
    int array[100];
    for (int i = 0; i < 100; i++) {
        array[i] = i - 50;
    }
    
    /* Pattern 3: Loop-dependent condition with modification */
    for (int i = 0; i < 100; i++) {
        if (array[i] > 0) {
            array[i] = 0;           /* Modifies test expression memory location */
            array[i] = -array[i];   /* Second modification */
        }
    }
    
    /* 3. Pointer aliasing scenarios */
    int *ptr1 = &array[10];
    int *ptr2 = &array[10];  /* Same location */
    
    /* Pattern 4: Pointer-based test with modification */
    if (*ptr1 > -20) {
        *ptr1 = 100;        /* Modifies memory tested in condition */
        *ptr2 = 200;        /* Aliased modification - may trigger alias analysis */
    }
    
    /* 4. Mixed data types and implicit conversions */
    float f = 3.14f;
    int y = 5;
    
    /* Pattern 5: Mixed types in condition */
    if ((float)y > 2.5f) {
        y = (int)(f * 10.0f);   /* Modifies variable involved in test expression */
        y = y % 7;              /* Additional arithmetic */
    }
    
    /* 5. Function calls with pointer parameters */
    process_pointer(&array[20]);
    
    /* 6. Complex aliasing scenario */
    process_aliased(array, 30, 30);  /* i == j creates aliasing */
    process_aliased(array, 40, 41);  /* i != j, no aliasing */
    
    /* 7. Volatile function */
    int result = process_volatile(&v);
    
    /* 8. Nested conditions with modifications */
    int a = 7, b = 8, c = 9;
    if (a > b) {
        a = b + c;
        c = a * 2;
    } else if (b > c) {
        b = c - a;
        a = b / 2;
    }
    
    /* 9. Atomic operations (prevent certain optimizations) */
    _Atomic int atomic_var = 0;
    if (atomic_var == 0) {
        atomic_var = 1;
        atomic_var = 2;  /* Multiple modifications */
    }
    
    /* 10. Short attractive blocks for if-conversion */
    int counter = 0;
    for (int i = 0; i < 50; i++) {
        /* Very short then block - good if-conversion candidate */
        if (array[i] < 0) {
            array[i] = -array[i];  /* Single modification */
        }
        
        /* Slightly longer but still candidate */
        if (counter < 25) {
            counter++;
            array[i] += counter;  /* Two instructions */
        }
    }
    
    /* Prevent dead code elimination */
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += array[i];
    }
    sum += x + y + result + counter + (int)atomic_var;
    
    printf("Result: %d\n", sum);
    return sum;
}

/* Function that modifies through pointer parameter */
void process_pointer(int *p) {
    /* Pattern: Pointer test with modification */
    if (*p > 0) {
        *p = -1;        /* Modifies tested memory */
        *p = *p * 2;    /* Additional modification */
    }
}

/* Function with potential array aliasing */
void process_aliased(int *arr, int i, int j) {
    /* Test one element, potentially modify another */
    if (arr[i] > 0) {
        arr[j] = 0;     /* May alias if i == j */
        arr[j] = -5;    /* Second modification */
    }
}

/* Function with volatile pointer */
int process_volatile(volatile int *v) {
    int local = 0;
    
    /* Volatile access in condition */
    if (*v > 0) {
        *v = *v - 1;    /* Modifies volatile memory */
        local = *v;     /* Additional instruction */
    }
    
    return local;
}
