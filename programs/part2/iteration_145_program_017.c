#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Function prototypes */
void process_pointer(int *p);
void process_array(int arr[], int n);
int conditional_modification(int x, int y);
float mixed_type_condition(int x, float y);

/* Global volatile to prevent optimization */
volatile int global_volatile = 0;

int main() {
    int result = 0;
    
    /* 1. Basic test with volatile variable */
    volatile int v = 10;
    if (v > 0) {
        v = 5;  /* Direct modification of test variable */
        v = v * 2;  /* Second modification */
    }
    result += v;
    
    /* 2. Pointer-based modification with potential aliasing */
    int a[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int *ptr1 = &a[0];
    int *ptr2 = &a[0];  /* Same location - definite aliasing */
    
    if (*ptr1 > 0) {
        *ptr2 = 0;  /* Modifies same memory as test expression */
        *ptr1 = *ptr1 + 1;  /* Another modification */
    }
    result += a[0];
    
    /* 3. Array access with index that might be the same */
    for (int i = 0; i < 10; i++) {
        if (a[i] > 5) {
            a[i] = 0;  /* Direct modification of tested element */
            a[i] = a[i] + 1;  /* Second instruction in then block */
        }
    }
    
    /* 4. Mixed data types with implicit conversions */
    int x = 7;
    float y = 3.5f;
    if ((float)x > y) {
        x = (int)(y * 2.0f);  /* Modifies x used in test expression */
        x = x + 1;  /* Additional modification */
    }
    result += x;
    
    /* 5. Function call with pointer modification */
    int local_var = 15;
    process_pointer(&local_var);
    result += local_var;
    
    /* 6. Process array with loop-dependent condition */
    int arr[5] = {10, 20, 30, 40, 50};
    process_array(arr, 5);
    for (int i = 0; i < 5; i++) {
        result += arr[i];
    }
    
    /* 7. Complex condition with multiple modifications */
    int counter = 0;
    for (int i = 0; i < 100; i++) {
        if (counter < 50) {
            counter++;  /* Modifies test variable */
            counter = counter * 1;  /* Second instruction */
            global_volatile = counter;  /* Third instruction with volatile */
        }
    }
    result += counter;
    
    /* 8. Atomic operations (prevent certain optimizations) */
    _Atomic int atomic_var = 0;
    if (atomic_var == 0) {
        atomic_var = 1;
        atomic_var = 2;  /* Multiple modifications */
    }
    result += atomic_var;
    
    /* 9. Nested conditions */
    int n = 25;
    if (n > 10) {
        if (n < 30) {
            n = n / 2;  /* Modification in inner then block */
            n = n * 3;  /* Second modification */
        }
    }
    result += n;
    
    /* 10. Register pressure scenario */
    int r1 = 1, r2 = 2, r3 = 3, r4 = 4, r5 = 5;
    if (r1 > 0) {
        r1 = r2 + r3;  /* Uses other registers */
        r2 = r1 * r4;  /* Creates dependency chain */
        r1 = r5 - r2;  /* Modifies test variable again */
    }
    result += r1 + r2;
    
    /* Prevent dead code elimination */
    printf("Result: %d\n", result);
    return result != 0 ? 0 : 1;
}

/* Function that modifies through pointer */
void process_pointer(int *p) {
    if (*p > 0) {
        *p = -1;  /* Direct modification of tested memory */
        *p = *p * 2;  /* Second instruction */
    }
}

/* Function with loop and array modification */
void process_array(int arr[], int n) {
    for (int i = 0; i < n; i++) {
        if (arr[i] > 25) {
            arr[i] = 25;  /* Modification of tested element */
            arr[i] = arr[i] - 1;  /* Second instruction */
        }
    }
}

/* Function with return value and condition */
int conditional_modification(int x, int y) {
    if (x > y) {
        x = y;  /* Modification */
        x = x + 1;  /* Second instruction */
    }
    return x;
}

/* Mixed types with implicit conversions */
float mixed_type_condition(int x, float y) {
    if ((float)x > y) {
        x = (int)(y * 2.0f);  /* Modifies x */
        x = x / 2;  /* Second modification */
    }
    return (float)x;
}
