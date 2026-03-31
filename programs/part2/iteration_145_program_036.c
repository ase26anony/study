#include <stdio.h>
#include <stdlib.h>

/* Function with pointer-based modification that could alias */
void process_pointer(int *p, int *q) {
    /* Pattern 1: Direct modification of test expression via pointer */
    if (*p > 0) {
        *p = -1;  /* Modifies the test expression directly */
        *q = *q + 1;  /* Additional instruction in then block */
    }
}

/* Function with array access that may alias */
void process_array(int arr[], int n, int idx) {
    /* Pattern 2: Array access with potential self-modification */
    if (arr[idx] > 10) {
        arr[idx] = 0;  /* Modifies the test expression */
        arr[idx] += 5;  /* Second modification - multiple non-label instructions */
    }
}

/* Function with volatile variable */
void process_volatile(volatile int *vp) {
    /* Pattern 3: Volatile prevents optimization */
    if (*vp > 100) {
        *vp = 50;  /* Modifies volatile test expression */
        int temp = *vp + 1;  /* Additional operation */
        (void)temp;  /* Use to prevent elimination */
    }
}

/* Function with mixed data types */
void process_mixed_types(int x, float y) {
    /* Pattern 4: Implicit conversions in test */
    if ((float)x > y) {
        x = (int)(y * 2.0f);  /* Modifies variable used in test */
        x = x + 1;  /* Second modification */
    }
}

/* Main function with various if-conversion candidates */
int main() {
    volatile int v1 = 42;
    volatile int v2 = 100;
    int arr[10];
    int *ptr1, *ptr2;
    int sum = 0;
    
    /* Initialize array */
    for (int i = 0; i < 10; i++) {
        arr[i] = i * 5;
    }
    
    /* Pattern 5: Loop-dependent condition with side effects */
    for (int i = 0; i < 10; i++) {
        if (arr[i] > 20) {
            arr[i] = arr[i] / 2;  /* Modifies test expression */
            arr[i] = arr[i] + 1;  /* Second instruction */
        }
    }
    
    /* Pattern 6: Multiple pointers that could alias */
    ptr1 = &arr[3];
    ptr2 = &arr[3];  /* Same location - definite alias */
    if (*ptr1 > 10) {
        *ptr2 = 0;  /* Modifies through aliased pointer */
        *ptr1 = *ptr1 + 1;  /* Additional modification */
    }
    
    /* Pattern 7: Nested conditions */
    int x = 15;
    int y = 25;
    if (x > 10) {
        if (y > 20) {
            x = y - x;  /* Modifies outer condition variable */
            y = y / 2;  /* Additional instruction */
        }
    }
    
    /* Pattern 8: Volatile test with multiple modifications */
    if (v1 > 0) {
        v1 = v1 * 2;  /* First modification */
        v1 = v1 - 10;  /* Second modification */
        v2 = v1 + 5;  /* Third instruction */
    }
    
    /* Pattern 9: Function calls with pointer arguments */
    process_pointer(&arr[5], &arr[6]);
    process_volatile(&v1);
    
    /* Pattern 10: Complex expression in test */
    int a = 30, b = 40;
    if ((a + b) > 50) {
        a = b - a;  /* Modifies part of test expression */
        b = a * 2;  /* Additional instruction */
    }
    
    /* Pattern 11: Character type with implicit conversion */
    char c = 'A';
    int count = 0;
    if (c > 64) {  /* ASCII 'A' is 65 */
        c = c + 1;  /* Modifies test variable */
        count = count + 1;  /* Additional instruction */
    }
    
    /* Pattern 12: Memory modification via different index (potential alias) */
    int i = 3, j = 3;  /* Same index - will alias */
    if (arr[i] > 0) {
        arr[j] = -1;  /* Modifies same location as test */
        arr[i] = arr[i] * 2;  /* Additional modification */
    }
    
    /* Calculate sum to prevent dead code elimination */
    for (int i = 0; i < 10; i++) {
        sum += arr[i];
    }
    sum += v1 + v2 + x + y + a + b + c + count;
    
    printf("Result: %d\n", sum);
    
    return sum > 0 ? 0 : 1;
}
