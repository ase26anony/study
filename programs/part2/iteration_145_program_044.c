#include <stdio.h>
#include <stdlib.h>

/* Function with pointer-based modification - aliasing scenario */
void process_with_alias(int *p, int *q) {
    /* Test expression uses *p, modification might affect it through aliasing */
    if (*p > 100) {
        *q = 50;           /* q might alias p */
        *p = *p - 10;      /* Direct modification of test expression */
        *q = *q + 5;       /* Another potential aliasing modification */
    }
}

/* Function with volatile variable */
void process_volatile(volatile int *vp) {
    /* Volatile prevents optimization, keeps the then block intact */
    if (*vp > 0) {
        *vp = *vp * 2;     /* Modifies test expression */
        *vp = *vp + 1;     /* Another modification */
    }
}

/* Function with mixed data types and implicit conversions */
void process_mixed_types(int x, float y) {
    /* Complex test expression with type conversion */
    if ((float)x > y) {
        x = (int)(y * 2.0f);  /* Modifies variable used in test */
        x = x + 1;            /* Another modification */
    }
}

/* Function with array access - potential aliasing through indices */
void process_array(int arr[], int i, int j) {
    /* Test uses arr[i], modification uses arr[j] - may alias if i == j */
    if (arr[i] > 0) {
        arr[j] = 0;          /* Potential modification of test expression */
        arr[i] = -1;         /* Direct modification of test expression */
        arr[j] = arr[j] + 2; /* Another potential aliasing modification */
    }
}

/* Function with short then block - good if-conversion candidate */
int simple_if_conversion(int a, int b) {
    int result = a;
    if (a > b) {
        result = b;          /* Single assignment - good for conditional move */
        result = result * 2; /* Second instruction to enter the loop */
    }
    return result;
}

/* Function with loop-dependent condition */
void process_loop(int data[], int n, int threshold) {
    for (int i = 0; i < n; i++) {
        /* Loop creates multiple basic blocks */
        if (data[i] > threshold) {
            data[i] = 0;      /* Modifies test expression */
            data[i] = data[i] - 1; /* Another modification */
        }
    }
}

/* Function with multiple modification patterns */
void complex_scenario(volatile int *v1, int *arr, int idx) {
    /* Mix of volatile and regular accesses */
    if (*v1 > 10) {
        arr[idx] = *v1;      /* Modification through array */
        *v1 = arr[idx] / 2;  /* Direct modification of volatile test expr */
        arr[idx] = arr[idx] + 1; /* Another array modification */
    }
}

int main() {
    /* Declare and initialize variables with different properties */
    volatile int volatile_var = 42;
    int regular_var = 100;
    int array[10] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int alias_array[5] = {20, 30, 40, 50, 60};
    
    int *ptr1 = &regular_var;
    int *ptr2 = &regular_var;  /* Same address - definite aliasing */
    int *ptr3 = &array[0];
    
    /* Test 1: Simple if-conversion candidate with modification */
    printf("Test 1 - Simple: %d -> ", regular_var);
    if (regular_var > 50) {
        regular_var = regular_var / 2;  /* Modifies test expression */
        regular_var = regular_var + 1;   /* Another modification */
    }
    printf("%d\n", regular_var);
    
    /* Test 2: Volatile variable - prevents optimization */
    printf("Test 2 - Volatile: %d -> ", volatile_var);
    if (volatile_var > 0) {
        volatile_var = volatile_var * 3;  /* Modifies volatile test expr */
        volatile_var = volatile_var - 10; /* Another modification */
    }
    printf("%d\n", volatile_var);
    
    /* Test 3: Aliasing through pointers */
    printf("Test 3 - Aliasing pointers: %d %d -> ", *ptr1, *ptr2);
    if (*ptr1 > 25) {
        *ptr2 = *ptr2 - 10;  /* Modifies through aliasing pointer */
        *ptr1 = *ptr1 * 2;   /* Direct modification */
    }
    printf("%d %d\n", *ptr1, *ptr2);
    
    /* Test 4: Array with potential aliasing indices */
    printf("Test 4 - Array aliasing: array[2]=%d, array[3]=%d -> ", 
           array[2], array[3]);
    int i = 2, j = 3;
    if (array[i] > 0) {
        array[j] = 99;       /* May not alias with array[i] */
        array[i] = array[i] * 2;  /* Direct modification */
        array[j] = array[j] - 50; /* Another potential aliasing mod */
    }
    printf("array[2]=%d, array[3]=%d\n", array[2], array[3]);
    
    /* Test 5: Same index - definite aliasing */
    printf("Test 5 - Definite array aliasing: array[4]=%d -> ", array[4]);
    i = j = 4;
    if (array[i] > 0) {
        array[j] = 0;        /* Definitely aliases with array[i] */
        array[i] = -5;       /* Direct modification */
    }
    printf("array[4]=%d\n", array[4]);
    
    /* Test 6: Mixed data types */
    printf("Test 6 - Mixed types: ");
    int int_val = 7;
    float float_val = 3.5f;
    if ((float)int_val > float_val) {
        int_val = (int)(float_val * 3.0f);  /* Modifies test variable */
        int_val = int_val % 5;              /* Another modification */
    }
    printf("int_val=%d\n", int_val);
    
    /* Test 7: Loop with side effects */
    printf("Test 7 - Loop processing: ");
    int loop_array[5] = {10, 20, 30, 40, 50};
    for (int k = 0; k < 5; k++) {
        if (loop_array[k] > 25) {
            loop_array[k] = loop_array[k] / 2;  /* Modifies test expr */
            loop_array[k] = loop_array[k] + k;  /* Another modification */
        }
    }
    for (int k = 0; k < 5; k++) {
        printf("%d ", loop_array[k]);
    }
    printf("\n");
    
    /* Test 8: Function calls with different patterns */
    printf("Test 8 - Function calls:\n");
    
    /* Call function with pointer aliasing */
    int x = 200, y = 0;
    process_with_alias(&x, &x);  /* Same pointer - definite aliasing */
    printf("  process_with_alias: x=%d\n", x);
    
    /* Call function with volatile */
    volatile int v = 15;
    process_volatile(&v);
    printf("  process_volatile: v=%d\n", v);
    
    /* Call function with array and potential aliasing */
    int test_arr[3] = {5, 10, 15};
    process_array(test_arr, 1, 1);  /* Same index - definite aliasing */
    printf("  process_array: arr[1]=%d\n", test_arr[1]);
    
    /* Call function with mixed types */
    process_mixed_types(8, 4.2f);
    
    /* Call simple if-conversion candidate multiple times */
    int sum = 0;
    for (int k = 0; k < 10; k++) {
        sum += simple_if_conversion(k, 5);
    }
    printf("  simple_if_conversion sum: %d\n", sum);
    
    /* Call loop processing function */
    int data[6] = {100, 200, 300, 400, 500, 600};
    process_loop(data, 6, 250);
    printf("  process_loop result: ");
    for (int k = 0; k < 6; k++) {
        printf("%d ", data[k]);
    }
    printf("\n");
    
    /* Final complex scenario */
    volatile int v2 = 30;
    int arr2[3] = {0, 0, 0};
    complex_scenario(&v2, arr2, 0);
    printf("Final - complex_scenario: v2=%d, arr2[0]=%d\n", v2, arr2[0]);
    
    return 0;
}
