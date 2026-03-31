#include <stdio.h>
#include <stdlib.h>
#include <stdatomic.h>

/* Function using pointer-based test expression modification */
void process_with_alias(int *p, int *q) {
    /* Test expression uses *p, then block modifies *q which might alias with p */
    if (*p > 100) {
        *q = 50;  /* Could modify same memory as *p if p == q */
        *p = *p - 10;  /* Direct modification of test expression */
    }
}

/* Function with volatile test variable */
void volatile_modification(volatile int *vp) {
    /* Volatile prevents optimization, ensures if-conversion analysis */
    if (*vp > 0) {
        *vp = *vp * 2;  /* Modifies test expression */
        *vp = *vp + 1;  /* Second modification in same block */
    }
}

/* Function with mixed data types and implicit conversions */
void mixed_types_modification(int *arr, float threshold) {
    float current = (float)*arr;
    
    /* Test with float, modification with int */
    if (current > threshold) {
        *arr = (int)(current * 0.5f);  /* Modifies source of test expression */
        arr[1] = *arr + 10;  /* Additional operation */
    }
}

/* Function with array access and potential aliasing */
void array_aliasing(int *a, int i, int j) {
    /* Test a[i], modify a[j] - could be same location if i == j */
    if (a[i] > 0) {
        a[j] = 0;  /* Potential modification of test expression */
        a[i] = a[i] - 1;  /* Definite modification */
    }
}

/* Function designed to be if-conversion candidate */
int ifcvt_candidate(int x, int y) {
    int result = x;
    
    /* Short then-block with test variable modification */
    if (x > y) {
        x = y + 5;  /* Modifies test variable */
        result = x * 2;  /* Additional operation */
    }
    
    return result;
}

/* Main function creating various if-conversion scenarios */
int main() {
    volatile int v1 = 10, v2 = 20;
    int arr[10] = {5, 15, 25, 35, 45, 55, 65, 75, 85, 95};
    int *ptr1 = &arr[0];
    int *ptr2 = &arr[1];
    int sum = 0;
    
    /* Scenario 1: Direct modification in then-block */
    for (int i = 0; i < 5; i++) {
        if (arr[i] > 20) {
            arr[i] = arr[i] / 2;  /* Modifies test expression */
            arr[i] = arr[i] + 1;  /* Second modification */
        }
        sum += arr[i];
    }
    
    /* Scenario 2: Volatile modification */
    volatile_modification(&v1);
    volatile_modification(&v2);
    sum += v1 + v2;
    
    /* Scenario 3: Pointer aliasing */
    process_with_alias(ptr1, ptr1);  /* p and q point to same location */
    process_with_alias(ptr1, ptr2);  /* Different locations */
    sum += *ptr1 + *ptr2;
    
    /* Scenario 4: Mixed types */
    mixed_types_modification(&arr[5], 30.5f);
    sum += arr[5];
    
    /* Scenario 5: Array aliasing with indices */
    for (int i = 0; i < 3; i++) {
        array_aliasing(arr, i, i);  /* i == j, definite aliasing */
        array_aliasing(arr, i, i+1); /* i != j, potential aliasing */
        sum += arr[i];
    }
    
    /* Scenario 6: Multiple if-conversion candidates */
    for (int i = 0; i < 10; i++) {
        int temp = ifcvt_candidate(arr[i], 50);
        sum += temp;
        
        /* Inline candidate with short then-block */
        if (arr[i] < 30) {
            arr[i] = arr[i] * 3;  /* Modifies test variable */
            arr[i] = arr[i] - 5;  /* Second operation */
        }
        sum += arr[i];
    }
    
    /* Scenario 7: Complex test expression with multiple variables */
    int x = 100, y = 200, z = 300;
    if (x + y > z) {
        x = y - 50;  /* Modifies part of test expression */
        z = x * 2;   /* Additional modification */
    }
    sum += x + y + z;
    
    /* Prevent dead code elimination */
    printf("Result sum: %d\n", sum);
    
    /* Additional volatile operations to prevent optimization */
    volatile int sink = sum;
    (void)sink;
    
    return sum > 0 ? 0 : 1;
}
