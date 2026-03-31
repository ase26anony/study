#include <stdio.h>
#include <stdlib.h>

/* Function with pointer-based modification that aliases test expression */
void process_with_alias(int *p, int *q) {
    /* This creates potential aliasing - p and q might point to same location */
    if (*p > 0) {
        *q = -1;  /* Could modify *p if p == q */
    }
}

/* Function with volatile variable to prevent optimization */
void process_volatile(volatile int *vp) {
    if (*vp > 100) {
        *vp = 50;  /* Directly modifies the test expression */
    }
}

/* Function with mixed types and implicit conversions */
void process_mixed_types(int x, float y) {
    if ((float)x > y) {
        x = (int)(y * 2.0f);  /* Modifies x which is part of test expression */
        x = x + 1;            /* Additional modification */
    }
}

/* Function with short then-block (good if-conversion candidate) */
int short_then_block(int a, int b) {
    if (a > b) {
        a = b;      /* First modification of test variable */
        a = a * 2;  /* Second modification */
        return a;
    }
    return 0;
}

/* Function with array access and loop-dependent condition */
void process_array(int *arr, int n, int threshold) {
    for (int i = 0; i < n; i++) {
        if (arr[i] > threshold) {
            arr[i] = 0;      /* Modifies array element used in test */
            arr[i] = -1;     /* Second modification */
        }
    }
}

/* Function with multiple non-label instructions in then block */
int complex_modification(int x, int y, int z) {
    if (x > 0) {
        x = y + z;      /* First arithmetic op modifying test var */
        x = x * 2;      /* Second arithmetic op */
        x = x / 3;      /* Third arithmetic op - total 3 non-label insns */
        return x;
    }
    return -1;
}

/* Function with pointer arithmetic and potential aliasing */
void pointer_arithmetic(int *base, int offset1, int offset2) {
    if (base[offset1] > 10) {
        base[offset2] = 20;  /* Could alias if offset1 == offset2 */
        base[offset1] = 30;  /* Definitely modifies test expression */
    }
}

int main() {
    volatile int v1 = 150;
    volatile int v2 = 200;
    int arr[10] = {5, 15, 25, 35, 45, 55, 65, 75, 85, 95};
    int x = 10, y = 20, z = 30;
    int result = 0;
    
    /* Test 1: Volatile variable modification */
    process_volatile(&v1);
    result += v1;
    
    /* Test 2: Mixed types with implicit conversion */
    process_mixed_types(x, 15.5f);
    result += x;
    
    /* Test 3: Short then-block candidate */
    result += short_then_block(50, 25);
    
    /* Test 4: Array processing with loop */
    process_array(arr, 10, 50);
    for (int i = 0; i < 10; i++) {
        result += arr[i];
    }
    
    /* Test 5: Complex modification with multiple instructions */
    result += complex_modification(100, 5, 10);
    
    /* Test 6: Aliasing test - pass same pointer twice */
    int alias_test = 42;
    process_with_alias(&alias_test, &alias_test);
    result += alias_test;
    
    /* Test 7: Pointer arithmetic with potential equal offsets */
    int base_arr[5] = {100, 200, 300, 400, 500};
    pointer_arithmetic(base_arr, 2, 2);  /* Same offset ensures aliasing */
    result += base_arr[2];
    
    /* Test 8: Nested if with volatile */
    volatile int v3 = 75;
    if (v3 > 50) {
        v3 = v3 - 25;  /* First modification */
        v3 = v3 * 2;   /* Second modification */
    }
    result += v3;
    
    /* Test 9: Multiple conditions in loop */
    int data[8] = {10, 20, 30, 40, 50, 60, 70, 80};
    for (int i = 0; i < 8; i++) {
        if (data[i] > 35) {
            data[i] = data[i] / 2;  /* Modifies test expression */
            data[i] = data[i] + 1;  /* Another modification */
        }
        result += data[i];
    }
    
    /* Test 10: Function pointer parameter with direct modification */
    int local_var = 88;
    if (local_var > 50) {
        local_var = 0;      /* First assignment */
        local_var = 99;     /* Second assignment - overwrites */
    }
    result += local_var;
    
    printf("Result: %d\n", result);
    return result;
}
