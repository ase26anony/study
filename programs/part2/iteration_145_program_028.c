#include <stdio.h>
#include <stdlib.h>

/* Function with pointer-based modification that aliases with test expression */
void process_pointer(int *p, int *q) {
    /* Pattern 1: Direct modification of test expression */
    if (*p > 0) {
        *p = -1;  /* Modifies the memory location being tested */
        *q = *p + 1;
    }
}

/* Function with array access where indices might alias */
void process_array(int arr[], int n, int idx1, int idx2) {
    /* Pattern 2: Array access with potential aliasing */
    if (arr[idx1] > 10) {
        arr[idx2] = 0;  /* Could modify same location if idx1 == idx2 */
        arr[idx1] = arr[idx1] * 2;  /* Definitely modifies test expression */
    }
}

/* Function with mixed data types and implicit conversions */
float process_mixed(int x, float y) {
    volatile float result = 0.0f;
    
    /* Pattern 3: Mixed types with conversion */
    if ((float)x > y) {
        x = (int)(y * 2.0f);  /* Modifies variable used in test expression */
        result = (float)x;
    }
    return result;
}

/* Function with loop-dependent conditions */
int process_loop(int data[], int n, int threshold) {
    int sum = 0;
    volatile int mod_count = 0;
    
    /* Pattern 4: Loop with side-effect modification */
    for (int i = 0; i < n; i++) {
        if (data[i] > threshold) {
            data[i] = threshold;  /* Modifies the test expression */
            mod_count++;
            sum += data[i];
        }
    }
    return sum;
}

/* Function with multiple short statements in then block */
int short_then_block(int a, int b, int c) {
    /* Pattern 5: Short then block with multiple operations */
    if (a > b) {
        a = b + c;      /* First modification */
        b = a * 2;      /* Second operation */
        c = a + b;      /* Third operation - uses modified test variable */
    }
    return a + b + c;
}

/* Function with volatile variables to prevent optimization */
void process_volatile(void) {
    volatile int v1 = 10;
    volatile int v2 = 20;
    int temp = 0;
    
    /* Pattern 6: Volatile test expression */
    if (v1 > 5) {
        v1 = 0;         /* Modifies volatile test variable */
        temp = v2;
        v2 = v1 + temp;
    }
}

/* Complex scenario with nested conditions */
void complex_scenario(int *ptr1, int *ptr2, int len) {
    volatile int guard = 1;
    
    for (int i = 0; i < len; i++) {
        /* Multiple test expressions that could be modified */
        if (ptr1[i] > ptr2[i]) {
            ptr1[i] = ptr2[i];      /* Modifies left side of comparison */
            ptr2[i] = ptr1[i] * 2;  /* Modifies right side */
            
            /* Additional short block */
            if (guard > 0) {
                guard = ptr1[i];    /* Modifies volatile guard */
            }
        }
    }
}

int main(void) {
    /* Initialize test data */
    int array1[10] = {5, 15, 3, 25, 8, 12, 7, 18, 9, 11};
    int array2[10] = {2, 8, 4, 6, 10, 3, 7, 5, 9, 1};
    volatile int volatile_var = 42;
    int result = 0;
    
    /* Test 1: Pointer-based modification with aliasing */
    int x = 10, y = 5;
    process_pointer(&x, &y);
    result += x + y;
    
    /* Test 2: Array processing with potential aliasing */
    process_array(array1, 10, 3, 3);  /* idx1 == idx2 ensures modification */
    for (int i = 0; i < 5; i++) {
        result += array1[i];
    }
    
    /* Test 3: Mixed data types */
    float f_result = process_mixed(15, 3.14f);
    result += (int)f_result;
    
    /* Test 4: Loop with modification */
    result += process_loop(array2, 10, 7);
    
    /* Test 5: Short then block */
    result += short_then_block(20, 10, 5);
    
    /* Test 6: Volatile operations */
    process_volatile();
    
    /* Test 7: Complex scenario */
    complex_scenario(array1, array2, 10);
    
    /* Use results to prevent dead code elimination */
    for (int i = 0; i < 10; i++) {
        result += array1[i] + array2[i];
    }
    
    printf("Result: %d\n", result);
    
    /* Additional volatile operations to ensure side effects */
    volatile int final_check = volatile_var;
    if (final_check > 0) {
        volatile_var = result % 100;
    }
    
    return result > 1000 ? 0 : 1;
}
