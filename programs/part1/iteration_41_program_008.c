/* Test program to trigger auto-increment/decrement optimization block in GCC */
/* Specifically targets lines 1352-1358 in auto-inc-dec.cc */

#include <stdio.h>
#include <stdlib.h>

/* Global array for testing */
int global_array[100];

/* Test 1: Simple parameter access with mixed addressing patterns */
int test_simple_mixed(int *base, int n) {
    int sum = 0;
    
    /* Simple register indirect access - should trigger the uncovered block */
    sum += *base;
    
    /* Register + constant offset */
    sum += base[5];
    
    /* Loop with pointer increment */
    int *ptr = base;
    for (int i = 0; i < n; i++) {
        sum += *ptr++;
    }
    
    /* Another simple register access after loop */
    sum += *base;
    
    return sum;
}

/* Test 2: Local pointer to global array */
int test_global_access(void) {
    int *p = &global_array[0];
    int sum = 0;
    
    /* Multiple simple register accesses */
    sum += *p;
    sum += p[0];  /* Same as *p but different syntax */
    
    /* Mixed with offset */
    sum += p[10];
    
    /* Loop with simple access inside */
    for (int i = 0; i < 10; i++) {
        if (i % 2 == 0) {
            /* Simple register access inside conditional */
            sum += *p;
        }
        p++;
    }
    
    return sum;
}

/* Test 3: Array traversal with multiple pointer variables */
int test_multiple_pointers(int *arr, int n) {
    int sum = 0;
    int *p1 = arr;
    int *p2 = &arr[n/2];
    
    /* Simple accesses with different pointers */
    sum += *p1;
    sum += *p2;
    
    /* Loop with both pointers */
    for (int i = 0; i < n/4; i++) {
        sum += *p1;
        sum += *p2;
        p1++;
        p2--;
    }
    
    /* Final simple access */
    sum += *arr;
    
    return sum;
}

/* Test 4: Volatile pointer to prevent optimization */
int test_volatile_access(volatile int *base, int n) {
    int sum = 0;
    
    /* Simple volatile register access */
    sum += *base;
    
    /* Loop with increment */
    volatile int *ptr = base;
    for (int i = 0; i < n; i++) {
        sum += *ptr;
        ptr++;
    }
    
    return sum;
}

/* Test 5: Struct access with simple pointer */
struct Data {
    int values[20];
    int count;
};

int test_struct_access(struct Data *data) {
    int sum = 0;
    int *p = data->values;
    
    /* Simple struct member access via pointer */
    sum += *p;
    sum += p[0];
    
    /* Loop through struct array */
    for (int i = 0; i < data->count && i < 20; i++) {
        sum += *p;
        p++;
    }
    
    return sum;
}

/* Test 6: Conditional simple access in nested loops */
int test_conditional_access(int *base, int rows, int cols) {
    int sum = 0;
    int *row_ptr = base;
    
    for (int r = 0; r < rows; r++) {
        /* Simple access at start of each row */
        sum += *row_ptr;
        
        for (int c = 0; c < cols; c++) {
            if (c == 0) {
                /* Simple access when column is 0 */
                sum += *row_ptr;
            }
            row_ptr++;
        }
    }
    
    return sum;
}

/* Main driver to ensure all code is executed */
int main(void) {
    int result = 0;
    
    /* Initialize global array */
    for (int i = 0; i < 100; i++) {
        global_array[i] = i * 2;
    }
    
    /* Local test arrays */
    int test_arr1[50];
    int test_arr2[30];
    struct Data test_data;
    
    for (int i = 0; i < 50; i++) {
        test_arr1[i] = i * 3;
    }
    
    for (int i = 0; i < 30; i++) {
        test_arr2[i] = i * 5;
    }
    
    for (int i = 0; i < 20; i++) {
        test_data.values[i] = i * 7;
    }
    test_data.count = 20;
    
    /* Run all tests */
    result += test_simple_mixed(test_arr1, 20);
    result += test_global_access();
    result += test_multiple_pointers(test_arr2, 30);
    result += test_volatile_access(test_arr1, 10);
    result += test_struct_access(&test_data);
    result += test_conditional_access(test_arr1, 5, 10);
    
    /* Print result to prevent dead code elimination */
    printf("Total checksum: %d\n", result);
    
    return result != 0 ? 0 : 1;
}
