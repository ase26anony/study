#include <stdio.h>
#include <stdlib.h>

/* Function that modifies test expression via pointer */
void process(int *p, int *q) {
    /* Pattern 1: Direct modification of test variable */
    if (*p > 0) {
        *p = -1;  /* Modifies the test expression */
        *q = *q + 1;
    }
}

/* Function with volatile test variable */
void volatile_test(volatile int *vp) {
    /* Pattern 2: Volatile prevents optimization */
    if (*vp > 10) {
        *vp = 5;  /* Modifies volatile test expression */
        int temp = *vp * 2;
        (void)temp;  /* Use temp to prevent elimination */
    }
}

/* Function with array aliasing possibilities */
void array_aliasing(int arr[], int n) {
    /* Pattern 3: Array access with potential aliasing */
    for (int i = 0; i < n; i++) {
        if (arr[i] > 100) {
            arr[i] = 0;  /* Modifies test expression */
            if (i > 0) arr[i-1]++;  /* Potential alias */
        }
    }
}

/* Function with mixed data types */
void mixed_types(float threshold) {
    /* Pattern 4: Mixed types and implicit conversions */
    volatile int counter = 0;
    float f = 3.14f;
    int x = 10;
    
    if ((float)x > threshold) {
        x = (int)(f * 2.0f);  /* Modifies x used in test expression */
        counter = x + 1;
    }
    
    /* Use results to prevent elimination */
    printf("Mixed types result: %d\n", counter);
}

/* Function with multiple modifications in then block */
void multi_modify(int *a, int *b) {
    /* Pattern 5: Multiple instructions modifying related values */
    if (*a > *b) {
        *a = *b;      /* First modification */
        *b = *a + 1;  /* Second modification */
        int tmp = *a * *b;  /* Third instruction */
        (void)tmp;
    }
}

/* Main function creating various if-conversion scenarios */
int main() {
    /* Setup test data */
    volatile int v1 = 15;
    volatile int v2 = 20;
    int arr[10] = {5, 150, 30, 200, 10, 300, 40, 250, 20, 350};
    int x = 25, y = 10;
    int *ptr1 = &x;
    int *ptr2 = &y;
    
    printf("Starting if-conversion coverage test...\n");
    
    /* Test 1: Direct pointer modification */
    process(ptr1, ptr2);
    printf("After process: x=%d, y=%d\n", x, y);
    
    /* Test 2: Volatile modification */
    volatile_test(&v1);
    printf("Volatile v1: %d\n", v1);
    
    /* Test 3: Array aliasing in loop */
    array_aliasing(arr, 10);
    printf("Array after aliasing: ");
    for (int i = 0; i < 10; i++) {
        printf("%d ", arr[i]);
    }
    printf("\n");
    
    /* Test 4: Mixed data types */
    mixed_types(2.5f);
    
    /* Test 5: Multiple modifications */
    int a = 100, b = 50;
    multi_modify(&a, &b);
    printf("Multi-modify: a=%d, b=%d\n", a, b);
    
    /* Test 6: Nested conditions with modification */
    int counter = 0;
    for (int i = 0; i < 10; i++) {
        if (arr[i] > 50) {
            arr[i] = arr[i] / 2;  /* Modifies test expression */
            counter++;
            if (counter > 2) {
                arr[i] = -arr[i];  /* Another modification */
            }
        }
    }
    printf("Final counter: %d\n", counter);
    
    /* Test 7: Complex test expression with modification */
    int base = 100;
    int *alias1 = &base;
    int *alias2 = &base;  /* Definitely aliases */
    
    if (*alias1 > 50) {
        *alias2 = 25;  /* Modifies what alias1 points to */
        base = base * 2;  /* Another modification */
    }
    printf("Alias test result: base=%d\n", base);
    
    /* Use all results to prevent dead code elimination */
    int sum = v1 + v2 + x + y + a + b + counter + base;
    for (int i = 0; i < 10; i++) sum += arr[i];
    
    printf("Final sum: %d\n", sum);
    
    return sum > 0 ? 0 : 1;
}
