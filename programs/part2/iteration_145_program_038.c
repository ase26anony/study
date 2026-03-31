#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Function that modifies test expression via pointer */
void modify_via_pointer(int *ptr) {
    /* Pattern 1: Direct modification of test expression */
    if (*ptr > 0) {
        *ptr = -1;  /* Modifies the test expression */
        *ptr += 2;  /* Additional modification */
    }
}

/* Function with volatile test variable */
void volatile_modification(volatile int *v) {
    /* Pattern 2: Volatile prevents optimization */
    if (*v > 10) {
        *v = 5;     /* Modifies volatile test expression */
        int temp = *v * 2;
        *v = temp;  /* Second modification */
    }
}

/* Function with array aliasing possibilities */
void array_aliasing(int arr[], int i, int j) {
    /* Pattern 3: Potential aliasing - arr[i] and arr[j] might be same */
    if (arr[i] > 0) {
        arr[j] = 0;     /* Could modify test expression if i == j */
        arr[i] = -5;    /* Definitely modifies test expression */
    }
}

/* Function with mixed data types */
void mixed_types(float threshold) {
    /* Pattern 4: Implicit conversions and type mixing */
    volatile int counter = 0;
    float f = 3.14f;
    
    if ((int)f > threshold) {
        counter = (int)(f * 2);  /* Modifies test-related variable */
        f = (float)counter / 2.0f; /* Additional modification */
    }
}

/* Function with loop-dependent condition */
void loop_dependent(int data[], int n, int threshold) {
    /* Pattern 5: Loop with side-effect modification */
    for (int i = 0; i < n; i++) {
        if (data[i] > threshold) {
            data[i] = 0;        /* Modifies test expression */
            data[i] += i;       /* Additional modification */
        }
    }
}

/* Function with short then-block (ideal candidate) */
void short_then_block(int *x, int *y) {
    /* Pattern 6: Short block attractive for if-conversion */
    if (*x > *y) {
        *x = *y;        /* Modifies test expression */
        *y = *x + 1;    /* Additional simple operation */
    }
}

/* Function with pointer arithmetic */
void pointer_arithmetic(int *base, int offset) {
    int *ptr = base + offset;
    
    /* Pattern 7: Complex test expression with pointer */
    if (*ptr > 100) {
        *ptr = 50;              /* Modifies test expression */
        *(ptr + 1) = *ptr * 2;  /* Additional operation */
    }
}

/* Main function creating various scenarios */
int main() {
    /* Setup test data */
    volatile int volatile_var = 15;
    int array1[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int array2[5] = {20, 30, 40, 50, 60};
    int simple_var = 25;
    float float_threshold = 2.5f;
    
    /* Test 1: Direct pointer modification */
    modify_via_pointer(&simple_var);
    
    /* Test 2: Volatile modification */
    volatile_modification(&volatile_var);
    
    /* Test 3: Array aliasing (force potential aliasing) */
    array_aliasing(array1, 2, 2);  /* i == j causes definite modification */
    array_aliasing(array1, 3, 4);  /* i != j for contrast */
    
    /* Test 4: Mixed types */
    mixed_types(float_threshold);
    
    /* Test 5: Loop-dependent with side effects */
    loop_dependent(array1, 10, 5);
    
    /* Test 6: Short then-block */
    int x = 10, y = 5;
    short_then_block(&x, &y);
    
    /* Test 7: Pointer arithmetic */
    pointer_arithmetic(array2, 2);
    
    /* Use results to prevent dead code elimination */
    int sum = 0;
    sum += simple_var + volatile_var + x + y;
    for (int i = 0; i < 10; i++) sum += array1[i];
    for (int i = 0; i < 5; i++) sum += array2[i];
    
    printf("Result sum: %d\n", sum);
    
    /* Additional complex scenario with nested conditions */
    {
        int a = 100, b = 200, c = 150;
        volatile int *vp = &a;
        
        /* Multiple if-statements in sequence */
        if (a > b) {
            a = b - 50;
            b = a * 2;
        }
        
        if (*vp > c) {
            *vp = c;
            c = *vp + 100;
        }
        
        /* Loop with if-conversion candidate */
        for (int i = 0; i < 5; i++) {
            if (array2[i] > 35) {
                array2[i] = 35;
                array2[i] += i;
            }
        }
        
        sum += a + b + c;
    }
    
    printf("Final sum: %d\n", sum);
    
    return sum > 0 ? 0 : 1;
}
